@echo off
setlocal EnableExtensions
if not defined VSCMD_ARG_TGT_ARCH (
  echo [FAIL] Run this from an x64 Visual Studio Developer Command Prompt.
  exit /b 2
)
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
  echo [FAIL] x64 MSVC tools required.
  exit /b 2
)
where cmake >nul 2>&1 || exit /b 3
cmake -S "%~dp0.." -B "%~dp0..\_local_windows" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release || exit /b 4
cmake --build "%~dp0..\_local_windows" || exit /b 5
ctest --test-dir "%~dp0..\_local_windows" --output-on-failure || exit /b 6
echo [PASS] GF00 LOCAL_BUILD_NOT_RELEASE. No release packaging or promotion.
exit /b 0
