param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'nls_dispatch_tests.exe'
$object=Join-Path $build 'nls_dispatch.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'nls_dispatch.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'NLS dispatch object compilation failed'}
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'nls_dispatch.cpp'),(Join-Path $PSScriptRoot 'nls_dispatch_tests.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'NLS dispatch fixture compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'NLS dispatch fixtures failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Recovered 0671d4/0672c0 dispatch, exact module order and procedure IDs, all arguments, signed counts, full 32-bit returns, effect order and zero-return behavior. Resolver/native/locale fallback are typed synthetic endpoints; no target/native NLS execution.';original_ABI_equality='unproven';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();executable_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}})}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
