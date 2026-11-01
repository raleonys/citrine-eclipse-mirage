param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$outputs=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build';$null=New-Item -ItemType Directory -Force -Path $build
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'application.cpp') -o (Join-Path $build 'application.o')
if($LASTEXITCODE -ne 0){throw 'Application compilation failed'}
$sources=@('anadius-config-application-source/application.cpp','anadius-config-application-source/application_tests.cpp','anadius-config-hash-source/hash_table.cpp','anadius-parser-source/parser.cpp','anadius-config-source/configuration.cpp','anadius-file-source/binary_io.cpp','anadius-file-source/native_io.cpp')|ForEach-Object {Join-Path $outputs $_}
& $compiler @flags -static @sources -o (Join-Path $build 'application_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Application tests compilation failed'}
$result=& (Join-Path $build 'application_tests.exe') (Join-Path $build 'generated-fixtures')
if($LASTEXITCODE -ne 0){throw 'Application tests failed'}
$result|Write-Output
$result|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
[ordered]@{result='PASS';compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;output=$result -join "`n";original_target_execution='none';real_user_cfg_read=$false;scope='Normal Config2 application with growth, positional reused sections/records and actual scalar helper sources';native_scope='Own generated fixture only; compatibility stream';runtime_equivalence='not established';native_allocation_exception_ABI='unproven'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
