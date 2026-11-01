param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$outputs=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build';$null=New-Item -ItemType Directory -Force -Path $build
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -static (Join-Path $PSScriptRoot 'queue.cpp') (Join-Path $PSScriptRoot 'queue_tests.cpp') -o (Join-Path $build 'queue_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Queue tests compilation failed'}
$queueOutput=& (Join-Path $build 'queue_tests.exe');if($LASTEXITCODE -ne 0){throw 'Queue checks failed'}
$sources=@('anadius-event-queue-source/queue.cpp','anadius-event-queue-source/adapters.cpp','anadius-achievement-runtime-source/runtime.cpp','anadius-achievement-runtime-source/utc.cpp','anadius-calendar-source/calendar.cpp','anadius-achievement-writer-source/writer.cpp','anadius-content-handlers-source/content_handlers.cpp','anadius-misc-handlers-source/misc_handlers.cpp','anadius-messages-source/messages.cpp','anadius-xml-reader-source/reader.cpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp','anadius-xml-source/memory_printer.cpp','anadius-path-source/paths.cpp','anadius-file-source/binary_io.cpp')|ForEach-Object{Join-Path $outputs $_}
& $compiler @flags -static @sources (Join-Path $PSScriptRoot 'integration_tests.cpp') -o (Join-Path $build 'integration_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Queue integration tests compilation failed'}
$integrationOutput=& (Join-Path $build 'integration_tests.exe');if($LASTEXITCODE -ne 0){throw 'Queue integration checks failed'}
$output=@($queueOutput)+@($integrationOutput);$output|Write-Output
$output|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
$hashes=[ordered]@{};foreach($path in @('queue.hpp','queue.cpp','adapters.hpp','adapters.cpp','queue_tests.cpp','integration_tests.cpp','build.ps1','CMakeLists.txt')){$hashes[$path]=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $path)).Hash.ToLowerInvariant()}
[ordered]@{result='PASS';compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;queue_checks=[int]([regex]::Match(($queueOutput-join "`n"),'PASS: (\d+)').Groups[1].Value);integration_checks=[int]([regex]::Match(($integrationOutput-join "`n"),'PASS: (\d+)').Groups[1].Value);output=$output-join "`n";source_hashes=$hashes;original_execution='none';real_user_files_read=$false;execution_scope='Synthetic guards/mutex/cipher and actual recovered IGO/grant/messages/XML source composition; no file IO';native_deque_CRT_ABI_concurrency='not established'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
