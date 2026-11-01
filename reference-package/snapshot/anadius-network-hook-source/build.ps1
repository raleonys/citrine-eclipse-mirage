param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'hook_tests.exe'
$arguments=@('-std=c++20','-O2','-Wall','-Wextra','-Werror','-static',
    (Join-Path $PSScriptRoot 'connect.cpp'),(Join-Path $PSScriptRoot 'hook_cleanup.cpp'),
    (Join-Path $PSScriptRoot 'hook_tests.cpp'),(Join-Path $PSScriptRoot '..\anadius-execution-source\execution.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Connect/cleanup compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Connect/cleanup checks failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='Synthetic connect/cleanup effects plus recovered Execution core; no real sockets/hooks/UI/exit/target execution';original_ABI_equality='unproven'} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
