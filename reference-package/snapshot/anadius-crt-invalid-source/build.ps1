param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'invalid_tests.exe'
$object=Join-Path $build 'invalid.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'invalid.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'Invalid parameter object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'invalid.cpp'),(Join-Path $PSScriptRoot 'invalid_tests.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Invalid parameter tests compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Invalid parameter tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Synthetic unaligned PTD storage, fake handlers, Watson/missing-PTD sentinel exceptions. No original DLL, game, native handler or terminal action.';original_ABI_equality='unproven';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();test_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
