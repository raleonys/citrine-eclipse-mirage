param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$outputs=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build';$null=New-Item -ItemType Directory -Force -Path $build
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'writer.cpp') -o (Join-Path $build 'writer.o')
if($LASTEXITCODE -ne 0){throw 'Achievement writer compilation failed'}
$sources=@('anadius-achievement-writer-source/writer.cpp','anadius-achievement-writer-source/writer_tests.cpp','anadius-path-source/paths.cpp','anadius-file-source/binary_io.cpp','anadius-file-source/native_io.cpp')|ForEach-Object {Join-Path $outputs $_}
& $compiler @flags -static @sources -o (Join-Path $build 'writer_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Achievement writer tests compilation failed'}
$result=& (Join-Path $build 'writer_tests.exe') (Join-Path $build 'generated-fixtures')
if($LASTEXITCODE -ne 0){throw 'Achievement writer tests failed'}
$result|Write-Output
$result|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
$hashes=[ordered]@{}
foreach($file in @('writer.hpp','writer.cpp','writer_tests.cpp','build.ps1','CMakeLists.txt','README.md')){$hashes[$file]=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $file)).Hash.ToLowerInvariant()}
[ordered]@{result='PASS';compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;output=$result -join "`n";checks=[int]([regex]::Match(($result-join "`n"),'PASS: (\d+)').Groups[1].Value);source_hashes=$hashes;original_target_execution='none';real_user_cfg_read=$false;native_scope='Own generated INI file via existing compatibility backend; mkdir and other effects controlled';runtime_equivalence='not established'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
