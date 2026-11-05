$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'use-tools.ps1')
$root=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$workRoot=(Resolve-Path -LiteralPath $PSScriptRoot).Path
$outputs=(Resolve-Path -LiteralPath (Join-Path $root 'outputs')).Path
$common=(Resolve-Path -LiteralPath (Join-Path $outputs 'source-workspace')).Path
$runName='rebuild-'+[DateTime]::UtcNow.ToString('yyyyMMddTHHmmssZ')
$run=Join-Path $workRoot $runName
if(Test-Path -LiteralPath $run){throw 'Rebuild run already exists'}
if([IO.Path]::GetFullPath($run) -ne (Join-Path $workRoot $runName)){throw 'Run path escapes workspace'}
$null=New-Item -ItemType Directory -Path $run
$history=Join-Path $run 'historical-common'
$null=New-Item -ItemType Directory -Path $history
foreach($name in @('build.ps1','fresh-run-checkpoint.ps1','source-snapshot.json','build-record.json','full-build-verification.json','documentation-snapshot.json')){
    $path=Join-Path $common $name
    if(Test-Path -LiteralPath $path){Copy-Item -LiteralPath $path -Destination (Join-Path $history $name)}
}
foreach($name in @('pending-source-status.json','active-work-plan.json','blockers.json','validation.json')){
    Copy-Item -LiteralPath (Join-Path $outputs ('dll-critical-path/'+$name)) -Destination (Join-Path $history $name)
}
foreach($name in @('package-receipt.json','package-manifest.json')){
    $path=Join-Path $outputs $name
    if(Test-Path -LiteralPath $path){Copy-Item -LiteralPath $path -Destination (Join-Path $run ('historical-'+$name))}
}
$moves=@(
    @{source=(Join-Path $common 'build');target=(Join-Path $history 'build');parent=$common;leaf='build'},
    @{source=(Join-Path $outputs 'hook-engine-source/testing/build');target=(Join-Path $run 'historical-hook-fixture-build');parent=(Join-Path $outputs 'hook-engine-source/testing');leaf='build'}
)
foreach($move in $moves){
    if(Test-Path -LiteralPath $move.source){
        $resolved=(Resolve-Path -LiteralPath $move.source).Path
        $expected=Join-Path ([IO.Path]::GetFullPath($move.parent)) $move.leaf
        if($resolved -ne $expected -or -not $resolved.StartsWith($outputs+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Unsafe build history move source'}
        if(-not ([IO.Path]::GetFullPath($move.target)).StartsWith($run+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase) -or (Test-Path -LiteralPath $move.target)){throw 'Unsafe build history move destination'}
        Move-Item -LiteralPath $resolved -Destination $move.target
    }
}
$fixtureRecord=Join-Path $outputs 'hook-engine-source/fixture-validation.json'
if(Test-Path -LiteralPath $fixtureRecord){Copy-Item -LiteralPath $fixtureRecord -Destination (Join-Path $run 'historical-hook-fixture-validation.json')}
$started=[DateTime]::UtcNow.ToString('o')
$compiler=Join-Path $env:ANADIUS_TOOLCHAIN_ROOT 'bin/clang++.exe'
@{started_at=$started;client_date='2026-11-05';client_timezone='Europe/Istanbul';user_authorized_rebuild=$true;user_authorized_package=$true;history=$history;toolchain=$env:ANADIUS_TOOLCHAIN_ROOT;compiler=(& $compiler --version | Select-Object -First 1);status='BUILD_RUNNING';originals_accessed=$false} | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $run 'run.json') -Encoding UTF8
$run | Set-Content -LiteralPath (Join-Path $workRoot 'current-rebuild-path.txt') -Encoding UTF8
$headers=@(Get-ChildItem -LiteralPath $outputs -Directory | Where-Object {$_.Name.EndsWith('-source')} | ForEach-Object {
    Get-ChildItem -LiteralPath $_.FullName -File -Recurse | Where-Object {$_.Extension -in '.hpp','.h'}
} | Sort-Object FullName | ForEach-Object {
    [ordered]@{path=$_.FullName;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}
})
$checkpoint=[ordered]@{schema='same-clean-run-compiler-checkpoint.v1';run_directory=$run;started_at=$started;headers=$headers;entries=@()}
$checkpoint | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath (Join-Path $run 'compile-checkpoint.json') -Encoding UTF8
Copy-Item -LiteralPath (Join-Path $run 'compile-checkpoint.json') -Destination (Join-Path $run 'compile-checkpoint-seed.json')
Write-Host ('Fresh run: '+$run)
& (Join-Path $common 'build.ps1') -ToolchainRoot $env:ANADIUS_TOOLCHAIN_ROOT -FreshRunCheckpoint (Join-Path $run 'compile-checkpoint.json') *>&1 | Tee-Object -FilePath (Join-Path $run 'full-common-build-current.log')
