param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$outputs=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build';$null=New-Item -ItemType Directory -Force -Path $build
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'timestamp.cpp') -o (Join-Path $build 'timestamp.o')
if($LASTEXITCODE -ne 0){throw 'Timestamp compilation failed'}
$sources=@('anadius-timestamp-source/timestamp.cpp','anadius-timestamp-source/timestamp_tests.cpp','anadius-calendar-source/calendar.cpp','anadius-achievement-storage-source/storage.cpp','anadius-achievement-writer-source/writer.cpp','anadius-xml-reader-source/reader.cpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp','anadius-xml-source/memory_printer.cpp','anadius-path-source/paths.cpp','anadius-file-source/binary_io.cpp','anadius-file-source/native_io.cpp')|ForEach-Object {Join-Path $outputs $_}
& $compiler @flags -static @sources -o (Join-Path $build 'timestamp_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Timestamp tests compilation failed'}
$result=& (Join-Path $build 'timestamp_tests.exe') (Join-Path $build 'generated-fixtures')
if($LASTEXITCODE -ne 0){throw 'Timestamp checks failed'}
$result|Write-Output
$result|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
$hashes=[ordered]@{};foreach($path in @('timestamp.hpp','timestamp.cpp','timestamp_tests.cpp','build.ps1','CMakeLists.txt')){$hashes[$path]=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $path)).Hash.ToLowerInvariant()}
[ordered]@{result='PASS';compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;checks=[int]([regex]::Match(($result-join "`n"),'PASS: (\d+)').Groups[1].Value);output=$result-join "`n";source_hashes=$hashes;original_execution='none';real_user_files_read=$false;native_scope='Own generated XML and INI files only; status/mkdir effects controlled';parser='Recovered classic ASCII fixed ISO branch; HostGetTime separately explicit compatibility; arbitrary locale and native STL ABI unproven';runtime_equivalence='not established'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
