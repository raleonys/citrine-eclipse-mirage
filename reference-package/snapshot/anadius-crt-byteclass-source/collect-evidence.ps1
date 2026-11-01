param([string]$Target='C:\Users\Skysect\Downloads\anadius64.dll')
$ErrorActionPreference='Stop'
$expected='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
if((Get-FileHash -LiteralPath $Target).Hash.ToLowerInvariant() -ne $expected){throw 'Target identity mismatch'}
$outputs=Split-Path -Parent $PSScriptRoot
$asmPath=Join-Path $outputs 'anadius-all-functions.asm.txt'
$pseudoPath=Join-Path $outputs 'anadius-all-functions.pseudo.c'
$asmHash=(Get-FileHash -LiteralPath $asmPath).Hash.ToLowerInvariant()
$pseudoHash=(Get-FileHash -LiteralPath $pseudoPath).Hash.ToLowerInvariant()
if($asmHash -ne '9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42' -or $pseudoHash -ne '26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'){throw 'Retained evidence identity mismatch'}
$asm=Get-Content -LiteralPath $asmPath -Raw
$pseudo=Get-Content -LiteralPath $pseudoPath -Raw
$excerpts=[Collections.Generic.List[string]]::new()
$functions=@(foreach($va in @('0x18006f934','0x18006f9a8')){
    $am=[regex]::Match($asm,'(?ms)^; '+$va+' .*?(?=^; 0x|\z)')
    $pm=[regex]::Match($pseudo,'(?m)^/\* '+$va+' .*?Evidence (ev_[a-z0-9]+)')
    if(!$am.Success -or !$pm.Success){throw 'Missing retained function'}
    $excerpts.Add($am.Value)
    [ordered]@{va=$va;assembly_evidence=[regex]::Match($am.Value,'Evidence (ev_[a-z0-9]+)').Groups[1].Value;pseudocode_evidence=$pm.Groups[1].Value}
})
Set-Content -LiteralPath (Join-Path $PSScriptRoot 'assembly-evidence.txt') -Value ($excerpts -join "`n") -Encoding UTF8
[ordered]@{target_sha256=$expected;source='Retained REA/Ghidra function evidence, no new active target';assembly_sha256=$asmHash;pseudocode_sha256=$pseudoHash;functions=$functions;scope='Bounded normal-flow source reconstruction and separately labeled new composition';target_execution='none';runtime_equivalence='not established'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
'PASS: retained address/evidence records extracted'
