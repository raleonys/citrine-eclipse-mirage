param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'argv.cpp') -o (Join-Path $build 'argv.o')
if($LASTEXITCODE -ne 0){throw 'argv source compilation failed'}
$parser=Join-Path $PSScriptRoot '..\anadius-crt-argv-parser-source\parse.cpp'
& $compiler @flags -c $parser -o (Join-Path $build 'parse.o')
if($LASTEXITCODE -ne 0){throw 'argv parser dependency compilation failed'}
$executable=Join-Path $build 'argv_tests.exe'
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'argv_tests.cpp'),(Join-Path $build 'argv.o'),(Join-Path $build 'parse.o'),'-o',$executable)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'argv fixture compilation failed'}
$result=& $executable
if($LASTEXITCODE -ne 0){throw 'argv fixtures failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;arguments=$arguments;tests=$result;scope='Synthetic owned raw allocations, module filename/DBCS/wildcard providers only; no target/native process execution';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});parser_sources=@('parse.hpp','parse.cpp')|ForEach-Object{[ordered]@{name=$_;sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot "..\anadius-crt-argv-parser-source\$_")).Hash.ToLowerInvariant()}};executable_sha256=(Get-FileHash -LiteralPath $executable).Hash.ToLowerInvariant()}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
