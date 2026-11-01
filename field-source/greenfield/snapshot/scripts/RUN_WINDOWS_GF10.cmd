@echo off
setlocal EnableExtensions
set "RC=0"
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (echo [BLOCKED] x64 VS Native Tools Prompt required & set "RC=2" & goto :DONE)
where cl >nul 2>&1 || (set "RC=3" & goto :DONE)
where cmake >nul 2>&1 || (set "RC=3" & goto :DONE)
where ctest >nul 2>&1 || (set "RC=3" & goto :DONE)
where python >nul 2>&1 || (set "RC=3" & goto :DONE)
set "ROOT=%~dp0.."
python "%~dp0verify_gf10.py" || (echo [BLOCKED] GF10 source verification failed & set "RC=4" & goto :DONE)
for %%C in (Release Debug) do (
  echo [GF10] Configuring %%C native local build
  cmake -S "%ROOT%" -B "%ROOT%\_local_windows_gf10_%%C" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%%C -DGF08_ENABLE_NATIVE_OWNER_OBJECTS=ON || (set "RC=5" & goto :DONE)
  cmake --build "%ROOT%\_local_windows_gf10_%%C" || (set "RC=6" & goto :DONE)
  ctest --test-dir "%ROOT%\_local_windows_gf10_%%C" --output-on-failure || (set "RC=7" & goto :DONE)
  python "%~dp0gf10_record_native.py" "%ROOT%\_local_windows_gf10_%%C" "%%C" || (set "RC=8" & goto :DONE)
)
:DONE
if "%RC%"=="0" (echo [GF10] LOCAL WINDOWS BUILD+HOST+STATIC_PE TESTS PASS - NOT_RELEASE) else (echo [GF10] BLOCKED/FAIL exit=%RC%)
echo [GF10] Real LSX / IOCP dispatch / Consumer / game NOT_RUN.
pause
exit /b %RC%
