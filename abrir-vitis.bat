@echo off
setlocal EnableExtensions DisableDelayedExpansion
rem Abre el workspace existente; nunca lo genera ni regenera.
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\setup.ps1" -Action Open %*
set "SITAU_EXIT_CODE=%ERRORLEVEL%"
if not "%SITAU_EXIT_CODE%"=="0" pause
endlocal & exit /b %SITAU_EXIT_CODE%
