param(
    [string]$BuildDir = "build\preloader_l_msvc",
    [ValidateSet("Release")]
    [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot 'build_common.ps1')
Assert-DevelopmentBuildPreflight

$Root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..\..")).Path
$Out = Resolve-ConfinedBuildDirectory $Root $BuildDir 'BuildDir'
$Dist = Join-Path $Root 'dist'
$BuildRoot = Join-Path $Root 'build'
$Lock = Acquire-ExclusiveBuildLock (Join-Path $BuildRoot '.preloader_l_production.lock')
$PublishTemp = $null

function Assert-CandidateIdentity {
    param(
        [Parameter(Mandatory=$true)][string]$Path,
        [Parameter(Mandatory=$true)][string]$ExpectedSha256,
        [Parameter(Mandatory=$true)][string]$Label,
        [Parameter(Mandatory=$true)][int]$ExitCode
    )
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        Fail-Build "Validated candidate missing at ${Label}: $Path" $ExitCode
    }
    $actual = (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actual -ne $ExpectedSha256) {
        Fail-Build "Validated candidate identity drift at $Label. expected=$ExpectedSha256 actual=$actual" $ExitCode
    }
    return $actual
}

try {
    $CMakeExe = Resolve-BoundExecutable 'cmake.exe'
    $PythonExe = Resolve-BoundExecutable 'python.exe'
    $CmdExe = Resolve-BoundExecutable 'cmd.exe'
    $vs = Find-VcVars64
    $vsPath = $vs[0]
    $vcvars = $vs[1]
    Import-VcVarsEnvironment $CmdExe $vcvars
    Clear-BuildSelectorEnvironment
    $ClExe = Resolve-BoundExecutable 'cl.exe'
    $LinkExe = Resolve-BoundExecutable 'link.exe'
    $NMakeExe = Resolve-BoundExecutable 'nmake.exe'

    if (Test-Path -LiteralPath $Out) { Remove-Item -LiteralPath $Out -Recurse -Force }
    New-Item -ItemType Directory -Path $Out -Force | Out-Null
    New-Item -ItemType Directory -Path $Dist -Force | Out-Null

    $GameDll = Join-Path $Dist 'preloader_l.dll'
    if (Test-Path -LiteralPath $GameDll) { Remove-Item -LiteralPath $GameDll -Force }

    $GoldenLogs = @(
        '01_URSYNEX_GOLDEN_FINALIZER.log',
        '02_ARYRA_GOLDEN_LAYOUT.log',
        '03_MUSTAVEX_GOLDEN_METADATA.log',
        '04_CORVEXON_GOLDEN_TRAIL.log',
        '05_FALCION_GOLDEN_CLEANING.log',
        'BUILD_TOOLCHAIN_PROVENANCE.log'
    )
    foreach ($name in $GoldenLogs) {
        $p = Join-Path $Dist $name
        if (Test-Path -LiteralPath $p) { Remove-Item -LiteralPath $p -Force }
    }
    Write-ToolchainProvenance (Join-Path $Dist 'BUILD_TOOLCHAIN_PROVENANCE.log') $vsPath $vcvars $CMakeExe $PythonExe $ClExe $LinkExe $NMakeExe

    Write-Host "MSVC installation: $vsPath"
    Write-Host "vcvars64: $vcvars"
    Write-Host "Bound CMake: $CMakeExe"
    Write-Host "Bound Python: $PythonExe"
    Write-Host "Bound CL: $ClExe"
    Write-Host "Bound LINK: $LinkExe"
    Write-Host "Bound NMAKE: $NMakeExe"

    Write-Host ""
    Write-Host "[1/6] MSVC Build"
    $configureArgs = @(
        '-S',$Root,'-B',$Out,'-G','NMake Makefiles',
        "-DCMAKE_BUILD_TYPE=$Config",
        '-DPRELOADER_L_ENABLE_DEBUG_PROTOTYPE=OFF',
        '-DPRELOADER_L_NATIVE_ACCEPTANCE=OFF',
        '-DCMAKE_SHARED_LINKER_FLAGS_RELEASE:STRING=/INCREMENTAL:NO /OPT:REF /OPT:ICF'
    )
    Invoke-ExternalChecked $CMakeExe $configureArgs 'CMake configure' 105
    Invoke-ExternalChecked $CMakeExe @('--build',$Out,'--target','preloader_l') 'CMake production build' 106
    Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/check_msvc_shipping_link_command.py'),$Out) 'Shipping link-policy audit' 107

    $candidates = @(
        (Join-Path $Out 'preloader.unsigned.dll'),
        (Join-Path $Out "$Config\preloader.unsigned.dll")
    )
    $Dll = $null
    foreach ($c in $candidates) { if (Test-Path -LiteralPath $c) { $Dll = $c; break } }
    if (-not $Dll) {
        $found = Get-ChildItem -Path $Out -Filter 'preloader.unsigned.dll' -Recurse -File | Select-Object -First 1
        if ($found) { $Dll = $found.FullName }
    }
    if (-not $Dll) { Fail-Build 'Build tamamlandi ama preloader.unsigned.dll bulunamadi.' 108 }

    Write-Host ""
    Write-Host "[2/6] URSYNEX Golden Finalizer"
    $UrsynexLog = Join-Path $Dist '01_URSYNEX_GOLDEN_FINALIZER.log'
    $UrsynexBefore = (Get-FileHash -LiteralPath $Dll -Algorithm SHA256).Hash.ToLowerInvariant()
    @('POLICY=URSYNEX Golden Finalizer',"TARGET=$Dll",'MODE=BOUNDED_MUTATING_POST_LINK_FINALIZER',"SHA256_BEFORE=$UrsynexBefore") | Set-Content -LiteralPath $UrsynexLog -Encoding UTF8
    & $PythonExe (Join-Path $Root 'scripts/ursynex_golden_finalizer.py') $Dll 2>&1 | Tee-Object -FilePath $UrsynexLog -Append
    $rc = $LASTEXITCODE
    $UrsynexAfter = (Get-FileHash -LiteralPath $Dll -Algorithm SHA256).Hash.ToLowerInvariant()
    Add-Content -LiteralPath $UrsynexLog -Value "SHA256_AFTER=$UrsynexAfter"
    if ($rc -ne 0) { Add-Content -LiteralPath $UrsynexLog -Value 'RESULT=FAIL'; Fail-Build "URSYNEX Golden Finalizer failed. See $UrsynexLog" $rc }
    Add-Content -LiteralPath $UrsynexLog -Value 'RESULT=PASS'
    $ValidatedCandidateHash = $UrsynexAfter
    Assert-CandidateIdentity $Dll $ValidatedCandidateHash 'post-URSYNEX validated candidate' 112 | Out-Null

    Write-Host ""
    Write-Host "[3/6] ARYRA Golden Layout"
    $AryraLog = Join-Path $Dist '02_ARYRA_GOLDEN_LAYOUT.log'
    $AryraHash = Assert-CandidateIdentity $Dll $ValidatedCandidateHash 'ARYRA precondition' 113
    @('POLICY=ARYRA Golden Layout',"TARGET=$Dll",'MODE=READ_ONLY_FAIL_CLOSED_POST_LINK',"SHA256=$AryraHash") | Set-Content -LiteralPath $AryraLog -Encoding UTF8
    & $PythonExe (Join-Path $Root 'scripts/verify_aryra_golden_layout.py') $Dll 2>&1 | Tee-Object -FilePath $AryraLog -Append
    $rc = $LASTEXITCODE
    Add-Content -LiteralPath $AryraLog -Value ('RESULT=' + $(if ($rc -eq 0) { 'PASS' } else { 'FAIL' }))
    if ($rc -ne 0) { Fail-Build "ARYRA Golden Layout failed. See $AryraLog" $rc }
    Assert-CandidateIdentity $Dll $ValidatedCandidateHash 'ARYRA postcondition' 114 | Out-Null

    Write-Host ""
    Write-Host "[4/6] MUSTAVEX Golden Metadata"
    $MustavexLog = Join-Path $Dist '03_MUSTAVEX_GOLDEN_METADATA.log'
    $MustavexHash = Assert-CandidateIdentity $Dll $ValidatedCandidateHash 'MUSTAVEX precondition' 115
    @('POLICY=MUSTAVEX Golden Metadata',"TARGET=$Dll",'MODE=READ_ONLY_FAIL_CLOSED_POST_FINALIZATION',"SHA256=$MustavexHash") | Set-Content -LiteralPath $MustavexLog -Encoding UTF8
    & $PythonExe (Join-Path $Root 'scripts/verify_mustavex_golden_metadata.py') $Dll 2>&1 | Tee-Object -FilePath $MustavexLog -Append
    $rc = $LASTEXITCODE
    Add-Content -LiteralPath $MustavexLog -Value ('RESULT=' + $(if ($rc -eq 0) { 'PASS' } else { 'FAIL' }))
    if ($rc -ne 0) { Fail-Build "MUSTAVEX Golden Metadata failed. See $MustavexLog" $rc }
    Assert-CandidateIdentity $Dll $ValidatedCandidateHash 'MUSTAVEX postcondition' 116 | Out-Null

    Write-Host ""
    Write-Host "[5/6] CORVEXON Golden Trail"
    $CorvexonLog = Join-Path $Dist '04_CORVEXON_GOLDEN_TRAIL.log'
    $CorvexonHash = Assert-CandidateIdentity $Dll $ValidatedCandidateHash 'CORVEXON precondition' 117
    @('POLICY=CORVEXON Golden Trail',"TARGET=$Dll",'MODE=READ_ONLY_FAIL_CLOSED_FINAL_BINARY_ACCEPTANCE',"SHA256=$CorvexonHash") | Set-Content -LiteralPath $CorvexonLog -Encoding UTF8
    & $PythonExe (Join-Path $Root 'scripts/verify_corvexon_golden_trail.py') $Dll --build-dir $Out 2>&1 | Tee-Object -FilePath $CorvexonLog -Append
    $rc = $LASTEXITCODE
    Add-Content -LiteralPath $CorvexonLog -Value ('RESULT=' + $(if ($rc -eq 0) { 'PASS' } else { 'FAIL' }))
    if ($rc -ne 0) { Fail-Build "CORVEXON Golden Trail failed. See $CorvexonLog" $rc }
    Assert-CandidateIdentity $Dll $ValidatedCandidateHash 'CORVEXON postcondition' 118 | Out-Null

    Write-Host ""
    Write-Host "[6/6] FALCION Golden Cleaning"
    $FalcionLog = Join-Path $Dist '05_FALCION_GOLDEN_CLEANING.log'
    Assert-CandidateIdentity $Dll $ValidatedCandidateHash 'FALCION precondition' 119 | Out-Null
    @('POLICY=FALCION Golden Cleaning',"TARGET=$Root",'MODE=FAIL_CLOSED_RELEASE_SOURCE_CLEANING',"VALIDATED_CANDIDATE_SHA256=$ValidatedCandidateHash",'SHA256=ACTIVE_SOURCE_MANIFEST') | Set-Content -LiteralPath $FalcionLog -Encoding UTF8
    & $PythonExe (Join-Path $Root 'scripts/check_falcion_golden_cleaning.py') 2>&1 | Tee-Object -FilePath $FalcionLog -Append
    $rc = $LASTEXITCODE
    Add-Content -LiteralPath $FalcionLog -Value ('SOURCE_CLEANING_RESULT=' + $(if ($rc -eq 0) { 'PASS' } else { 'FAIL' }))
    if ($rc -ne 0) { Add-Content -LiteralPath $FalcionLog -Value 'RESULT=FAIL'; Fail-Build "FALCION Golden Cleaning failed. See $FalcionLog" $rc }
    Add-Content -LiteralPath $FalcionLog -Value ''
    Add-Content -LiteralPath $FalcionLog -Value '[DATA_VIRTUAL_FOOTPRINT]'
    & $PythonExe (Join-Path $Root 'scripts/check_data_footprint.py') $Dll 2>&1 | Tee-Object -FilePath $FalcionLog -Append
    $rc = $LASTEXITCODE
    Add-Content -LiteralPath $FalcionLog -Value ('DATA_VIRTUAL_FOOTPRINT_RESULT=' + $(if ($rc -eq 0) { 'PASS' } else { 'FAIL' }))
    Add-Content -LiteralPath $FalcionLog -Value ('RESULT=' + $(if ($rc -eq 0) { 'PASS' } else { 'FAIL' }))
    if ($rc -ne 0) { Fail-Build "FALCION data-footprint gate failed. See $FalcionLog" $rc }
    Assert-CandidateIdentity $Dll $ValidatedCandidateHash 'FALCION postcondition' 120 | Out-Null

    # Same-directory staged publication. The authoritative path never observes a partial copy.
    $SourceHash = Assert-CandidateIdentity $Dll $ValidatedCandidateHash 'publication source' 121
    $PublishTemp = Join-Path $Dist ('.preloader_l.publish.' + [guid]::NewGuid().ToString('N') + '.tmp')
    Copy-Item -LiteralPath $Dll -Destination $PublishTemp
    $StagedHash = (Get-FileHash -LiteralPath $PublishTemp -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($StagedHash -ne $SourceHash) { Fail-Build 'Staged publication hash mismatch.' 109 }
    if (Test-Path -LiteralPath $GameDll) { Fail-Build 'Authoritative DLL path was repopulated before atomic publication.' 110 }
    [System.IO.File]::Move($PublishTemp,$GameDll)
    $PublishTemp = $null
    $hash = (Get-FileHash -LiteralPath $GameDll -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($hash -ne $SourceHash) { Fail-Build 'Published DLL hash mismatch after atomic rename.' 111 }
    if ($hash -ne $ValidatedCandidateHash) { Fail-Build 'Published DLL is not the validated Golden candidate identity.' 122 }

    Write-Host ""
    Write-Host 'LOCAL GOLDEN BUILD COMPLETE - NOT OFFICIAL LIBRARY PUBLICATION'
    Write-Host 'MSVC Build -> URSYNEX Golden Finalizer -> ARYRA Golden Layout -> MUSTAVEX Golden Metadata -> CORVEXON Golden Trail -> FALCION Golden Cleaning'
    Write-Host "DLL    : $GameDll"
    Write-Host "SHA256 : $hash"
    Write-Host 'Run .\scripts\build_orchestration\build_native_acceptance.cmd for native readiness. Shipping exports: DllMain @1, SURGE @2, DllShutdown @3.'
    Write-Host 'Call DllShutdown outside loader lock before final FreeLibrary; only return code 0 authorizes unload.'
} finally {
    if ($PublishTemp -and (Test-Path -LiteralPath $PublishTemp)) { Remove-Item -LiteralPath $PublishTemp -Force -ErrorAction SilentlyContinue }
    if ($Lock) { $Lock.Dispose() }
}
