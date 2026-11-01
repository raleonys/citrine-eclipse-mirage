@echo off
setlocal EnableExtensions DisableDelayedExpansion
title REDFIELD - crt Local Build
echo ================================================================
echo   REDFIELD RF37 GOLDEN BINDING FIX 01 - LOCAL BUILD / NOT RELEASE
echo   crt: separate objects, CRT and outputs
echo ================================================================
python -u "%~dp0build_crt_acceptance.py" %*
set "PRELOADER_L_CHILD_EXIT=%ERRORLEVEL%"
echo.
if "%PRELOADER_L_CHILD_EXIT%"=="0" (echo RESULT: SCOPED PASS - see receipt scope) else (echo RESULT: FAIL - see build log/failure evidence)
echo EXIT CODE: %PRELOADER_L_CHILD_EXIT%
echo OUTPUT: "%~dp0..\..\build\crt"
echo.
pause
exit /b %PRELOADER_L_CHILD_EXIT%
