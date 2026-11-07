# SPDX-License-Identifier: GPL-3.0-only

param([string]$BuildDir = "", [ValidateSet("Release","RelWithDebInfo","Debug")][string]$Config = "Release", [switch]$LauncherOwnsSummary)
& (Join-Path $PSScriptRoot "build_profile.ps1") -Mode "Crt" -BuildDir $BuildDir -Config $Config -LauncherOwnsSummary:$LauncherOwnsSummary
exit $LASTEXITCODE
