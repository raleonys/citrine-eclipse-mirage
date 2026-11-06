@echo off
REM SPDX-License-Identifier: GPL-3.0-only
setlocal EnableExtensions DisableDelayedExpansion
set "TARGET=%~dp0build_crt_acceptance.ps1"
set "LABEL=STATIC CRT ACCEPTANCE"
set "DESCRIPTION=Build and run static CRT and DLL interface tests"

title SURGE - %LABEL%
echo ============================================================
echo  SURGE - %LABEL%
echo  %DESCRIPTION%
echo ============================================================
echo.

if not exist "%TARGET%" (
  echo ERROR: Missing target "%TARGET%"
  set "RC=2"
  goto :completed
)

echo Running: build_crt_acceptance.ps1
echo.
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%TARGET%" -LauncherOwnsSummary %*
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
