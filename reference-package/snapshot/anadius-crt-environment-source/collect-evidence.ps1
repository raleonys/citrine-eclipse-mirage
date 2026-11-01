$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$pseudoPath=Join-Path $root 'anadius-all-functions.pseudo.c'
$asmPath=Join-Path $root 'anadius-all-functions.asm.txt'
$expectedPseudo='26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'
$expectedAsm='9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42'
if((Get-FileHash -LiteralPath $pseudoPath).Hash.ToLowerInvariant() -ne $expectedPseudo){throw 'Retained pseudo hash mismatch'}
if((Get-FileHash -LiteralPath $asmPath).Hash.ToLowerInvariant() -ne $expectedAsm){throw 'Retained assembly hash mismatch'}
$pseudo=[IO.File]::ReadAllText($pseudoPath)
$assembly=[IO.File]::ReadAllText($asmPath)
$addresses=@('180063a70','180064054','18006f9e4','180063b54','180063d80')
$excerpts=@()
$records=foreach($address in $addresses){
    $pm=[regex]::Match($pseudo,'(?m)^/\* 0x'+$address+' [^\r\n]*Evidence (ev_[0-9a-f]+)')
    $am=[regex]::Match($assembly,'(?ms)^; 0x'+$address+' [^\r\n]*Evidence (ev_[0-9a-f]+).*?(?=^; 0x|\z)')
    if(!$pm.Success -or !$am.Success){throw "Missing retained function $address"}
    $excerpts+=$am.Value.TrimEnd()
    [ordered]@{va='0x'+$address;pseudocode_evidence=$pm.Groups[1].Value;assembly_evidence=$am.Groups[1].Value}
}
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'selected-assembly.txt'),($excerpts -join "`r`n`r`n"),[Text.UTF8Encoding]::new($false))
[ordered]@{
    artifact='anadius64.dll';artifact_sha256='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';preferred_image_base='0x180000000'
    retained_pseudo_sha256=$expectedPseudo;retained_assembly_sha256=$expectedAsm;method='Retained REA/Ghidra function evidence; bounded excerpts, no target execution or new analysis session'
    entries=@($records)
    globals=@([ordered]@{va='0x1800a5288';width=8;meaning='current narrow environment pointer'},[ordered]@{va='0x1800a52a0';width=8;meaning='initial narrow environment pointer'})
    important_calls=@('063a70 calls, in branch-dependent order: 06f5f8, 06f9e4, 063b54, 0668c0','064054 tail JMP 063a70','06f9e4 -> IAT 07f040 GetEnvironmentStringsW / 06cd58 conversion / 06a308 malloc / 0668c0 / IAT 07f038 FreeEnvironmentStringsW','063b54 -> 066848 calloc / 05cff0 strcpy_s / 063d80 free_environment / 05c9a4 Watson / 0668c0','063d80 -> 0668c0')
    limits=@('Normal control flow only; no SEH/unwind reconstruction','Bounded metadata and domain guards are reconstruction-only interfaces','No native environment, conversion, heap or fatal-handler backend provided','Five local function entries include one tail thunk, not five independent algorithms')
    selected_assembly_sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'selected-assembly.txt')).Hash.ToLowerInvariant()
}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
