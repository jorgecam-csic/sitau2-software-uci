[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$Workspace,
    [string]$VitisHome,
    [switch]$PassThru
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
. (Join-Path $PSScriptRoot 'vitis.ps1')
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$Workspace = [IO.Path]::GetFullPath($Workspace)
if ($Workspace -eq $repo -or $Workspace.StartsWith($repo + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Workspace debe estar fuera del repositorio.' }
if ((Split-Path $Workspace -Leaf) -ne 'workspace') { throw 'Se requiere el workspace generado por setup.ps1.' }
& (Join-Path $PSScriptRoot 'setup.ps1') -Action Check -WorkRoot (Split-Path $Workspace)
$VitisHome = Resolve-VitisHome $VitisHome '2022.2'
$output = Join-Path $Workspace 'packages'
# Cada ejecucion conserva los paquetes anteriores y publica solo si ambos validan.
$run = Join-Path $output (Get-Date -Format 'yyyyMMdd-HHmmss-fff')
New-Item -ItemType Directory -Path $run -Force | Out-Null
$inputs = @{
    FSBL = Join-Path $Workspace 'Platform/export/Platform/sw/Platform/boot/fsbl.elf'
    BITSTREAM = Join-Path $Workspace 'Platform/export/Platform/hw/design_1_wrapper.bit'
    CPU0 = Join-Path $Workspace 'CPU0/Debug/CPU0.elf'
    CPU1 = Join-Path $Workspace 'CPU1/Debug/CPU1.elf'
}
foreach ($path in $inputs.Values) { if (!(Test-Path -LiteralPath $path -PathType Leaf)) { throw "Falta entrada: $path" } }
$beforeHashes = @{}
foreach ($key in $inputs.Keys) { $beforeHashes[$key] = (Get-FileHash -LiteralPath $inputs[$key] -Algorithm SHA256).Hash }
$bootgen = Join-Path $VitisHome 'bin/bootgen.bat'
if (!(Test-Path -LiteralPath $bootgen)) { throw "No se encuentra $bootgen" }
function Read-Partitions([string]$Path) {
    $data = [IO.File]::ReadAllBytes($Path)
    if ($data.Length -lt 160) { throw "Imagen truncada: $Path" }
    $offset = [BitConverter]::ToUInt32($data, 156)
    $parts = @()
    for ($i=0; $i -lt 14; $i++) {
        $pos = [long]$offset + 64*$i
        if ($pos+64 -gt $data.Length) { throw 'Cabecera fuera de la imagen.' }
        $words = @(0..15 | ForEach-Object { [BitConverter]::ToUInt32($data, [int]($pos+4*$_)) })
        [uint64]$sum = 0
        foreach ($word in $words) { $sum += $word }
        if (($sum -band 4294967295) -ne 4294967295) { throw 'Checksum de cabecera incorrecto.' }
        if (@($words[0..14] | Where-Object { $_ -ne 0 }).Count -eq 0) { return $parts }
        if (([long]$words[5]*4 + [long]$words[2]*4) -gt $data.Length) { throw 'Particion fuera de la imagen.' }
        $parts += @{load=$words[3];entry=$words[4];attributes=$words[6];bytes=[long]$words[0]*4}
    }
    throw 'No se encontro terminador de particiones.'
}
$products = @()
$oldPath = $env:PATH
try {
    $env:PATH = "$VitisHome\gnuwin\bin;$env:SystemRoot\System32;$env:SystemRoot;$env:SystemRoot\System32\Wbem"
    foreach ($name in @('cpu0-boot','cpu1-network')) {
        $bif = Get-Content -LiteralPath (Join-Path $repo "config/bootimage/$name.bif.in") -Raw
        foreach ($key in $inputs.Keys) { $bif = $bif.Replace("@$key@", $inputs[$key].Replace('\','/')) }
        $bifPath = Join-Path $run "$name.bif"
        [IO.File]::WriteAllText($bifPath, $bif, [Text.UTF8Encoding]::new($false))
        $bin = Join-Path $run "$name.bin"
        & $bootgen -arch zynq -image $bifPath -o $bin -w on | Out-Host
        if ($LASTEXITCODE -ne 0 -or !(Test-Path -LiteralPath $bin)) { throw "Bootgen fallo: $name" }
        $parts = @(Read-Partitions $bin)
        if ($name -eq 'cpu1-network') {
            if ($parts.Count -ne 1 -or $parts[0].attributes -ne 16 -or $parts[0].load -ne 0x18000000 -or $parts[0].entry -ne 0x18000000) { throw 'Paquete CPU1 incompatible con el cargador de red.' }
        } else {
            if ($parts.Count -ne 3 -or $parts[0].attributes -ne 16 -or $parts[0].load -ne 0 -or $parts[1].attributes -ne 32 -or $parts[2].attributes -ne 16 -or $parts[2].load -ne 0x100000 -or $parts[2].entry -ne 0x100000) { throw 'Composicion CPU0 inesperada.' }
        }
        $products += @{file="$name.bin";sha256=(Get-FileHash -LiteralPath $bin -Algorithm SHA256).Hash;partitions=$parts}
    }
    $artifactDefinitions = @(
        @{input='FSBL';file='programming/fsbl.elf';purpose='flash-programmer'},
        @{input='CPU0';file='debug/CPU0.elf';purpose='debug-symbols-cpu0'},
        @{input='CPU1';file='debug/CPU1.elf';purpose='debug-symbols-cpu1'}
    )
    $releaseArtifacts = @()
    foreach ($definition in $artifactDefinitions) {
        $destination = Join-Path $run $definition.file
        New-Item -ItemType Directory -Path (Split-Path $destination) -Force | Out-Null
        Copy-Item -LiteralPath $inputs[$definition.input] -Destination $destination
        $hash = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash
        if ($hash -ne $beforeHashes[$definition.input]) { throw "Copia de artefacto incorrecta: $($definition.file)" }
        $releaseArtifacts += @{file=$definition.file;purpose=$definition.purpose;sha256=$hash}
    }
    foreach ($key in $inputs.Keys) {
        if ((Get-FileHash -LiteralPath $inputs[$key] -Algorithm SHA256).Hash -ne $beforeHashes[$key]) { throw 'Entradas modificadas durante el empaquetado.' }
    }
    $inputHashes = @{}
    foreach ($key in $inputs.Keys) { $inputHashes[$key] = @{path=$inputs[$key];sha256=(Get-FileHash -LiteralPath $inputs[$key] -Algorithm SHA256).Hash} }
    $deployment = @{
        flash=@{image='cpu0-boot.bin';fsbl='programming/fsbl.elf';type='qspi-x2-single';offset=0}
        cpu1=@{image='cpu1-network.bin';transport='network';loadAddress='0x18000000'}
    }
    @{inputs=$inputHashes;products=$products;artifacts=$releaseArtifacts;deployment=$deployment;hardwareValidated=$false} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $run 'manifest.json') -Encoding UTF8
    [IO.File]::WriteAllText((Join-Path $output 'latest.txt'), $run, [Text.UTF8Encoding]::new($false))
    Write-Host "Paquetes verificados: $run"
    if ($PassThru) { return $run }
} finally { $env:PATH = $oldPath }
