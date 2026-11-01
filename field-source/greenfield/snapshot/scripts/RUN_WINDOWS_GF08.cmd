@echo off
setlocal EnableExtensions
rem GF08 native test-only; use in an authorized VS x64 Native Tools Prompt.
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (echo [BLOCKED] VS x64 Native Tools Prompt required & exit /b 2)
where cl >nul 2>&1 || (echo [BLOCKED] cl.exe missing & exit /b 3)
where cmake >nul 2>&1 || (echo [BLOCKED] cmake missing & exit /b 3)
where ctest >nul 2>&1 || (echo [BLOCKED] ctest missing & exit /b 3)
where python >nul 2>&1 || (echo [BLOCKED] Python missing & exit /b 3)
set "ROOT=%~dp0.."
python "%~dp0verify_gf08.py" || (echo [BLOCKED] source verification failed & exit /b 4)
for %%C in (Release Debug) do (
  set "OUT=%ROOT%\_local_windows_gf08_%%C"
  cmake -S "%ROOT%" -B "%ROOT%\_local_windows_gf08_%%C" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%%C -DGF08_ENABLE_NATIVE_OWNER_OBJECTS=ON || exit /b 5
  cmake --build "%ROOT%\_local_windows_gf08_%%C" || exit /b 6
  ctest --test-dir "%ROOT%\_local_windows_gf08_%%C" --output-on-failure || exit /b 7
  python "%~dp0gf08_record_native.py" "%ROOT%\_local_windows_gf08_%%C" "%%C" || exit /b 8
)
echo [GF08_NATIVE_COMPILE_AND_EXPORT_HOST_TESTS_COMPLETED_NOT_RELEASE]
echo [WARNING] No production LSX or game acceptance is claimed by this batch.
exit /b 0
