param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$archive=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../source-workspace/build/anadius_core_partial.a')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'lowio_lifecycle.cpp') -o (Join-Path $build 'lowio_lifecycle.o')
if($LASTEXITCODE -ne 0){throw 'Lowio lifecycle compilation failed'}
$exe=Join-Path $build 'lowio_lifecycle_tests.exe'
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'native_tests.cpp'),(Join-Path $build 'lowio_lifecycle.o'),$archive,'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Lowio lifecycle fixture compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Lowio lifecycle fixtures failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;arguments=$arguments;tests=$result;partial_archive_sha256=(Get-FileHash -LiteralPath $archive).Hash.ToLowerInvariant();sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});recovered_bodies=3;scope='3recovered lowio lifecycle bodies;mandatory lower ensure/inherited/standard providers;128lazy slots,64CSstride48,3ownedheap arrays192actualCSdeletes,capacitypreserved,recursivefixturelock7;nofilehandle APIs or complete lowio/CRT/ABI/SEH equivalence';original_target_execution='none'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
