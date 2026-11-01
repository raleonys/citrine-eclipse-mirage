param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'misc_handlers.cpp') '-o' (Join-Path $build 'misc_handlers.o')
if($LASTEXITCODE -ne 0){throw 'Compilation failed'}
& $compiler @flags '-static' (Join-Path $PSScriptRoot 'misc_handlers.cpp') (Join-Path $PSScriptRoot 'misc_handlers_tests.cpp') '-o' (Join-Path $build 'misc_handlers_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Mock test compilation failed'}
$output=& (Join-Path $build 'misc_handlers_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Mock tests failed'}
$output | Set-Content -LiteralPath (Join-Path $build 'validation.txt') -Encoding UTF8
$files=Get-ChildItem -LiteralPath $PSScriptRoot -File | Where-Object Name -ne 'validation.json'
[ordered]@{checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;tests=$output;original_execution='none';runtime_equivalence='unverified';files=@($files | ForEach-Object {[ordered]@{file=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}})} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$output
