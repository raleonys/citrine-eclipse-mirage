# Optional continuation of a user-authorized clean build. This never reuses a
# previous delivery: the caller must provide an explicit current-run manifest.
# Only compiler outputs may be reused. Every test and archive command runs again.
$checkpointPath=(Resolve-Path -LiteralPath $FreshRunCheckpoint).Path
$checkpoint=Get-Content -LiteralPath $checkpointPath -Raw | ConvertFrom-Json -AsHashtable
if($checkpoint.schema -ne 'same-clean-run-compiler-checkpoint.v1'){throw 'Unsupported checkpoint'}
$checkpointRun=(Resolve-Path -LiteralPath $checkpoint.run_directory).Path
$checkpointStarted=[DateTimeOffset]::Parse($checkpoint.started_at).UtcDateTime
if((Split-Path -Parent $checkpointPath) -ne $checkpointRun){throw 'Checkpoint must reside in its own run directory'}
$currentHeaders=@(Get-ChildItem -LiteralPath $outputs -Directory | Where-Object {$_.Name.EndsWith('-source')} | ForEach-Object {
    Get-ChildItem -LiteralPath $_.FullName -File -Recurse | Where-Object {$_.Extension -in '.hpp','.h'}
} | Sort-Object FullName | ForEach-Object {
    [ordered]@{path=$_.FullName;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}
})
if(($currentHeaders | ConvertTo-Json -Depth 4 -Compress) -cne ($checkpoint.headers | ConvertTo-Json -Depth 4 -Compress)){throw 'Header set changed: create a new verified checkpoint instead'}
$checkpointEntries=@{}
foreach($row in $checkpoint.entries){$checkpointEntries[$row.key]=$row}
$checkpointPrograms=@{}
function Get-CheckpointHash([string]$path){return (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()}
function Get-CheckpointKey([string]$program,[string[]]$arguments){
    $bytes=[Text.Encoding]::UTF8.GetBytes($program.ToLowerInvariant()+[char]0+($arguments -join [char]0))
    return [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($bytes)).ToLowerInvariant()
}
function Save-Checkpoint {
    $checkpoint.entries=@($checkpointEntries.Values | Sort-Object key)
    $checkpoint | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath ($checkpointPath+'.tmp') -Encoding UTF8
    Move-Item -LiteralPath ($checkpointPath+'.tmp') -Destination $checkpointPath -Force
}
function Invoke-Recorded([string]$program,[string[]]$arguments) {
    $command=[ordered]@{program=$program;arguments=$arguments}
    $outputIndex=[Array]::IndexOf($arguments,'-o')
    $eligible=$program -in $compiler,$cCompiler -and $outputIndex -ge 0
    if($eligible){
        $target=$arguments[$outputIndex+1]
        $eligible=(Split-Path -Parent $target) -eq $build -and ($target.EndsWith('.o') -or $target.EndsWith('_mock_tests.exe'))
    }
    if($eligible){
        if(-not $checkpointPrograms.ContainsKey($program)){$checkpointPrograms[$program]=Get-CheckpointHash $program}
        $key=Get-CheckpointKey $program $arguments
        $inputs=@($arguments | Where-Object {[IO.Path]::GetExtension($_) -in '.cpp','.c','.S','.a'} | ForEach-Object {
            [ordered]@{path=$_;sha256=Get-CheckpointHash $_}
        })
        $cached=$checkpointEntries[$key]
        $reusable=$null -ne $cached -and $cached.compiler_sha256 -eq $checkpointPrograms[$program] -and
            ($cached.inputs | ConvertTo-Json -Depth 4 -Compress) -ceq ($inputs | ConvertTo-Json -Depth 4 -Compress) -and
            (Test-Path -LiteralPath $target -PathType Leaf)
        if($reusable){
            $item=Get-Item -LiteralPath $target
            $reusable=$item.LastWriteTimeUtc -ge $checkpointStarted -and (Get-CheckpointHash $target) -eq $cached.output_sha256
        }
        if($reusable){
            $command.execution='REUSED_VERIFIED_SAME_CLEAN_RUN'
            $command.checkpoint_run=$checkpointRun
            $command.output_sha256=$cached.output_sha256
            $command.input_hashes=$inputs
            $commands.Add($command)
            return
        }
    }
    $command.execution='EXECUTED'
    $commands.Add($command)
    $result=& $program @arguments 2>&1
    if($LASTEXITCODE -ne 0){throw (($result -join "`n")+"`nExit code: $LASTEXITCODE`nFailed: $program")}
    if($eligible){
        $checkpointEntries[$key]=[ordered]@{key=$key;program=$program;arguments=$arguments;compiler_sha256=$checkpointPrograms[$program];inputs=$inputs;output=$target;output_sha256=Get-CheckpointHash $target;provenance='Successful compiler invocation in this clean run'}
        Save-Checkpoint
    }
    if($program.EndsWith('_mock_tests.exe')){Write-Host ('PASS '+[IO.Path]::GetFileNameWithoutExtension($program))}
    return $result
}
