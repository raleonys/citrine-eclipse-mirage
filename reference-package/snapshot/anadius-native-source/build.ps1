param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'native_tests.exe'
$sources=@('native_callbacks.cpp','original_effects.cpp','native_tests.cpp','..\anadius-registry-source\registry.cpp','..\anadius-host-callbacks-source\callbacks.cpp','..\anadius-network-hook-source\connect.cpp','..\anadius-execution-source\execution.cpp','..\anadius-host-init-source\host_init.cpp','..\anadius-path-source\paths.cpp','..\hook-engine-source\adapters.cpp') | ForEach-Object {Join-Path $PSScriptRoot $_}
$arguments=@('-std=c++20','-O2','-Wall','-Wextra','-Werror','-static')+$sources+@('-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Native callback compilation failed'}
$output=& $exe
if($LASTEXITCODE -ne 0){throw 'Native callback tests failed'}
$boundary=@(foreach($mode in @('--closed','--missing','--throw-original')) {
    & $exe $mode
    $code=$LASTEXITCODE
    if($code -ne 86){throw ("Unexpected terminal policy exit: "+$mode+' '+$code)}
    [ordered]@{mode=$mode;expected=86;actual=$code;result='PASS'}
})
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$output;terminal_cases=$boundary;scope='SDK function-pointer calls to own C++ synthetic originals; actual GetLastError/SetLastError TLS metadata; one test worker; no hooks/target DLL/process/socket/registry APIs';executable_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();original_ABI_runtime_equality='unproven'} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$output
'PASS: three required terminal exception policies verified in synthetic subprocesses'
