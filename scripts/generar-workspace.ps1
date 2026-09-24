# Accion explicita: crear siempre desde cero; Setup pregunta si existe el anterior.
if ($args.Count -ne 0) { throw 'Este script no admite parametros.' }
& (Join-Path $PSScriptRoot 'setup.ps1') -Action Setup
