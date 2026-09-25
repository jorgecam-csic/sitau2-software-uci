$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
. (Join-Path $PSScriptRoot '../scripts/build-record.ps1')
$root = Join-Path ([IO.Path]::GetTempPath()) ('sitau-release-tests-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $root | Out-Null
$count = 0
function Expect-Failure([scriptblock]$Action, [string]$Match) {
    $caught = $null
    try { & $Action } catch { $caught = $_.Exception.Message }
    if (!$caught -or $caught -notlike "*$Match*") { throw "Fallo esperado '$Match', recibido '$caught'" }
    $script:count++
}
function Write-Text([string]$Path, [string]$Value) {
    New-Item -ItemType Directory -Path (Split-Path $Path) -Force | Out-Null
    [IO.File]::WriteAllText($Path,$Value,[Text.UTF8Encoding]::new($false))
}
try {
    $output = Join-Path $root 'versions'
    New-Item -ItemType Directory -Path $output | Out-Null
    Assert-NewVersion $output '0.0.0'; $count++
    foreach ($value in @('../1.0.0','v1.0.0','1.0','1.0.0.0','01.0.0','1.0.0-rc1','-1.0.0','1.0.0 ','2147483648.0.0')) {
        Expect-Failure { Assert-NewVersion $output $value } 'version'
    }
    New-Item -ItemType Directory -Path (Join-Path $output '0.9.0'),(Join-Path $output '0.10.0') | Out-Null
    Expect-Failure { Assert-NewVersion $output '0.10.0' } 'existe'
    Expect-Failure { Assert-NewVersion $output '0.9.5' } 'superar'
    Assert-NewVersion $output '0.10.1'; $count++
    Assert-NewVersion $output '1.0.0'; $count++
    $repo = Join-Path $root 'repo'
    $workspace = Join-Path $root 'work/workspace'
    $vitis = Join-Path $root 'vitis'
    foreach ($name in @('src','config','scripts','artifacts')) { Write-Text (Join-Path $repo "$name/input.txt") $name }
    Write-Text (Join-Path $repo '.gitignore') "*.bak`nhidden.h`n"
    $null = Invoke-RepoGit $repo @('init','-q')
    $null = Invoke-RepoGit $repo @('add','.')
    $null = Invoke-RepoGit $repo @('-c','user.name=Test','-c','user.email=test@example.invalid','commit','-qm','fixture')
    Assert-CleanRepo $repo; $count++
    Write-Text (Join-Path $repo 'extra.txt') 'dirty'
    Expect-Failure { Assert-CleanRepo $repo } 'cambios pendientes'
    Remove-Item -LiteralPath (Join-Path $repo 'extra.txt')
    Write-Text (Join-Path $vitis 'data/version.bat') '2022.2'
    foreach ($path in (Get-PackageInputs $workspace).Values) { Write-Text $path 'fixture binary' }
    Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'Falta registro'
    $recordPath = Join-Path $workspace '.sitau-build.json'
    $record = @{schemaVersion=1;commit=(Get-RepoCommit $repo);inputs=(Get-BuildInputs $repo);products=(Get-BuildProducts $workspace);toolchain=@{versionFileSha256=(Get-FileHash (Join-Path $vitis 'data/version.bat')).Hash.ToLowerInvariant()}}
    $record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $recordPath -Encoding UTF8
    $null = Assert-BuildRecord $repo $workspace $vitis; $count++
    foreach ($name in @('src','config','scripts','artifacts')) { Write-Text (Join-Path $repo "$name/input.txt.bak") 'backup' }
    Assert-CleanRepo $repo
    $null = Assert-BuildRecord $repo $workspace $vitis; $count++
    Write-Text (Join-Path $repo 'src/input.txt.bak') 'changed backup'
    $null = Assert-BuildRecord $repo $workspace $vitis; $count++
    foreach ($name in @('src','config','scripts','artifacts')) { Remove-Item -LiteralPath (Join-Path $repo "$name/input.txt.bak") }
    $null = Assert-BuildRecord $repo $workspace $vitis; $count++
    foreach ($auxiliary in @('notes.MD','notes.markdown','README.txt','trace.log','edit.tmp','edit.swp','edit.swo','source.h~','.DS_Store','Thumbs.db','desktop.ini')) {
        foreach ($name in @('src','config','scripts','artifacts')) { Write-Text (Join-Path $repo "$name/$auxiliary") 'auxiliary' }
        $null = Assert-BuildRecord $repo $workspace $vitis; $count++
        foreach ($name in @('src','config','scripts','artifacts')) { Write-Text (Join-Path $repo "$name/$auxiliary") 'changed auxiliary' }
        $null = Assert-BuildRecord $repo $workspace $vitis; $count++
        foreach ($name in @('src','config','scripts','artifacts')) { Remove-Item -LiteralPath (Join-Path $repo "$name/$auxiliary") }
        $null = Assert-BuildRecord $repo $workspace $vitis; $count++
    }
    foreach ($inputName in @('src/check.c','src/check.h','src/check.S','src/check.ld','src/check.spec','src/resource.txt','config/settings.json','config/cpu.mss','config/boot.bif.in','scripts/recipe.ps1','scripts/recipe.tcl','artifacts/hardware.xsa','artifacts/image.bit','artifacts/boot.bin','artifacts/init.html')) {
        $inputPath = Join-Path $repo $inputName
        Write-Text $inputPath 'meaningful input'
        Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'Entradas cambiadas'
        Remove-Item -LiteralPath $inputPath
    }
    Write-Text (Join-Path $repo 'src/input.txt') 'changed'
    Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'Entradas cambiadas'
    Write-Text (Join-Path $repo 'src/input.txt') 'src'
    Write-Text (Join-Path $repo 'src/new.c') 'new'
    Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'Entradas cambiadas'
    Remove-Item -LiteralPath (Join-Path $repo 'src/new.c')
    $elf = (Get-PackageInputs $workspace).CPU0
    Write-Text $elf 'altered'
    Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'Binarios cambiados'
    Write-Text $elf 'fixture binary'
    Write-Text (Join-Path $vitis 'data/version.bat') 'other'
    Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'Instalacion Xilinx'
    Write-Text (Join-Path $vitis 'data/version.bat') '2022.2'
    $ideLock = Join-Path $workspace '.metadata/.lock'
    Write-Text $ideLock ''
    $handle = [IO.File]::Open($ideLock,'Open','ReadWrite','None')
    try { Expect-Failure { Assert-ClosedIde $workspace } 'Cierra Vitis' } finally { $handle.Dispose() }
    Assert-ClosedIde $workspace; $count++
    Write-Text (Join-Path $repo 'src/hidden.h') 'untracked header'
    Assert-CleanRepo $repo
    $withUntracked = @{schemaVersion=1;commit=(Get-RepoCommit $repo);inputs=(Get-BuildInputs $repo);products=(Get-BuildProducts $workspace);toolchain=$record.toolchain}
    $withUntracked | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $recordPath -Encoding UTF8
    Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'no versionada'
    Remove-Item -LiteralPath (Join-Path $repo 'src/hidden.h')
    $record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $recordPath -Encoding UTF8
    $null = Invoke-RepoGit $repo @('-c','user.name=Test','-c','user.email=test@example.invalid','commit','--allow-empty','-qm','another commit')
    Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'otro commit'
    Write-Host "PASS: $count pruebas de versiones, Git y procedencia. No requieren Xilinx."
} finally {
    $resolved = (Resolve-Path -LiteralPath $root).Path
    if ((Split-Path $resolved) -ne ([IO.Path]::GetTempPath().TrimEnd('\')) -or (Split-Path $resolved -Leaf) -notlike 'sitau-release-tests-*') { throw 'Ruta de pruebas inesperada' }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
