param([string]$InputDll='C:\Users\Skysect\Downloads\anadius64.dll')
$ErrorActionPreference='Stop'
$expected='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
$inputHash=(Get-FileHash -LiteralPath $InputDll).Hash.ToLowerInvariant()
if($inputHash -ne $expected){throw 'Input SHA-256 does not match the analyzed artifact'}
$pseudoPath=Join-Path $PSScriptRoot '..\anadius-all-functions.pseudo.c'
$asmPath=Join-Path $PSScriptRoot '..\anadius-all-functions.asm.txt'
$pseudoHash=(Get-FileHash -LiteralPath $pseudoPath).Hash.ToLowerInvariant()
$asmHash=(Get-FileHash -LiteralPath $asmPath).Hash.ToLowerInvariant()
if($pseudoHash -ne '26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb' -or $asmHash -ne '9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42'){throw 'Retained evidence hash mismatch'}
$pseudo=[regex]::Match((Get-Content -LiteralPath $pseudoPath -Raw),'(?ms)^/\* 0x18006f674 .*?(?=^/\* 0x|\z)').Value
$asm=[regex]::Match((Get-Content -LiteralPath $asmPath -Raw),'(?ms)^; 0x18006f674 .*?(?=^; 0x|\z)').Value
if(!$pseudo -or !$asm){throw 'Bounded function not found'}
$bytes=[IO.File]::ReadAllBytes($InputDll)
$pe=[BitConverter]::ToInt32($bytes,60)
$count=[BitConverter]::ToUInt16($bytes,$pe+6)
$optionalSize=[BitConverter]::ToUInt16($bytes,$pe+20)
$imageBase=[BitConverter]::ToUInt64($bytes,$pe+48)
$region=$null
for($i=0;$i -lt $count;$i++){
    $header=$pe+24+$optionalSize+$i*40
    $name=[Text.Encoding]::ASCII.GetString($bytes,$header,8).Trim([char]0)
    $rva=[BitConverter]::ToUInt32($bytes,$header+12)
    $rawSize=[BitConverter]::ToUInt32($bytes,$header+16)
    $rawOffset=[BitConverter]::ToUInt32($bytes,$header+20)
    if(0xa20b0 -ge $rva -and 0xa21b0 -le $rva+$rawSize){
        $flagOffset=$rawOffset+0xa20b0-$rva
        $tableOffset=$rawOffset+0xa20c0-$rva
        $tableBytes=[byte[]]$bytes[$tableOffset..($tableOffset+239)]
        $sha=[Security.Cryptography.SHA256]::Create()
        try{$tableHash=([BitConverter]::ToString($sha.ComputeHash($tableBytes))).Replace('-','').ToLowerInvariant()}finally{$sha.Dispose()}
        $rows=@(for($row=0;$row -lt 5;$row++){
            $offset=$tableOffset+$row*48
            [ordered]@{index=$row;file_offset=('0x{0:x}' -f $offset);virtual_address=('0x{0:x}' -f ($imageBase+0xa20c0+$row*48));codepage=[BitConverter]::ToUInt32($bytes,$offset);words=@(0..5|ForEach-Object{[BitConverter]::ToUInt16($bytes,$offset+4+$_*2)});ranges=@(0..3|ForEach-Object{$start=$offset+16+$_*8;,@($bytes[$start..($start+7)])})}
        })
        $region=[ordered]@{section=$name;section_rva=('0x{0:x}' -f $rva);section_raw_offset=('0x{0:x}' -f $rawOffset);section_raw_size=$rawSize;flags_va='0x1800a20b0';flags_file_offset=('0x{0:x}' -f $flagOffset);flags=@($bytes[$flagOffset..($flagOffset+3)]);table_va='0x1800a20c0';table_file_offset=('0x{0:x}' -f $tableOffset);table_byte_count=240;table_sha256=$tableHash;table_hex=([BitConverter]::ToString($tableBytes)).Replace('-','');rows=$rows}
    }
}
if(!$region){throw 'Shipped constants are not wholly contained in a raw section'}
[ordered]@{scope='Retained REA Ghidra normal flow plus read-only PE constant extraction; no binary execution, target modification or fresh Ghidra database';target='anadius64.dll';target_sha256=$inputHash;retained_pseudocode_sha256=$pseudoHash;retained_assembly_sha256=$asmHash;functions=@([ordered]@{address='0x18006f674';pseudocode_evidence='ev_82ac27f350ea82c6784171d9bc987418729f2f2712ec6c95b83fdb20a2058881';assembly_evidence='ev_d4c04fc86da4968083b43e855ee7c4f116d7b038613a196501c9c0b65b7e59c5';pseudocode=$pseudo;assembly=$asm});constant_extraction=$region;boundaries=@('Five builtin writable .data rows are reconstructed at their shipped immutable values; runtime mutation of original table globals is outside this module contract.','Locale identity globals 18008bf70/78/80/88 remain live getters and point to caller-owned UTF-16.','Successful CPInfo provider must terminate lead-byte pairs within its 12-byte array. Missing terminator throws an explicit reconstruction-boundary exception; no claim about original stack overread.','IsValidCodePage, GetCPInfo, getSystemCP and downstream table builders are required effects; isolated fixtures are synthetic and do not prove OS API behavior.','Security cookie epilogue and structured exception behavior are not reproduced by this normal-flow C++ source.')}|ConvertTo-Json -Depth 12|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
