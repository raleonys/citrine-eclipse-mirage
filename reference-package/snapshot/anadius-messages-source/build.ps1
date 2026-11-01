param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& node (Join-Path $PSScriptRoot 'validate-registry.cjs')
if($LASTEXITCODE -ne 0){throw 'Retained registry comparison failed'}
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'messages.cpp') '-o' (Join-Path $build 'messages.o')
if($LASTEXITCODE -ne 0){throw 'Messages object compilation failed'}
& $compiler @flags '-static' (Join-Path $PSScriptRoot 'messages.cpp') (Join-Path $PSScriptRoot 'messages_tests.cpp') '-o' (Join-Path $build 'messages_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Messages mock compilation failed'}
$testOutput=& (Join-Path $build 'messages_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Messages mock tests failed'}
$testOutput | Set-Content -LiteralPath (Join-Path $build 'validation.txt') -Encoding UTF8
$files=Get-ChildItem -LiteralPath $PSScriptRoot -File | Where-Object Extension -in @('.cpp','.hpp','.ps1','.md','.json','.cjs') | Where-Object Name -ne 'validation.json'
$retained=@('anadius-all-functions.asm.txt','anadius-all-functions.pseudo.c') | ForEach-Object {
    [ordered]@{file="../$_";sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot "..\$_")).Hash.ToLowerInvariant()}
}
[ordered]@{checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;mock_tests=$testOutput;native_adapter_present=$false;original_target_execution='none';runtime_equivalence='unverified';retained_evidence_files=@($retained);sources=@($files | ForEach-Object {[ordered]@{file=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}})} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$testOutput
