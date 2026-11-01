param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$core=Join-Path $PSScriptRoot 'registry.cpp'
$test=Join-Path $PSScriptRoot 'registry_tests.cpp'
$object=Join-Path $build 'registry.o'
& $compiler @flags '-c' $core '-o' $object
if($LASTEXITCODE -ne 0){throw 'Registry core compilation failed'}
$exe=Join-Path $build 'registry_tests.exe'
& $compiler @flags '-static' $core $test '-o' $exe
if($LASTEXITCODE -ne 0){throw 'Registry checks compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Registry checks failed'}
$sourceHashes=@(Get-ChildItem -LiteralPath $PSScriptRoot -File | Where-Object {$_.Extension -in '.cpp','.hpp'} | ForEach-Object {
    [ordered]@{path=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLower()}
})
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');
  compiler=(& $compiler --version | Select-Object -First 1);tests=$result;
  production_objects=1;test_groups=1;
  executable_sha256=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLower();
  object_sha256=(Get-FileHash -LiteralPath $object -Algorithm SHA256).Hash.ToLower();
  sources=$sourceHashes;
  execution_scope='Synthetic in-memory mocks only; no real registry read/write, original target, rebuilt target DLL, game, provider or hook execution';
  runtime_equivalence='not established'} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
