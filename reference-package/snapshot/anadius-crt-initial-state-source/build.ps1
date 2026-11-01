param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'initial_state.cpp') -o (Join-Path $build 'initial_state.o')
if($LASTEXITCODE -ne 0){throw 'Initial state object failed'}
$units=@('initial_state.cpp','native_tests.cpp','../anadius-crt-initial-locale-source/initial_locale_data.cpp','../anadius-crt-initial-multibyte-source/initial_mb.cpp','../anadius-crt-counter-native-source/counters.cpp','../anadius-crt-locale-ref-source/locale_ref.cpp')
$exe=Join-Path $build 'initial_state_tests.exe'
$arguments=$flags+@('-static')+@($units|ForEach-Object{Join-Path $PSScriptRoot $_})+@('-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Initial state tests compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Initial state tests failed'}
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=$compiler;arguments=$arguments;tests=$result;original_execution='none';new_original_bodies=0;scope='Owned relocated image initial data;3actual LONGs;actual recovered locale ref source;4own threads;CRT initializers and full runtime not executed';sources=@(Get-ChildItem -LiteralPath $PSScriptRoot -File|Where-Object{$_.Extension -in @('.hpp','.cpp')}|ForEach-Object{[ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}})} | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
