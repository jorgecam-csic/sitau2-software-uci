# Funciones compartidas para comprobar la procedencia de una compilacion.
$script:SitauGit = (Get-Command git -ErrorAction Stop).Source
function Invoke-RepoGit([string]$Repo, [string[]]$Arguments) {
    $result = & $script:SitauGit -c "safe.directory=$($Repo.Replace('\','/'))" -C $Repo @Arguments
    if ($LASTEXITCODE -ne 0) { throw 'Git no pudo consultar el repositorio.' }
    return $result
}
function Get-RepoCommit([string]$Repo) {
    return [string](Invoke-RepoGit $Repo @('rev-parse','HEAD'))
}
function Assert-CleanRepo([string]$Repo) {
    $status = @(Invoke-RepoGit $Repo @('status','--porcelain','--untracked-files=all'))
    if ($status.Count) { throw 'Git tiene cambios pendientes. Haz commit de los cambios antes de generar una version (incluidas entregas anteriores).' }
}
function Get-InputFiles([string[]]$Roots) {
    # Documentacion y auxiliares conocidos no son entradas del proyecto.
    # Conservar formatos ambiguos/desconocidos y archivos ignorados por Git:
    # una cabecera o un recurso sin versionar tambien puede afectar al build.
    $auxiliaryExtensions = @('.md','.markdown','.bak','.log','.tmp','.swp','.swo')
    $auxiliaryNames = @('README.txt','.DS_Store','Thumbs.db','desktop.ini')
    Get-ChildItem -LiteralPath $Roots -Recurse -File | Where-Object {
        $_.Extension -notin $auxiliaryExtensions -and
        $_.Name -notin $auxiliaryNames -and
        !$_.Name.EndsWith('~')
    }
}
function Get-HashMap([string]$Repo, $Files) {
    $result = [ordered]@{}
    foreach ($file in @($Files | Sort-Object FullName -Unique)) {
        $name = $file.FullName.Substring($Repo.Length+1).Replace('\','/')
        $result[$name] = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $result
}
function Get-WorkspaceRecipeFiles([string]$Repo) {
    # Solo entradas que cambian la plataforma, BSP, enlaces de fuentes o
    # biblioteca lwIP copiada. Empaquetado, publicacion y flash no pertenecen
    # a la estructura del workspace.
    $explicit = @(
        (Join-Path $Repo 'artifacts/dependencies-lock.json'),
        (Join-Path $Repo 'config/applications.tcl'),
        (Join-Path $Repo 'scripts/create-workspace.tcl')
    )
    $files = @()
    foreach ($path in $explicit) {
        if (!(Test-Path -LiteralPath $path -PathType Leaf)) { throw "Falta entrada de workspace: $path" }
        $files += Get-Item -LiteralPath $path
    }
    $files += @(Get-InputFiles @((Join-Path $Repo 'config/bsp'),(Join-Path $Repo 'config/lwip211')))
    return @($files | Sort-Object FullName -Unique)
}
function Get-SourceRelativeNames([string]$Repo) {
    return @(Get-InputFiles @((Join-Path $Repo 'src')) | ForEach-Object { $_.FullName.Substring($Repo.Length+1).Replace('\','/') } | Sort-Object)
}
function Get-WorkspaceFingerprint([string]$Repo) {
    $recipe = Get-HashMap $Repo (Get-WorkspaceRecipeFiles $Repo)
    $lines = @('workspace-recipe-schema:2')
    foreach ($key in $recipe.Keys) { $lines += "$key`:$($recipe[$key])" }
    $lines += 'source-paths:'
    $lines += @(Get-SourceRelativeNames $Repo)
    $sha = [Security.Cryptography.SHA256]::Create()
    try { return ([BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes(($lines -join "`n"))))).Replace('-','') }
    finally { $sha.Dispose() }
}
function Get-ActiveHardwareFiles([string]$Repo) {
    $artifacts = [IO.Path]::GetFullPath((Join-Path $Repo 'artifacts'))
    $lockPath = Join-Path $artifacts 'dependencies-lock.json'
    $lock = Get-Content -LiteralPath $lockPath -Raw | ConvertFrom-Json
    $package = [IO.Path]::GetFullPath((Join-Path $artifacts ([string]$lock.uci.package)))
    if (!$package.StartsWith($artifacts.TrimEnd('\') + '\',[StringComparison]::OrdinalIgnoreCase) -or
        !(Test-Path -LiteralPath $package -PathType Container)) { throw 'Paquete hardware activo no valido.' }
    return @((Get-Item -LiteralPath $lockPath)) + @(Get-InputFiles @($package))
}
function Get-BuildInputs([string]$Repo) {
    $files = @(Get-InputFiles @((Join-Path $Repo 'src')))
    $files += @(Get-WorkspaceRecipeFiles $Repo)
    $files += @(Get-ActiveHardwareFiles $Repo)
    return Get-HashMap $Repo $files
}
function Get-PackagingInputs([string]$Repo) {
    $files = @(Get-InputFiles @((Join-Path $Repo 'config/bootimage')))
    foreach ($relative in @('scripts/package.ps1','scripts/generar-nueva-version.ps1')) {
        $path = Join-Path $Repo $relative
        if (!(Test-Path -LiteralPath $path -PathType Leaf)) { throw "Falta receta de empaquetado: $path" }
        $files += Get-Item -LiteralPath $path
    }
    return Get-HashMap $Repo $files
}
function ConvertTo-Map($Value) {
    $result = @{}
    if ($Value -is [System.Collections.IDictionary]) {
        foreach ($key in $Value.Keys) { $result[$key] = $Value[$key] }
    } else {
        foreach ($property in $Value.PSObject.Properties) { $result[$property.Name] = $property.Value }
    }
    return $result
}
function Get-LegacyScopedBuildInputs([string]$Repo, $Inputs) {
    $all = ConvertTo-Map $Inputs
    $lock = Get-Content -LiteralPath (Join-Path $Repo 'artifacts/dependencies-lock.json') -Raw | ConvertFrom-Json
    $activePrefix = ('artifacts/' + ([string]$lock.uci.package).Trim('/').Replace('\','/') + '/')
    $result = [ordered]@{}
    foreach ($key in @($all.Keys | Sort-Object)) {
        if ($key -like 'src/*' -or $key -ceq 'config/applications.tcl' -or
            $key -like 'config/bsp/*' -or $key -like 'config/lwip211/*' -or
            $key -ceq 'scripts/create-workspace.tcl' -or
            $key -ceq 'artifacts/dependencies-lock.json' -or $key.StartsWith($activePrefix,[StringComparison]::OrdinalIgnoreCase)) {
            $result[$key] = $all[$key]
        }
    }
    return $result
}
function Test-LegacyWorkspaceCompatibility([string]$Repo, $Inputs) {
    try {
        $legacy = ConvertTo-Map $Inputs
        $currentRecipe = Get-HashMap $Repo (Get-WorkspaceRecipeFiles $Repo)
        $legacyRecipe = [ordered]@{}
        foreach ($key in $currentRecipe.Keys) {
            if (!$legacy.ContainsKey($key)) { return $false }
            $legacyRecipe[$key] = $legacy[$key]
        }
        Assert-SameMap $legacyRecipe $currentRecipe 'Receta de workspace cambiada'
        $legacySources = @($legacy.Keys | Where-Object { $_ -like 'src/*' } | Sort-Object)
        $currentSources = @(Get-SourceRelativeNames $Repo)
        return (($legacySources -join "`n") -ceq ($currentSources -join "`n"))
    } catch { return $false }
}
function Get-PackageInputs([string]$Workspace) {
    return [ordered]@{
        FSBL = Join-Path $Workspace 'Platform/export/Platform/sw/Platform/boot/fsbl.elf'
        BITSTREAM = Join-Path $Workspace 'Platform/export/Platform/hw/design_1_wrapper.bit'
        CPU0 = Join-Path $Workspace 'CPU0/Debug/CPU0.elf'
        CPU1 = Join-Path $Workspace 'CPU1/Debug/CPU1.elf'
    }
}
function Get-BuildProducts([string]$Workspace) {
    $result = [ordered]@{}
    $paths = Get-PackageInputs $Workspace
    foreach ($key in $paths.Keys) {
        $result[$key] = (Get-FileHash -LiteralPath $paths[$key] -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $result
}
function Assert-SameMap($Expected, $Actual, [string]$Message) {
    $expectedMap = ConvertTo-Map $Expected
    if ($expectedMap.Count -ne $Actual.Count) { throw $Message }
    foreach ($key in $Actual.Keys) {
        if (!$expectedMap.ContainsKey($key) -or $expectedMap[$key] -cne $Actual[$key]) { throw "$Message ($key)" }
    }
}
function Assert-ClosedIde([string]$Workspace) {
    $path = Join-Path $Workspace '.metadata/.lock'
    if (Test-Path -LiteralPath $path) {
        try { $handle = [IO.File]::Open($path,'Open','ReadWrite','None'); $handle.Dispose() }
        catch { throw 'Cierra Vitis antes de compilar o generar una version por consola.' }
    }
}
function Assert-BuildRecord([string]$Repo, [string]$Workspace, [string]$VitisHome) {
    $path = Join-Path $Workspace '.sitau-build.json'
    if (!(Test-Path -LiteralPath $path -PathType Leaf)) { throw 'Falta registro de compilacion. Ejecuta setup.ps1 -Action Build.' }
    $record = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
    if ($record.schemaVersion -notin @(1,2)) { throw 'Version de registro de compilacion no soportada.' }
    $recordInputs = if ($record.schemaVersion -eq 1) { Get-LegacyScopedBuildInputs $Repo $record.inputs } else { ConvertTo-Map $record.inputs }
    $tracked = @{}
    $trackedText = [string](Invoke-RepoGit $Repo @('ls-files','-z'))
    foreach ($name in $trackedText.Split([char]0)) { $tracked[$name] = $true }
    foreach ($name in $recordInputs.Keys) {
        if (!$tracked.ContainsKey($name)) { throw "Entrada de compilacion no versionada: $name" }
    }
    Assert-SameMap $recordInputs (Get-BuildInputs $Repo) 'Entradas de firmware cambiadas desde la compilacion. Ejecuta Build'
    Assert-SameMap $record.products (Get-BuildProducts $Workspace) 'Binarios cambiados desde la compilacion. Ejecuta Build'
    $toolHash = (Get-FileHash -LiteralPath (Join-Path $VitisHome 'data/version.bat') -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($record.toolchain.versionFileSha256 -ne $toolHash) { throw 'Instalacion Xilinx distinta de la registrada.' }
    return $record
}
function ConvertTo-ReleaseVersion([string]$Value) {
    if ($Value -cnotmatch '^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$') { throw 'Version invalida. Usa mayor.menor.parche, sin prefijos, sufijos ni ceros iniciales.' }
    try { return [version]::Parse($Value) }
    catch { throw 'Cada componente de version debe estar entre 0 y 2147483647.' }
}
function Assert-NewVersion([string]$Output, [string]$Value) {
    $requested = ConvertTo-ReleaseVersion $Value
    if (Test-Path -LiteralPath (Join-Path $Output $Value)) { throw 'La version ya existe; no se sobrescribe.' }
    foreach ($dir in (Get-ChildItem -LiteralPath $Output -Directory)) {
        if ($dir.Name -like '.pending-*') { continue }
        $existing = ConvertTo-ReleaseVersion $dir.Name
        if ($requested -le $existing) { throw "La nueva version debe superar $($dir.Name)." }
    }
}
