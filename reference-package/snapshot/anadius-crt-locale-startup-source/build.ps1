param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$archive=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../source-workspace/build/anadius_core_partial.a')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'locale_startup.cpp') -o (Join-Path $build 'locale_startup.o')
if($LASTEXITCODE -ne 0){throw 'Locale startup object compilation failed'}
$exe=Join-Path $build 'locale_startup_tests.exe'
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'native_tests.cpp'),(Join-Path $build 'locale_startup.o'),$archive,'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Locale startup fixture compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Locale startup fixtures failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;arguments=$arguments;tests=$result;partial_archive_sha256=(Get-FileHash -LiteralPath $archive).Hash.ToLowerInvariant();sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});recovered_bodies=3;scope='Recovered locale direct initialization/reset normal flow;owned image data,actual locale/PTD/FLS/heap sources,4ownworkers;native fixture CS provider,exceptiontablefixture;full CRT initializer sequence/MB startup/ABI/SEH equivalence excluded';original_target_execution='none'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
