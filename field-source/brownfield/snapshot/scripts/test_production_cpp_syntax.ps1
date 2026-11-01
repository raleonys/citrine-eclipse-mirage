param([string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path)
$ErrorActionPreference = 'Stop'
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) { Write-Host 'ACTIVE_PRODUCTION_CPP_SYNTAX_WINDOWS: FAIL - cl.exe unavailable'; exit 2 }

# Single source of truth: derive the exact normalized C++ TU list from the
# preloader_l CMake target instead of maintaining a second hand-copied list.
$CMakePath = Join-Path $Root 'CMakeLists.txt'
$CMake = Get-Content $CMakePath -Raw
$Block = [regex]::Match(
    $CMake,
    'add_library\(preloader_l SHARED\s*(?<body>.*?)\r?\n\)',
    [System.Text.RegularExpressions.RegexOptions]::Singleline)
if (-not $Block.Success) { Write-Host 'ACTIVE_PRODUCTION_CPP_SYNTAX_WINDOWS: FAIL - normalized CMake block'; exit 3 }
$Sources = @(
    [regex]::Matches(
        $Block.Groups['body'].Value,
        '(?m)^\s*(src/[A-Za-z0-9_./-]+\.cpp)\s*$') |
        ForEach-Object { $_.Groups[1].Value }
)
if ($Sources.Count -eq 0) { Write-Host 'ACTIVE_PRODUCTION_CPP_SYNTAX_WINDOWS: FAIL - empty normalized source list'; exit 4 }

$Common = @('/nologo','/std:c++17','/permissive-','/W4','/WX','/EHsc','/Zs',('/I' + (Join-Path $Root 'include')),('/I' + (Join-Path $Root 'third_party\minhook\upstream\include')),'/DWIN32_LEAN_AND_MEAN','/DNOMINMAX','/DPRELOADER_L_RELEASE_BOOTSTRAP=1','/DPRELOADER_L_TRACE_DIAGNOSTICS=0')
$Failed = @()
foreach ($Rel in $Sources) {
  $File = Join-Path $Root $Rel
  & cl.exe @Common $File | Out-Host
  if ($LASTEXITCODE -ne 0) { $Failed += $Rel }
}
if ($Failed.Count -ne 0) {
  Write-Host 'ACTIVE_PRODUCTION_CPP_SYNTAX_WINDOWS: FAIL'
  $Failed | ForEach-Object { Write-Host (' - ' + $_) }
  exit 1
}
Write-Host ('ACTIVE_PRODUCTION_CPP_SYNTAX_WINDOWS: PASS (' + $Sources.Count + '/' + $Sources.Count + ')')
