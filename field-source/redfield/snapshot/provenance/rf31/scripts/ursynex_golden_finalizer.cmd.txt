@echo off
setlocal EnableExtensions DisableDelayedExpansion
title REDFIELD - Local Golden Gate
python -u "%~dp0ursynex_golden_finalizer.py" %*
set "PRELOADER_L_CHILD_EXIT=%ERRORLEVEL%"
echo EXIT CODE: %PRELOADER_L_CHILD_EXIT%
pause
exit /b %PRELOADER_L_CHILD_EXIT%
