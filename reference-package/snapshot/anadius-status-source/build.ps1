param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'status_tests.exe'
& $compiler '-std=c++20' '-O2' '-Wall' '-Wextra' '-Werror' '-static' (Join-Path $PSScriptRoot 'status.cpp') (Join-Path $PSScriptRoot 'native_status.cpp') (Join-Path $PSScriptRoot 'status_tests.cpp') '-o' $exe
if($LASTEXITCODE -ne 0){throw 'Status source compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Status source tests failed'}
[ordered]@{checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);result='PASS';output=$result;executable_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();native_io='generated status-fixtures directory beside test executable only';original_targets='not loaded, executed or modified'} | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
