param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$outputRoot=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$sources=@('anadius-dispatch-source/integration_tests.cpp','anadius-dispatch-source/router.cpp','anadius-xml-source/memory_printer.cpp','anadius-xml-reader-source/reader.cpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp','anadius-crypto-source/crypto.cpp','anadius-messages-source/messages.cpp','anadius-handlers-source/handlers.cpp','anadius-content-handlers-source/content_handlers.cpp','anadius-misc-handlers-source/misc_handlers.cpp') | ForEach-Object {Join-Path $outputRoot $_}
$sources+=Join-Path $outputRoot 'anadius-license-source/license.cpp'
$exe=Join-Path $build 'integration_tests.exe'
& $compiler @flags '-static' @sources '-o' $exe
if($LASTEXITCODE -ne 0){throw 'Integration compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Integration checks failed'}
[ordered]@{checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);flags=$flags;result='PASS';tests=$result;original_execution='none';actual_modules=@('XML reader compatibility adapter','crypto','messages','router','handlers','serializer');reader='actual upstream source compatibility adapter plus synthetic fixtures';runtime_equivalence='not established';sources=@($sources | ForEach-Object {[ordered]@{path=$_.Substring($outputRoot.Length+1).Replace('\','/');sha256=(Get-FileHash -LiteralPath $_).Hash.ToLowerInvariant()}})} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
