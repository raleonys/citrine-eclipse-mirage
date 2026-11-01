param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$outputs=Split-Path -Parent $PSScriptRoot
$build=Join-Path $PSScriptRoot 'build'
$fixtures=Join-Path $build 'file-io-fixtures'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$sources=@('binary_io.cpp','native_io.cpp','license_files.cpp') | ForEach-Object {Join-Path $PSScriptRoot $_}
$dependencies=@('anadius-hash-source/hash.cpp','anadius-token-source/token.cpp','anadius-crypto-source/crypto.cpp','anadius-license-source/license.cpp','anadius-xml-source/memory_printer.cpp','anadius-xml-reader-source/reader.cpp','anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp') | ForEach-Object {Join-Path $outputs $_}
$results=foreach($test in @('binary_io_tests','license_files_tests')) {
    $exe=Join-Path $build ($test+'.exe')
    & $compiler @flags '-static' @sources @dependencies (Join-Path $PSScriptRoot ($test+'.cpp')) '-o' $exe
    if($LASTEXITCODE -ne 0){throw ('File module compilation failed '+$test)}
    $result=& $exe $fixtures
    if($LASTEXITCODE -ne 0){throw ('File checks failed '+$test)}
    [ordered]@{test=$test;result='PASS';output=$result;executable_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()}
}
[ordered]@{checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);result='PASS';tests=@($results);actual_io_scope=$fixtures;original_targets='not loaded, executed or modified';native_backend='chosen std::fstream compatibility implementation; original CRT/ABI equality unproven'} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$results | ConvertTo-Json -Depth 5
