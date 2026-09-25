@echo off
setlocal EnableExtensions DisableDelayedExpansion
if not "%~1"=="" (
    echo Este lanzador no admite parametros. Use scripts/setup.ps1 -Action Build para automatizacion o rutas alternativas.
    exit /b 1
)
rem Compila el workspace existente y genera los paquetes de desarrollo.
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\setup.ps1" -Action Build
set "SITAU_EXIT_CODE=%ERRORLEVEL%"
pause
endlocal & exit /b %SITAU_EXIT_CODE%
