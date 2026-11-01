param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$relativeSources=@('callbacks.cpp','callbacks_tests.cpp','..\anadius-registry-source\registry.cpp',
  '..\anadius-host-init-source\host_init.cpp','..\anadius-path-source\paths.cpp')
$sources=@($relativeSources | ForEach-Object {(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot $_)).Path})
$exe=Join-Path $build 'callbacks_tests.exe'
& $compiler @flags '-static' @sources '-o' $exe
if($LASTEXITCODE -ne 0){throw 'Host callback compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Host callback checks failed'}
$sourceHashes=@(Get-ChildItem -LiteralPath $PSScriptRoot -File | Where-Object {$_.Extension -in '.cpp','.hpp'} | ForEach-Object {
    [ordered]@{path=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLower()}
})
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');
  compiler=(& $compiler --version | Select-Object -First 1);tests=$result;
  executable_sha256=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLower();
  sources=$sourceHashes;dependency_sources=@(2..($relativeSources.Count-1) | ForEach-Object {
    [ordered]@{path=$relativeSources[$_];sha256=(Get-FileHash -LiteralPath $sources[$_] -Algorithm SHA256).Hash.ToLower()}
  });
  execution_scope='Synthetic in-memory mocks only; no native process launch, folder API, registry, original target or rebuilt target DLL executed';
  runtime_equivalence='not established'} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
