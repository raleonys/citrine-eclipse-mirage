param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$build=Join-Path $PSScriptRoot 'build';$null=New-Item -ItemType Directory -Force -Path $build
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c (Join-Path $PSScriptRoot 'calendar.cpp') -o (Join-Path $build 'calendar.o')
if($LASTEXITCODE -ne 0){throw 'Calendar production compilation failed'}
& $compiler @flags -static (Join-Path $PSScriptRoot 'calendar.cpp') (Join-Path $PSScriptRoot 'calendar_tests.cpp') -o (Join-Path $build 'calendar_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Calendar tests compilation failed'}
$result=& (Join-Path $build 'calendar_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Calendar tests failed'}
$result|Write-Output
$result|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
$hashes=[ordered]@{}
foreach($file in @('calendar.hpp','calendar.cpp','calendar_tests.cpp','build.ps1','CMakeLists.txt','README.md')){if(Test-Path -LiteralPath (Join-Path $PSScriptRoot $file)){$hashes[$file]=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $file)).Hash.ToLowerInvariant()}}
[ordered]@{result='PASS';compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;checks=[int]([regex]::Match(($result-join "`n"),'PASS: (\d+)').Groups[1].Value);output=$result -join "`n";source_hashes=$hashes;original_execution='none';OS_time_calls='none';native_ABI_equality='not established';scope='All32-bit input cells, logical exact recorded integer arithmetic; C++chrono valid positive-year dates and independent register instruction oracle'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
