param(
    [string]$ToolchainRoot=(Join-Path $PSScriptRoot '../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64'),
    [switch]$Diagnose
)
$ErrorActionPreference='Stop'
$outputs=Split-Path -Parent $PSScriptRoot
$workspace=Split-Path -Parent $outputs
$compiler=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/clang++.exe')).Path
$objdump=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/llvm-objdump.exe')).Path
$nm=(Resolve-Path -LiteralPath (Join-Path $ToolchainRoot 'bin/llvm-nm.exe')).Path
$snapshotPath=Join-Path $outputs 'source-workspace/source-snapshot.json'
$buildRecordPath=Join-Path $outputs 'source-workspace/build-record.json'
$snapshot=Get-Content -LiteralPath $snapshotPath -Raw | ConvertFrom-Json
$baseline=Get-Content -LiteralPath $buildRecordPath -Raw | ConvertFrom-Json
function Hash([string]$path){(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()}
if($baseline.result -ne 'PASS' -or $snapshot.build_record_sha256 -ne (Hash $buildRecordPath)){
    throw 'Missing or mismatched frozen common build; link gate did not run.'
}
if((Hash $compiler) -ne $baseline.compiler_sha256){throw 'Compiler differs from frozen build.'}
foreach($source in $snapshot.sources){
    if((Hash (Join-Path $outputs $source.path)) -ne $source.sha256){throw "Frozen source changed: $($source.path)"}
}
$archives=@{}
foreach($name in @('anadius_core_partial.a','hook_engine_compat.a')){
    $path=Join-Path $outputs "source-workspace/build/$name"
    $expected=@($baseline.archives | Where-Object file -eq $name)
    if($expected.Count -ne 1 -or (Hash $path) -ne $expected[0].sha256){throw "Frozen archive mismatch: $name"}
    $archives[$name]=$path
}
# B52: explicitly select the actual frozen production factory object in this
# STATIC LINK DIAGNOSTIC. It is not an implemented loader Resources factory.
$factoryRelative='source-workspace/build/configuration_factory.o'
$factoryObject=(Resolve-Path -LiteralPath (Join-Path $outputs $factoryRelative)).Path
$factorySource='anadius-configuration-factory-source/factory.cpp'
$factorySourcePath=(Resolve-Path -LiteralPath (Join-Path $outputs $factorySource)).Path
$factorySnapshot=@($snapshot.sources | Where-Object path -eq $factorySource)
if($factorySnapshot.Count -ne 1){throw 'Factory source missing from frozen common snapshot.'}
$factoryCommandIndex=-1
for($i=0;$i -lt $baseline.commands.Count;$i++){
    $a=@($baseline.commands[$i].arguments);$oi=[Array]::IndexOf($a,'-o')
    if($a -notcontains '-c' -or $oi -lt 0 -or $oi+1 -ge $a.Count){continue}
    if([IO.Path]::GetFullPath([string]$a[$oi+1]) -ne $factoryObject){continue}
    $sourceArgs=@($a | Where-Object {$_ -notmatch '^-' -and [IO.Path]::GetFileName([string]$_) -eq 'factory.cpp' -and [IO.Path]::GetFullPath([string]$_) -eq $factorySourcePath})
    if($sourceArgs.Count -ne 1 -or $factoryCommandIndex -ne -1){throw 'Factory object/common source command mismatch.'}
    $factoryCommandIndex=$i
}
if($factoryCommandIndex -lt 0){throw 'Factory object was not compiled by the frozen common build.'}
$factoryHash=Hash $factoryObject
# B53: explicitly select the actual frozen production factory object in this
# STATIC LINK DIAGNOSTIC. It is not an implemented loader Resources factory.
$servicesFactoryRelative='source-workspace/build/services_factory.o'
$servicesFactoryObject=(Resolve-Path -LiteralPath (Join-Path $outputs $servicesFactoryRelative)).Path
$servicesFactorySource='anadius-services-factory-source/factory.cpp'
$servicesFactorySourcePath=(Resolve-Path -LiteralPath (Join-Path $outputs $servicesFactorySource)).Path
$servicesFactorySnapshot=@($snapshot.sources | Where-Object path -eq $servicesFactorySource)
if($servicesFactorySnapshot.Count -ne 1){throw 'Factory source missing from frozen common snapshot.'}
$servicesFactoryCommandIndex=-1
for($i=0;$i -lt $baseline.commands.Count;$i++){
    $a=@($baseline.commands[$i].arguments);$oi=[Array]::IndexOf($a,'-o')
    if($a -notcontains '-c' -or $oi -lt 0 -or $oi+1 -ge $a.Count){continue}
    if([IO.Path]::GetFullPath([string]$a[$oi+1]) -ne $servicesFactoryObject){continue}
    $sourceArgs=@($a | Where-Object {$_ -notmatch '^-' -and [IO.Path]::GetFileName([string]$_) -eq 'factory.cpp' -and [IO.Path]::GetFullPath([string]$_) -eq $servicesFactorySourcePath})
    if($sourceArgs.Count -ne 1 -or $servicesFactoryCommandIndex -ne -1){throw 'Factory object/common source command mismatch.'}
    $servicesFactoryCommandIndex=$i
}
if($servicesFactoryCommandIndex -lt 0){throw 'Factory object was not compiled by the frozen common build.'}
$servicesFactoryHash=Hash $servicesFactoryObject
# B54: explicitly select the actual frozen production factory object in this
# STATIC LINK DIAGNOSTIC. It is not an implemented loader Resources factory.
$onlineFactoryRelative='source-workspace/build/online_factory.o'
$onlineFactoryObject=(Resolve-Path -LiteralPath (Join-Path $outputs $onlineFactoryRelative)).Path
$onlineFactorySource='anadius-online-factory-source/factory.cpp'
$onlineFactorySourcePath=(Resolve-Path -LiteralPath (Join-Path $outputs $onlineFactorySource)).Path
$onlineFactorySnapshot=@($snapshot.sources | Where-Object path -eq $onlineFactorySource)
if($onlineFactorySnapshot.Count -ne 1){throw 'Factory source missing from frozen common snapshot.'}
$onlineFactoryCommandIndex=-1
for($i=0;$i -lt $baseline.commands.Count;$i++){
    $a=@($baseline.commands[$i].arguments);$oi=[Array]::IndexOf($a,'-o')
    if($a -notcontains '-c' -or $oi -lt 0 -or $oi+1 -ge $a.Count){continue}
    if([IO.Path]::GetFullPath([string]$a[$oi+1]) -ne $onlineFactoryObject){continue}
    $sourceArgs=@($a | Where-Object {$_ -notmatch '^-' -and [IO.Path]::GetFileName([string]$_) -eq 'factory.cpp' -and [IO.Path]::GetFullPath([string]$_) -eq $onlineFactorySourcePath})
    if($sourceArgs.Count -ne 1 -or $onlineFactoryCommandIndex -ne -1){throw 'Factory object/common source command mismatch.'}
    $onlineFactoryCommandIndex=$i
}
if($onlineFactoryCommandIndex -lt 0){throw 'Factory object was not compiled by the frozen common build.'}
$onlineFactoryHash=Hash $onlineFactoryObject
# A fresh directory excludes stale DLL/map files from every diagnostic result.
$runName='run-'+(Get-Date).ToUniversalTime().ToString('yyyyMMddTHHmmssfffffffZ')
$run=Join-Path $PSScriptRoot "build/$runName"
$null=New-Item -ItemType Directory -Path $run
$commands=[System.Collections.Generic.List[object]]::new()
function Invoke-Tool([string]$program,[string[]]$toolArguments,[string]$log){
    $lines=& $program @toolArguments 2>&1
    $code=$LASTEXITCODE
    $lines | Set-Content -LiteralPath (Join-Path $run $log) -Encoding utf8
    $commands.Add([ordered]@{program=$program;arguments=@($toolArguments);exit_code=$code;log=$log})
    return $code
}
$entry=Join-Path $run 'entry.o'
$leaf=Join-Path $run 'export_leaf.o'
if((Invoke-Tool $compiler @('-std=c++20','-O2','-Wall','-Wextra','-Werror','-c',(Join-Path $PSScriptRoot 'entry.cpp'),'-o',$entry) 'entry-compile.log') -ne 0){throw 'Entry compilation failed; see run log.'}
if((Invoke-Tool $compiler @('-c',(Join-Path $PSScriptRoot 'export_leaf.S'),'-o',$leaf) 'leaf-compile.log') -ne 0){throw 'Export assembly failed; see run log.'}
if((Invoke-Tool $objdump @('-d',$leaf) 'export-leaf-disassembly.txt') -ne 0){throw 'Leaf inspection failed.'}
if((Invoke-Tool $nm @('--undefined-only','--demangle',$entry) 'entry-undefined-symbols.txt') -ne 0){throw 'Entry inspection failed.'}
if((Invoke-Tool $nm @('--defined-only','--demangle',$factoryObject) 'factory-defined-symbols.txt') -ne 0){throw 'Factory object inspection failed.'}
if((Invoke-Tool $nm @('--defined-only','--demangle',$servicesFactoryObject) 'services-factory-defined-symbols.txt') -ne 0){throw 'Services factory object inspection failed.'}
if((Invoke-Tool $nm @('--defined-only','--demangle',$onlineFactoryObject) 'online-factory-defined-symbols.txt') -ne 0){throw 'Online factory object inspection failed.'}
$dll=Join-Path $run 'anadius64.reconstruction.dll'
$map=Join-Path $run 'anadius64.reconstruction.map'
$linkArgs=@('-shared','-static',$entry,$leaf,$factoryObject,$servicesFactoryObject,$onlineFactoryObject,(Join-Path $PSScriptRoot 'anadius64.def'),
    $archives['anadius_core_partial.a'],$archives['hook_engine_compat.a'],
    '-lkernel32','-luser32','-lshell32','-lws2_32','-lmswsock',
    '-Wl,--exclude-all-symbols,--no-insert-timestamp,--verbose',"-Wl,-Map,$map",'-o',$dll)
$linkExit=Invoke-Tool $compiler $linkArgs 'link.log'
if((Hash $factoryObject) -ne $factoryHash){throw 'Factory object changed through link.'}
if((Hash $servicesFactoryObject) -ne $servicesFactoryHash){throw 'Services factory object changed through link.'}
if((Hash $onlineFactoryObject) -ne $onlineFactoryHash){throw 'Online factory object changed through link.'}
$linkText=Get-Content -LiteralPath (Join-Path $run 'link.log') -Raw
$undefined=@([regex]::Matches($linkText,'(?m)^.*undefined symbol: (.+)$') | ForEach-Object {$_.Groups[1].Value.Trim()} | Sort-Object -Unique)
$required=@()
$expectedBlocked=$required.Count -gt 0 -and $linkExit -ne 0 -and $undefined.Count -eq $required.Count
foreach($symbol in $required){if(@($undefined | Where-Object {$_ -match $symbol}).Count -ne 1){$expectedBlocked=$false}}
$created=Test-Path -LiteralPath $dll
if($created){$expectedBlocked=$false}
$linked=$linkExit -eq 0 -and $created -and $undefined.Count -eq 0
$sourceFiles=@('production_image.hpp','entry.cpp','export_leaf.S','anadius64.def','target.cmake','build-link-gate.ps1','verify-link-gate.py')
$hashes=[ordered]@{}
foreach($name in $sourceFiles){$hashes[$name]=Hash (Join-Path $PSScriptRoot $name)}
$record=[ordered]@{
    schema='dll-link-gate.validation.v1';package='B31';checked_at=(Get-Date).ToUniversalTime().ToString('o')
    invocation_mode=$(if($Diagnose){'diagnose'}else{'normal-build'})
    diagnostic_validation=$(if($linked){'LINKED_NOT_EXECUTED'}elseif($expectedBlocked){'PASS_EXPECTED_BLOCKER'}else{'FAIL'})
    production_dll_readiness=$(if($linked){'LINKED_NOT_RUNTIME_VALIDATED'}else{'BLOCKED'});dll_link_attempted=$true;dll_link_succeeded=($linkExit -eq 0)
    link_exit_code=$linkExit;dll_created=$created;dll_loaded_or_executed=$false;originals_accessed=$false
    frozen_source_files_verified=@($snapshot.sources).Count;common_test_groups_rerun=0
    baseline_common_build_record_sha256=Hash $buildRecordPath;baseline_source_snapshot_sha256=Hash $snapshotPath
    compiler_sha256=Hash $compiler;source_hashes=$hashes
    archive_hashes=@($baseline.archives | Where-Object {$_.file -in @('anadius_core_partial.a','hook_engine_compat.a')})
    explicit_configuration_factory=[ordered]@{
        object_path=$factoryRelative;object_sha256=$factoryHash
        source_path=$factorySource;source_sha256=$factorySnapshot[0].sha256
        common_compile_command_index=$factoryCommandIndex
        diagnostic_selection_only=$true;loader_factory_invoked=$false
        cmake_direct_source_configured=$true;cmake_target_executed=$false
    }
    explicit_services_factory=[ordered]@{
        object_path=$servicesFactoryRelative;object_sha256=$servicesFactoryHash
        source_path=$servicesFactorySource;source_sha256=$servicesFactorySnapshot[0].sha256
        common_compile_command_index=$servicesFactoryCommandIndex
        diagnostic_selection_only=$true;loader_factory_invoked=$false
        cmake_direct_source_configured=$true;cmake_target_executed=$false
    }
    explicit_online_factory=[ordered]@{
        object_path=$onlineFactoryRelative;object_sha256=$onlineFactoryHash
        source_path=$onlineFactorySource;source_sha256=$onlineFactorySnapshot[0].sha256
        common_compile_command_index=$onlineFactoryCommandIndex
        diagnostic_selection_only=$true;loader_factory_invoked=$false
        cmake_direct_source_configured=$true;cmake_target_executed=$false
    }
    undefined_symbols=$undefined;expected_missing_production_hooks=$required
    candidate_entry_strategy='compiler CRT once, concrete synchronous source image/application; never executed by this build'
    new_native_ABI_provider_closures=0;overall_completion_percentage=$null
    run_directory=$run;map_created=(Test-Path -LiteralPath $map);commands=@($commands)
}
$record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $run 'validation.json') -Encoding utf8
$record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'validation.json') -Encoding utf8
[pscustomobject]$record | Select-Object diagnostic_validation,production_dll_readiness,link_exit_code,dll_created,undefined_symbols,run_directory | ConvertTo-Json -Depth 4
if(!$expectedBlocked -and !$linked){throw 'Unexpected link result; inspect the retained link log.'}
if(!$Diagnose -and !$linked){exit 1}
