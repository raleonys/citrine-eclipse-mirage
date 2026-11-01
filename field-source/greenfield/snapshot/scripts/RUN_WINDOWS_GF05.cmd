@echo off
setlocal EnableExtensions
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (echo [BLOCKED] VS x64 Native Tools Prompt required & exit /b 2)
where cmake >nul 2>&1 || exit /b 3
where python >nul 2>&1 || exit /b 3
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
if /I not "%CONFIG%"=="Release" if /I not "%CONFIG%"=="Debug" (echo Usage: RUN_WINDOWS_GF05.cmd [Release^|Debug] & exit /b 2)
set "OUT=%~dp0..\_local_windows_gf05_%CONFIG%"
python "%~dp0verify_package.py" || exit /b 4
cmake -S "%~dp0.." -B "%OUT%" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%CONFIG% || exit /b 5
cmake --build "%OUT%" || exit /b 6
ctest --test-dir "%OUT%" --output-on-failure || exit /b 7
echo [GF05_LOCAL_WINDOWS_BUILD_PASS_ONLY_NOT_RELEASE] No LSX runtime claims.
exit /b 0
