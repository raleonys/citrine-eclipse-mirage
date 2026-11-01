param([string]$Target='C:\Users\Skysect\Downloads\anadius64.dll')
$ErrorActionPreference='Stop'
$expected='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
if((Get-FileHash -LiteralPath $Target).Hash.ToLowerInvariant() -ne $expected){throw 'Target identity mismatch'}
$bytes=[IO.File]::ReadAllBytes($Target)
$nt=[BitConverter]::ToInt32($bytes,0x3c)
if([BitConverter]::ToUInt16($bytes,$nt+24) -ne 0x20b){throw 'Expected PE32+'}
$sectionCount=[BitConverter]::ToUInt16($bytes,$nt+6)
$optionalSize=[BitConverter]::ToUInt16($bytes,$nt+20)
$imageBase=[BitConverter]::ToUInt64($bytes,$nt+48)
$sections=@(for($i=0;$i -lt $sectionCount;++$i){
    $at=$nt+24+$optionalSize+$i*40
    [ordered]@{name=[Text.Encoding]::ASCII.GetString($bytes,$at,8).Trim([char]0);rva=[BitConverter]::ToUInt32($bytes,$at+12);virtual_size=[BitConverter]::ToUInt32($bytes,$at+8);raw_size=[BitConverter]::ToUInt32($bytes,$at+16);raw=[BitConverter]::ToUInt32($bytes,$at+20)}
})
$definitions=@(
    @{name='a1550';va=[uint64]0x1800a1550;size=4},
    @{name='a1554';va=[uint64]0x1800a1554;size=4},
    @{name='a1558';va=[uint64]0x1800a1558;size=8},
    @{name='a1560';va=[uint64]0x1800a1560;size=8},
    @{name='a4788';va=[uint64]0x1800a4788;size=4}
)
$initial=@(foreach($field in $definitions){
    $rva=$field.va-$imageBase
    $matches=@($sections | Where-Object{$rva -ge $_.rva -and $rva+$field.size -le [uint64]$_.rva+$_.virtual_size})
    if($matches.Count -ne 1){throw 'Expected one virtual section mapping'}
    $section=$matches[0]
    $delta=$rva-$section.rva
    if($delta+$field.size -le $section.raw_size){
        $offset=[int]($section.raw+$delta)
        if($offset -lt 0 -or $offset+$field.size -gt $bytes.Length){throw 'File bounds'}
        $raw=[byte[]]::new($field.size)
        [Array]::Copy($bytes,$offset,$raw,0,$raw.Length)
        $kind='initialized file bytes'
    }elseif($delta -ge $section.raw_size){
        $offset=$null
        $raw=[byte[]]::new($field.size)
        $kind='PE virtual zero-fill; no file bytes'
    }else{throw 'Mixed raw/zero-fill range not supported'}
    $value=if($field.size -eq 4){[uint64][BitConverter]::ToUInt32($raw,0)}else{[BitConverter]::ToUInt64($raw,0)}
    [ordered]@{field=$field.name;va='0x'+$field.va.ToString('x');size=$field.size;section=$section.name;section_rva=$section.rva;section_virtual_size=$section.virtual_size;section_raw_size=$section.raw_size;file_offset=$offset;mapping=$kind;little_endian_hex=[Convert]::ToHexString($raw).ToLowerInvariant();initial_value_hex='0x'+$value.ToString('x')}
})
[ordered]@{target_sha256=$expected;method='Direct bounded read-only PE mapping after exact SHA check; not a new REA Evidence result';scope='Image initialization only, before any loader/CRT writes';fields=$initial}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'initial-globals.json') -Encoding UTF8
$outputs=Split-Path -Parent $PSScriptRoot
$asmPath=Join-Path $outputs 'anadius-all-functions.asm.txt'
$pseudoPath=Join-Path $outputs 'anadius-all-functions.pseudo.c'
$asmHash=(Get-FileHash -LiteralPath $asmPath).Hash.ToLowerInvariant()
$pseudoHash=(Get-FileHash -LiteralPath $pseudoPath).Hash.ToLowerInvariant()
if($asmHash -ne '9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42' -or $pseudoHash -ne '26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'){throw 'Retained evidence identity mismatch'}
$asm=Get-Content -LiteralPath $asmPath -Raw
$pseudo=Get-Content -LiteralPath $pseudoPath -Raw
$am=[regex]::Match($asm,'(?ms)^; 0x18004b26c .*?(?=^; 0x|\z)')
$pm=[regex]::Match($pseudo,'(?m)^/\* 0x18004b26c .*?Evidence (ev_[a-z0-9]+)')
if(!$am.Success -or !$pm.Success){throw 'Missing retained evidence'}
Set-Content -LiteralPath (Join-Path $PSScriptRoot 'assembly-evidence.txt') -Value $am.Value -Encoding UTF8
[ordered]@{target_sha256=$expected;source='Retained REA/Ghidra function evidence plus separately labeled bounded PE global reads';function_va='0x18004b26c';assembly_evidence=[regex]::Match($am.Value,'Evidence (ev_[a-z0-9]+)').Groups[1].Value;pseudocode_evidence=$pm.Groups[1].Value;assembly_sha256=$asmHash;pseudocode_sha256=$pseudoHash;initial_globals='initial-globals.json';scope='CPUID/XGETBV selection algorithm with mandatory register providers; no host feature probe';limitations='Caller-owned State is a new model, not packed target globals. Defaults are zero; original initial values are separately recorded. Native discovery/loader/SEH/ABI equivalence remains unproven.'}|ConvertTo-Json -Depth 5|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
'PASS: CPU retained evidence and bounded initial-global mappings recorded'
