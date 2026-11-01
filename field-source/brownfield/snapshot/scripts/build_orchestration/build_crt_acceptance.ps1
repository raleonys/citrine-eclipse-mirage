param(
    [string]$CrtBuildDir = "build\preloader_l_crt_shipping_msvc",
    [string]$NativeBuildDir = "build\preloader_l_crt_native_msvc",
    [ValidateSet("Release","RelWithDebInfo","Debug")][string]$Config = "Release"
)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'build_common.ps1')
Assert-DevelopmentBuildPreflight
$Root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$CrtOut=Resolve-ConfinedBuildDirectory $Root $CrtBuildDir 'CrtBuildDir'
$NatOut=Resolve-ConfinedBuildDirectory $Root $NativeBuildDir 'NativeBuildDir'
$Dist=Join-Path $Root 'dist\crt_acceptance'
$CMakeExe=Resolve-BoundExecutable 'cmake.exe'; $PythonExe=Resolve-BoundExecutable 'python.exe'; $CmdExe=Resolve-BoundExecutable 'cmd.exe'
$vs=Find-VcVars64; $vsPath=$vs[0]; $vcvars=$vs[1]
Import-VcVarsEnvironment $CmdExe $vcvars
Clear-BuildSelectorEnvironment
$ClExe=Resolve-BoundExecutable 'cl.exe'; $LinkExe=Resolve-BoundExecutable 'link.exe'; $NMakeExe=Resolve-BoundExecutable 'nmake.exe'
foreach($p in @($CrtOut,$NatOut,$Dist)){if(Test-Path -LiteralPath $p){Remove-Item -LiteralPath $p -Recurse -Force};New-Item -ItemType Directory -Path $p -Force|Out-Null}
Write-ToolchainProvenance (Join-Path $Dist 'BUILD_TOOLCHAIN_PROVENANCE.log') $vsPath $vcvars $CMakeExe $PythonExe $ClExe $LinkExe $NMakeExe

Write-Host '[0/6] CRT source/runtime policy'
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/check_crt_transition.py'),'--source') 'static CRT source/runtime policy' 120

function Invoke-CMakeBuild {
 param([string]$Out,[string[]]$Extra,[string[]]$Targets)
 $args=@('-S',$Root,'-B',$Out,'-G','NMake Makefiles',"-DCMAKE_BUILD_TYPE=$Config",'-DPRELOADER_L_ENABLE_DEBUG_PROTOTYPE=OFF','-DPRELOADER_L_COMPATIBILITY_CUSTOM_ENTRY=OFF','-DCMAKE_SHARED_LINKER_FLAGS_RELEASE:STRING=/INCREMENTAL:NO /OPT:REF /OPT:ICF')+$Extra
 Invoke-ExternalChecked $CMakeExe $args "CRT configure $Out" 105
 Invoke-ExternalChecked $CMakeExe (@('--build',$Out,'--target')+$Targets) "CRT build $Out" 106
}

Write-Host '[1/6] Static /MT + /GS CRT shipping build'
Invoke-CMakeBuild $CrtOut @('-DPRELOADER_L_NATIVE_ACCEPTANCE=OFF') @('preloader_l','preloader_l_release_host')
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/check_msvc_shipping_link_command.py'),$CrtOut) 'CRT shipping link policy' 126
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/check_crt_transition.py'),'--build-dir',$CrtOut,'--expect-runtime','crt') 'CRT generated runtime flags' 127
$CrtDll=Join-Path $CrtOut 'preloader.unsigned.dll';if(-not(Test-Path -LiteralPath $CrtDll)){Fail-Build 'CRT shipping DLL missing' 128}
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/ursynex_golden_finalizer.py'),$CrtDll) 'CRT URSYNEX' 129
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/verify_aryra_golden_layout.py'),$CrtDll) 'CRT ARYRA' 130
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/verify_mustavex_golden_metadata.py'),$CrtDll) 'CRT MUSTAVEX' 131
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/verify_corvexon_golden_trail.py'),$CrtDll,'--build-dir',$CrtOut) 'CRT CORVEXON' 132
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/check_crt_transition.py'),'--binary',$CrtDll) 'static CRT import verification' 139

Write-Host '[2/6] SHIPPING_BLACKBOX public lifecycle + GetConfig'
$ReleaseHost=Join-Path $CrtOut 'preloader_l_release_host.exe';if(-not(Test-Path -LiteralPath $ReleaseHost)){Fail-Build 'release black-box host missing' 133}
$BlackboxDir=Join-Path $CrtOut 'shipping_blackbox';if(Test-Path -LiteralPath $BlackboxDir){Remove-Item -LiteralPath $BlackboxDir -Recurse -Force};New-Item -ItemType Directory -Path $BlackboxDir -Force|Out-Null
Copy-Item -LiteralPath $CrtDll -Destination (Join-Path $BlackboxDir 'preloader_l.dll');Copy-Item -LiteralPath $ReleaseHost -Destination (Join-Path $BlackboxDir 'preloader_l_release_host.exe')
Push-Location $BlackboxDir;& (Join-Path $BlackboxDir 'preloader_l_release_host.exe');$blackboxRc=$LASTEXITCODE;Pop-Location
if($blackboxRc-ne 0){Fail-Build "shipping black-box lifecycle failed ($blackboxRc)" $blackboxRc}

Write-Host '[3/6] Static CRT native-acceptance build'
Invoke-CMakeBuild $NatOut @('-DPRELOADER_L_NATIVE_ACCEPTANCE=ON') @('preloader_l','preloader_l_native_acceptance_host')
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/check_msvc_shipping_link_command.py'),$NatOut) 'native CRT link policy' 135
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/check_crt_transition.py'),'--build-dir',$NatOut,'--expect-runtime','crt') 'native CRT generated flags' 136
$NatDll=Join-Path $NatOut 'preloader_l_native_test.dll';$AcceptanceHost=Join-Path $NatOut 'preloader_l_native_acceptance_host.exe';if(-not(Test-Path -LiteralPath $NatDll)-or-not(Test-Path -LiteralPath $AcceptanceHost)){Fail-Build 'native CRT artifacts missing' 137}
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/check_native_exports.py'),$NatDll) 'native CRT export surface' 138

Write-Host '[4/6] Native CRT scenarios'
$scenarios=@(
 'reload_generation','shipping_shutdown','zero','one','growth','parallel','active_shutdown','blocked_shutdown','f02_delivery_stall_parallel','ids07_send_idle_timeout','connect_redirect_lifecycle','wave4_resource_retry',
 'path_ascii_sso7','path_unicode_heap','path_long_replace','xml_no_bom','xml_bom','xml_large','xml_invalid_retain','xml_replace','xml_clear',
 'prestart_stop','ptd_fls_lifecycle',
 'frame_fragmentation','frame_coalescing','frame_malformed','protocol_route_surface','store_content_entitlement','achievement_event_delivery'
)
foreach($scenario in $scenarios){$env:PRELOADER_L_ACCEPTANCE_SCENARIO=$scenario;Push-Location $NatOut;& $AcceptanceHost;$rc=$LASTEXITCODE;Pop-Location;if($rc-ne 0){Remove-Item Env:PRELOADER_L_ACCEPTANCE_SCENARIO -ErrorAction SilentlyContinue;Fail-Build "CRT native scenario '$scenario' failed ($rc)" $rc}}
Remove-Item Env:PRELOADER_L_ACCEPTANCE_SCENARIO -ErrorAction SilentlyContinue
foreach($stress in @('active_shutdown','blocked_shutdown')){for($i=1;$i -le 4;$i++){$env:PRELOADER_L_ACCEPTANCE_SCENARIO=$stress;Push-Location $NatOut;& $AcceptanceHost;$rc=$LASTEXITCODE;Pop-Location;if($rc-ne 0){Remove-Item Env:PRELOADER_L_ACCEPTANCE_SCENARIO -ErrorAction SilentlyContinue;Fail-Build "CRT shutdown stress $stress round $i failed ($rc)" $rc}}}
Remove-Item Env:PRELOADER_L_ACCEPTANCE_SCENARIO -ErrorAction SilentlyContinue

Write-Host '[5/6] Static CRT evidence and hash'
Copy-Item -LiteralPath $CrtDll -Destination (Join-Path $Dist 'preloader_l_static_crt_shipping.dll')
$ch=(Get-FileHash -LiteralPath $CrtDll -Algorithm SHA256).Hash.ToLowerInvariant()
Write-Host '[6/6] Static CRT acceptance closure'
Write-Host 'CRT STATIC ACCEPTANCE: PASS'
Write-Host "static_crt_shipping_sha256: $ch"
Write-Host 'Static shipping is /MT + /GS + default _DllMainCRTStartup; do not infer game/Golden release promotion.'
