param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'termination_tests.exe'
$object=Join-Path $build 'termination.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'termination.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'Termination object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'termination.cpp'),(Join-Path $PSScriptRoot '..\anadius-crt-onexit-source\onexit.cpp'),(Join-Path $PSScriptRoot 'termination_tests.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Termination tests compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Termination tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Synthetic opaque storage, image bytes, recursive locks, callbacks and terminal exception providers only. No actual module, native process termination, original DLL or target EXE execution.';original_ABI_equality='unproven';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();test_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
