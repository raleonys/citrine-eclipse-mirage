@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "RC=0"
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (echo [BLOCKED] GF13_REAL_WINDOWS_X64_VS_REQUIRED & set "RC=2" & goto :DONE)
where cl >nul 2>&1 || (echo [BLOCKED] CL_NOT_FOUND & set "RC=3" & goto :DONE)
where cmake >nul 2>&1 || (echo [BLOCKED] CMAKE_NOT_FOUND & set "RC=3" & goto :DONE)
where ctest >nul 2>&1 || (echo [BLOCKED] CTEST_NOT_FOUND & set "RC=3" & goto :DONE)
where python >nul 2>&1 || (echo [BLOCKED] PYTHON_NOT_FOUND & set "RC=3" & goto :DONE)
set "PYTHONDONTWRITEBYTECODE=1"
set "ROOT=%~dp0.."
python "%~dp0verify_gf13.py" || (echo [BLOCKED] GF13_SOURCE_VERIFY & set "RC=4" & goto :DONE)
for %%C in (Release Debug) do (
 echo [GF13] Exact native Windows x64 source+original dispatch link %%C
 cmake -S "%ROOT%" -B "%ROOT%\_local_windows_gf13_%%C" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%%C -DGF13_ENABLE_ORIGINAL_DISPATCH_NATIVE_LINK=ON || (set "RC=5" & goto :DONE)
 cmake --build "%ROOT%\_local_windows_gf13_%%C" --target gf_preloader gf_export_host gf13_original_dispatch_native || (set "RC=6" & goto :DONE)
 if exist "%ROOT%\_local_windows_gf13_%%C\GF13_TESTS_%%C.xml" del /q "%ROOT%\_local_windows_gf13_%%C\GF13_TESTS_%%C.xml"
 ctest --test-dir "%ROOT%\_local_windows_gf13_%%C" --output-on-failure --output-junit "%ROOT%\_local_windows_gf13_%%C\GF13_TESTS_%%C.xml" -R "^(gf_windows_export_shape|gf13_original_dispatch_real_windows)$" || (set "RC=7" & goto :DONE)
 python "%~dp0gf13_record_native.py" "%ROOT%\_local_windows_gf13_%%C" "%%C" || (set "RC=8" & goto :DONE)
)
:DONE
if "%RC%"=="0" (echo [GF13] LOCAL WINDOWS ORIGINAL DISPATCH LINK PASS - NOT_RELEASE) else (echo [GF13] BLOCKED/FAIL exit=%RC%)
echo [GF13] Shipping SURGE/IOCP/LSX/Consumer/game NOT_RUN.
pause
exit /b %RC%
