param(
    [Parameter(Mandatory=$true)][ValidateSet('Compile','Debug','Native','Crt','Golden')][string]$Mode,
    [string]$BuildDir = '',
    [ValidateSet('Release','RelWithDebInfo','Debug')][string]$Config = 'Release'
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'build_common.ps1')
Assert-DevelopmentBuildPreflight
$Root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
if ($Mode -eq 'Debug') { $Config = 'Debug' }
if ($Mode -eq 'Golden' -and $Config -ne 'Release') { Fail-Build 'Golden requires the Release profile.' 64 }
if ([string]::IsNullOrWhiteSpace($BuildDir)) { $BuildDir = 'build\' + $Mode.ToLowerInvariant() + '_msvc' }
$Out = Resolve-ConfinedBuildDirectory $Root $BuildDir 'BuildDir'
$EnvironmentBefore = @{}
Get-ChildItem Env: | ForEach-Object { $EnvironmentBefore[$_.Name] = $_.Value }
$Lock = $null
try {
    $Lock = Acquire-ExclusiveBuildLock (Join-Path $Out '.build.lock')
    $CMakeExe = Resolve-BoundExecutable 'cmake.exe'
    $CmdExe = Resolve-BoundExecutable 'cmd.exe'
    $vs = Find-VcVars64
    Import-VcVarsEnvironment $CmdExe $vs[1]
    Clear-BuildSelectorEnvironment
    $env:PYTHONDONTWRITEBYTECODE = '1'
    $PythonExe = Resolve-BoundExecutable 'python.exe'
    $ClExe = Resolve-BoundExecutable 'cl.exe'
    $LinkExe = Resolve-BoundExecutable 'link.exe'
    $NMakeExe = Resolve-BoundExecutable 'nmake.exe'
    Invoke-ExternalChecked $PythonExe @('-B',(Join-Path $Root 'scripts/clear_tool_python_cache.py'),'--root',$Root,'--quiet','--report',(Join-Path $Out 'PYTHON_CACHE_CLEANUP.json')) 'Previous first-party Python bytecode cleanup' 115
    New-Item -ItemType Directory -Path $Out -Force | Out-Null
    Write-ToolchainProvenance (Join-Path $Out 'BUILD_TOOLCHAIN_PROVENANCE.log') $vs[0] $vs[1] $CMakeExe $PythonExe $ClExe $LinkExe $NMakeExe
    $Testing = if ($Mode -in @('Native','Crt')) { 'ON' } else { 'OFF' }
    $GoldenLink = if ($Mode -eq 'Golden') { 'ON' } else { 'OFF' }
    $ProfileLabel = @{
        Compile = 'COMPILE ONLY'
        Debug = 'DEBUG BUILD'
        Native = 'NATIVE ACCEPTANCE'
        Crt = 'STATIC CRT ACCEPTANCE'
        Golden = 'PRODUCTION BUILD'
    }[$Mode]
    Write-Host "SURGE: $ProfileLabel / $Config"
    Write-Host '[1/3] Configure'
    Invoke-ExternalChecked $CMakeExe @('-S',$Root,'-B',$Out,'-G','NMake Makefiles',"-DCMAKE_BUILD_TYPE=$Config","-DBUILD_TESTING=$Testing","-DPRELOADER_L_BUILD_SOURCE_API_DLL=$Testing","-DPRELOADER_L_GOLDEN_LINK_PROFILE=$GoldenLink") 'CMake configure' 105
    Write-Host '[2/3] Compile and link'
    $BuildArgs = @('--build',$Out)
    if ($Mode -eq 'Crt') {
        $BuildArgs += @('--target','preloader_l_crt_host','preloader_l_source_host','preloader_l_runtime_host','dll_public_c_consumer')
    }
    Invoke-ExternalChecked $CMakeExe $BuildArgs 'MSVC build' 106
    $Dll = Join-Path $Out 'preloader.unsigned.dll'
    if (-not (Test-Path -LiteralPath $Dll -PathType Leaf)) { Fail-Build 'DLL missing after link.' 108 }
    $BeforeHash = (Get-FileHash -LiteralPath $Dll -Algorithm SHA256).Hash.ToLowerInvariant()
    $Receipt = @{classification='NOT_RELEASE'; build_succeeded=$true; mode=$Mode; configuration=$Config; dll_sha256=$BeforeHash; compiler=$ClExe; compiler_sha256=(Get-FileHash -LiteralPath $ClExe -Algorithm SHA256).Hash.ToLowerInvariant(); cmake_cache_sha256=(Get-FileHash -LiteralPath (Join-Path $Out 'CMakeCache.txt') -Algorithm SHA256).Hash.ToLowerInvariant(); source_manifest_sha256=(Get-FileHash -LiteralPath (Join-Path $Root 'metadata/ACTIVE_SOURCE_MANIFEST.json') -Algorithm SHA256).Hash.ToLowerInvariant(); real_game_acceptance='NOT_RUN'}
    $Receipt['validation_result'] = 'NOT_COMPLETED'
    $Receipt | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $Out 'LOCAL_BUILD_RECEIPT.json') -Encoding UTF8
    Write-Host '[3/3] Validate selected profile'
    $GoldenBlocked = $false
    $ReportedDll = $Dll
    $ReportedHash = $BeforeHash
    Invoke-ExternalChecked $PythonExe @('-B',(Join-Path $Root 'scripts/check_dll_exports.py'),$Dll,'--report',(Join-Path $Out 'PUBLIC_DLL_ABI_CHECK.json')) 'Application C export names and ordinals' 113
    Invoke-ExternalChecked $PythonExe @('-B',(Join-Path $Root 'tools/inspect_dll.py'),$Dll,'--report',(Join-Path $Out 'DLL_RESOURCE_CHECK.json')) 'Application resource contract' 114
    if ($Mode -in @('Native','Crt')) {
        $CTestExe = Resolve-BoundExecutable 'ctest.exe'
        $TestArgs = @('--test-dir',$Out,'--output-on-failure','--output-junit',(Join-Path $Out 'CTEST_RESULTS.xml'))
        if ($Mode -eq 'Crt') { $TestArgs += @('-R','^(crt_runtime_smoke|dll_source_smoke|dll_public_.*)$') }
        Invoke-ExternalChecked $CTestExe $TestArgs 'Windows source/CRT tests (not game acceptance)' 109
    } elseif ($Mode -eq 'Golden') {
        $GoldenArgs = @('-B',(Join-Path $Root 'scripts/run_golden_pipeline.py'),'--dll',$Dll,'--build-dir',$Out,'--output-dir',(Join-Path $Out 'golden-local'))
        & $PythonExe @GoldenArgs
        $GoldenCode = $LASTEXITCODE
        if ($GoldenCode -eq 3) { $GoldenBlocked = $true }
        elseif ($GoldenCode -ne 0) {
            if ($GoldenCode -lt 1 -or $GoldenCode -gt 255) { $GoldenCode = 110 }
            Fail-Build "Local Golden chain basarisiz (exit $GoldenCode)." $GoldenCode
        }
        else {
            $GoldenReport = Get-Content -LiteralPath (Join-Path $Out 'golden-local/GOLDEN_REPORT.json') -Raw | ConvertFrom-Json
            $ReportedDll = [System.IO.Path]::GetFullPath((Join-Path $Root 'dist\preloader_l.dll'))
            if ($GoldenReport.result -ne 'LOCAL_GOLDEN_PASS' -or [System.IO.Path]::GetFullPath($GoldenReport.dist_artifact) -ne $ReportedDll -or -not (Test-Path -LiteralPath $ReportedDll -PathType Leaf)) {
                Fail-Build 'Golden completed without the required local dist artifact.' 116
            }
            $ReportedHash = (Get-FileHash -LiteralPath $ReportedDll -Algorithm SHA256).Hash.ToLowerInvariant()
            if ($ReportedHash -ne $GoldenReport.sha256) { Fail-Build 'Local dist hash differs from the validated Golden DLL.' 117 }
            $Receipt['dist_artifact'] = $ReportedDll
            $Receipt['dist_sha256'] = $ReportedHash
            $Receipt['dist_sha256_file'] = $GoldenReport.dist_sha256_file
            $Receipt['dist_receipt'] = $GoldenReport.dist_receipt
        }
    } else {
        Invoke-ExternalChecked (Join-Path $Out 'preloader_l_runtime_host.exe') @($Dll) 'Application public API and real load/socket/shutdown/unload cycles' 111
    }
    if ((Get-FileHash -LiteralPath $Dll -Algorithm SHA256).Hash.ToLowerInvariant() -ne $BeforeHash) { Fail-Build 'Validation changed the build DLL.' 112 }
    $Receipt['validation_result'] = if ($GoldenBlocked) { 'BLOCKED_TARGET_SEMANTICS' } else { 'PASS_SELECTED_PROFILE' }
    $Receipt['validation_exit_code'] = if ($GoldenBlocked) { 3 } else { 0 }
    $Receipt | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $Out 'LOCAL_BUILD_RECEIPT.json') -Encoding UTF8
    if ($GoldenBlocked) {
        Write-Host 'LOCAL Golden: BLOCKED - product PE targets are incomplete; compilation and resource/export checks completed.' -ForegroundColor Yellow
        Write-Host "DLL: $Dll"
        Write-Host "SHA256: $BeforeHash"
        Write-Host 'Exit code 3. See golden-local/GOLDEN_REPORT.json. This is not Golden acceptance.'
        exit 3
    }
    Write-Host "$ProfileLabel`: PASS"
    Write-Host "DLL: $ReportedDll"
    Write-Host "SHA256: $ReportedHash"
} finally {
    if ($null -ne $Lock) { $Lock.Dispose() }
    Get-ChildItem Env: | ForEach-Object {
        if (-not $EnvironmentBefore.ContainsKey($_.Name)) { [Environment]::SetEnvironmentVariable($_.Name,$null,'Process') }
    }
    foreach ($Name in $EnvironmentBefore.Keys) { [Environment]::SetEnvironmentVariable($Name,$EnvironmentBefore[$Name],'Process') }
}
