param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'argv_bridge.cpp') -o (Join-Path $build 'argv_bridge.o')
if($LASTEXITCODE -ne 0){throw 'Startup bridge compilation failed'}
$units=@('argv_bridge_tests.cpp','../anadius-crt-startup-bridge-source/startup_bridge.cpp','../anadius-crt-argv-source/argv.cpp','../anadius-crt-argv-parser-source/parse.cpp','../anadius-crt-byteclass-source/byteclass.cpp','../anadius-crt-support-source/support.cpp','../anadius-crt-environment-source/environment.cpp','../anadius-crt-termination-source/termination.cpp','../anadius-crt-onexit-source/onexit.cpp','../anadius-crt-cache-source/cache.cpp','../anadius-vcrt-bridge-source/bridge.cpp','../anadius-crt-cpu-source/cpu.cpp','../anadius-vcrt-ptd-source/vcrt_ptd.cpp','../anadius-vcrt-api-source/api.cpp','../anadius-crt-lock-source/locks.cpp','../anadius-crt-entry-source/entry.cpp','../anadius-crt-initializers-source/initializers.cpp','../anadius-crt-ptd-source/ptd.cpp','../anadius-crt-ptd-data-source/ptd_data.cpp','../anadius-crt-heap-source/heap.cpp','../anadius-crt-glue-source/crt_glue.cpp','../anadius-crt-error-source/errors.cpp','../anadius-crt-invalid-source/invalid.cpp','../anadius-crt-runtime-source/runtime.cpp','../anadius-thread-native-source/win32_effects.cpp','../anadius-thread-source/threads.cpp','../anadius-source/bootstrap.cpp')
$exe=Join-Path $build 'argv_bridge_tests.exe'
$arguments=$flags+@('-static')+@($units|ForEach-Object{Join-Path $PSScriptRoot $_})+@((Join-Path $build 'argv_bridge.o'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Startup bridge fixture compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Startup bridge fixtures failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;arguments=$arguments;tests=$result;scope='Actual recovered argv/parser/classification/heap/error/invalid/Entry/VCRT/ACRT/PTD/environment/cexit/onexit/support cores; synthetic providers; no target/native command-line execution';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});shared_fixture_sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot '../anadius-crt-startup-bridge-source/test_fixtures.hpp')).Hash.ToLowerInvariant();executable_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
