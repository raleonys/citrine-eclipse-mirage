param([string]$BuildDir = "", [ValidateSet("Release","RelWithDebInfo","Debug")][string]$Config = "Release")
& (Join-Path $PSScriptRoot "build_profile.ps1") -Mode "Native" -BuildDir $BuildDir -Config $Config
exit $LASTEXITCODE
