@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "TARGET=%~dp0run_golden.py"
set "LABEL=RUN_GOLDEN"
set "DESCRIPTION=Run five Golden gates and publish only on PASS"

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

echo Running: run_golden.py
echo.
python "%TARGET%" %*
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
