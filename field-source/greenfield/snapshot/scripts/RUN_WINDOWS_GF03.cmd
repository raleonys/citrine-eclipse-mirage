@echo off
setlocal EnableExtensions
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
 echo [BLOCKED] Run in x64 VS Developer Command Prompt.
 exit /b 2
)
where cmake >nul 2>&1 || exit /b 3
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
if /I not "%CONFIG%"=="Release" if /I not "%CONFIG%"=="Debug" (
 echo Usage: RUN_WINDOWS_GF03.cmd [Release^|Debug]
 exit /b 2
)
set "BUILD_DIR=%~dp0..\_local_windows_gf03_%CONFIG%"
python "%~dp0verify_package.py" || exit /b 4
cmake -S "%~dp0.." -B "%BUILD_DIR%" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%CONFIG% || exit /b 5
cmake --build "%BUILD_DIR%" || exit /b 6
ctest --test-dir "%BUILD_DIR%" --output-on-failure || exit /b 7
echo [PASS] GF03 Windows LOCAL_BUILD_NOT_RELEASE. Exact SHA + PE export must be independently recorded.
exit /b 0
