param([string]$ToolchainRoot = (Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=Join-Path $ToolchainRoot 'bin\x86_64-w64-mingw32-clang++.exe'
if (-not (Test-Path -LiteralPath $compiler)) {throw "Missing compiler $compiler"}
$null=New-Item -ItemType Directory -Path (Join-Path $PSScriptRoot 'build') -Force
$build=Join-Path $PSScriptRoot 'build'
$common=@('-std=c++20','-Wall','-Wextra','-Werror','-O2')
foreach ($source in @('execution','windows_platform')) {
    & $compiler @common '-c' (Join-Path $PSScriptRoot ($source+'.cpp')) '-o' (Join-Path $build ($source+'.o'))
    if ($LASTEXITCODE -ne 0) {throw "Compilation failed $source"}
}
& $compiler @common '-static' (Join-Path $PSScriptRoot 'execution.cpp') (Join-Path $PSScriptRoot 'execution_tests.cpp') '-o' (Join-Path $build 'execution_mock_tests.exe')
if ($LASTEXITCODE -ne 0) {throw 'Synthetic test compilation failed'}
$testOutput=& (Join-Path $build 'execution_mock_tests.exe') 2>&1
if ($LASTEXITCODE -ne 0) {throw ('Synthetic tests failed: '+($testOutput -join "`n"))}
$record=[ordered]@{
    compiler=(& $compiler '--version') -join "`n"
    compiler_driver_sha256=(Get-FileHash -LiteralPath $compiler -Algorithm SHA256).Hash.ToLowerInvariant()
    distribution='Official llvm-mingw20261006 UCRT x64'
    verified_toolchain_archive_sha256='317492c456aa27ee607a5919f1d2d38dcdc1112516a24d0bf4b00d078f52d17a'
    compile_flags=$common;module_objects=@('execution.o','windows_platform.o')
    synthetic_test_output=$testOutput -join "`n";synthetic_test_exit=0
    original_target_execution='none';runtime_equivalence='not established'
    original_artifact_sha256='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
    source_files=@(@('execution.hpp','execution.cpp','windows_platform.hpp','windows_platform.cpp','execution_tests.cpp','build.ps1','CMakeLists.txt') | ForEach-Object {
        [ordered]@{name=$_;sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $_) -Algorithm SHA256).Hash.ToLowerInvariant()}
    })
    retained_evidence_files=@(@('anadius-all-functions.asm.txt','anadius-all-functions.pseudo.c','anadius-recovery-index.json') | ForEach-Object {
        [ordered]@{name=$_;sha256=(Get-FileHash -LiteralPath (Join-Path (Split-Path $PSScriptRoot -Parent) $_) -Algorithm SHA256).Hash.ToLowerInvariant()}
    })
    built_files=@(@('execution.o','windows_platform.o','execution_mock_tests.exe') | ForEach-Object {
        [ordered]@{name=$_;sha256=(Get-FileHash -LiteralPath (Join-Path $build $_) -Algorithm SHA256).Hash.ToLowerInvariant()}
    })
}
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'validation.json'),($record|ConvertTo-Json -Depth 6),(New-Object Text.UTF8Encoding($false)))
$record|ConvertTo-Json -Depth 6
