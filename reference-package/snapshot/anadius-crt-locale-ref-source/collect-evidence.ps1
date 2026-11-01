$ErrorActionPreference='Stop'
$outputs=Split-Path -Parent $PSScriptRoot
$dll='C:\Users\Skysect\Downloads\anadius64.dll'
$asm=Join-Path $outputs 'anadius-all-functions.asm.txt'
$pseudo=Join-Path $outputs 'anadius-all-functions.pseudo.c'
$expectedDll='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
$expectedAsm='9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42'
$expectedPseudo='26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'
function Hash([string]$path){(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()}
if((Hash $dll) -ne $expectedDll){throw 'Target identity mismatch'}
if((Hash $asm) -ne $expectedAsm){throw 'Retained assembly identity mismatch'}
if((Hash $pseudo) -ne $expectedPseudo){throw 'Retained pseudocode identity mismatch'}
$a=Get-Content -Raw -LiteralPath $asm
$p=Get-Content -Raw -LiteralPath $pseudo
$addresses=@('180070634','1800708c0','180070838','180070898')
$assembly=@();$pseudocode=@();$functions=@()
foreach($address in $addresses){
    $am=[regex]::Match($a,'(?ms)^; 0x'+$address+' .*?(?=^; 0x|\z)').Value
    $pm=[regex]::Match($p,'(?ms)^/\* 0x'+$address+' .*?(?=^/\* 0x|\z)').Value
    if(!$am -or !$pm){throw "Missing evidence: $address"}
    $assembly+=$am;$pseudocode+=$pm
    $functions+=[ordered]@{va='0x'+$address;assembly_evidence=[regex]::Match($am,'ev_[0-9a-f]+').Value;pseudocode_evidence=[regex]::Match($pm,'ev_[0-9a-f]+').Value}
}
$selectedAsm=Join-Path $PSScriptRoot 'selected.asm.txt'
$selectedPseudo=Join-Path $PSScriptRoot 'selected.pseudo.c'
[IO.File]::WriteAllText($selectedAsm,($assembly -join "`n"),[Text.UTF8Encoding]::new($false))
[IO.File]::WriteAllText($selectedPseudo,($pseudocode -join "`n"),[Text.UTF8Encoding]::new($false))
[ordered]@{target_sha256=$expectedDll;assembly_sha256=$expectedAsm;pseudocode_sha256=$expectedPseudo;origin='Bounded extraction of previously retained REA/Ghidra static function evidence. Parent confirmed current aligned REA6.5.0 server and available Ghidra12.1.4; no new target analysis or execution.';functions=$functions;selected_assembly_sha256=(Hash $selectedAsm);selected_pseudocode_sha256=(Hash $selectedPseudo);anchors=@('070634..070674 / 0708d1..070916: first +10, optional e0/f0/e8/100 reference order','070674..0706b1 / 070916..070955: six records, cursor+38, sentinel cursor-10, narrow cursor-18, counter cursor-8, stride20','0706b3..0706ba / 070957..07095e: load time+120 after prior counter effects','070838..07085e / 070898..0708bc: NULL/static identity INT_MAX, XADD +15c, full32-bit new return');limits=@('Original source/compiler/runtime/ABI equality unproven','Atomic storage and address providers remain abstract and must uphold live-address and atomic ordering semantics','No heap free, native locale, target execution or SEH/unwind recovery claim','add locale NULL is an original fault-domain input, not a new null no-op')} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
