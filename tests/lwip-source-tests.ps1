$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
. (Join-Path $repo 'scripts/lwip-source.ps1')
$root = Join-Path ([IO.Path]::GetTempPath()) ('sitau-lwip-source-tests-' + [guid]::NewGuid().ToString('N'))
$fixture = Join-Path $root 'repo'
$vitis = Join-Path $root 'vitis'
$relative = 'data/embeddedsw/ThirdParty/sw_services/lwip211_v1_8/src/contrib/ports/xilinx/netif'
$count = 0
function Write-Text([string]$Path,[string]$Text) {
    New-Item -ItemType Directory -Path (Split-Path $Path) -Force | Out-Null
    [IO.File]::WriteAllText($Path,$Text,[Text.UTF8Encoding]::new($false))
}
function Expect-Failure([scriptblock]$Action,[string[]]$Messages) {
    $caught = $null
    try { & $Action | Out-Null } catch { $caught = $_.Exception.Message }
    if (!$caught) { throw 'Se acepto un caso invalido.' }
    foreach ($message in $Messages) {
        if (!$caught.Contains($message)) { throw "Diagnostico incompleto: $caught; falta: $message" }
    }
    $script:count++
}
try {
    $patches = @()
    foreach ($name in @('xadapter.c','xemacpsif_physpeed.c')) {
        $custom = Join-Path $fixture "config/lwip211/$name"
        $source = Join-Path $vitis "$relative/$name"
        Write-Text $custom "custom $name`n"
        Write-Text $source "stock $name`n"
        $patches += @{file=$name;sha256=(Get-FileHash $custom).Hash.ToLowerInvariant();stockSha256=(Get-FileHash $source).Hash.ToLowerInvariant()}
    }
    $patches | ConvertTo-Json | Set-Content (Join-Path $fixture 'config/lwip211/manifest.json') -Encoding UTF8
    foreach ($first in @('stock','custom')) {
        foreach ($second in @('stock','custom')) {
            Write-Text (Join-Path $vitis "$relative/xadapter.c") "$first xadapter.c`n"
            Write-Text (Join-Path $vitis "$relative/xemacpsif_physpeed.c") "$second xemacpsif_physpeed.c`n"
            $before = @(Get-ChildItem $vitis -Recurse -File | Get-FileHash | ForEach-Object Hash) -join ','
            $results = @(Assert-LwipSource $fixture $vitis)
            if ($results.Count -ne 2) { throw 'Numero de resultados incorrecto' }
            for ($i=0;$i -lt 2;$i++) {
                $expectedKind = if (@($first,$second)[$i] -eq 'stock') { 'original auditado' } else { 'personalizacion SITAU2 conocida' }
                if ($results[$i].kind -ne $expectedKind) { throw 'Clasificacion incorrecta' }
            }
            $after = @(Get-ChildItem $vitis -Recurse -File | Get-FileHash | ForEach-Object Hash) -join ','
            if ($before -ne $after) { throw 'Instalacion modificada' }
            $count++
        }
    }
    foreach ($patch in $patches) {
        $source = Join-Path $vitis "$relative/$($patch.file)"
        foreach ($text in @('unknown',"custom $($patch.file)`r`n")) {
            Write-Text $source $text
            Expect-Failure { Assert-LwipSource $fixture $vitis } @($source,'observado:','permitidos:',$patch.stockSha256,$patch.sha256)
        }
        Remove-Item -LiteralPath $source
        Expect-Failure { Assert-LwipSource $fixture $vitis } @($source,'ausente',$patch.stockSha256,$patch.sha256)
        Write-Text $source "custom $($patch.file)`n"
        $custom = Join-Path $fixture "config/lwip211/$($patch.file)"
        Write-Text $custom 'altered'
        Expect-Failure { Assert-LwipSource $fixture $vitis } @($custom,'Personalizacion modificada','observado:',$patch.sha256)
        Remove-Item -LiteralPath $custom
        Expect-Failure { Assert-LwipSource $fixture $vitis } @($custom,'Falta personalizacion',$patch.sha256)
        Write-Text $custom "custom $($patch.file)`n"
    }
    # Integracion real con setup.ps1: el origen invalido debe fallar antes de
    # crear, archivar o abrir un workspace. Los lanzadores falsos nunca se usan.
    foreach ($name in @('xsct','vitis','bootgen')) { Write-Text (Join-Path $vitis "bin/$name.bat") "@echo off`r`nexit /b 99`r`n" }
    Write-Text (Join-Path $vitis 'data/version.bat') 'SET XILINX_VERSION_VITIS=2022.2'
    foreach ($action in @('Setup','Build','Open')) {
        $work = Join-Path $root "work-$action"
        $sentinel = Join-Path $work 'workspace/sentinel.txt'
        Write-Text $sentinel 'preserve'
        $log = Join-Path $root "$action.log"
        $exe = Join-Path $env:SystemRoot 'System32/WindowsPowerShell/v1.0/powershell.exe'
        $previous = $ErrorActionPreference
        try {
            $ErrorActionPreference='Continue'
            & $exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repo 'scripts/setup.ps1') -Action $action -VitisHome $vitis -WorkRoot $work *> $log
            $code = $LASTEXITCODE
        } finally { $ErrorActionPreference=$previous }
        if ($code -eq 0 -or !(Select-String -LiteralPath $log -SimpleMatch 'Biblioteca de Vitis distinta de las aceptadas')) { throw "Fallo de integracion: $action" }
        if ([IO.File]::ReadAllText($sentinel) -ne 'preserve' -or @(Get-ChildItem $work -Force).Count -ne 1) { throw 'Workspace modificado antes de validar' }
        $count++
    }
    $fresh = Join-Path $root 'work-fresh'
    $log = Join-Path $root 'fresh.log'
    $previous = $ErrorActionPreference
    try {
        $ErrorActionPreference='Continue'
        & $exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repo 'scripts/setup.ps1') -Action Setup -VitisHome $vitis -WorkRoot $fresh *> $log
        $code = $LASTEXITCODE
    } finally { $ErrorActionPreference=$previous }
    if ($code -eq 0 -or !(Select-String -LiteralPath $log -SimpleMatch 'Biblioteca de Vitis distinta de las aceptadas') -or (Test-Path -LiteralPath $fresh)) { throw 'Se creo el entorno antes de validar el origen' }
    $count++
    Write-Host "PASS: $count pruebas de origen lwIP, diagnosticos y rechazo previo a tocar el workspace."
} finally {
    if (Test-Path -LiteralPath $root) {
        $resolved = (Resolve-Path -LiteralPath $root).Path
        if ((Split-Path $resolved) -ne ([IO.Path]::GetTempPath().TrimEnd('\')) -or (Split-Path $resolved -Leaf) -notlike 'sitau-lwip-source-tests-*') { throw 'Ruta de pruebas inesperada' }
        Remove-Item -LiteralPath $resolved -Recurse -Force
    }
}
