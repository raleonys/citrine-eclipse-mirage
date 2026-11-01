param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
$object=Join-Path $build 'native_api.o'
& $compiler @flags -c (Join-Path $PSScriptRoot 'native_api.cpp') -o $object
if($LASTEXITCODE -ne 0){throw 'NLS native adapter compilation failed'}
$exe=Join-Path $build 'native_tests.exe'
$arguments=$flags+@('-static',(Join-Path $PSScriptRoot 'native_tests.cpp'),$object,'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'NLS native fixture compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'NLS native fixtures failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;scope='SDK typed native API adapter. Typed WINAPI fixture calls and real Windows NLS calls on bounded synthetic strings/CP info only. No original DLL/game execution, files, registry, hooks or threads.';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}});executable_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant()}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
