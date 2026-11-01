$ErrorActionPreference='Stop'
$outputs=Split-Path -Parent $PSScriptRoot
$target='C:\Users\Skysect\Downloads\anadius64.dll'
$targetHash='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
$assemblyPath=Join-Path $outputs 'anadius-all-functions.asm.txt'
$pseudoPath=Join-Path $outputs 'anadius-all-functions.pseudo.c'
$assemblyHash='9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42'
$pseudoHash='26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'
if((Get-FileHash -LiteralPath $target).Hash.ToLowerInvariant() -ne $targetHash){throw 'Target hash changed'}
if((Get-FileHash -LiteralPath $assemblyPath).Hash.ToLowerInvariant() -ne $assemblyHash){throw 'Retained assembly hash changed'}
if((Get-FileHash -LiteralPath $pseudoPath).Hash.ToLowerInvariant() -ne $pseudoHash){throw 'Retained pseudocode hash changed'}
$assembly=Get-Content -LiteralPath $assemblyPath -Raw
$pseudo=Get-Content -LiteralPath $pseudoPath -Raw
$functions=@()
$selectedAsm=@();$selectedPseudo=@()
foreach($va in @('18006ccc8','18006cd58','180066c1c')) {
    $a=[regex]::Match($assembly,'(?ms)^; 0x'+$va+' .*?(?=^; 0x|\z)').Value
    $p=[regex]::Match($pseudo,'(?ms)^/\* 0x'+$va+' .*?(?=^/\* 0x|\z)').Value
    if(-not $a -or -not $p){throw "Missing retained function $va"}
    $selectedAsm+=$a;$selectedPseudo+=$p
    $functions+=[ordered]@{va="0x$va";assembly_evidence=[regex]::Match($a,'ev_[0-9a-f]{64}').Value;pseudocode_evidence=[regex]::Match($p,'ev_[0-9a-f]{64}').Value}
}
$asmOut=Join-Path $PSScriptRoot 'selected.asm.txt'
$pseudoOut=Join-Path $PSScriptRoot 'selected.pseudo.c'
[IO.File]::WriteAllText($asmOut,($selectedAsm -join "`n"),[Text.UTF8Encoding]::new($false))
[IO.File]::WriteAllText($pseudoOut,($selectedPseudo -join "`n"),[Text.UTF8Encoding]::new($false))
$bytes=[IO.File]::ReadAllBytes($target)
$pe=[BitConverter]::ToInt32($bytes,60)
$sectionCount=[BitConverter]::ToUInt16($bytes,$pe+6)
$table=$pe+24+[BitConverter]::ToUInt16($bytes,$pe+20)
$raw=$null
for($i=0;$i -lt $sectionCount;$i++) {
    $s=$table+40*$i;$rva=[BitConverter]::ToUInt32($bytes,$s+12)
    $size=[BitConverter]::ToUInt32($bytes,$s+16)
    if(0x8b8f8 -ge $rva -and 0x8b8f8+24 -le $rva+$size) {
        $raw=0x8b8f8-$rva+[BitConverter]::ToUInt32($bytes,$s+20)
    }
}
if($null -eq $raw){throw 'Candidate/name range is not file backed'}
[byte[]]$slice=$bytes[$raw..($raw+23)]
$candidate=[BitConverter]::ToUInt32($slice,0)
if($candidate -ne 16 -or [Text.Encoding]::ASCII.GetString($slice,8,15) -ne "AreFileApisANSI`0"){throw 'Unexpected candidate/name bytes'}
$sha=[Security.Cryptography.SHA256]::Create()
try{$sliceHash=([BitConverter]::ToString($sha.ComputeHash($slice))).Replace('-','').ToLowerInvariant()}finally{$sha.Dispose()}
[ordered]@{
    target_sha256=$targetHash;assembly_sha256=$assemblyHash;pseudocode_sha256=$pseudoHash
    origin='Bounded extraction of previously retained REA/Ghidra evidence. Parent confirmed REA6.5.0 aligned catalog and available Ghidra12.1.4; current session closed. Additional candidate/name bytes read statically from hash-verified PE without loading it.'
    functions=$functions
    selected_assembly_sha256=(Get-FileHash -LiteralPath $asmOut).Hash.ToLowerInvariant()
    selected_pseudocode_sha256=(Get-FileHash -LiteralPath $pseudoOut).Hash.ToLowerInvariant()
    candidate_table=[ordered]@{begin_va='0x18008b8f8';end_va_exclusive='0x18008b8fc';name_va='0x18008b900';raw_offset=$raw;length=24;hex=([BitConverter]::ToString($slice)).Replace('-','').ToLowerInvariant();sha256=$sliceHash;candidate=$candidate;module='kernel32';procedure_slot=0}
    limits=@('Native conversion and function invocation remain required pure provider effects.','Preserves observed flag filtering, pointer transformations and full EAX result; does not implement Windows code page conversion.','BOOL storage is four bytes and is cleared before the conversion call for code pages65000 and65001, even if conversion fails.','No target or native/provider DLL execution; no original compiler/ABI or SEH equivalence claim.')
} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
