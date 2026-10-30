Set-StrictMode -Version Latest

function Fail-Build {
    param([string]$Message, [int]$Code = 1)
    if ($Code -lt 1 -or $Code -gt 255) { $Code = 1 }
    Write-Host ""
    Write-Host "ERROR: $Message"
    exit $Code
}

# Local build is not an official release/promotion transaction.
# No Library access, G0 release intent, or publication receipt is required to compile.
# Retain toolchain, build-directory confinement, source/build failures and binary QA.
function Assert-DevelopmentBuildPreflight {
    $root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
    if (-not (Test-Path -LiteralPath (Join-Path $root 'CMakeLists.txt') -PathType Leaf)) {
        throw 'DEVELOPMENT BUILD BLOCKED: CMakeLists.txt missing.'
    }
}

function Resolve-BoundExecutable {
    param([string]$Name)
    $cmd = Get-Command $Name -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $cmd) { Fail-Build "$Name bulunamadi." 127 }
    return $cmd.Source
}


# Lexical confinement does not resolve existing Windows junction/symlink segments.
# Reject them before constructing build output. This is not a handle-pinned
# protection against a concurrent path swap by another local process.
function Get-BuildPathComponentViolation {
    param([string]$Root,[string]$Candidate)
    $rootPath = [System.IO.Path]::GetFullPath($Root).TrimEnd([System.IO.Path]::DirectorySeparatorChar,[System.IO.Path]::AltDirectorySeparatorChar)
    $candidatePath = [System.IO.Path]::GetFullPath($Candidate)
    $prefix = $rootPath + [System.IO.Path]::DirectorySeparatorChar
    if (-not $candidatePath.StartsWith($prefix,[System.StringComparison]::OrdinalIgnoreCase)) {
        return 'Candidate is outside the repository root.'
    }
    $relative = $candidatePath.Substring($prefix.Length)
    $parts = $relative.Split([char[]]@([System.IO.Path]::DirectorySeparatorChar,[System.IO.Path]::AltDirectorySeparatorChar),[System.StringSplitOptions]::RemoveEmptyEntries)
    $current = $rootPath
    try {
        $rootItem = Get-Item -LiteralPath $current -Force -ErrorAction Stop
        if (($rootItem.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
            return 'Repository root is a reparse point.'
        }
        foreach ($part in $parts) {
            # Enumerating the parent also detects dangling junction/link directory
            # entries that Test-Path can report as missing.
            $matches = @(Get-ChildItem -LiteralPath $current -Force -ErrorAction Stop |
                Where-Object { [string]::Equals($_.Name,$part,[System.StringComparison]::OrdinalIgnoreCase) })
            if ($matches.Count -eq 0) { return $null }
            if ($matches.Count -ne 1) { return 'Ambiguous build path component.' }
            $item = $matches[0]
            if (($item.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
                return 'Build path contains a junction or other reparse point: ' + $item.FullName
            }
            if (-not $item.PSIsContainer) {
                return 'Build path component is not a directory: ' + $item.FullName
            }
            $current = Join-Path $current $part
        }
    } catch {
        return 'Unable to verify build path components: ' + $_.Exception.Message
    }
    return $null
}

function Resolve-ConfinedBuildDirectory {
    param([string]$Root, [string]$RelativePath, [string]$Label)
    if ([string]::IsNullOrWhiteSpace($RelativePath)) { Fail-Build "$Label bos olamaz." 64 }
    if ([System.IO.Path]::IsPathRooted($RelativePath)) { Fail-Build "$Label mutlak yol olamaz: $RelativePath" 64 }
    $buildRoot = [System.IO.Path]::GetFullPath((Join-Path $Root 'build'))
    $candidate = [System.IO.Path]::GetFullPath((Join-Path $Root $RelativePath))
    $prefix = $buildRoot.TrimEnd([System.IO.Path]::DirectorySeparatorChar,[System.IO.Path]::AltDirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar
    if (-not $candidate.StartsWith($prefix,[System.StringComparison]::OrdinalIgnoreCase)) {
        Fail-Build "$Label repository build/ siniri disina cikiyor: $RelativePath" 64
    }
    if ($candidate.Equals($buildRoot,[System.StringComparison]::OrdinalIgnoreCase)) {
        Fail-Build "$Label build kokunun kendisi olamaz." 64
    }
    $componentViolation = Get-BuildPathComponentViolation $Root $candidate
    if ($componentViolation) { Fail-Build "$Label reparse path check failed: $componentViolation" 64 }
    return $candidate
}

function Find-VcVars64 {
    $vswhereCandidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'),
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\vswhere.exe')
    )
    $vswhere = $null
    foreach ($c in $vswhereCandidates) {
        if ($c -and (Test-Path -LiteralPath $c)) { $vswhere = $c; break }
    }
    if (-not $vswhere) { Fail-Build 'vswhere.exe bulunamadi.' 101 }
    $vsPath = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | Select-Object -First 1)
    if (-not $vsPath) { Fail-Build 'MSVC C++ x64 toolset bulunamadi.' 102 }
    $vcvars = Join-Path $vsPath 'VC\Auxiliary\Build\vcvars64.bat'
    if (-not (Test-Path -LiteralPath $vcvars)) { Fail-Build "vcvars64.bat bulunamadi: $vcvars" 103 }
    return @($vsPath,$vcvars)
}

function Import-VcVarsEnvironment {
    param([string]$CmdExe,[string]$VcVars)
    $command = 'call "' + $VcVars + '" >nul && set'
    $lines = & $CmdExe /d /s /c $command
    $rc = $LASTEXITCODE
    if ($rc -ne 0) { Fail-Build "vcvars64.bat basarisiz (exit $rc)." $rc }
    foreach ($line in $lines) {
        $idx = $line.IndexOf('=')
        if ($idx -le 0) { continue }
        $name = $line.Substring(0,$idx)
        $value = $line.Substring($idx+1)
        [Environment]::SetEnvironmentVariable($name,$value,'Process')
    }
}

function Clear-BuildSelectorEnvironment {
    foreach ($name in @('CL','_CL_','LINK','_LINK_','LINK_REPRO','CC','CXX','CMAKE_GENERATOR','CMAKE_GENERATOR_INSTANCE','CMAKE_TOOLCHAIN_FILE')) {
        [Environment]::SetEnvironmentVariable($name,$null,'Process')
    }
}

function Get-ToolRecord {
    param([string]$Label,[string]$Path)
    $item = Get-Item -LiteralPath $Path
    $hash = (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
    $version = $item.VersionInfo.FileVersion
    if ([string]::IsNullOrWhiteSpace($version)) { $version = '<none>' }
    return "$Label|path=$Path|file_version=$version|sha256=$hash"
}

function Write-ToolchainProvenance {
    param(
        [string]$Path,[string]$VsPath,[string]$VcVars,
        [string]$CMakeExe,[string]$PythonExe,[string]$ClExe,[string]$LinkExe,[string]$NMakeExe
    )
    $lines = @(
        'PRELOADER_L_TOOLCHAIN_PROVENANCE_V1',
        "VS_INSTALLATION=$VsPath",
        "VCVARS64=$VcVars",
        (Get-ToolRecord 'cmake' $CMakeExe),
        (Get-ToolRecord 'python' $PythonExe),
        (Get-ToolRecord 'cl' $ClExe),
        (Get-ToolRecord 'link' $LinkExe),
        (Get-ToolRecord 'nmake' $NMakeExe)
    )
    $lines | Set-Content -LiteralPath $Path -Encoding UTF8
}

function Acquire-ExclusiveBuildLock {
    param([string]$Path)
    try {
        $parent = Split-Path -Parent $Path
        New-Item -ItemType Directory -Path $parent -Force | Out-Null
        return [System.IO.File]::Open($Path,[System.IO.FileMode]::OpenOrCreate,[System.IO.FileAccess]::ReadWrite,[System.IO.FileShare]::None)
    } catch {
        Fail-Build "Baska bir production build bu checkout'u kullaniyor: $Path" 73
    }
}

function Invoke-ExternalChecked {
    param([string]$Exe,[string[]]$Arguments,[string]$Label,[int]$FallbackCode = 1)
    & $Exe @Arguments
    $rc = $LASTEXITCODE
    if ($rc -ne 0) {
        if ($rc -lt 1 -or $rc -gt 255) { $rc = $FallbackCode }
        Fail-Build "$Label basarisiz (exit $rc)." $rc
    }
}
