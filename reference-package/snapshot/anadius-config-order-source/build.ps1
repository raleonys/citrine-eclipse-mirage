param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'order.cpp') -o (Join-Path $build 'order.o')
if($LASTEXITCODE -ne 0){throw 'Config order production compile failed'}
& $compiler @flags -static (Join-Path $PSScriptRoot 'order.cpp') (Join-Path $PSScriptRoot 'order_tests.cpp') (Join-Path $PSScriptRoot '../anadius-parser-source/parser.cpp') (Join-Path $PSScriptRoot '../anadius-config-source/configuration.cpp') -o (Join-Path $build 'order_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Config order tests compile failed'}
$result=& (Join-Path $build 'order_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Config order tests failed'}
$result|Write-Output
$result|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
[ordered]@{result='PASS';compiler=((& $compiler --version)-join "`n");flags=$flags;checks=[int]([regex]::Match(($result-join "`n"),'PASS: (\d+)').Groups[1].Value);tested_scope='synthetic parser bytes and bounded first Config2 copy; <=8 unique children per container';original_execution=$false;user_cfg_read=$false;full_function_bodies_recovered=0;later_section_copy_order='unresolved'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
