param(
    [string]$SourceRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path,
    [string]$ExplicitMinHookSource = ''
)
$ErrorActionPreference='Stop'
$Pin='8af6b4acae5a9388fd742b56fa79ece89d96f823'
$Dest=Join-Path $SourceRoot 'third_party\minhook\upstream'
$CommitFile=Join-Path $SourceRoot 'third_party\minhook\UPSTREAM_COMMIT.txt'
$ManifestFile=Join-Path $SourceRoot 'third_party\minhook\UPSTREAM_MANIFEST.sha256'
$required=@(
 'include\MinHook.h','src\buffer.c','src\buffer.h','src\hook.c','src\trampoline.c','src\trampoline.h',
 'src\hde\hde64.c','src\hde\hde64.h','src\hde\pstdint.h','src\hde\table64.h','LICENSE.txt'
)
function Test-Shape([string]$p) {
    if(-not (Test-Path -LiteralPath $p -PathType Container)){ return $false }
    foreach($rel in $required){ if(-not (Test-Path -LiteralPath (Join-Path $p $rel) -PathType Leaf)){ return $false } }
    return $true
}
$candidates=New-Object System.Collections.Generic.List[string]
if($ExplicitMinHookSource){ $candidates.Add((Resolve-Path $ExplicitMinHookSource).Path) }
$build=Join-Path $SourceRoot 'build'
if(Test-Path $build){
    Get-ChildItem -LiteralPath $build -Directory -Recurse -ErrorAction SilentlyContinue |
      Where-Object { $_.Name -eq 'preloader_l_minhook-src' } |
      ForEach-Object { $candidates.Add($_.FullName) }
}
$src=$null
foreach($c in $candidates){
    if(-not (Test-Shape $c)){ continue }
    $git=Get-Command git.exe -ErrorAction SilentlyContinue
    if(-not $git){ throw 'git.exe is required to prove the cached MinHook commit before vendoring.' }
    $head=(& $git.Source -C $c rev-parse HEAD 2>$null).Trim()
    if($LASTEXITCODE -ne 0){ continue }
    if($head -ne $Pin){ continue }
    $dirty=& $git.Source -C $c status --porcelain --untracked-files=all
    if($LASTEXITCODE -ne 0 -or $dirty){ throw "MinHook cache is not pristine: $c" }
    $src=$c; break
}
if(-not $src){
    throw "No pristine cached MinHook checkout at exact commit $Pin was found. Point -ExplicitMinHookSource at the exact checkout used by the successful Production/Debug build."
}
$tmp="$Dest.__new"
if(Test-Path $tmp){ Remove-Item -LiteralPath $tmp -Recurse -Force }
New-Item -ItemType Directory -Path $tmp | Out-Null
$tracked=@(& $git.Source -C $src ls-tree -r --name-only $Pin)
if($LASTEXITCODE -ne 0 -or $tracked.Count -eq 0){ throw 'Unable to enumerate the pinned MinHook Git tree.' }
foreach($rel in $tracked){
    if([string]::IsNullOrWhiteSpace($rel)){ continue }
    $from=Join-Path $src $rel
    if(-not (Test-Path -LiteralPath $from -PathType Leaf)){ throw "Pinned tracked member missing: $rel" }
    $to=Join-Path $tmp $rel
    $parent=Split-Path -Parent $to
    if(-not (Test-Path -LiteralPath $parent)){ New-Item -ItemType Directory -Path $parent -Force | Out-Null }
    Copy-Item -LiteralPath $from -Destination $to -Force
}
foreach($rel in $required){ if(-not (Test-Path -LiteralPath (Join-Path $tmp $rel) -PathType Leaf)){ throw "Copied tree missing $rel" } }
if(Test-Path $Dest){ Remove-Item -LiteralPath $Dest -Recurse -Force }
Move-Item -LiteralPath $tmp -Destination $Dest
Set-Content -LiteralPath $CommitFile -Value $Pin -Encoding Ascii
$rows=Get-ChildItem -LiteralPath $Dest -File -Recurse | Sort-Object FullName | ForEach-Object {
    $rel=$_.FullName.Substring($Dest.Length+1).Replace('\','/')
    $h=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    "$h  $rel"
}
Set-Content -LiteralPath $ManifestFile -Value $rows -Encoding Ascii
Write-Host "VENDORED_MINHOOK: PASS"
Write-Host "commit=$Pin"
Write-Host "source=$src"
Write-Host "files=$($rows.Count)"
Write-Host "destination=$Dest"
