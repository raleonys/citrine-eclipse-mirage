param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build';$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$sources=@('encoding.cpp','native_encoding.cpp','encoding_tests.cpp') | ForEach-Object {Join-Path $PSScriptRoot $_}
$exe=Join-Path $build 'encoding_tests.exe'
& $compiler @flags '-static' @sources '-o' $exe
if($LASTEXITCODE -ne 0){throw 'Encoding compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Encoding checks failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);tests=$result;actual_native_scope='Win32 conversions on synthetic strings; linked CRT locale query';original_execution='none';original_CRT_EH_ABI_equality='unproven'} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
