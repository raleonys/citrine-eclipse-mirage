param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'locale_ref_tests.exe'
$object=Join-Path $build 'locale_ref.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'locale_ref.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'Locale reference object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'locale_ref.cpp'),(Join-Path $PSScriptRoot 'locale_ref_tests.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Locale reference tests compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Locale reference tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Synthetic address/counter providers, exact effect ordering, 32-bit wrap and fault-domain checks. No target or native locale/atomic execution.';original_ABI_equality='unproven';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();test_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
