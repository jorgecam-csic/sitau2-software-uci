[CmdletBinding()]
param(
    [ValidateSet('Setup','Build','Open','Verify','Check')][string]$Action = 'Setup',
    [string]$VitisHome = 'E:\Xilinx\Vitis\2022.2',
    [string]$WorkRoot
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
# make puede heredar PSModulePath de PowerShell 7 al lanzar Windows PowerShell 5.
# Cargar los modulos de la instancia que ejecuta realmente esta receta.
Import-Module (Join-Path $PSHOME 'Modules/Microsoft.PowerShell.Utility') -ErrorAction Stop
Import-Module (Join-Path $PSHOME 'Modules/Microsoft.PowerShell.Management') -ErrorAction Stop
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if (!$WorkRoot) { $WorkRoot = Join-Path (Split-Path $repo) ((Split-Path $repo -Leaf) + '-work') }
$WorkRoot = [IO.Path]::GetFullPath($WorkRoot)
if ($WorkRoot.TrimEnd('\') -eq $repo.TrimEnd('\') -or $WorkRoot.StartsWith($repo.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) {
    throw 'WorkRoot debe estar fuera del repositorio.'
}
$workspace = Join-Path $WorkRoot 'workspace'
$lockPath = Join-Path $repo 'artifacts/dependencies-lock.json'
$lock = Get-Content -LiteralPath $lockPath -Raw | ConvertFrom-Json
if ($lock.schemaVersion -ne 1 -or $lock.toolchain -ne '2022.2') { throw 'Version de lock o herramientas no soportada.' }
function Resolve-InTree([string]$Root, [string]$Relative) {
    $full = [IO.Path]::GetFullPath((Join-Path $Root $Relative))
    if (!$full.StartsWith($Root.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) { throw "Ruta fuera de su directorio: $Relative" }
    return $full
}
$artifacts = Join-Path $repo 'artifacts'
$package = Resolve-InTree $artifacts $lock.uci.package
$manifest = Get-Content -LiteralPath (Join-Path $package 'manifest.json') -Raw | ConvertFrom-Json
foreach ($file in $manifest.files) {
    $path = Resolve-InTree $package $file.path
    if (!(Test-Path -LiteralPath $path -PathType Leaf)) { throw "Falta dependencia: $path" }
    if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $file.sha256) { throw "Hash incorrecto: $path" }
}
$xsa = Resolve-InTree $package $lock.uci.file
if ((Get-FileHash -LiteralPath $xsa -Algorithm SHA256).Hash -ne $lock.uci.sha256) { throw 'El XSA no coincide con dependencies-lock.json.' }
if (@($manifest.files | Where-Object { $_.path -eq $lock.uci.file -and $_.sha256 -eq $lock.uci.sha256 }).Count -ne 1) { throw 'Lock y manifiesto no coinciden.' }
Write-Host "Dependencia verificada: $($manifest.id)"
if ($Action -eq 'Verify') { return }
$xsct = Join-Path $VitisHome 'bin/xsct.bat'
$vitis = Join-Path $VitisHome 'bin/vitis.bat'
if ($Action -ne 'Check') {
    if (!(Test-Path -LiteralPath $xsct) -or !(Test-Path -LiteralPath $vitis)) { throw "No se encuentra Vitis: $VitisHome" }
    $version = Get-Content -LiteralPath (Join-Path $VitisHome 'data/version.bat') -Raw
    if ($version -notmatch '2022\.2') { throw 'Se necesita Vitis Classic 2022.2.' }
}
# La huella incluye recetas y configuracion; los fuentes pueden editarse normalmente.
$inputs = @(Get-ChildItem -LiteralPath (Join-Path $repo 'config'),$PSScriptRoot -Recurse -File | Sort-Object FullName)
$recipe = @($lockPath) + @($inputs.FullName)
$fingerprintText = (($recipe | ForEach-Object { $_.Substring($repo.Length) + ':' + (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash }) -join "`n")
$sha = [Security.Cryptography.SHA256]::Create()
try { $fingerprint = ([BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes($fingerprintText)))).Replace('-','') } finally { $sha.Dispose() }
function Assert-CustomLwip {
    $libraryRoot = Join-Path $workspace 'software-repository/sw_services/lwip211_v1_08_s'
    $metadataPath = Join-Path $libraryRoot 'data/lwip211.mld'
    if (!(Test-Path -LiteralPath $metadataPath -PathType Leaf)) { throw 'Falta la biblioteca personalizada lwip211 1.08.s. No se permite usar la original.' }
    $metadata = Get-Content -LiteralPath $metadataPath -Raw
    if ($metadata -notmatch 'OPTION\s+VERSION\s*=\s*1\.08\.s\s*;' -or $metadata -notmatch 'BEGIN\s+LIBRARY\s+lwip211\b') { throw 'Identidad de lwIP personalizada incorrecta.' }
    $bspRoot = Join-Path $workspace 'Platform/ps7_cortexa9_0/standalone_domain/bsp'
    foreach ($mssPath in @((Join-Path $bspRoot 'system.mss'),(Join-Path $workspace 'Platform/export/Platform/sw/Platform/standalone_domain/system.mss'))) {
        $mss = Get-Content -LiteralPath $mssPath -Raw
        $blocks = [regex]::Matches($mss, '(?ms)^\s*BEGIN LIBRARY\s*$.*?^\s*END\s*$')
        $lwipBlocks = @($blocks | Where-Object { $_.Value -match 'LIBRARY_NAME\s*=\s*lwip211\b' })
        if ($lwipBlocks.Count -ne 1 -or $lwipBlocks[0].Value -notmatch '(?m)LIBRARY_VER\s*=\s*1\.08\.s\s*$') { throw "BSP no selecciona lwip211 1.08.s: $mssPath" }
    }
    foreach ($patch in (Get-Content -LiteralPath (Join-Path $repo 'config/lwip211/manifest.json') -Raw | ConvertFrom-Json)) {
        foreach ($root in @($libraryRoot,(Join-Path $bspRoot 'ps7_cortexa9_0/libsrc/lwip211_v1_08_s'))) {
            $file = Join-Path $root ('src/contrib/ports/xilinx/netif/' + $patch.file)
            if (!(Test-Path -LiteralPath $file -PathType Leaf)) { throw "Falta personalizacion lwIP: $file" }
            if ((Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash -ne $patch.sha256) { throw "Contenido lwIP distinto de la variante SITAU2: $file" }
        }
    }
}
if ($Action -eq 'Check') {
    $checkState = Get-Content -LiteralPath (Join-Path $workspace '.sitau-workspace.json') -Raw | ConvertFrom-Json
    if (!$checkState.ready -or $checkState.repo -ne $repo -or $checkState.fingerprint -ne $fingerprint) {
        throw 'Workspace desactualizado. Cierra Vitis y ejecuta generar-workspace.bat.'
    }
    Assert-CustomLwip
    return
}
foreach ($patch in (Get-Content -LiteralPath (Join-Path $repo 'config/lwip211/manifest.json') -Raw | ConvertFrom-Json)) {
    $stock = Join-Path $VitisHome ('data/embeddedsw/ThirdParty/sw_services/lwip211_v1_8/src/contrib/ports/xilinx/netif/' + $patch.file)
    if ((Get-FileHash -LiteralPath $stock -Algorithm SHA256).Hash -ne $patch.stockSha256) { throw "Biblioteca de Vitis distinta de la auditada: $($patch.file)" }
    $custom = Join-Path $repo ('config/lwip211/' + $patch.file)
    if ((Get-FileHash -LiteralPath $custom -Algorithm SHA256).Hash -ne $patch.sha256) { throw "Personalizacion modificada: $($patch.file). Revisa su manifiesto." }
}
New-Item -ItemType Directory -Path $WorkRoot -Force | Out-Null
$guard = [IO.File]::Open((Join-Path $WorkRoot 'workflow.lock'), [IO.FileMode]::OpenOrCreate, [IO.FileAccess]::ReadWrite, [IO.FileShare]::None)
$oldPath = $env:PATH
try {
    $statePath = Join-Path $workspace '.sitau-workspace.json'
    if ($Action -eq 'Setup' -and (Test-Path -LiteralPath $workspace)) {
        if (!(Test-Path -LiteralPath $statePath)) { throw 'No se archiva un workspace sin marcador SITAU. Usa otro WorkRoot.' }
        $previousState = Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
        if ($previousState.repo -ne $repo) { throw 'El workspace pertenece a otro repositorio. No se modifica.' }
        if ((Get-Item -LiteralPath $workspace).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'El workspace no puede ser un enlace.' }
        $ideLock = Join-Path $workspace '.metadata/.lock'
        if (Test-Path -LiteralPath $ideLock) {
            try { $probe = [IO.File]::Open($ideLock,'Open','ReadWrite','None'); $probe.Dispose() }
            catch { throw 'El workspace esta abierto. Cierra Vitis antes de generar uno nuevo.' }
        }
        $archive = Join-Path $WorkRoot ('workspace-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
        # Ambos destinos son hijos directos del WorkRoot explicito, nunca del repositorio.
        if ((Split-Path $workspace) -ne $WorkRoot -or (Split-Path $archive) -ne $WorkRoot) { throw 'Destino de archivo no valido.' }
        Write-Host "Workspace existente: $workspace"
        Write-Host "Se conservara en: $archive"
        $answer = Read-Host 'Archivar el anterior y generar uno nuevo desde cero con las recetas actuales? [s/N]'
        if ($answer.Trim() -notmatch '^(s|si)$') { Write-Host 'Cancelado. Workspace anterior intacto.'; return }
        Move-Item -LiteralPath $workspace -Destination $archive
        Write-Host "Workspace anterior conservado en $archive"
    }
    $create = !(Test-Path -LiteralPath $workspace)
    if (!$create) {
        if (!(Test-Path -LiteralPath $statePath)) { throw 'Workspace ajeno. Usa otro WorkRoot.' }
        $state = Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
        if ($state.repo -ne $repo -or $state.fingerprint -ne $fingerprint -or !$state.ready) { throw 'Workspace desactualizado o incompleto. Cierra Vitis y ejecuta generar-workspace.bat.' }
    } else {
        if ($Action -ne 'Setup') { throw 'No existe el workspace. Ejecuta primero generar-workspace.bat.' }
        New-Item -ItemType Directory -Path $workspace | Out-Null
        @{repo=$repo;fingerprint=$fingerprint;ready=$false} | ConvertTo-Json | Set-Content -LiteralPath $statePath -Encoding UTF8
    }
    $env:PATH = "$VitisHome\gnuwin\bin;$env:SystemRoot\System32;$env:SystemRoot;$env:SystemRoot\System32\Wbem"
    $logs = Join-Path $WorkRoot 'logs'
    New-Item -ItemType Directory -Path $logs -Force | Out-Null
    function Invoke-Recipe([string]$Mode) {
        $logFile = Join-Path $logs ((Get-Date -Format 'yyyyMMdd-HHmmss') + "-$Mode.log")
        Write-Host "XSCT $Mode. Log: $logFile"
        # Windows PowerShell 5 convierte stderr nativo en ErrorRecord, incluso
        # para avisos. XSCT comunica el fallo con exit code y marcador final.
        $previousPreference = $ErrorActionPreference
        $nativeExit = 1
        Push-Location $workspace
        try {
            $ErrorActionPreference = 'Continue'
            & $xsct (Join-Path $PSScriptRoot 'create-workspace.tcl') $Mode $repo $workspace $xsa $VitisHome 2>&1 | Tee-Object -FilePath $logFile
            $nativeExit = $LASTEXITCODE
        } finally {
            Pop-Location
            $ErrorActionPreference = $previousPreference
        }
        if ($nativeExit -ne 0) { throw "XSCT fallo ($nativeExit). Consulta $logFile" }
        if (!(Select-String -LiteralPath $logFile -SimpleMatch "SITAU_OK:$Mode" -Quiet)) { throw "XSCT no confirmo el resultado. Consulta $logFile" }
    }
    if ($create) {
        Invoke-Recipe 'setup'
        Assert-CustomLwip
        @{repo=$repo;fingerprint=$fingerprint;ready=$true;xsaSha256=$lock.uci.sha256} | ConvertTo-Json | Set-Content -LiteralPath $statePath -Encoding UTF8
    }
    if (!$create) { Assert-CustomLwip }
    if ($create) {
      foreach ($projectName in @('CPU0','CPU1','Both_CPUs_system','CPU1_system')) {
        # Un prebuildStep de CDT puede ignorar errores. makefile.init detiene make.
        $checkScript = (Join-Path $PSScriptRoot 'setup.ps1').Replace('\','/')
        $checkRoot = $WorkRoot.Replace('\','/')
        $makeCheck = @'
SITAU_CHECK := $(shell "__POWERSHELL__" -NoProfile -ExecutionPolicy Bypass -File "__SCRIPT__" -Action Check -WorkRoot "__ROOT__" >nul 2>&1 && echo SITAU_READY)
ifeq ($(filter SITAU_READY,$(SITAU_CHECK)),)
$(error Workspace SITAU desactualizado o dependencia invalida. Ejecute scripts/setup.ps1 -Action Check para ver el detalle)
endif
'@
        $checkExe = (Join-Path $env:SystemRoot 'System32/WindowsPowerShell/v1.0/powershell.exe').Replace('\','/')
        $makeCheck = $makeCheck.Replace('__SCRIPT__',$checkScript).Replace('__ROOT__',$checkRoot).Replace('__POWERSHELL__',$checkExe)
        [IO.File]::WriteAllText((Join-Path $workspace "$projectName/makefile.init"), $makeCheck + "`n", [Text.UTF8Encoding]::new($false))
      }
    }
    if ($Action -eq 'Build') {
        Invoke-Recipe 'build'
        Assert-CustomLwip
        & (Join-Path $PSScriptRoot 'package.ps1') -Workspace $workspace -VitisHome $VitisHome
    }
    if ($Action -eq 'Open') {
        Push-Location (Join-Path $VitisHome 'bin')
        try { & $vitis -workspace $workspace } finally { Pop-Location }
    }
    Write-Host "Workspace: $workspace"
} finally {
    $env:PATH = $oldPath
    $guard.Dispose()
}
