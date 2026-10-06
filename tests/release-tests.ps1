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
function Assert-Equal([string]$Expected, [string]$Actual, [string]$Message) {
    if ($Expected -cne $Actual) { throw $Message }
    $script:count++
}
function Assert-Different([string]$Expected, [string]$Actual, [string]$Message) {
    if ($Expected -ceq $Actual) { throw $Message }
    $script:count++
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
    Write-Text (Join-Path $repo 'src/input.c') 'source'
    Write-Text (Join-Path $repo 'config/applications.tcl') "applications`nconfiguration`n"
    Write-Text (Join-Path $repo 'config/bsp/cpu0.mss') 'bsp'
    Write-Text (Join-Path $repo 'config/lwip211/manifest.json') '[]'
    Write-Text (Join-Path $repo 'config/lwip211/xadapter.c') 'lwip'
    Write-Text (Join-Path $repo 'config/bootimage/cpu0-boot.bif.in') 'bootimage'
    Write-Text (Join-Path $repo 'config/jtag/CPU0-CPU1 JTAG.launch') 'profile'
    Write-Text (Join-Path $repo 'scripts/create-workspace.tcl') 'workspace recipe'
    Write-Text (Join-Path $repo 'scripts/package.ps1') 'package recipe'
    Write-Text (Join-Path $repo 'scripts/generar-nueva-version.ps1') 'release recipe'
    Write-Text (Join-Path $repo 'scripts/grabar-flash.ps1') 'flash recipe'
    Write-Text (Join-Path $repo 'artifacts/dependencies-lock.json') '{"uci":{"package":"uci/active","file":"design.xsa","sha256":"fixture"}}'
    Write-Text (Join-Path $repo 'artifacts/uci/active/manifest.json') '{"id":"active"}'
    Write-Text (Join-Path $repo 'artifacts/uci/active/design.xsa') 'active hardware'
    Write-Text (Join-Path $repo 'artifacts/uci/inactive/design.xsa') 'inactive hardware'
    Write-Text (Join-Path $repo '.gitignore') "*.bak`nhidden.h`n"
    $null = Invoke-RepoGit $repo @('init','-q')
    $null = Invoke-RepoGit $repo @('add','.')
    $null = Invoke-RepoGit $repo @('-c','user.name=Test','-c','user.email=test@example.invalid','commit','-qm','fixture')
    Assert-CleanRepo $repo; $count++
    Write-Text (Join-Path $repo 'extra.txt') 'dirty'
    Expect-Failure { Assert-CleanRepo $repo } 'cambios pendientes'
    Remove-Item -LiteralPath (Join-Path $repo 'extra.txt')

    $baseFingerprint = Get-WorkspaceFingerprint $repo
    Write-Text (Join-Path $repo 'src/input.c') 'source edited'
    Assert-Equal $baseFingerprint (Get-WorkspaceFingerprint $repo) 'Editar un fuente no debe regenerar el workspace.'
    Write-Text (Join-Path $repo 'src/input.c') 'source'
    Write-Text (Join-Path $repo 'src/new.c') 'new source'
    Assert-Different $baseFingerprint (Get-WorkspaceFingerprint $repo) 'Anadir un fuente debe regenerar el workspace.'
    Remove-Item -LiteralPath (Join-Path $repo 'src/new.c')
    Write-Text (Join-Path $repo 'config/bootimage/cpu0-boot.bif.in') 'bootimage edited'
    Assert-Equal $baseFingerprint (Get-WorkspaceFingerprint $repo) 'Bootimage no debe regenerar el workspace.'
    Write-Text (Join-Path $repo 'config/bootimage/cpu0-boot.bif.in') 'bootimage'
    Write-Text (Join-Path $repo 'config/jtag/CPU0-CPU1 JTAG.launch') 'profile edited'
    Assert-Equal $baseFingerprint (Get-WorkspaceFingerprint $repo) 'JTAG no debe regenerar el workspace.'
    Write-Text (Join-Path $repo 'config/jtag/CPU0-CPU1 JTAG.launch') 'profile'
    Write-Text (Join-Path $repo 'config/bsp/cpu0.mss') 'bsp edited'
    Assert-Different $baseFingerprint (Get-WorkspaceFingerprint $repo) 'BSP debe regenerar el workspace.'
    Write-Text (Join-Path $repo 'config/bsp/cpu0.mss') 'bsp'

    Write-Text (Join-Path $vitis 'data/version.bat') '2022.2'
    foreach ($path in (Get-PackageInputs $workspace).Values) { Write-Text $path 'fixture binary' }
    Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'Falta registro'
    $recordPath = Join-Path $workspace '.sitau-build.json'
    $record = @{schemaVersion=2;commit=(Get-RepoCommit $repo);inputs=(Get-BuildInputs $repo);products=(Get-BuildProducts $workspace);toolchain=@{versionFileSha256=(Get-FileHash (Join-Path $vitis 'data/version.bat')).Hash.ToLowerInvariant()}}
    $record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $recordPath -Encoding UTF8
    $null = Assert-BuildRecord $repo $workspace $vitis; $count++

    foreach ($auxiliary in @('notes.MD','notes.markdown','README.txt','trace.log','edit.tmp','edit.swp','edit.swo','source.h~','.DS_Store','Thumbs.db','desktop.ini','backup.bak')) {
        Write-Text (Join-Path $repo "src/$auxiliary") 'auxiliary'
        $null = Assert-BuildRecord $repo $workspace $vitis; $count++
        Remove-Item -LiteralPath (Join-Path $repo "src/$auxiliary")
    }
    foreach ($relative in @('config/bootimage/extra.bif.in','scripts/flash-extra.ps1','artifacts/uci/inactive/extra.xsa')) {
        Write-Text (Join-Path $repo $relative) 'not firmware input'
        $null = Assert-BuildRecord $repo $workspace $vitis; $count++
        Remove-Item -LiteralPath (Join-Path $repo $relative)
    }
    foreach ($relative in @('src/check.c','config/bsp/extra.mss','config/lwip211/extra.c','artifacts/uci/active/extra.bit')) {
        $inputPath = Join-Path $repo $relative
        Write-Text $inputPath 'firmware input'
        Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'Entradas de firmware cambiadas'
        Remove-Item -LiteralPath $inputPath
    }
    Write-Text (Join-Path $repo 'src/input.c') 'changed'
    Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'Entradas de firmware cambiadas'
    Write-Text (Join-Path $repo 'src/input.c') 'source'

    $packaging = Get-PackagingInputs $repo
    Write-Text (Join-Path $repo 'config/bootimage/cpu0-boot.bif.in') 'changed packaging'
    $changedPackaging = Get-PackagingInputs $repo
    if ($packaging['config/bootimage/cpu0-boot.bif.in'] -ceq $changedPackaging['config/bootimage/cpu0-boot.bif.in']) { throw 'El empaquetado debe registrar sus propias recetas.' }
    $count++
    $null = Assert-BuildRecord $repo $workspace $vitis; $count++
    Write-Text (Join-Path $repo 'config/bootimage/cpu0-boot.bif.in') 'bootimage'

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
    $withUntracked = @{schemaVersion=2;commit=(Get-RepoCommit $repo);inputs=(Get-BuildInputs $repo);products=(Get-BuildProducts $workspace);toolchain=$record.toolchain}
    $withUntracked | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $recordPath -Encoding UTF8
    Expect-Failure { Assert-BuildRecord $repo $workspace $vitis } 'no versionada'
    Remove-Item -LiteralPath (Join-Path $repo 'src/hidden.h')

    $record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $recordPath -Encoding UTF8
    $null = Invoke-RepoGit $repo @('-c','user.name=Test','-c','user.email=test@example.invalid','commit','--allow-empty','-qm','documentation-only commit')
    $null = Assert-BuildRecord $repo $workspace $vitis; $count++

    $legacyInputs = Get-HashMap $repo (Get-InputFiles @((Join-Path $repo 'src'),(Join-Path $repo 'config'),(Join-Path $repo 'scripts'),(Join-Path $repo 'artifacts')))
    $legacy = @{schemaVersion=1;commit=$record.commit;inputs=$legacyInputs;products=$record.products;toolchain=$record.toolchain}
    $legacy | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $recordPath -Encoding UTF8
    $null = Assert-BuildRecord $repo $workspace $vitis; $count++
    if (!(Test-LegacyWorkspaceCompatibility $repo $legacyInputs)) { throw 'El workspace anterior compatible fue rechazado.' }
    $count++
    $legacyWithAux = @{}
    foreach ($key in $legacyInputs.Keys) { $legacyWithAux[$key] = $legacyInputs[$key] }
    $legacyWithAux['src/README.md'] = 'hash auxiliar antiguo'
    $sha = [Security.Cryptography.SHA256]::Create()
    try {
        $crlf = [Text.Encoding]::UTF8.GetBytes("applications`r`nconfiguration`r`n")
        $legacyWithAux['config/applications.tcl'] = ([BitConverter]::ToString($sha.ComputeHash($crlf))).Replace('-','').ToLowerInvariant()
    } finally { $sha.Dispose() }
    if (!(Test-LegacyWorkspaceCompatibility $repo $legacyWithAux)) { throw 'CRLF y README del workspace anterior fueron rechazados.' }
    $count++
    $legacyWithAux['config/applications.tcl'] = 'hash de un cambio real'
    if (Test-LegacyWorkspaceCompatibility $repo $legacyWithAux) { throw 'Un cambio real de receta fue aceptado.' }
    $count++

    Write-Host "PASS: $count pruebas de versiones, alcance del workspace y procedencia. No requieren Xilinx."
} finally {
    $resolved = (Resolve-Path -LiteralPath $root).Path
    if ((Split-Path $resolved) -ne ([IO.Path]::GetTempPath().TrimEnd('\')) -or (Split-Path $resolved -Leaf) -notlike 'sitau-release-tests-*') { throw 'Ruta de pruebas inesperada' }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
