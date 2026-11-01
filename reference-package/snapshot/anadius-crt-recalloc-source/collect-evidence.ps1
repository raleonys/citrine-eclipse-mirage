$ErrorActionPreference='Stop'
$outputs=Split-Path -Parent $PSScriptRoot
$asmPath=Join-Path $outputs 'anadius-all-functions.asm.txt'
$pseudoPath=Join-Path $outputs 'anadius-all-functions.pseudo.c'
$target='C:\Users\Skysect\Downloads\anadius64.dll'
$expectedTarget='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
$expectedAsm='9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42'
$expectedPseudo='26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'
foreach($item in @(@{path=$target;hash=$expectedTarget},@{path=$asmPath;hash=$expectedAsm},@{path=$pseudoPath;hash=$expectedPseudo})) {
    if((Get-FileHash -LiteralPath $item.path).Hash.ToLowerInvariant() -ne $item.hash){throw 'Evidence digest mismatch'}
}
$asmText=Get-Content -LiteralPath $asmPath -Raw
$pseudoText=Get-Content -LiteralPath $pseudoPath -Raw
$asmSelections=@()
$pseudoSelections=@()
foreach($address in @('180070434','1800773bc')) {
    $asmMatch=[regex]::Match($asmText,"(?ms)^; 0x$address .*?(?=^; 0x|\z)")
    $pseudoMatch=[regex]::Match($pseudoText,"(?ms)^/\* 0x$address .*?(?=^/\* 0x|\z)")
    if(-not $asmMatch.Success -or -not $pseudoMatch.Success){throw "Missing bounded function $address"}
    $asmSelections+=$asmMatch.Value.TrimEnd()
    $pseudoSelections+=$pseudoMatch.Value.TrimEnd()
}
$asmSelections -join "`r`n`r`n" | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'selected-assembly.txt') -Encoding UTF8
$pseudoSelections -join "`r`n`r`n" | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'selected-pseudocode.c') -Encoding UTF8
[ordered]@{
    target_sha256=$expectedTarget
    assembly_sha256=$expectedAsm
    pseudocode_sha256=$expectedPseudo
    origin='Bounded extraction of previously retained REA/Ghidra static function evidence; parent confirmed current aligned REA server and available Ghidra, session closed.'
    functions=@(
        [ordered]@{va='0x180070434';name='_recalloc_base';assembly_evidence='ev_7e1624e0b30bf83e9630135e9326fc52509545478e914f4f92ab1946bd09bc8d';pseudocode_evidence='ev_9575693c4de551ec68c18128cf0492408533bb470914e6324e3ee9dad0e01b73';anchors=@('070451..070471: quotient guard and error 12','070473..070484: optional old HeapSize before product/realloc','07048e: call actual realloc 06e3a0','070496..0704ac: non-null result, unsigned capacity comparison, zero range to 0793d0')},
        [ordered]@{va='0x1800773bc';name='_msize_base';assembly_evidence='ev_75d7961f2f61cf3a76a487e943f4092ac52dd3ebe56702780a872685729102dc';pseudocode_evidence='ev_bd297f0ce14c7ad28c668344e4c9d97f16fe4369dd089690d59d2e8f7167fefb';anchors=@('0773c0..0773d5: NULL test, error22, invalid05c954, -1 return','0773de..0773ee: HeapSize tail jump through IAT18007f018, flags0, heap1800a5a10')}
    )
    selected_assembly_sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'selected-assembly.txt')).Hash.ToLowerInvariant()
    selected_pseudocode_sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'selected-pseudocode.c')).Hash.ToLowerInvariant()
    limits=@('Original runtime/ABI/compiler equivalence unproven','Provider retains valid live byte-array storage and preservation semantics','PTRDIFF_MAX growth guard is a new explicit source support boundary, not original behavior','HeapSize/invalid-parameter/zero providers required; no native API or target execution','No SEH or exception unwind cleanup recovery claim')
} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
