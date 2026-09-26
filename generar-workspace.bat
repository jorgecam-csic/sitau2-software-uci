@echo off
setlocal EnableExtensions DisableDelayedExpansion
if not "%~1"=="" (
    echo Este script no admite parametros.
    exit /b 1
)
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\generar-workspace.ps1"
set "SITAU_EXIT_CODE=%ERRORLEVEL%"
pause
endlocal & exit /b %SITAU_EXIT_CODE%
