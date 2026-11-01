param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$node=(Get-Command node -ErrorAction Stop).Source
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
& $node (Join-Path $PSScriptRoot 'extract.cjs') | Out-Null
if($LASTEXITCODE -ne 0){throw 'Hash-bound data extraction failed'}
$exe=Join-Path $build 'initial_mb_tests.exe'
$object=Join-Path $build 'initial_mb.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'initial_mb.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'Initial multibyte object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'initial_mb.cpp'),(Join-Path $PSScriptRoot 'native_tests.cpp'),(Join-Path $PSScriptRoot '..\anadius-crt-multibyte-tables-source\multibyte_tables.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Initial multibyte tests compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Initial multibyte tests failed'}
& $node (Join-Path $PSScriptRoot 'verify.cjs') $exe
if($LASTEXITCODE -ne 0){throw 'Compiled data snapshot comparison failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Only own fixture executed; hash-bound input read only; 1065 compiled data bytes independently compared to PE file; reset fixture uses recovered source and owned memory. No original DLL execution or native callbacks.';original_ABI_equality='unproven';new_original_function_bodies=0;object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();test_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
& $node (Join-Path $PSScriptRoot 'manifest.cjs')
if($LASTEXITCODE -ne 0){throw 'Evidence/provenance manifest failed'}
$result
