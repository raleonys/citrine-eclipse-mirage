param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$outputs=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Force -Path $build
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'pipeline.cpp') -o (Join-Path $build 'pipeline.o')
if($LASTEXITCODE -ne 0){throw 'Pipeline compilation failed'}
$sources=@('anadius-config-pipeline-source/pipeline.cpp','anadius-config-pipeline-source/pipeline_tests.cpp','anadius-config-order-source/order.cpp','anadius-parser-source/parser.cpp','anadius-config-source/configuration.cpp','anadius-file-source/binary_io.cpp','anadius-file-source/native_io.cpp')|ForEach-Object {Join-Path $outputs $_}
& $compiler @flags -static @sources -o (Join-Path $build 'pipeline_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Pipeline test compilation failed'}
$result=& (Join-Path $build 'pipeline_tests.exe') (Join-Path $build 'generated-fixtures')
if($LASTEXITCODE -ne 0){throw 'Pipeline tests failed'}
$result|Write-Output
$result|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
[ordered]@{result='PASS';compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;output=$result -join "`n";original_target_execution='none';real_user_cfg_read=$false;native_fixture_scope='build/generated-fixtures/config-pipeline-generated-fixture.cfg';scope='selected Config2 first-copy document; scalar-only application bridge';consumer_collection_order='unresolved; bridge rejects';runtime_equivalence='not established'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
