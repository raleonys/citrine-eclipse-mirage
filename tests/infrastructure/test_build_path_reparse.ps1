# Windows-only native negative regression. Requires a normal user-created junction.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '..\..\scripts\build_orchestration\build_common.ps1')
. (Join-Path $PSScriptRoot '..\..\scripts\build_orchestration\build_path_pinning.ps1')
$base = Join-Path ([System.IO.Path]::GetTempPath()) ('surge-build-guard-' + [Guid]::NewGuid().ToString('N'))
$root = Join-Path $base 'repo'
$outside = Join-Path $base 'outside'
$junction = $null
$swapJunction = $null
try {
    New-Item -ItemType Directory -Path (Join-Path $root 'build') -Force | Out-Null
    New-Item -ItemType Directory -Path $outside -Force | Out-Null
    $legit = Join-Path $root 'build\valid'
    if (Get-BuildPathComponentViolation $root $legit) { throw 'Clean absent build leaf rejected' }
    $junction = Join-Path $root 'build\redirect'
    & cmd.exe /d /c ('mklink /J "' + $junction + '" "' + $outside + '"') | Out-Null
    if ($LASTEXITCODE -ne 0) { throw 'Junction fixture creation failed' }
    $reason = Get-BuildPathComponentViolation $root (Join-Path $junction 'nested')
    if (-not $reason -or $reason -notmatch 'reparse') { throw 'Existing junction accepted' }
    $file = Join-Path $root 'build\ordinary.txt'
    [System.IO.File]::WriteAllText($file,'test')
    $reason = Get-BuildPathComponentViolation $root (Join-Path $file 'nested')
    if ($reason -notmatch 'not a directory') { throw 'File path component accepted' }
    $reason = Get-BuildPathComponentViolation $root (Join-Path $outside 'escape')
    if ($reason -notmatch 'outside') { throw 'External path accepted' }
    # Simulate replacement after the old lexical-only precheck.
    $swapPath = Join-Path $root 'build\swapped'
    $savedPath = Join-Path $root 'build\original'
    [System.IO.Directory]::CreateDirectory($swapPath) | Out-Null
    if (Get-BuildPathComponentViolation $root $swapPath) { throw 'Initial valid component rejected' }
    [System.IO.Directory]::Move($swapPath,$savedPath)
    $swapJunction = $swapPath
    & cmd.exe /d /c ('mklink /J "' + $swapJunction + '" "' + $outside + '"') | Out-Null
    if ($LASTEXITCODE -ne 0) { throw 'Substituted junction creation failed' }
    $denied = $false
    try {
        $unexpected = Enter-PinnedBuildPath $root $swapPath
        foreach ($handle in $unexpected) { $handle.Dispose() }
    } catch {
        $denied = $true
    }
    if (-not $denied) { throw 'Substituted junction was accepted' }
    if (Test-Path -LiteralPath (Join-Path $outside '.build.lock')) { throw 'Lock escaped build root' }
    & cmd.exe /d /c ('rmdir "' + $swapJunction + '"') | Out-Null
    if ($LASTEXITCODE -ne 0) { throw 'Substituted junction removal failed' }
    $swapJunction = $null
    [System.IO.Directory]::Move($savedPath,$swapPath)

    # A pinned component must reject rename while handles are live.
    $pinned = Enter-PinnedBuildPath $root $swapPath
    $lock = $null
    $renamed = Join-Path $root 'build\renamed'
    try {
        $renameFailed = $false
        try { [System.IO.Directory]::Move($swapPath,$renamed) }
        catch { $renameFailed = $true }
        if (-not $renameFailed) {
            [System.IO.Directory]::Move($renamed,$swapPath)
            throw 'Live pinned handle allowed rename'
        }
        $lock = Acquire-ExclusiveBuildLock (Join-Path $swapPath '.build.lock')
        if (-not (Test-Path -LiteralPath (Join-Path $swapPath '.build.lock') -PathType Leaf)) {
            throw 'Pinned directory lock missing'
        }
        if (Test-Path -LiteralPath (Join-Path $outside '.build.lock')) { throw 'External build lock created' }
    } finally {
        if ($null -ne $lock) { $lock.Dispose() }
        foreach ($handle in $pinned) { $handle.Dispose() }
    }
    [System.IO.Directory]::Move($swapPath,$renamed)
    [System.IO.Directory]::Move($renamed,$swapPath)
    Write-Host 'BUILD_PATH_REPARSE_NEGATIVES: PASS (Windows fixture only)'
} finally {
    if ($null -ne $swapJunction -and (Test-Path -LiteralPath $swapJunction)) {
        & cmd.exe /d /c ('rmdir "' + $swapJunction + '"') | Out-Null
        if ($LASTEXITCODE -ne 0) { throw 'Substituted junction cleanup failed' }
    }
    if ($null -ne $junction -and (Test-Path -LiteralPath $junction)) {
        # Remove the junction itself, never recursively traverse its target.
        & cmd.exe /d /c ('rmdir "' + $junction + '"') | Out-Null
        if ($LASTEXITCODE -ne 0 -or (Test-Path -LiteralPath $junction)) {
            throw 'Fixture junction cleanup failed; refusing recursive deletion'
        }
    }
    Remove-Item -LiteralPath $base -Force -Recurse -ErrorAction SilentlyContinue
}
