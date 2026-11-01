@echo off
setlocal EnableExtensions DisableDelayedExpansion
color 0B
cls
echo ============================================================
echo   GF14  ^|  REAL WINDOWS MSVC NATIVE EVIDENCE  ^|  NOT_RELEASE
echo ============================================================
echo [INFO] x64 MSVC, /MT, /MTd, original dispatch, export/LoadLibrary
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
  echo [BLOCKED] REAL VS x64 Native Tools Command Prompt required.
  set "RC=2"
  goto :DONE
)
where python >nul 2>&1
if errorlevel 1 (
  echo [BLOCKED] Python not found.
  set "RC=3"
  goto :DONE
)
set "PYTHONDONTWRITEBYTECODE=1"
python "%~dp0build_native_acceptance.py" "%~dp0..\.."
set "RC=%ERRORLEVEL%"
:DONE
if "%RC%"=="0" (
  color 0A
  echo [PASS] Native test evidence collected; NOT_RELEASE.
) else (
  color 0C
  echo [FAIL/BLOCKED] Native test evidence NOT accepted.
)
echo [GF14] Numerical exit code: %RC%
echo [GF14] No canonical or shipping SURGE mutation performed.
pause
exit /b %RC%
