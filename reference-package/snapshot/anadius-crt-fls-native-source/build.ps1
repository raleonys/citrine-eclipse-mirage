param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$commands=[System.Collections.Generic.List[object]]::new()
function Run([string[]]$arguments){
    $commands.Add([ordered]@{program=$compiler;arguments=$arguments})
    $output=& $compiler @arguments 2>&1
    if($LASTEXITCODE -ne 0){throw ($output -join "`n")}
}
Run ($flags+@('-c',(Join-Path $PSScriptRoot 'fls.cpp'),'-o',(Join-Path $build 'fls.o')))
$sources=@('native_tests.cpp','../anadius-crt-glue-source/crt_glue.cpp','../anadius-crt-heap-native-source/native_api.cpp','../anadius-crt-heap-native-bridge-source/bridge.cpp','../anadius-crt-ptd-source/ptd.cpp','../anadius-crt-ptd-data-source/ptd_data.cpp','../anadius-crt-heap-source/heap.cpp','../anadius-crt-error-source/errors.cpp','../anadius-crt-invalid-source/invalid.cpp','../anadius-crt-runtime-source/runtime.cpp','../anadius-thread-native-source/win32_effects.cpp','../anadius-thread-source/threads.cpp','../anadius-source/bootstrap.cpp')
$executable=Join-Path $build 'native_tests.exe'
Run ($flags+@('-static')+@($sources|ForEach-Object{Join-Path $PSScriptRoot $_})+@((Join-Path $build 'fls.o'),'-o',$executable))
$result=& $executable 2>&1
if($LASTEXITCODE -ne 0){throw ($result -join "`n")}
$terminalCases=foreach($mode in @('--open-binding','--callback-sentinel','--callback-throw')){
    $commands.Add([ordered]@{program=$executable;arguments=@($mode);expected_exit=88;scope='own test fatal boundary'})
    & $executable $mode
    if($LASTEXITCODE -ne 88){throw ('FLS terminal boundary failed: '+$mode)}
    [ordered]@{mode=$mode;exit_code=$LASTEXITCODE;result='PASS'}
}
[ordered]@{result='PASS';terminal_boundary_cases=@($terminalCases);checked_at=(Get-Date).ToUniversalTime().ToString('o');commands=$commands;output=$result -join "`n";target_execution='none';scope='Real Win32 FLS/heap and own joined std::thread; recovered PTD/data/glue/heap/error source chain; injected allocation/slot/publish/release failures, deterministic reentry; controlled locale refs/locks/atomic/globals/abort; no original target execution or complete DLL/ABI/SEH equivalence';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});executable_sha256=(Get-FileHash -LiteralPath $executable).Hash.ToLowerInvariant()}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
