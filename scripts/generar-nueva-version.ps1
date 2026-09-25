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
    $inputHashes = [ordered]@{}
    foreach ($key in @('FSBL','BITSTREAM','CPU0','CPU1')) {
        $inputHashes[$key] = $packageManifest.inputs.$key.sha256.ToLowerInvariant()
    }
    Assert-SameMap $record.products $inputHashes 'El paquete no corresponde al build registrado'
    $manifest = [ordered]@{
        schemaVersion=1; version=$Version; createdUtc=[DateTime]::UtcNow.ToString('o')
        software=[ordered]@{commit=$record.commit;dirty=$false;buildCompletedUtc=$record.completedUtc;inputs=$record.inputs}
        toolchain=$record.toolchain
        hardware=(Get-Content -LiteralPath (Join-Path $repo 'artifacts/dependencies-lock.json') -Raw | ConvertFrom-Json)
        inputs=$inputHashes; products=$packageManifest.products; hardwareValidated=$false
    }
    $manifest | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath (Join-Path $stage 'manifest.json') -Encoding UTF8
    $readme = "# Entrega $Version`n`nCommit de origen: $($record.commit).`n`n- cpu0-boot.bin: FSBL + bitstream UCI + CPU0; arranque en flash.`n- cpu1-network.bin: solo CPU1; carga por red en RAM. No grabar CPU1 en flash.`n- manifest.json: procedencia, herramientas, entradas, hashes y particiones.`n`nLa generacion no certifica pruebas en placa. Conservar esta carpeta sin modificar.`n`n[Volver](../README.md)`n"
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
