param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$archive=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../source-workspace/build/anadius_core_partial.a')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'bindings.cpp') -o (Join-Path $build 'bindings.o')
if($LASTEXITCODE -ne 0){throw 'Pair bindings compilation failed'}
$exe=Join-Path $build 'bindings_tests.exe'
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'bindings_tests.cpp'),(Join-Path $build 'bindings.o'),$archive,'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Pair bindings test compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Pair bindings fixtures failed'}
& node (Join-Path $PSScriptRoot 'collect-evidence.cjs')
if($LASTEXITCODE -ne 0){throw 'Pair bindings compiled table comparison failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;arguments=$arguments;tests=$result;partial_archive_sha256=(Get-FileHash -LiteralPath $archive).Hash.ToLowerInvariant();sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});recovered_bodies=0;scope='New composition:16 mutable fixed-size opaque-ID ACRT pairs invoking existing recovered sources;provider boundaries and non-pair policies remain mandatory;not canonical native globals or original ABI';original_target_execution='none'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
