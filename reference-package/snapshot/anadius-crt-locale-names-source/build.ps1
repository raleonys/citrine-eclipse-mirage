param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'locale_names_tests.exe'
$object=Join-Path $build 'locale_names.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'locale_names.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'Locale names object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'locale_names.cpp'),(Join-Path $PSScriptRoot 'locale_names_tests.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Locale names tests compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Locale names tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Owned memory; all 228 names and mixed case, all proper prefixes and invalid suffixes, 226 reachable names and two original misses, complete 65536 UTF16 code unit domain versus nine comparison endpoints, read order/count/NUL and address wrapping. No target execution or Windows locale API calls.';original_ABI_equality='unproven; logical typed interface and immutable shipped tables';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();test_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
