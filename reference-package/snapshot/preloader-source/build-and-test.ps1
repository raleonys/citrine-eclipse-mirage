param(
    [string]$ToolchainRoot = (Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64')
)
$ErrorActionPreference = 'Stop'
$compiler = Join-Path $ToolchainRoot 'bin\clang++.exe'
if (-not (Test-Path -LiteralPath $compiler)) { throw "Compiler not found: $compiler" }
$core = Join-Path $PSScriptRoot 'preloader_core.cpp'
$tests = Join-Path $PSScriptRoot 'mock_tests.cpp'
$object = Join-Path $PSScriptRoot 'preloader_core.o'
$testExecutable = Join-Path $PSScriptRoot 'preloader_mock_tests.exe'
& $compiler -std=c++20 -O2 -Wall -Wextra -Werror -static -c $core -o $object
if ($LASTEXITCODE -ne 0) { throw "Core compilation failed: $LASTEXITCODE" }
& $compiler -std=c++20 -O2 -Wall -Wextra -Werror -static $core $tests -o $testExecutable
if ($LASTEXITCODE -ne 0) { throw "Mock test compilation failed: $LASTEXITCODE" }
# This executable contains our reconstructed source and mocks only.
# It never loads either supplied DLL or the extracted preloader.
& $testExecutable
if ($LASTEXITCODE -ne 0) { throw "Mock tests failed: $LASTEXITCODE" }
