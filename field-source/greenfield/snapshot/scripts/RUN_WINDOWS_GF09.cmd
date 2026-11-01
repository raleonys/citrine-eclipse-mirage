@echo off
setlocal EnableExtensions
rem GF09 local-only native build/ABI evidence; no release publication.
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (echo [BLOCKED] VS x64 Native Tools Prompt required & exit /b 2)
where cl >nul 2>&1 || (echo [BLOCKED] MSVC cl missing & exit /b 3)
where cmake >nul 2>&1 || (echo [BLOCKED] cmake missing & exit /b 3)
where ctest >nul 2>&1 || (echo [BLOCKED] ctest missing & exit /b 3)
where python >nul 2>&1 || (echo [BLOCKED] python missing & exit /b 3)
set "ROOT=%~dp0.."
python "%~dp0verify_gf09.py" || (echo [BLOCKED] GF09 source integrity & exit /b 4)
for %%C in (Release Debug) do (
  cmake -S "%ROOT%" -B "%ROOT%\_local_windows_gf09_%%C" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%%C -DGF08_ENABLE_NATIVE_OWNER_OBJECTS=ON || exit /b 5
  cmake --build "%ROOT%\_local_windows_gf09_%%C" || exit /b 6
  ctest --test-dir "%ROOT%\_local_windows_gf09_%%C" --output-on-failure || exit /b 7
  python "%~dp0gf09_record_native.py" "%ROOT%\_local_windows_gf09_%%C" "%%C" || exit /b 8
)
echo [GF09_NATIVE_ABI_AND_OWNER_OBJECTS_COMPLETED_LOCAL_NOT_RELEASE]
echo [WARNING] Real LSX, full dispatcher, Windows IOCP session and game are NOT claimed.
exit /b 0
