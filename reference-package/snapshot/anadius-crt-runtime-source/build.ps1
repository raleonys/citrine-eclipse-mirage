param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'runtime_tests.exe'
$object=Join-Path $build 'runtime.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'runtime.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'Runtime object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'runtime.cpp'),
    (Join-Path $PSScriptRoot '..\anadius-thread-source\threads.cpp'),
    (Join-Path $PSScriptRoot '..\anadius-source\bootstrap.cpp'),
    (Join-Path $PSScriptRoot 'runtime_tests.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Runtime compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Runtime tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Synthetic loader/procedure/policy and recovered thread composition. No module loading, PEB access, original DLL or game execution.';original_ABI_equality='unproven';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();test_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
