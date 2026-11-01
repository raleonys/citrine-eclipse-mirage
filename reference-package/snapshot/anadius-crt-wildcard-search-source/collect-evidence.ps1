$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$expectedAssembly='9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42'
$expectedPseudo='26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'
$assembly=Join-Path $root 'anadius-all-functions.asm.txt'
$pseudo=Join-Path $root 'anadius-all-functions.pseudo.c'
if((Get-FileHash -LiteralPath $assembly).Hash.ToLowerInvariant() -ne $expectedAssembly){throw 'retained assembly digest mismatch'}
if((Get-FileHash -LiteralPath $pseudo).Hash.ToLowerInvariant() -ne $expectedPseudo){throw 'retained pseudo digest mismatch'}
$addresses=@('18006e984','18006e800','180077168','18006004c','18006e430','1800768f0','180058b04','180066c1c')
$assemblyText=[System.IO.File]::ReadAllText($assembly)
$pseudoText=[System.IO.File]::ReadAllText($pseudo)
$functions=@()
$selectedAssembly=@()
$selectedPseudo=@()
foreach($address in $addresses){
    $asmMatch=[regex]::Match($assemblyText,"(?ms)^; 0x$address [^\r\n]*\r?\n.*?(?=^; 0x|\z)")
    $pseudoMatch=[regex]::Match($pseudoText,"(?ms)^/\* 0x$address [^\r\n]*\r?\n.*?(?=^/\* 0x|\z)")
    if(-not $asmMatch.Success -or -not $pseudoMatch.Success){throw "retained function missing: $address"}
    $selectedAssembly+=$asmMatch.Value.TrimEnd()
    $selectedPseudo+=$pseudoMatch.Value.TrimEnd()
    $functions+=[ordered]@{address="0x$address";role=$(if($address -eq '18006e984'){'reconstructed'}else{'dependency-only'});assembly_evidence=[regex]::Match($asmMatch.Value,'Evidence (ev_[a-f0-9]+)').Groups[1].Value;pseudo_evidence=[regex]::Match($pseudoMatch.Value,'Evidence (ev_[a-f0-9]+)').Groups[1].Value}
}
[System.IO.File]::WriteAllText((Join-Path $PSScriptRoot 'selected-assembly.txt'),($selectedAssembly -join "`r`n`r`n"))
[System.IO.File]::WriteAllText((Join-Path $PSScriptRoot 'selected-pseudo.txt'),($selectedPseudo -join "`r`n`r`n"))
[ordered]@{
 artifact='anadius64.dll';sha256='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';image_base='0x180000000';
 method='Static inspection of retained hash-bound REA/Ghidra assembly and pseudocode; no new native target session or execution';
 retained=[ordered]@{assembly_sha256=$expectedAssembly;pseudo_sha256=$expectedPseudo};functions=$functions;
 decisive_instructions=@(
  '06e9c4..06e9e4: backward scan calls DBCS previous helper until start or separator bitmask 0x200000000801 over unsigned byte minus 0x2f',
  '06e9e6..06ea08: colon is accepted only at pattern+1; other colon positions immediately append the literal pattern',
  '06ea29..06ea52: separator prefix includes separator byte; nonseparator prefix length is zero',
  '06ea61..06eac1 and 06eb58..06ebb4: separate LocaleUpdate and 066c1c decision/cleanup regions before both conversion directions',
  '06eadf/06eaeb/06eaf4: conversion full-EAX failure selects NULL then calls FindFirstFileExW with remaining arguments all zero',
  '06eafd..06eb28: INVALID_HANDLE_VALUE falls back to actual append, then owned wide allocation is freed',
  '06eb2d..06eb36: initial vector length is signed (end-begin) SAR 3',
  '06ebcf/06ebd4/06ebd8: conversion failure selects NULL, then immediately dereferences it; fault recovery is not reconstructed',
  '06ebd8..06ebfa: skip exactly dot and dotdot; no file attribute filter',
  '06ec05..06ec1e: actual append receives filename, original pattern and preserved prefix; owned narrow result is freed after success',
  '06ec85..06ecad: append failure frees owned narrow result, closes handle, frees owned wide result, returns low DWORD error',
  '06ec2a/06ec33: FindNextFileW tests full EAX; zero ends enumeration irrespective of LastError',
  '06ec3b..06ec62: if count changed, sort only appended slots using actual comparator identity 06e41c, width 8',
  '06ec67..06ec83: sort precedes FindClose, followed by owned wide free and return zero'
 );
 dependencies=@('Actual append_06e800 from sibling wildcard storage source','Mandatory DBCS previous_077168','Mandatory complete inlined locale selection/cleanup region','Mandatory conversions 06004c and 06e430','Mandatory FindFirstFileExW/FindNextFileW/FindClose providers','Mandatory sort 0768f0 with comparator 06e41c; no lexical sorting assumption');
 source_choices=@('Opaque uint64 addresses in provider-owned storage instead of forging host pointers','Nonprogressing or out-of-range previous provider and narrow NULL fault domain throw domain_error; no original error return or cleanup invented','Search source stores explicit Conversion data/owned/status instead of compiler stack-buffer layout');
 limitations=@('Normal-flow reconstruction only; SEH, cookie checking and exact original ABI/layout are not reconstructed','Locale, conversions, DBCS previous, find APIs and sort remain mandatory providers in this module','No native enumeration, filesystem changes, original or rebuilt DLL execution','Fixture sort records boundaries only, does not establish lexical order or reconstruct the sorting algorithm','Tests establish model invariants, not binary runtime equivalence')
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
'PASS retained excerpts and evidence manifest'
