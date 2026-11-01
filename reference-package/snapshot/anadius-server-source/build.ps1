param([string]$ToolchainRoot = (Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=Join-Path $ToolchainRoot 'bin\x86_64-w64-mingw32-clang++.exe'
if (-not (Test-Path -LiteralPath $compiler)) {throw "Missing compiler $compiler"}
$build=Join-Path $PSScriptRoot 'build'
$null=New-Item -ItemType Directory -Path $build -Force
$flags=@('-std=c++20','-O2','-Wall','-Wextra','-Werror')
& $compiler @flags '-c' (Join-Path $PSScriptRoot 'server.cpp') '-o' (Join-Path $build 'server.o')
if ($LASTEXITCODE -ne 0) {throw 'Server source compilation failed'}
$executionSource=Join-Path $PSScriptRoot '..\anadius-execution-source\execution.cpp'
& $compiler @flags '-static' (Join-Path $PSScriptRoot 'server.cpp') (Join-Path $PSScriptRoot 'server_tests.cpp') $executionSource '-o' (Join-Path $build 'server_mock_tests.exe')
if ($LASTEXITCODE -ne 0) {throw 'Synthetic test compilation failed'}
$result=& (Join-Path $build 'server_mock_tests.exe') 2>&1
if ($LASTEXITCODE -ne 0) {throw ('Synthetic tests failed: '+($result -join "`n"))}
$record=[ordered]@{
    compiler=(& $compiler '--version') -join "`n"
    compiler_driver_sha256=(Get-FileHash -LiteralPath $compiler -Algorithm SHA256).Hash.ToLowerInvariant()
    verified_toolchain_archive_sha256='317492c456aa27ee607a5919f1d2d38dcdc1112516a24d0bf4b00d078f52d17a'
    compile_flags=$flags;synthetic_test_output=$result -join "`n";synthetic_test_exit=0
    original_artifact_sha256='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
    target_execution='none';real_socket_or_network_operations='none';original_runtime_equivalence='not established'
    source_files=@(@('server.hpp','server.cpp','server_tests.cpp','build.ps1','CMakeLists.txt','README.md','evidence-map.json','..\anadius-execution-source\execution.hpp','..\anadius-execution-source\execution.cpp') | ForEach-Object {
        [ordered]@{name=$_;sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $_) -Algorithm SHA256).Hash.ToLowerInvariant()}
    })
    retained_evidence_files=@(@('anadius-all-functions.asm.txt','anadius-all-functions.pseudo.c','mac-symbol-comparison\application-symbols.json') | ForEach-Object {
        [ordered]@{name=$_;sha256=(Get-FileHash -LiteralPath (Join-Path (Split-Path $PSScriptRoot -Parent) $_) -Algorithm SHA256).Hash.ToLowerInvariant()}
    })
    built_files=@(@('server.o','server_mock_tests.exe') | ForEach-Object {
        [ordered]@{name=$_;sha256=(Get-FileHash -LiteralPath (Join-Path $build $_) -Algorithm SHA256).Hash.ToLowerInvariant()}
    })
}
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'validation.json'),($record|ConvertTo-Json -Depth 6),(New-Object Text.UTF8Encoding($false)))
$record|ConvertTo-Json -Depth 6
