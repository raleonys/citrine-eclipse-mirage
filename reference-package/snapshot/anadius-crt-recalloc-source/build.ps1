param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'recalloc_tests.exe'
$object=Join-Path $build 'recalloc.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'recalloc.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'Recalloc object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'recalloc.cpp'),(Join-Path $PSScriptRoot 'recalloc_tests.cpp'),(Join-Path $PSScriptRoot '..\anadius-crt-realloc-source\realloc.cpp'),(Join-Path $PSScriptRoot '..\anadius-crt-heap-source\heap.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Recalloc tests compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Recalloc tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Synthetic storage/size/reallocation/error/callback providers and actual recovered realloc and heap sources. No target or native HeapSize/HeapReAlloc execution.';original_ABI_equality='unproven';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();test_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
