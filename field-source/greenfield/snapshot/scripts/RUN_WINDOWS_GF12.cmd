@echo off
setlocal EnableExtensions
set "RC=0"
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (echo [BLOCKED] VS x64 Native Tools required & set "RC=2" & goto :DONE)
where cl >nul 2>&1 || (set "RC=3" & goto :DONE)
where cmake >nul 2>&1 || (set "RC=3" & goto :DONE)
where ctest >nul 2>&1 || (set "RC=3" & goto :DONE)
where python >nul 2>&1 || (set "RC=3" & goto :DONE)
set "ROOT=%~dp0.."
python "%~dp0verify_gf12.py" || (set "RC=4" & goto :DONE)
for %%C in (Release Debug) do (
  cmake -S "%ROOT%" -B "%ROOT%\_local_windows_gf12_%%C" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%%C -DGF08_ENABLE_NATIVE_OWNER_OBJECTS=ON || (set "RC=5" & goto :DONE)
  cmake --build "%ROOT%\_local_windows_gf12_%%C" || (set "RC=6" & goto :DONE)
  ctest --test-dir "%ROOT%\_local_windows_gf12_%%C" --output-on-failure || (set "RC=7" & goto :DONE)
  python "%~dp0gf11_record_native.py" "%ROOT%\_local_windows_gf12_%%C" "%%C" || (set "RC=8" & goto :DONE)
)
:DONE
if "%RC%"=="0" (echo [GF12] LOCAL Windows source/export acceptance - NOT_RELEASE) else (echo [GF12] BLOCKED/FAIL exit=%RC%)
echo [GF12] Production dispatcher final link, live LSX and Consumer NOT_RUN.
pause
exit /b %RC%
