[CmdletBinding()]
param(
    [string]$Version,
    [string]$VitisHome,
    [string]$TargetId,
    [string]$Url,
    [switch]$Check
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Import-Module (Join-Path $PSHOME 'Modules/Microsoft.PowerShell.Utility') -ErrorAction Stop
Import-Module (Join-Path $PSHOME 'Modules/Microsoft.PowerShell.Management') -ErrorAction Stop
. (Join-Path $PSScriptRoot 'vitis.ps1')
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$output = Join-Path $repo 'output'

function Get-VersionDirectories([string]$Root) {
    $versions = @()
    foreach ($directory in (Get-ChildItem -LiteralPath $Root -Directory)) {
        if ($directory.Name -cnotmatch '^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$') { continue }
        try { $parsed = [version]::Parse($directory.Name) } catch { continue }
        $versions += [pscustomobject]@{Name=$directory.Name;Parsed=$parsed;Path=$directory.FullName}
    }
    return @($versions | Sort-Object Parsed -Descending)
}

function Resolve-InRelease([string]$Root, [string]$Relative) {
    if ([string]::IsNullOrWhiteSpace($Relative) -or [IO.Path]::IsPathRooted($Relative)) { throw "Ruta de entrega no valida: $Relative" }
    $full = [IO.Path]::GetFullPath((Join-Path $Root $Relative))
    if (!$full.StartsWith($Root.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) { throw "Ruta fuera de la entrega: $Relative" }
    return $full
}

function Get-SingleEntry($Entries, [string]$File, [string]$Kind) {
    $matches = @($Entries | Where-Object { $_.file -eq $File })
    if ($matches.Count -ne 1) { throw "$Kind ausente o duplicado en el manifiesto: $File" }
    return $matches[0]
}

function Assert-ManifestFile([string]$Root, $Entry) {
    $path = Resolve-InRelease $Root ([string]$Entry.file)
    if (!(Test-Path -LiteralPath $path -PathType Leaf)) { throw "Falta archivo de entrega: $($Entry.file)" }
    $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
    if ($hash -ine ([string]$Entry.sha256)) { throw "Hash incorrecto: $($Entry.file)" }
    return $path
}

if (!(Test-Path -LiteralPath $output -PathType Container)) { throw 'No existe el directorio output.' }
if ((Get-Item -LiteralPath $output).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'output no puede ser un enlace.' }
$versions = @(Get-VersionDirectories $output)
if (!$versions.Count) { throw 'No hay entregas versionadas en output.' }
if (!$Version) {
    Write-Host 'Entregas disponibles:'
    for ($i=0; $i -lt $versions.Count; $i++) {
        $candidateManifest = Join-Path $versions[$i].Path 'manifest.json'
        $details = ''
        if (Test-Path -LiteralPath $candidateManifest -PathType Leaf) {
            try {
                $candidate = Get-Content -LiteralPath $candidateManifest -Raw | ConvertFrom-Json
                $commit = if ($candidate.software.commit) { ([string]$candidate.software.commit).Substring(0, [Math]::Min(12, ([string]$candidate.software.commit).Length)) } else { 'sin-commit' }
                $details = "  commit $commit"
            } catch { $details = '  manifiesto no legible' }
        }
        Write-Host ("[{0}] {1}{2}" -f ($i+1), $versions[$i].Name, $details)
    }
    $selectionText = Read-Host 'Selecciona el numero de entrega'
    $selection = 0
    if (![int]::TryParse($selectionText, [ref]$selection) -or $selection -lt 1 -or $selection -gt $versions.Count) { throw 'Seleccion no valida.' }
    $selected = $versions[$selection-1]
} else {
    $selected = @($versions | Where-Object { $_.Name -ceq $Version })
    if ($selected.Count -ne 1) { throw "No existe output/$Version." }
    $selected = $selected[0]
}
$release = [IO.Path]::GetFullPath($selected.Path)
if ((Get-Item -LiteralPath $release).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'La entrega no puede ser un enlace.' }
if (@(Get-ChildItem -LiteralPath $release -Recurse -Force | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }).Count) { throw 'La entrega no puede contener enlaces.' }
$manifestPath = Join-Path $release 'manifest.json'
if (!(Test-Path -LiteralPath $manifestPath -PathType Leaf)) { throw 'Falta manifest.json en la entrega.' }
$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
if ($manifest.schemaVersion -ne 2 -or $manifest.version -cne $selected.Name) { throw 'La entrega no usa el esquema autocontenido esperado.' }
if (@($manifest.products).Count -ne 2 -or @($manifest.artifacts).Count -ne 3) { throw 'Numero de productos o artefactos inesperado.' }
$imageEntry = Get-SingleEntry $manifest.products 'cpu0-boot.bin' 'Producto'
$networkEntry = Get-SingleEntry $manifest.products 'cpu1-network.bin' 'Producto'
$fsblEntry = Get-SingleEntry $manifest.artifacts 'programming/fsbl.elf' 'Artefacto'
$cpu0Entry = Get-SingleEntry $manifest.artifacts 'debug/CPU0.elf' 'Artefacto'
$cpu1Entry = Get-SingleEntry $manifest.artifacts 'debug/CPU1.elf' 'Artefacto'
if ($fsblEntry.purpose -cne 'flash-programmer' -or $cpu0Entry.purpose -cne 'debug-symbols-cpu0' -or
    $cpu1Entry.purpose -cne 'debug-symbols-cpu1') { throw 'Finalidad de los artefactos inesperada.' }
foreach ($product in $manifest.products) { $null = Assert-ManifestFile $release $product }
foreach ($artifact in $manifest.artifacts) { $null = Assert-ManifestFile $release $artifact }
$image = Resolve-InRelease $release ([string]$imageEntry.file)
$fsbl = Resolve-InRelease $release ([string]$fsblEntry.file)
if ($manifest.deployment.flash.image -cne 'cpu0-boot.bin' -or
    $manifest.deployment.flash.fsbl -cne 'programming/fsbl.elf' -or
    $manifest.deployment.flash.type -cne 'qspi-x2-single' -or
    [uint64]$manifest.deployment.flash.offset -ne 0 -or
    $manifest.deployment.cpu1.image -cne 'cpu1-network.bin' -or
    $manifest.deployment.cpu1.transport -cne 'network' -or
    $manifest.deployment.cpu1.loadAddress -cne '0x18000000') { throw 'Configuracion de despliegue inesperada en el manifiesto.' }
$expected = @('README.md','manifest.json') + @($manifest.products | ForEach-Object { [string]$_.file }) + @($manifest.artifacts | ForEach-Object { [string]$_.file })
$actual = @(Get-ChildItem -LiteralPath $release -Recurse -File | ForEach-Object { $_.FullName.Substring($release.Length+1).Replace('\','/') })
$expectedSorted = @($expected | Sort-Object)
$actualSorted = @($actual | Sort-Object)
if (@($expected | Sort-Object -Unique).Count -ne $expected.Count -or
    ($expectedSorted -join "`n") -cne ($actualSorted -join "`n")) { throw 'La entrega contiene archivos ausentes, duplicados o inesperados.' }
$VitisHome = Resolve-VitisHome $VitisHome '2022.2'
$programFlash = Join-Path $VitisHome 'bin/program_flash.bat'
if (!(Test-Path -LiteralPath $programFlash -PathType Leaf)) { throw "No se encuentra $programFlash" }
Write-Host "Entrega verificada: $($selected.Name)"
Write-Host "Commit: $($manifest.software.commit)"
Write-Host "Imagen: $image"
Write-Host 'Destino: QSPI single x2, offset 0x00000000'
if (!$manifest.hardwareValidated) { Write-Warning 'Esta entrega no consta como validada funcionalmente en placa.' }
if ($Check) { Write-Host 'Comprobacion terminada; no se ha accedido a la placa.'; return }

Write-Host 'Destinos JTAG visibles:'
$targetArgs = @('-jtagtargets')
if ($Url) { $targetArgs += @('-url',$Url) }
& $programFlash @targetArgs | Out-Host
if ($LASTEXITCODE -ne 0) { throw 'No se pudieron enumerar los destinos JTAG.' }
if (!$TargetId) { $TargetId = Read-Host 'Target ID (vacio para usar el primero detectado)' }
if ($TargetId -and $TargetId -cnotmatch '^[0-9]+$') { throw 'Target ID no valido.' }
$targetDescription = if ($TargetId) { "target $TargetId" } else { 'primer target detectado' }
Write-Warning "Se borrara y programara la QSPI del $targetDescription con la entrega $($selected.Name)."
$answer = Read-Host 'Escribe PROGRAMAR para continuar'
if ($answer -cne 'PROGRAMAR') { Write-Host 'Cancelado; no se ha modificado la placa.'; return }
$arguments = @('-f',$image,'-offset','0','-fsbl',$fsbl,'-flash_type','qspi-x2-single','-verify')
if ($TargetId) { $arguments += @('-target_id',$TargetId) }
if ($Url) { $arguments += @('-url',$Url) }
& $programFlash @arguments | Out-Host
if ($LASTEXITCODE -ne 0) { throw "Fallo al programar la QSPI ($LASTEXITCODE)." }
Write-Host 'QSPI programada y verificada. Configura la placa en modo de arranque QSPI y reiniciala.'
