param(
    [string]$BuildDir = "build\preloader_l_native_acceptance_msvc",
    [ValidateSet("Release","RelWithDebInfo","Debug")][string]$Config = "Release"
)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'build_common.ps1')
Assert-DevelopmentBuildPreflight
$Root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$Out=Resolve-ConfinedBuildDirectory $Root $BuildDir 'BuildDir'
$Dist=Join-Path $Root 'dist\native_acceptance'
$CMakeExe=Resolve-BoundExecutable 'cmake.exe'; $PythonExe=Resolve-BoundExecutable 'python.exe'; $CmdExe=Resolve-BoundExecutable 'cmd.exe'
$vs=Find-VcVars64; $vsPath=$vs[0]; $vcvars=$vs[1]
Import-VcVarsEnvironment $CmdExe $vcvars
Clear-BuildSelectorEnvironment
$ClExe=Resolve-BoundExecutable 'cl.exe'; $LinkExe=Resolve-BoundExecutable 'link.exe'; $NMakeExe=Resolve-BoundExecutable 'nmake.exe'
if(Test-Path -LiteralPath $Out){Remove-Item -LiteralPath $Out -Recurse -Force}
if(Test-Path -LiteralPath $Dist){Remove-Item -LiteralPath $Dist -Recurse -Force}
New-Item -ItemType Directory -Path $Out -Force|Out-Null
New-Item -ItemType Directory -Path $Dist -Force|Out-Null
Write-ToolchainProvenance (Join-Path $Dist 'BUILD_TOOLCHAIN_PROVENANCE.log') $vsPath $vcvars $CMakeExe $PythonExe $ClExe $LinkExe $NMakeExe

Invoke-ExternalChecked $CMakeExe @('-S',$Root,'-B',$Out,'-G','NMake Makefiles',"-DCMAKE_BUILD_TYPE=$Config",'-DPRELOADER_L_ENABLE_DEBUG_PROTOTYPE=OFF','-DPRELOADER_L_NATIVE_ACCEPTANCE=ON') 'Native acceptance CMake configure' 105
Invoke-ExternalChecked $CMakeExe @('--build',$Out,'--target','preloader_l','preloader_l_native_acceptance_host') 'Native acceptance build' 106
$BuiltNativeDll=Join-Path $Out 'preloader_l_native_test.dll'; $BuiltHostExe=Join-Path $Out 'preloader_l_native_acceptance_host.exe'
if(-not(Test-Path -LiteralPath $BuiltNativeDll)){Fail-Build 'preloader_l_native_test.dll bulunamadi' 108}
if(-not(Test-Path -LiteralPath $BuiltHostExe)){Fail-Build 'acceptance host bulunamadi' 109}
# Finalize one fresh acceptance artifact set; validation, hashing and execution uses these canonical paths only.
$NativeDll=Join-Path $Dist 'preloader_l_native_test.dll'; $NativeHost=Join-Path $Dist 'preloader_l_native_acceptance_host.exe'
Copy-Item -LiteralPath $BuiltNativeDll -Destination $NativeDll
Copy-Item -LiteralPath $BuiltHostExe -Destination $NativeHost
$allowed=@('preloader_l_native_test.dll','preloader_l_native_acceptance_host.exe','BUILD_TOOLCHAIN_PROVENANCE.log')
$unexpected=@(Get-ChildItem $Dist -File|Where-Object{$allowed -notcontains $_.Name})
if($unexpected.Count-ne 0){Fail-Build ('unexpected stale acceptance artifact(s): '+(($unexpected|ForEach-Object Name)-join ', ')) 110}
if(Test-Path -LiteralPath (Join-Path $Dist 'preloader_l.dll')){Fail-Build 'native acceptance directory contains stale production preloader_l.dll' 111}
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/check_data_footprint.py'),$NativeDll) '.data footprint audit' 112
Invoke-ExternalChecked $PythonExe @((Join-Path $Root 'scripts/check_native_exports.py'),$NativeDll) 'native acceptance export surface' 113

$preHash=(Get-FileHash -LiteralPath $NativeDll -Algorithm SHA256).Hash.ToLowerInvariant(); $hostHash=(Get-FileHash -LiteralPath $NativeHost -Algorithm SHA256).Hash.ToLowerInvariant()
$scenarios=@(
 'reload_generation','shipping_shutdown','zero','one','growth','parallel','active_shutdown','blocked_shutdown','f02_delivery_stall_parallel','ids07_send_idle_timeout','connect_redirect_lifecycle','wave4_resource_retry',
 'path_ascii_sso7','path_unicode_heap','path_long_replace','xml_no_bom','xml_bom','xml_large','xml_invalid_retain','xml_replace','xml_clear',
 'prestart_stop','ptd_fls_lifecycle',
 'frame_fragmentation','frame_coalescing','frame_malformed','protocol_route_surface','store_content_entitlement','achievement_event_delivery'
)
foreach($scenario in $scenarios){
 $env:PRELOADER_L_ACCEPTANCE_SCENARIO=$scenario
 Push-Location $Dist
 & $NativeHost
 $hostRc=$LASTEXITCODE
 Pop-Location
 if($hostRc-ne 0){Remove-Item Env:PRELOADER_L_ACCEPTANCE_SCENARIO -ErrorAction SilentlyContinue; Fail-Build "native acceptance scenario '$scenario' failed ($hostRc)" $hostRc}
}
Remove-Item Env:PRELOADER_L_ACCEPTANCE_SCENARIO -ErrorAction SilentlyContinue
foreach($stressScenario in @('active_shutdown','blocked_shutdown')){for($round=1;$round -le 4;$round++){$env:PRELOADER_L_ACCEPTANCE_SCENARIO=$stressScenario;Push-Location $Dist;& $NativeHost;$hostRc=$LASTEXITCODE;Pop-Location;if($hostRc-ne 0){Remove-Item Env:PRELOADER_L_ACCEPTANCE_SCENARIO -ErrorAction SilentlyContinue;Fail-Build "native shutdown stress '$stressScenario' round $round failed ($hostRc)" $hostRc}}}
Remove-Item Env:PRELOADER_L_ACCEPTANCE_SCENARIO -ErrorAction SilentlyContinue
$postHash=(Get-FileHash -LiteralPath $NativeDll -Algorithm SHA256).Hash.ToLowerInvariant(); if($preHash-ne$postHash){Fail-Build 'native-test DLL changed during acceptance run' 114}
Write-Host 'NATIVE ACCEPTANCE: PASS'
Write-Host "DLL: $NativeDll"; Write-Host "DLL_SHA256_BEFORE: $preHash"; Write-Host "DLL_SHA256_AFTER : $postHash"; Write-Host "HOST_SHA256      : $hostHash"
Write-Host 'Canonical native harness coverage: lifecycle/reload, worker/socket, config/path/XML, hook/IAT/quiescence/PTD-FLS, framing, route/store/achievement-event, cleanup fault injection.'
Write-Host 'Manual game/network acceptance remains a separate evidence class.'
