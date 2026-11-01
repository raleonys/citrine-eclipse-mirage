param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'locale_update.cpp') -o (Join-Path $build 'locale_update.o')
if($LASTEXITCODE -ne 0){throw 'Locale update source compilation failed'}
$executable=Join-Path $build 'locale_update_tests.exe'
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'locale_update_tests.cpp'),(Join-Path $build 'locale_update.o'),'-o',$executable)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Locale update fixture compilation failed'}
$result=& $executable
if($LASTEXITCODE -ne 0){throw 'Locale update fixtures failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;arguments=$arguments;tests=$result;scope='Synthetic live locale/context/PTD tokens only; no target/native locale calls';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});executable_sha256=(Get-FileHash -LiteralPath $executable).Hash.ToLowerInvariant()}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
