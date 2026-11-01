@echo off
setlocal EnableExtensions
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (echo [BLOCKED] x64 Developer Command Prompt required & exit /b 2)
where cmake >nul 2>&1 || exit /b 3
where python >nul 2>&1 || exit /b 3
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
if /I not "%CONFIG%"=="Release" if /I not "%CONFIG%"=="Debug" (echo Usage: RUN_WINDOWS_GF04.cmd [Release^|Debug] & exit /b 2)
set "OUT=%~dp0..\_local_windows_gf04_%CONFIG%"
python "%~dp0verify_package.py" || exit /b 4
cmake -S "%~dp0.." -B "%OUT%" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%CONFIG% || exit /b 5
cmake --build "%OUT%" || exit /b 6
ctest --test-dir "%OUT%" --output-on-failure || exit /b 7
echo [SOURCE_LINKED_WINDOWS_LOCAL_BUILD_PASS_ONLY] GF04 is NOT_RELEASE.
exit /b 0
