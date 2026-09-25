[CmdletBinding()]
param(
    [string]$Version,
    [string]$VitisHome,
    [string]$WorkRoot
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Import-Module (Join-Path $PSHOME 'Modules/Microsoft.PowerShell.Utility') -ErrorAction Stop
Import-Module (Join-Path $PSHOME 'Modules/Microsoft.PowerShell.Management') -ErrorAction Stop
. (Join-Path $PSScriptRoot 'build-record.ps1')
. (Join-Path $PSScriptRoot 'vitis.ps1')
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if (!$WorkRoot) { $WorkRoot = Join-Path (Split-Path $repo) ((Split-Path $repo -Leaf) + '-work') }
$WorkRoot = [IO.Path]::GetFullPath($WorkRoot)
if ($WorkRoot -eq $repo -or $WorkRoot.StartsWith($repo+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'WorkRoot debe estar fuera del repositorio.' }
$workspace = Join-Path $WorkRoot 'workspace'
$VitisHome = Resolve-VitisHome $VitisHome '2022.2'
$output = Join-Path $repo 'output'
if (!$Version) { $Version = Read-Host 'Nueva version (mayor.menor.parche)' }
$null = ConvertTo-ReleaseVersion $Version
if (!(Test-Path -LiteralPath $output -PathType Container)) { throw 'Falta output. Restaura la carpeta del repositorio.' }
if ((Get-Item -LiteralPath $output).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'output no puede ser un enlace.' }
$releaseGuard = $null
$workGuard = $null
$stage = $null
try {
    $releaseGuard = [IO.File]::Open((Join-Path $output '.release.lock'),'OpenOrCreate','ReadWrite','None')
    Assert-NewVersion $output $Version
    Assert-CleanRepo $repo
    $workGuard = [IO.File]::Open((Join-Path $WorkRoot 'workflow.lock'),'OpenOrCreate','ReadWrite','None')
    Assert-ClosedIde $workspace
    & (Join-Path $PSScriptRoot 'setup.ps1') -Action Check -WorkRoot $WorkRoot
    $record = Assert-BuildRecord $repo $workspace $VitisHome
    $package = & (Join-Path $PSScriptRoot 'package.ps1') -Workspace $workspace -VitisHome $VitisHome -PassThru
    $null = Assert-BuildRecord $repo $workspace $VitisHome
    $packageManifest = Get-Content -LiteralPath (Join-Path $package 'manifest.json') -Raw | ConvertFrom-Json
    $stage = Join-Path $output ('.pending-' + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $stage | Out-Null
    foreach ($product in $packageManifest.products) {
        if ($product.file -notin @('cpu0-boot.bin','cpu1-network.bin')) { throw 'Producto inesperado.' }
        $destination = Join-Path $stage $product.file
        Copy-Item -LiteralPath (Join-Path $package $product.file) -Destination $destination
        if ((Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash -ne $product.sha256) { throw 'Hash de salida incorrecto.' }
    }
    $expectedArtifacts = @{
        'programming/fsbl.elf'='flash-programmer'
        'debug/CPU0.elf'='debug-symbols-cpu0'
        'debug/CPU1.elf'='debug-symbols-cpu1'
    }
    if (@($packageManifest.artifacts).Count -ne $expectedArtifacts.Count) { throw 'Numero de artefactos de entrega inesperado.' }
    $seenArtifacts = @{}
    foreach ($artifact in $packageManifest.artifacts) {
        $relative = [string]$artifact.file
        if (!$expectedArtifacts.ContainsKey($relative) -or $seenArtifacts.ContainsKey($relative) -or
            $artifact.purpose -cne $expectedArtifacts[$relative] -or [IO.Path]::IsPathRooted($relative)) { throw "Artefacto inesperado o duplicado: $relative" }
        $seenArtifacts[$relative] = $true
        $source = [IO.Path]::GetFullPath((Join-Path $package $relative))
        $destination = [IO.Path]::GetFullPath((Join-Path $stage $relative))
        if (!$source.StartsWith($package.TrimEnd('\') + '\',[StringComparison]::OrdinalIgnoreCase) -or
            !$destination.StartsWith($stage.TrimEnd('\') + '\',[StringComparison]::OrdinalIgnoreCase)) { throw "Ruta de artefacto no valida: $relative" }
        New-Item -ItemType Directory -Path (Split-Path $destination) -Force | Out-Null
        Copy-Item -LiteralPath $source -Destination $destination
        if ((Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash -ne $artifact.sha256) { throw "Hash de artefacto incorrecto: $relative" }
    }
    if ($packageManifest.deployment.flash.image -cne 'cpu0-boot.bin' -or
        $packageManifest.deployment.flash.fsbl -cne 'programming/fsbl.elf' -or
        $packageManifest.deployment.flash.type -cne 'qspi-x2-single' -or
        [uint64]$packageManifest.deployment.flash.offset -ne 0 -or
        $packageManifest.deployment.cpu1.image -cne 'cpu1-network.bin' -or
        $packageManifest.deployment.cpu1.transport -cne 'network' -or
        $packageManifest.deployment.cpu1.loadAddress -cne '0x18000000') { throw 'Configuracion de despliegue inesperada.' }
    $inputHashes = [ordered]@{}
    foreach ($key in @('FSBL','BITSTREAM','CPU0','CPU1')) {
        $inputHashes[$key] = $packageManifest.inputs.$key.sha256.ToLowerInvariant()
    }
    Assert-SameMap $record.products $inputHashes 'El paquete no corresponde al build registrado'
    $manifest = [ordered]@{
        schemaVersion=2; version=$Version; createdUtc=[DateTime]::UtcNow.ToString('o')
        software=[ordered]@{commit=$record.commit;dirty=$false;buildCompletedUtc=$record.completedUtc;inputs=$record.inputs}
        toolchain=$record.toolchain
        hardware=(Get-Content -LiteralPath (Join-Path $repo 'artifacts/dependencies-lock.json') -Raw | ConvertFrom-Json)
        inputs=$inputHashes; products=$packageManifest.products; artifacts=$packageManifest.artifacts
        deployment=$packageManifest.deployment; hardwareValidated=$false
    }
    $manifest | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath (Join-Path $stage 'manifest.json') -Encoding UTF8
    $readme = "# Entrega $Version`n`nCommit de origen: $($record.commit).`n`n- cpu0-boot.bin: FSBL + bitstream UCI + CPU0; arranque en flash.`n- cpu1-network.bin: solo CPU1; carga por red en RAM. No grabar CPU1 en flash.`n- programming/fsbl.elf: auxiliar temporal para grabar la QSPI mediante JTAG.`n- debug/CPU0.elf y debug/CPU1.elf: ejecutables con simbolos de esta compilacion.`n- manifest.json: procedencia, herramientas, entradas, hashes, despliegue y particiones.`n`nDesde la raiz del repositorio, usar grabar-flash.bat para seleccionar y programar una entrega. La generacion no certifica pruebas en placa. Conservar esta carpeta sin modificar.`n`n[Volver](../README.md)`n"
    [IO.File]::WriteAllText((Join-Path $stage 'README.md'),$readme,[Text.UTF8Encoding]::new($false))
    Assert-CleanRepo $repo
    $null = Assert-BuildRecord $repo $workspace $VitisHome
    Assert-NewVersion $output $Version
    [IO.Directory]::Move($stage, (Join-Path $output $Version))
    $stage = $null
    Write-Host "Version publicada localmente: output/$Version"
    Write-Host "Revisa la entrega y haz commit de output/$Version. No se ha hecho commit ni push."
} finally {
    if ($stage -and (Test-Path -LiteralPath $stage)) {
        $resolved = (Resolve-Path -LiteralPath $stage).Path
        if ((Split-Path $resolved) -ne $output -or (Split-Path $resolved -Leaf) -notlike '.pending-*') { throw 'Ruta temporal inesperada; no se elimina.' }
        Remove-Item -LiteralPath $resolved -Recurse -Force
    }
    if ($workGuard) { $workGuard.Dispose() }
    if ($releaseGuard) { $releaseGuard.Dispose() }
}
