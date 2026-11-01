param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$outputs=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build';$null=New-Item -ItemType Directory -Force -Path $build
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'storage.cpp') -o (Join-Path $build 'storage.o')
if($LASTEXITCODE -ne 0){throw 'Storage compilation failed'}
$sources=@('anadius-achievement-storage-source/storage.cpp','anadius-achievement-storage-source/storage_tests.cpp','anadius-achievement-writer-source/writer.cpp','anadius-xml-reader-source/reader.cpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp','anadius-xml-source/memory_printer.cpp','anadius-path-source/paths.cpp','anadius-file-source/binary_io.cpp','anadius-file-source/native_io.cpp','anadius-config-startup-source/startup.cpp','anadius-config-application-source/application.cpp','anadius-config-hash-source/hash_table.cpp','anadius-parser-source/parser.cpp','anadius-config-source/configuration.cpp','anadius-startup-source/startup.cpp')|ForEach-Object {Join-Path $outputs $_}
& $compiler @flags -static @sources -o (Join-Path $build 'storage_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Storage tests compilation failed'}
$result=& (Join-Path $build 'storage_tests.exe') (Join-Path $build 'generated-fixtures')
if($LASTEXITCODE -ne 0){throw 'Storage tests failed'}
$result|Write-Output
$result|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
$hashes=[ordered]@{}
foreach($path in @('storage.hpp','storage.cpp','storage_tests.cpp','build.ps1','CMakeLists.txt')){$hashes[$path]=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $path)).Hash.ToLowerInvariant()}
[ordered]@{result='PASS';compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;checks=[int]([regex]::Match(($result-join "`n"),'PASS: (\d+)').Groups[1].Value);output=$result -join "`n";source_hashes=$hashes;original_execution='none';user_cfg_read=$false;native_scope='Only generated XML/INI fixtures; timestamp/status/directory/config OS providers controlled';runtime_equivalence='not established'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
