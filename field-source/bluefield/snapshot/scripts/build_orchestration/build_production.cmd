@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "TARGET=%~dp0build_production.ps1"
set "LABEL=PRODUCTION BUILD"
set "DESCRIPTION=Build the preloader_l Windows DLL as a LOCAL_BUILD"

title preloader_l - %LABEL%
echo ============================================================
echo  preloader_l - %LABEL%
echo  %DESCRIPTION%
echo ============================================================
echo.

if not exist "%TARGET%" (
  echo ERROR: Missing target "%TARGET%"
  set "RC=2"
  goto :completed
)

echo Running: build_production.ps1
echo.
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%TARGET%" %*
set "RC=%ERRORLEVEL%"

:completed
echo.
if "%RC%"=="0" (
  powershell.exe -NoLogo -NoProfile -Command "$lines = @('============================================================', '  %LABEL%: PASS', '  Exit code: %RC%', '============================================================'); foreach ($line in $lines) { Write-Host $line -ForegroundColor Green }"
  if errorlevel 1 (
    echo ============================================================
    echo  %LABEL%: PASS
    echo  Exit code: %RC%
    echo ============================================================
  )
) else (
  powershell.exe -NoLogo -NoProfile -Command "$lines = @('============================================================', '  %LABEL%: FAIL', '  Exit code: %RC%', '============================================================'); foreach ($line in $lines) { Write-Host $line -ForegroundColor Red }"
  if errorlevel 1 (
    echo ============================================================
    echo  %LABEL%: FAIL
    echo  Exit code: %RC%
    echo ============================================================
  )
)
echo.
echo Press any key to close this operator window.
pause >nul
exit /b %RC%
