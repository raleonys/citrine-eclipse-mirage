param([string]$ToolchainRoot = (Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference = 'Stop'
$compiler = (Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin\clang++.exe')).Path
$build = Join-Path $PSScriptRoot 'build'
[void](New-Item -ItemType Directory -Path $build -Force)
$flags = @('-std=c++20','-O2','-Wall','-Wextra','-Werror')
foreach ($source in @('libraries','native_system')) {
    & $compiler @flags '-c' (Join-Path $PSScriptRoot "$source.cpp") '-o' (Join-Path $build "$source.o")
    if ($LASTEXITCODE -ne 0) { throw "$source compilation failed" }
}
# Native adapter is deliberately not linked into this mock test executable.
& $compiler @flags '-static' (Join-Path $PSScriptRoot 'libraries.cpp') (Join-Path $PSScriptRoot 'libraries_tests.cpp') '-o' (Join-Path $build 'libraries_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Mock test compilation failed' }
$testOutput = & (Join-Path $build 'libraries_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Mock test execution failed' }
$testOutput | Set-Content -LiteralPath (Join-Path $build 'validation.txt') -Encoding UTF8
$files = Get-ChildItem -LiteralPath $PSScriptRoot -File | Where-Object Extension -in @('.cpp','.hpp','.ps1','.md')
$audit = Join-Path $PSScriptRoot 'audit.json'
$retained = Join-Path $PSScriptRoot '..\anadius-all-functions.asm.txt'
[ordered]@{compiler=(& $compiler --version | Select-Object -First 1);compiler_sha256=(Get-FileHash -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags;object_compilation='passed';mock_tests=$testOutput;original_dll_loaded=$false;native_adapter_called=$false;full_runtime_equivalence=$false;static_audit=[ordered]@{file='audit.json';sha256=(Get-FileHash -LiteralPath $audit).Hash.ToLowerInvariant();retained_assembly_sha256=(Get-FileHash -LiteralPath $retained).Hash.ToLowerInvariant()};sources=@($files | ForEach-Object { [ordered]@{file=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()} })} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
$testOutput
