param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$commands=[System.Collections.Generic.List[object]]::new()
function Run([string[]]$arguments){
    $commands.Add([ordered]@{program=$compiler;arguments=$arguments})
    $output=& $compiler @arguments 2>&1
    if($LASTEXITCODE -ne 0){throw ($output -join "`n")}
}
Run ($flags+@('-c',(Join-Path $PSScriptRoot 'initializers.cpp'),'-o',(Join-Path $build 'initializers.o')))
$sources=@('initializer_tests.cpp','../anadius-crt-entry-source/entry.cpp','../anadius-crt-onexit-source/onexit.cpp','../anadius-crt-cache-source/cache.cpp','../anadius-crt-runtime-source/runtime.cpp','../anadius-crt-ptd-source/ptd.cpp','../anadius-thread-source/threads.cpp','../anadius-source/bootstrap.cpp')
$executable=Join-Path $build 'initializer_tests.exe'
Run ($flags+@('-static')+@($sources|ForEach-Object{Join-Path $PSScriptRoot $_})+@((Join-Path $build 'initializers.o'),'-o',$executable))
$result=& $executable 2>&1
if($LASTEXITCODE -ne 0){throw ($result -join "`n")}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');commands=$commands;output=$result -join "`n";target_execution='none';scope='Recovered table walks/outer CRT/cookie/cache/onexit and PTD cores; owned opaque table/storage fixtures; synthetic app/VCRT/CPU/argv/environment/OS effects';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});executable_sha256=(Get-FileHash -LiteralPath $executable).Hash.ToLowerInvariant()}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
