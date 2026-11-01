param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'hash_table.cpp') -o (Join-Path $build 'hash_table.o')
if($LASTEXITCODE -ne 0){throw 'Hash table production compilation failed'}
& $compiler @flags -static (Join-Path $PSScriptRoot 'hash_table.cpp') (Join-Path $PSScriptRoot 'hash_table_tests.cpp') (Join-Path $PSScriptRoot '../anadius-config-order-source/order.cpp') (Join-Path $PSScriptRoot '../anadius-parser-source/parser.cpp') (Join-Path $PSScriptRoot '../anadius-config-source/configuration.cpp') -o (Join-Path $build 'hash_table_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Hash table test compilation failed'}
$output=& (Join-Path $build 'hash_table_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Hash table tests failed'}
$output|Write-Output
$output|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
[ordered]@{result='PASS';compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;checks=[int]([regex]::Match(($output-join "`n"),'PASS: (\d+)').Groups[1].Value);output=$output-join "`n";original_execution='none';user_cfg_read=$false;full_original_bodies_added=0;scope='Factor1 logical hash/list growth, inherited clone, destination-retained assignment, recursive positional reuse; synthetic inputs only';ABI_equivalence='not established'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
