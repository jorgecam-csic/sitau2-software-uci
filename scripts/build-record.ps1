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
    # Los respaldos *.bak no son entradas del proyecto. No excluir de forma
    # general lo ignorado por Git: una cabecera ignorada si puede compilarse.
    Get-ChildItem -LiteralPath $Roots -Recurse -File | Where-Object { $_.Extension -ine '.bak' }
}
function Get-BuildInputs([string]$Repo) {
    $result = [ordered]@{}
    foreach ($root in @('src','config','scripts','artifacts')) {
        foreach ($file in (Get-InputFiles @((Join-Path $Repo $root)) | Sort-Object FullName)) {
            $name = $file.FullName.Substring($Repo.Length+1).Replace('\','/')
            $result[$name] = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        }
    }
    return $result
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
    $expectedMap = @{}
    if ($Expected -is [System.Collections.IDictionary]) {
        foreach ($key in $Expected.Keys) { $expectedMap[$key] = $Expected[$key] }
    } else {
        foreach ($property in $Expected.PSObject.Properties) { $expectedMap[$property.Name] = $property.Value }
    }
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
    if ($record.schemaVersion -ne 1 -or $record.commit -ne (Get-RepoCommit $Repo)) { throw 'La compilacion pertenece a otro commit. Ejecuta Build despues del commit.' }
    $tracked = @{}
    $trackedText = [string](Invoke-RepoGit $Repo @('ls-files','-z'))
    foreach ($name in $trackedText.Split([char]0)) { $tracked[$name] = $true }
    foreach ($property in $record.inputs.PSObject.Properties) {
        if (!$tracked.ContainsKey($property.Name)) { throw "Entrada de compilacion no versionada: $($property.Name)" }
    }
    Assert-SameMap $record.inputs (Get-BuildInputs $Repo) 'Entradas cambiadas desde la compilacion. Ejecuta Build'
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
