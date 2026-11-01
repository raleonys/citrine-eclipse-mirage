$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$target='C:\Users\Skysect\Downloads\anadius64.dll'
$asmPath=Join-Path $root 'anadius-all-functions.asm.txt'
$pseudoPath=Join-Path $root 'anadius-all-functions.pseudo.c'
$targetHash=(Get-FileHash -LiteralPath $target).Hash.ToLowerInvariant()
$asmHash=(Get-FileHash -LiteralPath $asmPath).Hash.ToLowerInvariant()
$pseudoHash=(Get-FileHash -LiteralPath $pseudoPath).Hash.ToLowerInvariant()
if($targetHash -ne '9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'){throw 'Original target hash mismatch'}
if($asmHash -ne '9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42'){throw 'Retained assembly hash mismatch'}
if($pseudoHash -ne '26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'){throw 'Retained pseudocode hash mismatch'}
$asm=Get-Content -LiteralPath $asmPath -Raw
$pseudo=Get-Content -LiteralPath $pseudoPath -Raw
$assemblyParts=[System.Collections.Generic.List[string]]::new()
$pseudoParts=[System.Collections.Generic.List[string]]::new()
$functions=@(foreach($suffix in @('0706c0','070a40','071068','070860','071730','0713bc')){
    $address='0x180'+$suffix
    $a=[regex]::Match($asm,'(?ms)^; '+$address+'.*?(?=^; 0x|\z)').Value
    $p=[regex]::Match($pseudo,'(?ms)^/\* '+$address+'.*?(?=^/\* 0x|\z)').Value
    if(!$a -or !$p){throw "Missing retained function $address"}
    $assemblyParts.Add($a)
    $pseudoParts.Add($p)
    [ordered]@{va=$address;assembly_evidence=[regex]::Match($a,'ev_[a-f0-9]+').Value;pseudocode_evidence=[regex]::Match($p,'ev_[a-f0-9]+').Value}
})
$selectedAsm=Join-Path $PSScriptRoot 'selected-functions.asm.txt'
$selectedPseudo=Join-Path $PSScriptRoot 'selected-functions.pseudo.c'
[System.IO.File]::WriteAllText($selectedAsm,($assemblyParts -join "`n"),[System.Text.UTF8Encoding]::new($false))
[System.IO.File]::WriteAllText($selectedPseudo,($pseudoParts -join "`n"),[System.Text.UTF8Encoding]::new($false))
[ordered]@{
    target_sha256=$targetHash;assembly_sha256=$asmHash;pseudocode_sha256=$pseudoHash
    origin='Bounded extraction of retained REA/Ghidra 12.1.4 evidence for exact original target SHA. Parent verified current REA 6.5.0 aligned catalog and Ghidra candidate availability; no active binary binding and no target execution.'
    functions=$functions
    selected_assembly_sha256=(Get-FileHash -LiteralPath $selectedAsm).Hash.ToLowerInvariant()
    selected_pseudocode_sha256=(Get-FileHash -LiteralPath $selectedPseudo).Hash.ToLowerInvariant()
    limits=@('Normal flow only; no native memory fault or SEH/unwind behavior reconstructed.','Opaque storage addresses are supplied to explicit required effects and never dereferenced or executed as original target addresses.','Original source, vendor/compiler identity and ABI equivalence remain unproven.','Pointer-array count is full 64-bit RDX; unsigned modular endpoint semantics retained. Very long valid loops require suitable provider storage and are not silently truncated.','Free calls are effect boundaries, including NULL calls; callers may bind the recovered free source. No native free provider is supplied by this module.')
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
'locale_free evidence: target/assembly/pseudocode hashes PASS; 6 bounded functions recorded'
