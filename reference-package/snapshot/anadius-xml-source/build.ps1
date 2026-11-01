param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'memory_printer.cpp') '-o' (Join-Path $build 'memory_printer.o')
if($LASTEXITCODE -ne 0){throw 'XML printer compilation failed'}
& $compiler @flags '-static' (Join-Path $PSScriptRoot 'memory_printer.cpp') (Join-Path $PSScriptRoot 'printer_tests.cpp') (Join-Path $PSScriptRoot '..\anadius-messages-source\messages.cpp') (Join-Path $PSScriptRoot '..\anadius-handlers-source\handlers.cpp') '-o' (Join-Path $build 'printer_tests.exe')
if($LASTEXITCODE -ne 0){throw 'XML printer tests compilation failed'}
$output=& (Join-Path $build 'printer_tests.exe')
if($LASTEXITCODE -ne 0){throw 'XML printer tests failed'}
$output | Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
$files=Get-ChildItem -LiteralPath $PSScriptRoot -File | Where-Object Name -ne 'validation.json'
[ordered]@{compiler=(& $compiler --version | Select-Object -First 1);flags=$flags;result='PASS';tests=$output;original_execution='none';real_serializer=$true;reader_backend='synthetic';full_xml_parser=$false;runtime_equivalence='unverified';files=@($files | ForEach-Object {[ordered]@{file=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}})} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$output
