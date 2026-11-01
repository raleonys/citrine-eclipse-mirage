param([string]$BuildDir = "build\preloader_l_debug_msvc")

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'build_common.ps1')
Assert-DevelopmentBuildPreflight
$Root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$Out = Resolve-ConfinedBuildDirectory $Root $BuildDir 'BuildDir'
$CMakeExe = Resolve-BoundExecutable 'cmake.exe'
$PythonExe = Resolve-BoundExecutable 'python.exe'
$CmdExe = Resolve-BoundExecutable 'cmd.exe'
$vs = Find-VcVars64; $vsPath=$vs[0]; $vcvars=$vs[1]
Import-VcVarsEnvironment $CmdExe $vcvars
Clear-BuildSelectorEnvironment
$ClExe=Resolve-BoundExecutable 'cl.exe'; $LinkExe=Resolve-BoundExecutable 'link.exe'; $NMakeExe=Resolve-BoundExecutable 'nmake.exe'
if(Test-Path -LiteralPath $Out){Remove-Item -LiteralPath $Out -Recurse -Force}
New-Item -ItemType Directory -Path $Out -Force|Out-Null
Write-ToolchainProvenance (Join-Path $Out 'BUILD_TOOLCHAIN_PROVENANCE.log') $vsPath $vcvars $CMakeExe $PythonExe $ClExe $LinkExe $NMakeExe

Write-Host '[1/2] Configure + build DEBUG prototype and debug hosts'
Invoke-ExternalChecked $CMakeExe @('-S',$Root,'-B',$Out,'-G','NMake Makefiles','-DCMAKE_BUILD_TYPE=Debug','-DPRELOADER_L_ENABLE_DEBUG_PROTOTYPE=ON','-DPRELOADER_L_NATIVE_ACCEPTANCE=OFF') 'Debug CMake configure' 105
Invoke-ExternalChecked $CMakeExe @('--build',$Out,'--target','preloader_l_debug','preloader_l_debug_host','preloader_l_runtime_host','preloader_l_session_host','preloader_l_socket_host') 'Debug build' 106
$DebugDll=Join-Path $Out 'preloader_l_debug.dll'
if(-not(Test-Path -LiteralPath $DebugDll)){$found=Get-ChildItem -Path $Out -Filter 'preloader_l_debug.dll' -Recurse -File|Select-Object -First 1;if($found){$DebugDll=$found.FullName}}
if(-not(Test-Path -LiteralPath $DebugDll)){Fail-Build 'preloader_l_debug.dll bulunamadi.' 108}
$hash=(Get-FileHash -LiteralPath $DebugDll -Algorithm SHA256).Hash.ToLowerInvariant()
Write-Host ''
Write-Host '[2/2] DEBUG BUILD READY'
Write-Host "DLL    : $DebugDll"
Write-Host "SHA256 : $hash"
Write-Host 'Trace diagnostics: ENABLED'
Write-Host 'Shipping Golden pipeline is not applied to the debug prototype.'
