param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$outputs=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build';$null=New-Item -ItemType Directory -Force -Path $build
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$handlerSources=@('anadius-session-source/handler.cpp','anadius-messages-source/messages.cpp','anadius-xml-reader-source/reader.cpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp','anadius-xml-source/memory_printer.cpp')|ForEach-Object{Join-Path $outputs $_}
& $compiler @flags -static @handlerSources (Join-Path $PSScriptRoot 'handler_tests.cpp') -o (Join-Path $build 'handler_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Handler tests compilation failed'}
$handlerOutput=& (Join-Path $build 'handler_tests.exe');if($LASTEXITCODE -ne 0){throw 'Handler tests failed'}
$sources=@('anadius-session-source/handler.cpp','anadius-session-source/routing.cpp','anadius-server-source/server.cpp','anadius-execution-source/execution.cpp','anadius-messages-source/messages.cpp','anadius-crypto-source/crypto.cpp','anadius-dispatch-source/router.cpp','anadius-handlers-source/handlers.cpp','anadius-event-queue-source/queue.cpp','anadius-event-queue-source/adapters.cpp','anadius-achievement-runtime-source/runtime.cpp','anadius-achievement-runtime-source/utc.cpp','anadius-calendar-source/calendar.cpp','anadius-achievement-writer-source/writer.cpp','anadius-content-handlers-source/content_handlers.cpp','anadius-misc-handlers-source/misc_handlers.cpp','anadius-xml-reader-source/reader.cpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp','anadius-xml-source/memory_printer.cpp','anadius-path-source/paths.cpp','anadius-file-source/binary_io.cpp')|ForEach-Object{Join-Path $outputs $_}
& $compiler @flags -static @sources (Join-Path $PSScriptRoot 'session_tests.cpp') -o (Join-Path $build 'session_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Session integration tests compilation failed'}
$sessionOutput=& (Join-Path $build 'session_tests.exe');if($LASTEXITCODE -ne 0){throw 'Session integration tests failed'}
$output=@($handlerOutput)+@($sessionOutput);$output|Write-Output
$output|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
$hashes=[ordered]@{};foreach($path in @('handler.hpp','handler.cpp','routing.hpp','routing.cpp','handler_tests.cpp','session_tests.cpp','build.ps1','CMakeLists.txt')){$hashes[$path]=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $path)).Hash.ToLowerInvariant()}
[ordered]@{result='PASS';compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;handler_checks=[int]([regex]::Match(($handlerOutput-join "`n"),'PASS: (\d+)').Groups[1].Value);integration_checks=[int]([regex]::Match(($sessionOutput-join "`n"),'PASS: (\d+)').Groups[1].Value);output=$output-join "`n";source_hashes=$hashes;original_execution='none';real_user_files_read=$false;execution_scope='Actual Session/messages/XML/AES/Router/selected handlers/shared event queue with memory transport and queued fixture scheduling; no socket or file IO';native_ASIO_lifetime_CRT_ABI_concurrency='not established';new_original_procedure_bodies=0}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
