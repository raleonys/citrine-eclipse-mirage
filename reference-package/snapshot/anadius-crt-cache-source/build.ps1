param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'cache_tests.exe'
$object=Join-Path $build 'cache.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'cache.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'Cache object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'cache.cpp'),
    (Join-Path $PSScriptRoot '..\anadius-crt-runtime-source\runtime.cpp'),
    (Join-Path $PSScriptRoot '..\anadius-thread-source\threads.cpp'),
    (Join-Path $PSScriptRoot '..\anadius-source\bootstrap.cpp'),
    (Join-Path $PSScriptRoot 'cache_tests.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Cache compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Cache tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Synthetic cookie entropy, 31 procedure cache cells, 21 module unload slots and encoded globals; no native API, original DLL or game execution.';original_ABI_equality='unproven';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();test_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result

