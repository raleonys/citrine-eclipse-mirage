param([string]$ToolchainRoot = (Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference = 'Stop'
$compiler = Join-Path $ToolchainRoot 'bin\x86_64-w64-mingw32-clang++.exe'
if (!(Test-Path -LiteralPath $compiler)) { throw "Compiler not found: $compiler" }
$build = Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force -Path $build | Out-Null
& $compiler -std=c++20 -Wall -Wextra -Werror -O2 -c (Join-Path $PSScriptRoot 'bootstrap.cpp') -o (Join-Path $build 'bootstrap.o')
if ($LASTEXITCODE -ne 0) { throw 'Module compilation failed' }
& $compiler -std=c++20 -Wall -Wextra -Werror -O2 -static (Join-Path $PSScriptRoot 'bootstrap.cpp') (Join-Path $PSScriptRoot 'bootstrap_tests.cpp') -o (Join-Path $build 'bootstrap_contract_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Contract test compilation failed' }
& (Join-Path $build 'bootstrap_contract_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Contract tests failed' }
