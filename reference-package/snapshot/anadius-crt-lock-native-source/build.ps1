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
Run ($flags+@('-c',(Join-Path $PSScriptRoot 'native.cpp'),'-o',(Join-Path $build 'native.o')))
$executable=Join-Path $build 'native_tests.exe'
Run ($flags+@('-static',(Join-Path $PSScriptRoot 'native_tests.cpp'),(Join-Path $build 'native.o'),'-o',$executable))
$result=& $executable 2>&1
if($LASTEXITCODE -ne 0){throw ($result -join "`n")}
$commands.Add([ordered]@{program=$executable;arguments=@('--live-bank');expected_exit=88;scope='own injected lifetime boundary'})
& $executable --live-bank
if($LASTEXITCODE -ne 88){throw 'Live native bank terminal boundary failed'}
[ordered]@{result='PASS';terminal_boundary_cases=@([ordered]@{mode='--live-bank';exit_code=$LASTEXITCODE;result='PASS'});checked_at=(Get-Date).ToUniversalTime().ToString('o');commands=$commands;output=$result -join "`n";target_execution='none';new_recovered_bodies=0;scope='Typed native ACRT14/VCRT1 critical-section banks; injected SDK edge guards and full BOOL; actual Windows recursive critical section and two joined own workers; no original target, hidden lifecycle, loader equivalence, ABI/SEH proof';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();executable_sha256=(Get-FileHash -LiteralPath $executable).Hash.ToLowerInvariant()}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
