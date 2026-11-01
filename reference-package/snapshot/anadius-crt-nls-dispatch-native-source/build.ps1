param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'native_tests.exe'
$object=Join-Path $build 'native.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'native.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'NLS SDK object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'native.cpp'),(Join-Path $PSScriptRoot 'native_tests.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'NLS SDK tests compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'NLS SDK tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Windows x64 typed SDK calls with injected WINAPI fixtures and real LCMapStringW/Ex/LocaleNameToLCID on own synthetic literal strings. Existing kernel32 GetModuleHandleW/GetProcAddress only; no LoadLibrary or original/rebuilt target artifact execution.';original_class_ABI_equality='not claimed';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();test_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
