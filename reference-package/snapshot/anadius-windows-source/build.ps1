param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$outputs=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build';$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$sources=@('anadius-windows-source/windows_effects.cpp','anadius-windows-source/windows_tests.cpp','anadius-status-source/status.cpp','anadius-status-source/native_status.cpp','anadius-path-source/paths.cpp','anadius-encoding-source/encoding.cpp','anadius-encoding-source/native_encoding.cpp','anadius-file-source/binary_io.cpp','anadius-file-source/native_io.cpp','anadius-file-source/license_files.cpp','anadius-hash-source/hash.cpp','anadius-token-source/token.cpp','anadius-crypto-source/crypto.cpp','anadius-license-source/license.cpp','anadius-xml-source/memory_printer.cpp','anadius-xml-reader-source/reader.cpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp') | ForEach-Object {Join-Path $outputs $_}
$exe=Join-Path $build 'windows_tests.exe'
& $compiler @flags '-static' @sources '-o' $exe
if($LASTEXITCODE -ne 0){throw 'Windows effects compilation failed'}
$result=& $exe (Join-Path $build 'windows-fixtures')
if($LASTEXITCODE -ne 0){throw 'Windows effects checks failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');tests=$result;actual_native_scope='generated windows-fixtures only; Win32 status and conversions on generated paths/strings';original_targets='not loaded/executed/modified';runtime_equivalence='not established'} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
