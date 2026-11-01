param([string]$ToolchainRoot = (Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$compiler=Join-Path $ToolchainRoot 'bin\x86_64-w64-mingw32-clang++.exe'
if(!(Test-Path -LiteralPath $compiler)){throw "Compiler missing: $compiler"}
$build=Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force -Path $build | Out-Null
& $compiler -std=c++20 -Wall -Wextra -Werror -O2 -c (Join-Path $PSScriptRoot 'configuration.cpp') -o (Join-Path $build 'configuration.o')
if($LASTEXITCODE -ne 0){throw 'Configuration module compilation failed'}
& $compiler -std=c++20 -Wall -Wextra -Werror -O2 -static (Join-Path $PSScriptRoot 'configuration.cpp') (Join-Path $PSScriptRoot 'configuration_tests.cpp') -o (Join-Path $build 'configuration_contract_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Configuration tests compilation failed'}
$testOutput=& (Join-Path $build 'configuration_contract_tests.exe')
if($LASTEXITCODE -ne 0){throw 'Configuration contract tests failed'}
$testOutput | Write-Output
$testOutput | Set-Content -LiteralPath (Join-Path $build 'test-output.txt') -Encoding UTF8
$assertions=[int]([regex]::Match(($testOutput -join "`n"),'PASS: (\d+)').Groups[1].Value)
$validation=[ordered]@{
    status='PASS'; scope='Configuration application logic + synthetic/mock contracts; not standalone DLL';
    compiler=((& $compiler --version) -join "`n");
    compiler_sha256=(Get-FileHash -LiteralPath $compiler -Algorithm SHA256).Hash.ToLower();
    language='C++20';flags=@('-Wall','-Wextra','-Werror','-O2');
    compilation_units=@('configuration.cpp','configuration_tests.cpp');
    mock_assertions=$assertions; target_execution=$false; target_dll_loading=$false;
    original_cfg_read_by_tests=$false; real_filesystem_adapter_used=$false;
    real_environment_writes=$false; runtime_equivalence='not established';
    recovered_function_semantics=11; primary_routines=2; helper_routines=9;
    section_qualified_setting_keys=46; raw_grammar_recovered=$false;
    original_msvc_abi_recovered=$false;
    commands=@('clang++ -std=c++20 -Wall -Wextra -Werror -O2 -c configuration.cpp -o build/configuration.o',
       'clang++ -std=c++20 -Wall -Wextra -Werror -O2 -static configuration.cpp configuration_tests.cpp -o build/configuration_contract_tests.exe',
       'build/configuration_contract_tests.exe');
    files=@()
}
foreach($relative in @('configuration.hpp','configuration.cpp','configuration_tests.cpp','build.ps1','CMakeLists.txt','evidence.json','README.md','build/configuration.o','build/configuration_contract_tests.exe')) {
    $file=Join-Path $PSScriptRoot $relative
    if(Test-Path -LiteralPath $file) {
        $validation.files+=@{path=$relative;bytes=(Get-Item -LiteralPath $file).Length;sha256=(Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLower()}
    }
}
$validation | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding UTF8
