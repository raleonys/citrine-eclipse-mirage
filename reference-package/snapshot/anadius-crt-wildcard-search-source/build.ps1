param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'search.cpp') -o (Join-Path $build 'search.o')
if($LASTEXITCODE -ne 0){throw 'wildcard search source compilation failed'}
$storage=Join-Path $PSScriptRoot '..\anadius-crt-wildcard-storage-source\storage.cpp'
& $compiler @flags -c $storage -o (Join-Path $build 'storage.o')
if($LASTEXITCODE -ne 0){throw 'wildcard storage dependency compilation failed'}
$executable=Join-Path $build 'search_tests.exe'
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'search_tests.cpp'),(Join-Path $build 'search.o'),(Join-Path $build 'storage.o'),'-o',$executable)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'wildcard search fixture compilation failed'}
$result=& $executable
if($LASTEXITCODE -ne 0){throw 'wildcard search fixtures failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;arguments=$arguments;tests=$result;scope='Opaque-address synthetic allocation, conversion, locale, find and sort providers; no target execution, filesystem enumeration, native API calls or original writes';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});storage_sources=@('storage.hpp','storage.cpp')|ForEach-Object{[ordered]@{name=$_;sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot "..\anadius-crt-wildcard-storage-source\$_")).Hash.ToLowerInvariant()}};executable_sha256=(Get-FileHash -LiteralPath $executable).Hash.ToLowerInvariant()}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
