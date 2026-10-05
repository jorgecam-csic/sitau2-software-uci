# Validar el origen sin modificar la instalacion ni la variante mantenida.
function Assert-LwipSource([string]$Repo, [string]$VitisHome) {
    $patches = Get-Content -LiteralPath (Join-Path $Repo 'config/lwip211/manifest.json') -Raw | ConvertFrom-Json
    # Comprobar primero todas las copias mantenidas antes de aceptar un origen.
    foreach ($patch in $patches) {
        $custom = Join-Path $Repo ('config/lwip211/' + $patch.file)
        if (!(Test-Path -LiteralPath $custom -PathType Leaf)) {
            throw "Falta personalizacion: $custom. SHA-256 esperado: $($patch.sha256)"
        }
        $actual = (Get-FileHash -LiteralPath $custom -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actual -ne $patch.sha256) {
            throw "Personalizacion modificada: $custom. SHA-256 observado: $actual; esperado: $($patch.sha256)"
        }
    }
    $results = @()
    foreach ($patch in $patches) {
        $source = Join-Path $VitisHome ('data/embeddedsw/ThirdParty/sw_services/lwip211_v1_8/src/contrib/ports/xilinx/netif/' + $patch.file)
        $expected = "original=$($patch.stockSha256); SITAU2=$($patch.sha256)"
        if (!(Test-Path -LiteralPath $source -PathType Leaf)) {
            throw "Falta archivo lwIP de Vitis: $source. SHA-256 observado: ausente; permitidos: $expected"
        }
        $actual = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actual -eq $patch.stockSha256) { $kind = 'original auditado' }
        elseif ($actual -eq $patch.sha256) { $kind = 'personalizacion SITAU2 conocida' }
        else { throw "Biblioteca de Vitis distinta de las aceptadas: $source. SHA-256 observado: $actual; permitidos: $expected" }
        $results += [pscustomobject]@{file=$patch.file;path=$source;sha256=$actual;kind=$kind}
    }
    return $results
}
