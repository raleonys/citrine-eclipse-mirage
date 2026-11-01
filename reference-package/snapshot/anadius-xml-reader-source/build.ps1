param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$sources=@('reader.cpp','vendor/tinyxml2/tinyxml2.cpp','reader_tests.cpp') | ForEach-Object {Join-Path $PSScriptRoot $_}
$sources+=Join-Path $PSScriptRoot '..\anadius-xml-source\memory_printer.cpp'
$exe=Join-Path $build 'reader_tests.exe'
& $compiler @flags '-static' @sources '-o' $exe
if($LASTEXITCODE -ne 0){throw 'Reader compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Reader tests failed'}
$files=Get-ChildItem -LiteralPath $PSScriptRoot -File -Recurse | Where-Object { $_.FullName -notlike '*\build\*' -and $_.Name -ne 'validation.json' }
[ordered]@{checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);result='PASS';tests=$result;reader='actual upstream source compatibility adapter';original_version_match='unproven';target_execution='none';runtime_equivalence='not established';files=@($files | ForEach-Object {[ordered]@{path=$_.FullName.Substring($PSScriptRoot.Length+1).Replace('\','/');sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}})} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
