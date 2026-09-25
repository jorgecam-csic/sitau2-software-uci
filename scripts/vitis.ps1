function Test-VitisHome([string]$Path, [string]$Version = '2022.2') {
    if ([string]::IsNullOrWhiteSpace($Path)) { return $false }
    try { $fullPath = [IO.Path]::GetFullPath($Path) } catch { return $false }
    foreach ($relative in @('bin\xsct.bat','bin\vitis.bat','bin\bootgen.bat','data\version.bat')) {
        if (![IO.File]::Exists([IO.Path]::Combine($fullPath, $relative))) { return $false }
    }
    try { $versionText = [IO.File]::ReadAllText([IO.Path]::Combine($fullPath, 'data\version.bat')) } catch { return $false }
    return $versionText -match ([regex]::Escape($Version))
}

function Resolve-VitisHome([string]$VitisHome, [string]$Version = '2022.2') {
    if (![string]::IsNullOrWhiteSpace($VitisHome)) {
        if (!(Test-VitisHome $VitisHome $Version)) {
            throw "No se encuentra una instalacion completa de Vitis Classic $Version en: $VitisHome"
        }
        return [IO.Path]::GetFullPath($VitisHome).TrimEnd('\')
    }

    $candidates = New-Object 'System.Collections.Generic.List[string]'
    function Add-VitisCandidate([string]$Path) {
        if (![string]::IsNullOrWhiteSpace($Path)) { $candidates.Add($Path) }
    }

    # El instalador unificado registra la raiz (por ejemplo D:\Xilinx), no el
    # directorio Vitis completo. Se consultan las vistas de 32 y 64 bits.
    $uninstallRoots = @(
        'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall',
        'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall',
        'HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall'
    )
    foreach ($root in $uninstallRoots) {
        if (!(Test-Path -LiteralPath $root)) { continue }
        foreach ($key in (Get-ChildItem -LiteralPath $root -ErrorAction SilentlyContinue)) {
            $entry = Get-ItemProperty -LiteralPath $key.PSPath -ErrorAction SilentlyContinue
            if (!$entry -or $entry.DisplayName -notmatch '(?i)\b(Vitis|Xilinx Design Tools)\b' -or
                (($entry.DisplayVersion -ne $Version) -and ($entry.DisplayName -notmatch ([regex]::Escape($Version))))) { continue }
            if ($entry.InstallLocation) {
                Add-VitisCandidate $entry.InstallLocation
                Add-VitisCandidate ([IO.Path]::Combine([string]$entry.InstallLocation, 'Vitis', $Version))
            }
            if ($entry.DisplayIcon -and ([string]$entry.DisplayIcon) -match '^(.*?)[\\/]\.xinstall[\\/]') {
                Add-VitisCandidate ([IO.Path]::Combine($Matches[1], 'Vitis', $Version))
            }
        }
    }

    # XILINX_VITIS queda definido al entrar desde un command prompt de Vitis.
    Add-VitisCandidate $env:XILINX_VITIS

    # Ultimo recurso para instalaciones copiadas o entradas de registro ausentes.
    foreach ($drive in (Get-PSDrive -PSProvider FileSystem -ErrorAction SilentlyContinue)) {
        foreach ($vendor in @('Xilinx','AMD')) {
            Add-VitisCandidate ([IO.Path]::Combine($drive.Root, $vendor, 'Vitis', $Version))
        }
    }

    $seen = @{}
    foreach ($candidate in $candidates) {
        try { $fullPath = [IO.Path]::GetFullPath($candidate).TrimEnd('\') } catch { continue }
        $key = $fullPath.ToUpperInvariant()
        if ($seen.ContainsKey($key)) { continue }
        $seen[$key] = $true
        if (Test-VitisHome $fullPath $Version) {
            Write-Host "Vitis detectado: $fullPath"
            return $fullPath
        }
    }
    throw "No se encuentra Vitis Classic $Version. Instala esa version o indica -VitisHome 'C:\ruta\Vitis\$Version'."
}
