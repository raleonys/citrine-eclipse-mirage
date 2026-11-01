param(
    [string]$BuildDir = "build\preloader_l_compile_msvc",
    [ValidateSet('Release','RelWithDebInfo','Debug')][string]$Config = 'Release'
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'build_common.ps1')
Assert-DevelopmentBuildPreflight
$Root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$Out = Resolve-ConfinedBuildDirectory $Root $BuildDir 'BuildDir'
$CMakeExe = Resolve-BoundExecutable 'cmake.exe'
$CmdExe = Resolve-BoundExecutable 'cmd.exe'
$vs = Find-VcVars64
$vsPath = $vs[0]
$vcvars = $vs[1]
Import-VcVarsEnvironment $CmdExe $vcvars
Clear-BuildSelectorEnvironment
$ClExe = Resolve-BoundExecutable 'cl.exe'
$LinkExe = Resolve-BoundExecutable 'link.exe'
$NMakeExe = Resolve-BoundExecutable 'nmake.exe'
New-Item -ItemType Directory -Path $Out -Force | Out-Null
Write-Host 'MODE: COMPILE_ONLY (CMake/MSVC; no Golden, no G0, no Library publication)'
Invoke-ExternalChecked $CMakeExe @('-S',$Root,'-B',$Out,'-G','NMake Makefiles',"-DCMAKE_BUILD_TYPE=$Config",'-DPRELOADER_L_ENABLE_DEBUG_PROTOTYPE=OFF','-DPRELOADER_L_NATIVE_ACCEPTANCE=OFF','-DCMAKE_SHARED_LINKER_FLAGS_RELEASE:STRING=/INCREMENTAL:NO /OPT:REF /OPT:ICF') 'CMake configure' 105
Invoke-ExternalChecked $CMakeExe @('--build',$Out,'--target','preloader_l') 'MSVC compile/link' 106
$Dll = Join-Path $Out 'preloader.unsigned.dll'
if (-not (Test-Path -LiteralPath $Dll -PathType Leaf)) { Fail-Build 'Unsigned build artifact missing' 108 }
$Hash = (Get-FileHash -LiteralPath $Dll -Algorithm SHA256).Hash.ToLowerInvariant()
Write-Host 'COMPILE_ONLY: PASS'
Write-Host "LOCAL_ARTIFACT: $Dll"
Write-Host "LOCAL_SHA256: $Hash"
Write-Host 'NOT_RELEASE: Unfinalized DLL. No shipping/Golden/native/game/QA/publication acceptance is claimed.'
