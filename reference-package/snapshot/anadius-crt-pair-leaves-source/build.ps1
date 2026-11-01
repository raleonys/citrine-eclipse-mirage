param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$archive=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../source-workspace/build/anadius_core_partial.a')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'pair_leaves.cpp') -o (Join-Path $build 'pair_leaves.o')
if($LASTEXITCODE -ne 0){throw 'Pair leaf compilation failed'}
$exe=Join-Path $build 'pair_leaf_tests.exe'
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'native_tests.cpp'),(Join-Path $build 'pair_leaves.o'),$archive,'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Pair leaf test compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Pair leaf tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;arguments=$arguments;tests=$result;partial_archive_sha256=(Get-FileHash -LiteralPath $archive).Hash.ToLowerInvariant();source_hashes=[ordered]@{'pair_leaves.cpp'=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'pair_leaves.cpp')).Hash.ToLowerInvariant();'pair_leaves.hpp'=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'pair_leaves.hpp')).Hash.ToLowerInvariant();'native_tests.cpp'=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'native_tests.cpp')).Hash.ToLowerInvariant()};recovered_bodies=4;scope='Four ACRT leaf callbacks normal flow/AL;shared heap/VCRT/argv states;actual existing VCRT and argv algorithms;injected bounded owned command buffers;no user commandline or target execution'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
