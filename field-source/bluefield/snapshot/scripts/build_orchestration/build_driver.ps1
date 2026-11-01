# SPDX-License-Identifier: GPL-3.0-only
[CmdletBinding()]
param([ValidateSet('PRODUCTION','DEBUG','NATIVE','CRT','DIAGNOSTIC')][string]$Profile='PRODUCTION')
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$profileRoot=Join-Path $root ('build\'+$Profile.ToLowerInvariant())
$build=if($Profile -eq 'PRODUCTION'){Join-Path $profileRoot 'msvc'}else{$profileRoot}
$reportRoot=Join-Path $root 'build\reports'
New-Item -ItemType Directory -Force -Path $profileRoot,$build,$reportRoot | Out-Null
$report=Join-Path $reportRoot ('build_'+$Profile.ToLowerInvariant()+'.json')
$result=1
$buildLock=$null
. (Join-Path $PSScriptRoot 'build_support.ps1')
try {
    $buildLock=Open-BuildLock (Join-Path $profileRoot '.operator.lock')
    Clear-CompilerSelectors
    $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if(-not (Test-Path -LiteralPath $vswhere)){throw 'Visual Studio Installer/vswhere was not found.'}
    $instances=(& $vswhere -all -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json | ConvertFrom-Json)
    if(-not $instances){throw 'No installed Visual Studio C++ x64 toolchain was found.'}
    $instance=$instances | Sort-Object {[Version]$_.installationVersion} -Descending | Select-Object -First 1
    $cmakeCommand=Get-Command cmake.exe -ErrorAction SilentlyContinue
    $cmake=if($cmakeCommand){$cmakeCommand.Source}else{Join-Path $instance.installationPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'}
    if(-not (Test-Path -LiteralPath $cmake)){throw 'CMake is required; install the Visual Studio CMake component.'}
    $capabilities=(& $cmake -E capabilities | ConvertFrom-Json)
    if($LASTEXITCODE -ne 0){$result=$LASTEXITCODE;throw 'CMake capabilities failed.'}
    if($Profile -eq 'PRODUCTION'){
        $vcvars=Join-Path $instance.installationPath 'VC\Auxiliary\Build\vcvars64.bat'
        if(-not (Test-Path -LiteralPath $vcvars)){throw 'vcvars64.bat is missing.'}
        $command='call "'+$vcvars+'" >nul && set'
        $lines=& $env:ComSpec /d /s /c $command
        if($LASTEXITCODE -ne 0){$result=$LASTEXITCODE;throw 'MSVC x64 environment setup failed.'}
        foreach($line in $lines){$i=$line.IndexOf('=');if($i -gt 0){[Environment]::SetEnvironmentVariable($line.Substring(0,$i),$line.Substring($i+1),'Process')}}
        Clear-CompilerSelectors
        $generator=$capabilities.generators | Where-Object {$_.name -eq 'NMake Makefiles'} | Select-Object -First 1
    } else {
        $major=([Version]$instance.installationVersion).Major
        $generator=$capabilities.generators | Where-Object {$_.name -match ('^Visual Studio '+$major+' ')} | Select-Object -First 1
    }
    if(-not $generator){throw 'The required CMake generator is unavailable.'}
    $configuration=if($Profile -in @('DEBUG','CRT')){'Debug'}else{'Release'}
    $testing=if($Profile -eq 'PRODUCTION'){'OFF'}else{'ON'}
    if($Profile -eq 'PRODUCTION'){
        & $cmake -S $root -B $build -G $generator.name "-DCMAKE_BUILD_TYPE=Release" "-DPRELOADER_L_PROFILE=$Profile" "-DBUILD_TESTING=$testing"
    } else {
        & $cmake -S $root -B $build -G $generator.name -A x64 "-DCMAKE_GENERATOR_INSTANCE=$($instance.installationPath)" "-DPRELOADER_L_PROFILE=$Profile" "-DBUILD_TESTING=$testing"
    }
    if($LASTEXITCODE -ne 0){$result=$LASTEXITCODE;throw 'CMake configure failed.'}
    if($Profile -eq 'PRODUCTION'){& $cmake --build $build}else{& $cmake --build $build --config $configuration --parallel}
    if($LASTEXITCODE -ne 0){$result=$LASTEXITCODE;throw 'MSVC compile/link failed.'}
    $base=if($Profile -eq 'PRODUCTION'){'preloader_l'}elseif($Profile -eq 'DEBUG'){'preloader_l_debug'}elseif($Profile -eq 'NATIVE'){'preloader_l_native_test'}elseif($Profile -eq 'DIAGNOSTIC'){'preloader_l_diagnostic'}else{'preloader_l_crt_test'}
    $dll=if($Profile -eq 'PRODUCTION'){Join-Path $build ($base+'.dll')}else{Join-Path (Join-Path $build $configuration) ($base+'.dll')}
    if(-not (Test-Path -LiteralPath $dll) -or (Get-Item -LiteralPath $dll).Length -eq 0){throw 'The DLL output is missing or empty.'}
    $python=Get-Command python.exe -ErrorAction SilentlyContinue
    if(-not $python){throw 'Python 3 is required for the PE contract verifier.'}
    $env:PYTHONDONTWRITEBYTECODE='1'
    $goldenToolSelftests='NOT_RUN'
    if($Profile -eq 'PRODUCTION'){
        & $python.Source (Join-Path $root 'tools\golden\scripts\test_pe_metadata_tools.py')
        if($LASTEXITCODE -ne 0){$result=$LASTEXITCODE;throw 'Golden protocol tool selftests failed; this is not final-DLL acceptance.'}
        $goldenToolSelftests='PASS'
    }
    & $python.Source (Join-Path $root 'scripts\qa\check_pe.py') $dll
    if($LASTEXITCODE -ne 0){$result=$LASTEXITCODE;throw 'PE/exports/CRT contract failed.'}
    if($testing -eq 'ON'){
        $ctest=Join-Path (Split-Path -Parent $cmake) 'ctest.exe'
        & $ctest --test-dir $build -C $configuration --output-on-failure
        if($LASTEXITCODE -ne 0){$result=$LASTEXITCODE;throw 'Native/core executable tests failed.'}
    }
    if($Profile -in @('NATIVE','CRT')){
        & $python.Source (Join-Path $root 'tests\native\test_dll_load.py') $dll
        if($LASTEXITCODE -ne 0){$result=$LASTEXITCODE;throw 'Real DLL load/hooks/unload fixture failed.'}
        & $python.Source (Join-Path $root 'tests\native\test_dll_load.py') $dll --online-fixture (Join-Path (Split-Path -Parent $dll) 'preloader_l_online.dll')
        if($LASTEXITCODE -ne 0){$result=$LASTEXITCODE;throw 'Version-2 online provider native ABI fixture failed.'}
    }
    $destination=Join-Path $root ('dist\'+$Profile.ToLowerInvariant())
    New-Item -ItemType Directory -Force -Path $destination | Out-Null
    $goldenStatus='NOT_APPLICABLE_PROFILE'
    if($Profile -eq 'PRODUCTION'){
        $goldenReport=Join-Path $reportRoot 'golden_production.json'
        $published=Join-Path $destination ($base+'.dll')
        & $python.Source (Join-Path $root 'scripts\qa\run_golden.py') --dll $dll --build-dir $build --output $published --report $goldenReport
        if($LASTEXITCODE -ne 0){$result=$LASTEXITCODE;throw 'Golden pipeline failed; no newly accepted DLL was published.'}
        $goldenReceipt=Get-Content -LiteralPath $goldenReport -Raw | ConvertFrom-Json
        if($goldenReceipt.status -ne 'PASS' -or -not $goldenReceipt.published){throw 'Golden receipt does not authorize this DLL output.'}
        $dll=$published
        $goldenStatus='PASS'
    } else {
        Copy-Item -LiteralPath $dll -Destination (Join-Path $destination ($base+'.dll')) -Force
        if(-not (Test-Path -LiteralPath (Join-Path $destination 'surge.cfg'))){Copy-Item -LiteralPath (Join-Path $root 'surge.cfg.example') -Destination (Join-Path $destination 'surge.cfg.example') -Force}
    }
    $result=0
    @{status='PASS';classification='LOCAL_BUILD';profile=$Profile;configuration=$configuration;dll=$base+'.dll';sha256=(Get-FileHash -LiteralPath $dll -Algorithm SHA256).Hash.ToLowerInvariant();golden_tool_selftests=$goldenToolSelftests;golden=$goldenStatus;game_acceptance='NOT_RUN';exit_code=0} | ConvertTo-Json | Set-Content -LiteralPath $report -Encoding UTF8
    Write-Host "Golden pipeline: $goldenStatus"
    Write-Host "PASS $Profile LOCAL_BUILD; DLL: $destination\$base.dll"
} catch {
    if($result -eq 0){$result=1}
    @{status='FAIL';classification='LOCAL_BUILD';profile=$Profile;exit_code=$result;error=$_.Exception.Message} | ConvertTo-Json | Set-Content -LiteralPath $report -Encoding UTF8
    Write-Error -ErrorRecord $_ -ErrorAction Continue
} finally {
    if($null -ne $buildLock){$buildLock.Dispose()}
}
Write-Host "Report: $report"
exit $result
