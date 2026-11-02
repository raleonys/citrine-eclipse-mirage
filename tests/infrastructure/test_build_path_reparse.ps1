# SPDX-License-Identifier: GPL-3.0-only

# Real Windows fixtures only. All junction targets and ACL changes are owned temp paths.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '..\..\scripts\build_orchestration\build_common.ps1')
. (Join-Path $PSScriptRoot '..\..\scripts\build_orchestration\build_path_pinning.ps1')
$script:checks = 0
function Require([bool]$Condition,[string]$Message) {
    $script:checks++
    if (-not $Condition) { throw $Message }
}
function Invoke-FixtureActor([scriptblock]$Action,[object[]]$Arguments) {
    $actor = Start-Job -ScriptBlock $Action -ArgumentList $Arguments
    try {
        if ($null -eq (Wait-Job -Job $actor -Timeout 10)) { throw 'Fixture actor timed out' }
        $answer = @(Receive-Job -Job $actor -ErrorAction Stop)
        Require ($actor.State -eq 'Completed' -and $answer.Count -eq 1) 'Fixture actor did not return one completed result'
        Require ($answer[0].ActorPid -ne $PID) 'Race must execute in a different Windows process'
        return $answer[0]
    } finally {
        if ($actor.State -eq 'Running') { Stop-Job -Job $actor }
        Remove-Job -Job $actor -Force
    }
}
$renameActor = {
    param([string]$PathsJson)
    $ErrorActionPreference = 'Stop'
    $results = @()
    foreach ($path in (ConvertFrom-Json $PathsJson)) {
        $moved = $path + '.rename-control'
        $denied = $false
        $code = 0
        try { [System.IO.Directory]::Move($path,$moved) }
        catch {
            $code = $_.Exception.GetBaseException().HResult -band 0xffff
            if ($code -ne 5 -and $code -ne 32) { throw }
            $denied = $true
        }
        if (-not $denied) { [System.IO.Directory]::Move($moved,$path) }
        $results += [pscustomobject]@{ Path=$path; Denied=$denied; NativeCode=$code }
    }
    [pscustomobject]@{ ActorPid=$PID; Results=$results }
}
function Check-Renames([string[]]$Paths,[bool]$Denied) {
    $answer = Invoke-FixtureActor $renameActor @((ConvertTo-Json -InputObject $Paths -Compress))
    $rows = @($answer.Results)
    Require ($rows.Count -eq $Paths.Count) 'Actor omitted a selected path component'
    for ($index=0; $index -lt $Paths.Count; $index++) {
        Require ($rows[$index].Path -eq $Paths[$index] -and $rows[$index].Denied -eq $Denied) 'Selected-component rename result differs from the expected sharing boundary'
    }
    Write-Host ('rename_actor_pid=' + $answer.ActorPid + ' denied=' + $Denied + ' components=' + $rows.Count)
}
function Check-ParentRelease([string]$Root) {
    $build = Join-Path $Root 'build'
    [System.IO.Directory]::Move($build,($build + '.released'))
    [System.IO.Directory]::Move(($build + '.released'),$build)
    [System.IO.Directory]::Move($Root,($Root + '.released'))
    [System.IO.Directory]::Move(($Root + '.released'),$Root)
    $script:checks+=2
}
function Remove-FixtureJunction([string]$Path) {
    if (-not $Path) { return }
    $item = Get-Item -LiteralPath $Path -Force -ErrorAction SilentlyContinue
    if ($null -eq $item) { return }
    if (($item.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -eq 0) {
        throw 'Refusing junction cleanup on an unexpected ordinary entry'
    }
    & cmd.exe /d /c ('rmdir "' + $Path + '"') | Out-Null
    if ($LASTEXITCODE -ne 0 -or (Test-Path -LiteralPath $Path)) {
        throw 'Fixture junction cleanup failed; refusing recursive deletion'
    }
}
function Check-ProfileCleanup([string]$ProfilePath) {
    $parseTokens = $null
    $parseErrors = $null
    $tree = [System.Management.Automation.Language.Parser]::ParseFile($ProfilePath,[ref]$parseTokens,[ref]$parseErrors)
    Require ($parseErrors.Count -eq 0) 'Build profile syntax is invalid'
    $outer = @($tree.EndBlock.Statements | Where-Object { $_ -is [System.Management.Automation.Language.TryStatementAst] })
    Require ($outer.Count -eq 1 -and $null -ne $outer[0].Finally) 'Expected the actual profile cleanup block'
    $text = $outer[0].Finally.Extent.Text
    Require ($text.StartsWith('{') -and $text.EndsWith('}')) 'Unexpected cleanup AST extent'
    $cleanup = [scriptblock]::Create($text.Substring(1,$text.Length-2))
    Add-Type -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.IO;
public sealed class BuildPinningCleanupProbe : IDisposable {
    public static readonly List<string> Trace = new List<string>();
    private readonly string name;
    private readonly bool fail;
    public BuildPinningCleanupProbe(string name, bool fail) { this.name=name; this.fail=fail; }
    public void Dispose() {
        Trace.Add(name);
        if (fail) throw new IOException("injected lock release failure");
    }
}
'@
    $EnvironmentBefore = @{}
    Get-ChildItem Env: | ForEach-Object { $EnvironmentBefore[$_.Name] = $_.Value }
    $marker = 'SURGE_TEST_PATH_CLEANUP_' + [Guid]::NewGuid().ToString('N')
    [Environment]::SetEnvironmentVariable($marker,'temporary','Process')
    $Lock = New-Object BuildPinningCleanupProbe('lock',$true)
    $PinnedDirectories = New-Object 'System.Collections.Generic.List[System.IDisposable]'
    $PinnedDirectories.Add((New-Object BuildPinningCleanupProbe('parent',$false)))
    $PinnedDirectories.Add((New-Object BuildPinningCleanupProbe('leaf',$false)))
    $expected = $false
    try {
        try { & $cleanup }
        catch {
            if ($_.Exception.GetBaseException().Message -ne 'injected lock release failure') { throw }
            $expected = $true
        }
        Require $expected 'Lock disposal fault did not propagate'
        Require (([BuildPinningCleanupProbe]::Trace -join ',') -eq 'lock,leaf,parent') 'Actual profile cleanup skipped or reordered pins after lock failure'
        Require ($PinnedDirectories.Count -eq 0) 'Actual profile cleanup retained disposed handles'
        Require ($null -eq [Environment]::GetEnvironmentVariable($marker,'Process')) 'Environment was not restored after the cleanup fault'
    } finally {
        [Environment]::SetEnvironmentVariable($marker,[NullString]::Value,'Process')
        Exit-PinnedBuildPath $PinnedDirectories
    }
}
$base = Join-Path ([System.IO.Path]::GetTempPath()) ('surge-build-guard-' + [Guid]::NewGuid().ToString('N'))
$root = Join-Path $base 'repo'
$outside = Join-Path $base 'outside'
$junction = $null
$swapJunction = $null
$pinned = $null
$lock = $null
try {
    [System.IO.Directory]::CreateDirectory((Join-Path $root 'build')) | Out-Null
    [System.IO.Directory]::CreateDirectory($outside) | Out-Null
    $sentinel = Join-Path $outside 'sentinel.txt'
    [System.IO.File]::WriteAllText($sentinel,'outside sentinel')
    $sentinelTime = [System.IO.File]::GetLastWriteTimeUtc($sentinel)
    function Check-Outside {
        Require ([System.IO.File]::ReadAllText($sentinel) -eq 'outside sentinel') 'Outside sentinel was changed or removed'
        Require ([System.IO.File]::GetLastWriteTimeUtc($sentinel) -eq $sentinelTime) 'Outside sentinel was rewritten'
        Require (([System.IO.Directory]::GetFiles($outside)).Count -eq 1) 'Unexpected lock or output escaped to the outside target'
        Require (([System.IO.Directory]::GetDirectories($outside)).Count -eq 0) 'Unexpected directory escaped to the outside target'
    }
    $legit = Join-Path $root 'build\valid'
    Require (-not (Get-BuildPathComponentViolation $root $legit)) 'Clean absent build leaf rejected'
    $junction = Join-Path $root 'build\redirect'
    & cmd.exe /d /c ('mklink /J "' + $junction + '" "' + $outside + '"') | Out-Null
    Require ($LASTEXITCODE -eq 0) 'Junction fixture creation failed'
    $reason = Get-BuildPathComponentViolation $root (Join-Path $junction 'nested')
    Require ($reason -match 'reparse') 'Existing junction accepted'
    $rejected = $false
    try { $pinned = Enter-PinnedBuildPath $root (Join-Path $junction 'nested') }
    catch {
        if ($_.Exception.GetBaseException().Message -notmatch 'reparse point') { throw }
        $rejected = $true
    } finally { Exit-PinnedBuildPath $pinned; $pinned=$null }
    Require $rejected 'Intermediate junction was followed before child creation'
    Check-Outside
    Check-ParentRelease $root
    $file = Join-Path $root 'build\ordinary.txt'
    [System.IO.File]::WriteAllText($file,'test')
    Require ((Get-BuildPathComponentViolation $root (Join-Path $file 'nested')) -match 'not a directory') 'File path component accepted'
    Require ((Get-BuildPathComponentViolation $root (Join-Path $outside 'escape')) -match 'outside') 'External path accepted'

    # Positive creation, real output/lock and release controls.
    $created = Join-Path $root 'build\created\a\b'
    $pinned = Enter-PinnedBuildPath $root $created
    try {
        $lock = Acquire-ExclusiveBuildLock (Join-Path $created '.build.lock')
        [System.IO.File]::WriteAllText((Join-Path $created 'output.txt'),'local output')
        Require ([System.IO.File]::ReadAllText((Join-Path $created 'output.txt')) -eq 'local output') 'Pinned path blocks legitimate output'
    } finally {
        if ($null -ne $lock) { $lock.Dispose(); $lock=$null }
        Exit-PinnedBuildPath $pinned
        $pinned=$null
    }
    Check-Outside

    # Parent observes a valid path; a second process replaces it before use.
    $swapPath = Join-Path $root 'build\swapped'
    $savedPath = Join-Path $root 'build\original'
    [System.IO.Directory]::CreateDirectory($swapPath) | Out-Null
    Require (-not (Get-BuildPathComponentViolation $root $swapPath)) 'Initial valid component rejected'
    $swapJunction = $swapPath
    $swapped = Invoke-FixtureActor {
        param([string]$Source,[string]$Saved,[string]$Target)
        $ErrorActionPreference = 'Stop'
        [System.IO.Directory]::Move($Source,$Saved)
        & cmd.exe /d /c ('mklink /J "' + $Source + '" "' + $Target + '"') | Out-Null
        if ($LASTEXITCODE -ne 0) { throw 'Actor junction creation failed' }
        [pscustomobject]@{ ActorPid=$PID; Swapped=$true }
    } @($swapPath,$savedPath,$outside)
    Require $swapped.Swapped 'Actor did not perform the validation/use substitution'

    # RED control: the old precheck + lock sequence follows the replaced entry.
    # Both resulting files are deliberately inside our owned outside fixture.
    $lock = Acquire-ExclusiveBuildLock (Join-Path $swapPath '.build.lock')
    try {
        [System.IO.File]::WriteAllText((Join-Path $swapPath 'baseline-output.txt'),'old sequence')
        Require ([System.IO.File]::Exists((Join-Path $outside '.build.lock'))) 'Old-sequence lock escape control was not exercised'
        Require ([System.IO.File]::Exists((Join-Path $outside 'baseline-output.txt'))) 'Old-sequence write escape control was not exercised'
    } finally { $lock.Dispose(); $lock=$null }
    [System.IO.File]::Delete((Join-Path $outside '.build.lock'))
    [System.IO.File]::Delete((Join-Path $outside 'baseline-output.txt'))
    Check-Outside
    Write-Host ('swap_actor_pid=' + $swapped.ActorPid + ' old_sequence_escape=observed')

    # GREEN: the actual pin -> lock -> output order must reject before use.
    $rejected = $false
    try {
        $pinned = Enter-PinnedBuildPath $root $swapPath
        $lock = Acquire-ExclusiveBuildLock (Join-Path $swapPath '.build.lock')
        [System.IO.File]::WriteAllText((Join-Path $swapPath 'guard-output.txt'),'unexpected')
    } catch {
        if ($_.Exception.GetBaseException().Message -notmatch 'reparse point') { throw }
        $rejected = $true
    } finally {
        if ($null -ne $lock) { $lock.Dispose(); $lock=$null }
        Exit-PinnedBuildPath $pinned
        $pinned=$null
    }
    Require $rejected 'Substituted junction was accepted'
    Check-Outside
    Check-ParentRelease $root
    Remove-FixtureJunction $swapJunction
    $swapJunction=$null
    [System.IO.Directory]::Move($savedPath,$swapPath)
    Check-Outside

    # Without pins every selected entry can move. With live pins the second
    # process must be denied; after release the same operations succeed again.
    $selected = Join-Path $root 'build\pinned\leaf'
    [System.IO.Directory]::CreateDirectory($selected) | Out-Null
    $paths = @($root,(Join-Path $root 'build'),(Join-Path $root 'build\pinned'),$selected)
    Check-Renames $paths $false
    $pinned = Enter-PinnedBuildPath $root $selected
    $captured = @($pinned.ToArray())
    try { Check-Renames $paths $true }
    finally { Exit-PinnedBuildPath $pinned; Exit-PinnedBuildPath $pinned; $pinned=$null }
    Require ((@($captured | Where-Object { -not $_.IsClosed })).Count -eq 0) 'A directory handle survived explicit release'
    Check-Renames $paths $false
    Check-Outside

    # A real ACL denial after parent acquisition must unwind the partial chain.
    # Restore the owned fixture's ACL through .NET, without provider attribute reads.
    $blocked = Join-Path $root 'build\denied'
    [System.IO.Directory]::CreateDirectory($blocked) | Out-Null
    $savedAcl = [System.IO.Directory]::GetAccessControl($blocked)
    $restrictedAcl = [System.IO.Directory]::GetAccessControl($blocked)
    $userSid = [System.Security.Principal.WindowsIdentity]::GetCurrent().User
    $deny = New-Object System.Security.AccessControl.FileSystemAccessRule(
        $userSid,[System.Security.AccessControl.FileSystemRights]::ReadAttributes,
        [System.Security.AccessControl.AccessControlType]::Deny)
    $restrictedAcl.AddAccessRule($deny)
    $denied = $false
    try {
        [System.IO.Directory]::SetAccessControl($blocked,$restrictedAcl)
        try { $pinned = Enter-PinnedBuildPath $root $blocked }
        catch {
            $native = $_.Exception.GetBaseException()
            if ($native -isnot [System.ComponentModel.Win32Exception] -or $native.NativeErrorCode -ne 5) { throw }
            $denied = $true
        }
    } finally {
        Exit-PinnedBuildPath $pinned
        $pinned=$null
        [System.IO.Directory]::SetAccessControl($blocked,$savedAcl)
    }
    Require $denied 'Owned ACL denial was not observed as ERROR_ACCESS_DENIED'
    Check-ParentRelease $root
    Check-Outside

    # A non-directory fails after opening the earlier components, without
    # leaving handles behind or creating children through that entry.
    $rejected = $false
    try { $pinned = Enter-PinnedBuildPath $root (Join-Path $file 'nested') }
    catch {
        if ($_.Exception.GetBaseException().Message -notmatch 'not a directory') { throw }
        $rejected = $true
    } finally { Exit-PinnedBuildPath $pinned; $pinned=$null }
    Require $rejected 'Opened non-directory component was accepted'
    Check-ParentRelease $root
    Check-Outside
    Check-ProfileCleanup (Join-Path $PSScriptRoot '..\..\scripts\build_orchestration\build_profile.ps1')
    Write-Host ('BUILD_PATH_PINNING: PASS cross-process swap/rename, ACL denial, rollback, output confinement; checks=' + $script:checks)
    Write-Host 'BUILD_PATH_REPARSE_NEGATIVES: PASS (Windows fixture only)'
} finally {
    if ($null -ne $lock) { $lock.Dispose() }
    Exit-PinnedBuildPath $pinned
    Remove-FixtureJunction $swapJunction
    Remove-FixtureJunction $junction
    Remove-Item -LiteralPath $base -Force -Recurse -ErrorAction Stop
}
