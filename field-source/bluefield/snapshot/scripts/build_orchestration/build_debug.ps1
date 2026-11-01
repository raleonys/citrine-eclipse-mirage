# SPDX-License-Identifier: GPL-3.0-only
[CmdletBinding()]
param()
& (Join-Path $PSScriptRoot 'build_driver.ps1') -Profile 'DEBUG'
exit $LASTEXITCODE
