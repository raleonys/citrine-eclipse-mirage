param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$outputs=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build';$null=New-Item -ItemType Directory -Force -Path $build
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$utcSources=@('anadius-achievement-runtime-source/utc.cpp','anadius-calendar-source/calendar.cpp')|ForEach-Object{Join-Path $outputs $_}
& $compiler @flags -static @utcSources (Join-Path $PSScriptRoot 'utc_tests.cpp') -o (Join-Path $build 'utc_tests.exe')
if($LASTEXITCODE -ne 0){throw 'UTC test compilation failed'}
$utcOutput=& (Join-Path $build 'utc_tests.exe')
if($LASTEXITCODE -ne 0){throw 'UTC checks failed'}
$runtimeSources=@('anadius-achievement-runtime-source/runtime.cpp','anadius-achievement-runtime-source/utc.cpp','anadius-calendar-source/calendar.cpp','anadius-achievement-storage-source/storage.cpp','anadius-achievement-writer-source/writer.cpp','anadius-timestamp-source/timestamp.cpp','anadius-xml-reader-source/reader.cpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp','anadius-xml-source/memory_printer.cpp','anadius-content-handlers-source/content_handlers.cpp','anadius-path-source/paths.cpp','anadius-file-source/binary_io.cpp','anadius-file-source/native_io.cpp')|ForEach-Object{Join-Path $outputs $_}
& $compiler @flags -static @runtimeSources (Join-Path $PSScriptRoot 'runtime_tests.cpp') -o (Join-Path $build 'runtime_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Runtime integration test compilation failed'}
$runtimeOutput=& (Join-Path $build 'runtime_tests.exe') (Join-Path $build 'generated-fixtures')
if($LASTEXITCODE -ne 0){throw 'Runtime integration checks failed'}
$output=@($utcOutput)+@($runtimeOutput);$output|Write-Output
$output|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
$hashes=[ordered]@{};foreach($path in @('utc.hpp','utc.cpp','runtime.hpp','runtime.cpp','utc_tests.cpp','runtime_tests.cpp','build.ps1','CMakeLists.txt')){$hashes[$path]=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $path)).Hash.ToLowerInvariant()}
[ordered]@{result='PASS';compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;utc_checks=[int]([regex]::Match(($utcOutput-join "`n"),'PASS: (\d+)').Groups[1].Value);runtime_checks=[int]([regex]::Match(($runtimeOutput-join "`n"),'PASS: (\d+)').Groups[1].Value);output=$output-join "`n";source_hashes=$hashes;original_execution='none';real_user_files_read=$false;native_scope='Own generated XML and INI only; clock/locks/events and non-achievement providers controlled';host_clock_formatter='Explicit selected-runtime compatibility, not original CRT/locale/STL ABI equality';runtime_equivalence='not established'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
