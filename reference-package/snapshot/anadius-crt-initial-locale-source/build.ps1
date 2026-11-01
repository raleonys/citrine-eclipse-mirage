param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'initial_locale_data_tests.exe'
$object=Join-Path $build 'initial_locale_data.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'initial_locale_data.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'Initial locale data object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'initial_locale_data.cpp'),(Join-Path $PSScriptRoot 'initial_locale_data_tests.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Initial locale data test compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Initial locale data tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);arguments=$arguments;tests=$result;scope='Generated C++ data graph only; synthetic numeric addresses; no original DLL execution';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();test_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
