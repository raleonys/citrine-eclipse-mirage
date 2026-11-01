param([string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$source=Join-Path $PSScriptRoot 'native_shims.cpp'
$core=Join-Path $PSScriptRoot '..\preloader-source\preloader_core.cpp'
$object=Join-Path $PSScriptRoot 'native_shims.o'
$testExecutable=Join-Path $PSScriptRoot 'preloader_native_mock_tests.exe'
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags -c $source -o $object
if($LASTEXITCODE -ne 0){throw 'Native shim compilation failed'}
& $compiler @flags -static $source $core (Join-Path $PSScriptRoot 'mock_tests.cpp') -o $testExecutable
if($LASTEXITCODE -ne 0){throw 'Native shim mock test compilation failed'}
$log=[System.Collections.Generic.List[string]]::new()
$result=& $testExecutable 2>&1
if($LASTEXITCODE -ne 0){throw ($result -join "`n")}
$log.Add(($result -join "`n"))
$modes=[ordered]@{'--unconfigured'=94;'--unbound-next'=70;'--unbound-mutex'=70;'--unbound-query'=70;'--unbound-entry'=70;'--escaped'=93;'--rebind'=71;'--closed'=70}
foreach($item in $modes.GetEnumerator()) {
    & $testExecutable $item.Key
    $exitCode=$LASTEXITCODE
    if($exitCode -ne $item.Value){throw "Unexpected exit for $($item.Key): $exitCode"}
    $log.Add("$($item.Key): required terminal policy exit $exitCode PASS")
}
$log | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.txt') -Encoding utf8
$record=[ordered]@{
    kind='local-compilation-and-synthetic-tests-not-original-execution'
    production_objects=1
    suites=1
    boundary_subprocess_cases=$modes.Count
    compiler=$compiler
    flags=$flags
    object_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $object).Hash.ToLowerInvariant()
    test_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $testExecutable).Hash.ToLowerInvariant()
    execution='Only this synthetic test executable; no original/rebuilt DLL, hooks, generated trampolines or real certificate/mutex/module APIs.'
}
$record | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'build-record.json') -Encoding utf8
$log
