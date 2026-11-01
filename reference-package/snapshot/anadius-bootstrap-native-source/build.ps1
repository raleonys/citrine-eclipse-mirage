param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$exe=Join-Path $build 'bootstrap_native_tests.exe'
$object=Join-Path $build 'native_entries.o'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'native_entries.cpp') '-o' $object
if($LASTEXITCODE -ne 0){throw 'Bootstrap native object compilation failed'}
$arguments=@($flags)+@('-static',(Join-Path $PSScriptRoot 'native_entries.cpp'),(Join-Path $PSScriptRoot '..\anadius-thread-source\threads.cpp'),(Join-Path $PSScriptRoot '..\anadius-source\bootstrap.cpp'),(Join-Path $PSScriptRoot 'native_entry_tests.cpp'),'-o',$exe)
& $compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Bootstrap native test compilation failed'}
$result=& $exe
if($LASTEXITCODE -ne 0){throw 'Bootstrap native normal tests failed'}
$terminal=@()
foreach($case in @(@{mode='--unconfigured';expected=84},@{mode='--closed';expected=83},@{mode='--throw-worker';expected=83},@{mode='--terminate';expected=83})){
    & $exe $case.mode
    $actual=$LASTEXITCODE
    if($actual -ne $case.expected){throw "Bootstrap native terminal $($case.mode): $actual"}
    $terminal+=@{mode=$case.mode;expected_exit=$case.expected;actual_exit=$actual;result='PASS'}
}
$nm=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\llvm-nm.exe')).Path
$undefined=@(& $nm '--undefined-only' $object)
if($LASTEXITCODE -ne 0){throw 'Bootstrap native object symbol inspection failed'}
$imports=@($undefined|Where-Object{$_ -match '__imp_'})
if(@($imports|Where-Object{$_ -notmatch '__imp_(GetLastError|SetLastError)$'}).Count -ne 0){throw 'Unexpected SDK import in Bootstrap native object'}
[ordered]@{result='PASS';object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();imports=$imports;undefined_symbols=$undefined;scope='Direct SDK imports in production object limited to own-thread LastError metadata; C++ runtime/synchronization supplied by toolchain'}|ConvertTo-Json -Depth 5|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'symbol-validation.json') -Encoding UTF8
$headers=@()
foreach($name in @('process.h','windows.h','minwinbase.h')){
    $path=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot "include\$name")).Path
    $headers+=@{header=$name;path=$path;sha256=(Get-FileHash -LiteralPath $path).Hash.ToLowerInvariant()}
}
[ordered]@{kind='Installed LLVM-MinGW SDK declarations used for compile-time entry checks';headers=$headers;source_copied=$false;original_MSVC_CRT_SEH_match='unproven'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'sdk-provenance.json') -Encoding UTF8
[ordered]@{result='PASS';checked_at=(Get-Date).ToUniversalTime().ToString('o');compiler=(& $compiler --version|Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();arguments=$arguments;tests=$result;terminal_cases=$terminal;object_sha256=(Get-FileHash -LiteralPath $object).Hash.ToLowerInvariant();executable_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();scope='Compiled rebuilt SDK/C++ entry shims; mock Bootstrap dependencies and CRT effects; one owned synthetic worker, LastError metadata only; no input target execution';original_ABI_equality='unproven'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$result
