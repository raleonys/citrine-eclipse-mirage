# SPDX-License-Identifier: GPL-3.0-only

param([string]$BuildDir = "", [ValidateSet("Release","RelWithDebInfo","Debug")][string]$Config = "Release")
& (Join-Path $PSScriptRoot "build_profile.ps1") -Mode "Golden" -BuildDir $BuildDir -Config $Config
exit $LASTEXITCODE
