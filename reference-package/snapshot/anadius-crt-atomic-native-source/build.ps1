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
$object=Join-Path $build 'atomic.o'
Run ($flags+@('-c',(Join-Path $PSScriptRoot 'atomic.cpp'),'-o',$object))
$objdump=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\llvm-objdump.exe')).Path
$disassembly=& $objdump -d -C $object 2>&1
if($LASTEXITCODE -ne 0){throw ($disassembly -join "`n")}
$disassembly|Set-Content -LiteralPath (Join-Path $build 'atomic.disasm.txt') -Encoding UTF8
$executable=Join-Path $build 'native_tests.exe'
Run ($flags+@('-static',(Join-Path $PSScriptRoot 'native_tests.cpp'),$object,'-o',$executable))
$commands.Add([ordered]@{program=$executable;arguments=@();scope='own native atomic fixture only'})
$result=& $executable 2>&1
if($LASTEXITCODE -ne 0){throw ($result -join "`n")}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');commands=$commands;output=$result -join "`n";target_execution='none';scope='Owned aligned LONG storage with real Windows Interlocked operations; synthetic values and joined own std::threads; no arbitrary record-byte casts, original target execution or original ABI/SEH equivalence';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();disassembly_sha256=(Get-FileHash -LiteralPath (Join-Path $build 'atomic.disasm.txt')).Hash.ToLowerInvariant();executable_sha256=(Get-FileHash -LiteralPath $executable).Hash.ToLowerInvariant()}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
