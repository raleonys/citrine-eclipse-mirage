param([string]$CompilerBin = "$PSScriptRoot/../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64/bin")
$ErrorActionPreference = 'Stop'
$moduleRoot = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$taskBuild = Join-Path $moduleRoot 'testing/build'
New-Item -ItemType Directory -Force -Path $taskBuild | Out-Null
$clang = Join-Path $CompilerBin 'clang.exe'
$clangpp = Join-Path $CompilerBin 'clang++.exe'
$nm = Join-Path $CompilerBin 'llvm-nm.exe'
$taskObjects = @()
$taskCommands = @()
foreach ($unit in @('hook','buffer','trampoline','hde/hde64')) {
    $source = Join-Path $moduleRoot "vendor/minhook/src/$unit.c"
    $name = $unit.Replace('/','_')
    $obj = Join-Path $taskBuild "$name.fake.obj"
    $argsC = @('-std=c17','-O2','-Wall','-Wextra','-Werror','-include', (Join-Path $moduleRoot 'testing/fake_win32.h'), '-c', $source,'-o',$obj)
    & $clang @argsC
    if ($LASTEXITCODE -ne 0) { throw "C compilation failed: $unit" }
    $taskObjects += $obj
    $taskCommands += [ordered]@{ executable = $clang; arguments = $argsC }
}
$exe = Join-Path $taskBuild 'hook-engine-fixture-tests.exe'
$argsCpp = @('-std=c++20','-O2','-Wall','-Wextra','-Werror','-static', (Join-Path $moduleRoot 'fixture_tests.cpp'), (Join-Path $moduleRoot 'testing/fake_win32.cpp')) + $taskObjects + @('-o',$exe)
& $clangpp @argsCpp
if ($LASTEXITCODE -ne 0) { throw 'C++ fixture compilation/link failed' }
$taskCommands += [ordered]@{ executable = $clangpp; arguments = $argsCpp }
$undefined = & $nm --undefined-only @taskObjects
if ($LASTEXITCODE -ne 0) { throw 'Object undefined-symbol check failed' }
$forbidden = $undefined | Where-Object { $_ -match '__imp_' }
if ($forbidden) { throw "Test C objects retain unexpected imported native API: $forbidden" }
$runOutput = & $exe
if ($LASTEXITCODE -ne 0) { throw 'Fixture test run failed' }
$runOutput | Write-Output
$checks = [int]([regex]::Match(($runOutput -join "`n"), '(\d+) checks passed').Groups[1].Value)
$record = [ordered]@{
    schema = 'rea-minhook-upstream-fixture-validation-v1'
    result = 'PASS'
    checks = $checks
    upstream = 'Official MinHook 1.3.4 compatibility dependency; not recovered original source'
    engine_units = @('hook.c','buffer.c','trampoline.c','hde64.c')
    execution = 'Only the synthetic fixture EXE was executed. All selected engine Win32 API call sites redirected at compile time to ReaTest functions. No native hook APIs, real thread enumeration/suspension/context changes, executable allocations or code execution.'
    address_scope = 'Owned aligned std::array data buffers only; generated trampolines and relays read as bytes, never called'
    coverage = @('Lifecycle/status/failures','5-byte patch/restore and absolute relay','7-byte hotpatch/restore','Queued/all hooks','80-hook table and buffer growth','Actual HDE64 decoding','Actual RIP/CALL/Jcc/JMP relocation','Fake instruction pointer mapping and thread failure paths','Fake module/symbol resolution')
    limits = @('No Windows memory protection or real hook execution validation','No concurrency or real scheduling validation','Fake VirtualAlloc returns reachable owned aligned pages rather than reserving requested OS addresses','No claim of original bundled MinHook exact-version equivalence')
    commands = $taskCommands
    objects = @($taskObjects | ForEach-Object { [ordered]@{ path = $_; sha256 = (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash.ToLowerInvariant() } })
    imported_api_symbols_in_fixture_C_objects = @($forbidden)
    undefined_symbols = @($undefined)
    executable = [ordered]@{ path = $exe; sha256 = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant() }
    output = @($runOutput)
}
$record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $moduleRoot 'fixture-validation.json') -Encoding utf8
