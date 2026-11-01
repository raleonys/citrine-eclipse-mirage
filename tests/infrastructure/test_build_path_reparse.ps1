# SPDX-License-Identifier: GPL-3.0-only

# Windows-only native negative regression. Requires a normal user-created junction.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '..\..\scripts\build_orchestration\build_common.ps1')
$base = Join-Path ([System.IO.Path]::GetTempPath()) ('surge-build-guard-' + [Guid]::NewGuid().ToString('N'))
$root = Join-Path $base 'repo'
$outside = Join-Path $base 'outside'
$junction = $null
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
    Write-Host 'BUILD_PATH_REPARSE_NEGATIVES: PASS (Windows fixture only)'
} finally {
    if ($null -ne $junction -and (Test-Path -LiteralPath $junction)) {
        # Remove the junction itself, never recursively traverse its target.
        & cmd.exe /d /c ('rmdir "' + $junction + '"') | Out-Null
        if ($LASTEXITCODE -ne 0 -or (Test-Path -LiteralPath $junction)) {
            throw 'Fixture junction cleanup failed; refusing recursive deletion'
        }
    }
    Remove-Item -LiteralPath $base -Force -Recurse -ErrorAction SilentlyContinue
}
