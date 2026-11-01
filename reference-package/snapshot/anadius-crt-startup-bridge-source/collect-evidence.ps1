param([string]$Target='C:\Users\Skysect\Downloads\anadius64.dll')
$ErrorActionPreference='Stop'
$expected='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
if((Get-FileHash -LiteralPath $Target).Hash.ToLowerInvariant() -ne $expected){throw 'Target identity mismatch'}
$bytes=[IO.File]::ReadAllBytes($Target)
$nt=[BitConverter]::ToInt32($bytes,0x3c)
$sectionCount=[BitConverter]::ToUInt16($bytes,$nt+6)
$optionalSize=[BitConverter]::ToUInt16($bytes,$nt+20)
$imageBase=[BitConverter]::ToUInt64($bytes,$nt+48)
$sections=@(for($i=0;$i -lt $sectionCount;++$i){
    $at=$nt+24+$optionalSize+$i*40
    [ordered]@{rva=[BitConverter]::ToUInt32($bytes,$at+12);virtual_size=[BitConverter]::ToUInt32($bytes,$at+8);raw_size=[BitConverter]::ToUInt32($bytes,$at+16);raw=[BitConverter]::ToUInt32($bytes,$at+20)}
})
$tables=@(foreach($definition in @(@{name='preterminators';va=[uint64]0x18007f728;count=4},@{name='terminators';va=[uint64]0x18007f750;count=1})){
    $rva=$definition.va-$imageBase;$length=$definition.count*8
    $matches=@($sections|Where-Object{$rva -ge $_.rva -and $rva+$length -le [uint64]$_.rva+[Math]::Min($_.raw_size,$_.virtual_size)})
    if($matches.Count -ne 1){throw 'Expected one initialized file mapping'}
    $offset=[int]($rva-$matches[0].rva+$matches[0].raw)
    if($offset -lt 0 -or $offset+$length -gt $bytes.Length){throw 'Table bounds'}
    $entries=@(for($i=0;$i -lt $definition.count;++$i){
        $value=[BitConverter]::ToUInt64($bytes,$offset+$i*8)
        [ordered]@{index=$i;original_va='0x'+$value.ToString('x');requires_rebuilt_binding=($value -ne 0)}
    })
    $hasher=[Security.Cryptography.SHA256]::Create()
    try{$hash=[Convert]::ToHexString($hasher.ComputeHash($bytes,$offset,$length)).ToLowerInvariant()}finally{$hasher.Dispose()}
    [ordered]@{name=$definition.name;begin='0x'+$definition.va.ToString('x');end='0x'+($definition.va+$length).ToString('x');file_offset=$offset;bytes=$length;sha256=$hash;entries=$entries}
})
[ordered]@{target_sha256=$expected;method='Direct bounded read-only PE mapping after exact SHA check; not new REA Evidence';tables=$tables;limits='Original VA metadata only; nonzero entries require rebuilt callback bindings. No original table address is callable.'}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'termination-tables.json') -Encoding UTF8
[ordered]@{kind='New source composition, not additional original functions';target_sha256=$expected;entry_overrides=@('0x18004c1e0','0x18004c1f0','0x18004c1fc','0x18004c218','0x180064054','0x18005cdb8','0x18005cd9c');range_core='0x180064614';source_evidence=@('../anadius-crt-support-source/evidence.json','../anadius-crt-environment-source/evidence.json','../anadius-crt-termination-source/evidence.json','../anadius-crt-initializers-source/evidence.json','../anadius-vcrt-bridge-source/evidence.json','../anadius-crt-entry-source/evidence.json','../anadius-crt-onexit-source/evidence.json','../anadius-crt-cache-source/evidence.json');table_metadata='termination-tables.json';limits='Full initializer callback bindings/native providers, final shared-state and cookie setup, loader/SEH/unwind, runtime equivalence remain open';target_execution='none'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
'PASS: source composition provenance and termination table metadata recorded'
