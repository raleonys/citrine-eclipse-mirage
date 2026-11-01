param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$archive=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../source-workspace/build/anadius_core_partial.a')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'environment_cleanup.cpp') -o (Join-Path $build 'environment_cleanup.o')
if($LASTEXITCODE -ne 0){throw 'Environment cleanup object compilation failed'}
$exe=Join-Path $build 'environment_cleanup_tests.exe'
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'native_tests.cpp'),(Join-Path $build 'environment_cleanup.o'),$archive,'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Environment cleanup fixture compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Environment cleanup tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;arguments=$arguments;tests=$result;partial_archive_sha256=(Get-FileHash -LiteralPath $archive).Hash.ToLowerInvariant();source_hashes=[ordered]@{ 'environment_cleanup.cpp'=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'environment_cleanup.cpp')).Hash.ToLowerInvariant();'environment_cleanup.hpp'=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'environment_cleanup.hpp')).Hash.ToLowerInvariant();'native_tests.cpp'=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'native_tests.cpp')).Hash.ToLowerInvariant()};recovered_bodies=4;scope='Four cleanup normal-flow bodies;actual existing bounded free_environment core;owned live char-pointer vectors only;UTF16 payload cleanup views are not native wide ABI;no original execution or user environment access'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
