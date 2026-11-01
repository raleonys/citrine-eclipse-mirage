param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=Join-Path $ToolchainRoot 'bin\x86_64-w64-mingw32-clang++.exe'
$build=Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force -Path $build|Out-Null
& $compiler -std=c++20 -O2 -Wall -Wextra -Werror -c (Join-Path $PSScriptRoot 'parser.cpp') -o (Join-Path $build 'parser.o')
if($LASTEXITCODE -ne 0){throw 'Parser compilation failed'}
& $compiler -std=c++20 -O2 -Wall -Wextra -Werror -static (Join-Path $PSScriptRoot 'parser.cpp') (Join-Path $PSScriptRoot 'parser_tests.cpp') (Join-Path $PSScriptRoot '..\anadius-config-source\configuration.cpp') -o (Join-Path $build 'parser_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Parser tests compilation failed'}
$output=& (Join-Path $build 'parser_tests.exe')
$output|Write-Output
if($LASTEXITCODE -ne 0){throw 'Parser synthetic tests failed'}
$output|Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
$v=[ordered]@{status='PASS';compiler=((& $compiler --version)-join "`n");flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror');synthetic_assertions=[int]([regex]::Match(($output-join "`n"),'PASS: (\d+)').Groups[1].Value);target_execution=$false;real_cfg_used=$false;file_io_implemented=$false;binary_iteration_order_implemented=$false;runtime_equivalence='not established';files=@()}
foreach($r in @('parser.hpp','parser.cpp','parser_tests.cpp','build.ps1','CMakeLists.txt','README.md','evidence.json','build/parser.o','build/parser_tests.exe')){
 $f=Join-Path $PSScriptRoot $r
 if(Test-Path -LiteralPath $f){$v.files+=@{path=$r;bytes=(Get-Item -LiteralPath $f).Length;sha256=(Get-FileHash -LiteralPath $f -Algorithm SHA256).Hash.ToLower()}}
}
$v|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
