param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'hash.cpp') '-o' (Join-Path $build 'hash.o')
if($LASTEXITCODE -ne 0){throw 'Hash object compilation failed'}
& $compiler @flags '-static' (Join-Path $PSScriptRoot 'hash.cpp') (Join-Path $PSScriptRoot 'hash_tests.cpp') '-o' (Join-Path $build 'hash_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Hash tests compilation failed'}
$result=& (Join-Path $build 'hash_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Hash deterministic checks failed'}
$result | Set-Content -LiteralPath (Join-Path $build 'validation.txt') -Encoding UTF8
[ordered]@{checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);flags=$flags;tests=$result;target_execution='none';runtime_equivalence='unverified';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File | Where-Object Name -ne 'validation.json' | ForEach-Object {[ordered]@{file=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}})} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
