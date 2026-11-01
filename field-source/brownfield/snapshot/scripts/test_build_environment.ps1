param([string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path)
$ErrorActionPreference='Stop'
. (Join-Path $Root 'scripts\build_orchestration\build_common.ps1')
$Names=@('CL','_CL_','LINK','_LINK_','LINK_REPRO','CC','CXX','CMAKE_GENERATOR','CMAKE_GENERATOR_INSTANCE','CMAKE_TOOLCHAIN_FILE')
foreach($Name in $Names){ [Environment]::SetEnvironmentVariable($Name,'BUILD_SELECTOR_SENTINEL','Process') }
Clear-BuildSelectorEnvironment
$Failed=@()
foreach($Name in $Names){ if($null -ne [Environment]::GetEnvironmentVariable($Name,'Process')){ $Failed += $Name } }
if($Failed.Count -ne 0){ Write-Host 'BUILD_ENVIRONMENT: FAIL'; $Failed | ForEach-Object { Write-Host (' - '+$_) }; exit 1 }

# Build-wrapper exit propagation regression.  Run the shared helper in a child
# PowerShell process because Fail-Build intentionally terminates its process.
# A controlled cmd.exe exit 37 must propagate exactly and code after the helper
# must remain unreachable.
$Shell=(Get-Process -Id $PID).Path
$Temp=Join-Path ([System.IO.Path]::GetTempPath()) ('preloader_l_build_exit_'+[Guid]::NewGuid().ToString('N'))
$Probe=Join-Path $Temp 'probe.ps1'
$Marker=Join-Path $Temp 'unexpected_continuation.txt'
New-Item -ItemType Directory -Path $Temp -Force | Out-Null
try {
    $Common=(Join-Path $Root 'scripts\build_orchestration\build_common.ps1').Replace("'","''")
    $MarkerQuoted=$Marker.Replace("'","''")
    $Body=@"
`$ErrorActionPreference='Stop'
. '$Common'
Invoke-ExternalChecked `$env:ComSpec @('/d','/c','exit /b 37') 'controlled-exit' 99
[System.IO.File]::WriteAllText('$MarkerQuoted','BAD')
exit 0
"@
    [System.IO.File]::WriteAllText($Probe,$Body,[System.Text.UTF8Encoding]::new($false))
    & $Shell -NoProfile -ExecutionPolicy Bypass -File $Probe
    $Rc=$LASTEXITCODE
    if($Rc -ne 37 -or (Test-Path -LiteralPath $Marker)){
        Write-Host 'BUILD_EXIT_PROPAGATION: FAIL'
        Write-Host (' - child exit='+$Rc+' marker='+(Test-Path -LiteralPath $Marker))
        exit 1
    }
} finally {
    Remove-Item -LiteralPath $Temp -Recurse -Force -ErrorAction SilentlyContinue
}
Write-Host 'BUILD_EXIT_PROPAGATION: PASS (controlled exit 37 propagated; continuation blocked)'
Write-Host ('BUILD_ENVIRONMENT: PASS ('+$Names.Count+'/'+$Names.Count+')')
