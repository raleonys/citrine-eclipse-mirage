$ErrorActionPreference='Stop'
$parent=Split-Path -Parent $PSScriptRoot
$pseudoPath=Join-Path $parent 'anadius-all-functions.pseudo.c'
$asmPath=Join-Path $parent 'anadius-all-functions.asm.txt'
$pseudoHash=(Get-FileHash -LiteralPath $pseudoPath).Hash.ToLowerInvariant()
$asmHash=(Get-FileHash -LiteralPath $asmPath).Hash.ToLowerInvariant()
if($pseudoHash -ne '26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'){throw 'Retained pseudocode identity mismatch'}
if($asmHash -ne '9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42'){throw 'Retained assembly identity mismatch'}
$pseudo=[regex]::Match((Get-Content -LiteralPath $pseudoPath -Raw),'(?ms)^/\* 0x18006e210 .*?(?=^/\* 0x|\z)').Value
$asm=[regex]::Match((Get-Content -LiteralPath $asmPath -Raw),'(?ms)^; 0x18006e210 .*?(?=^; 0x|\z)').Value
if(!$pseudo -or !$asm){throw 'Missing bounded function evidence'}
[ordered]@{scope='Exact retained REA Ghidra function, no new database or target execution';target='anadius64.dll';target_sha256='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';retained_pseudocode_sha256=$pseudoHash;retained_assembly_sha256=$asmHash;functions=@([ordered]@{address='0x18006e210';pseudocode_evidence='ev_55d88e9cae0384ad87f9dd04be6ed1385a4141630d9ea934b9d1296fdde5d839';assembly_evidence='ev_ff659bc38c0cecbd6049ae735ce486d23e30ad115dccbbb5d59e32f94f015a52';pseudocode=$pseudo;assembly=$asm})}|ConvertTo-Json -Depth 5|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
