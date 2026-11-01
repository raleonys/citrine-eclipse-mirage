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
    [ordered]@{name=[Text.Encoding]::ASCII.GetString($bytes,$at,8).Trim([char]0);rva=[BitConverter]::ToUInt32($bytes,$at+12);virtual_size=[BitConverter]::ToUInt32($bytes,$at+8);raw_size=[BitConverter]::ToUInt32($bytes,$at+16);raw=[BitConverter]::ToUInt32($bytes,$at+20)}
})
$fields=@(foreach($field in @(@{name='printf_options';va=[uint64]0x1800a37d0;size=8},@{name='scanf_options';va=[uint64]0x1800a37d8;size=8},@{name='slist_header';va=[uint64]0x1800a4d70;size=16},@{name='tls_callback';va=[uint64]0x1800a5a70;size=8})){
    $rva=$field.va-$imageBase
    $matches=@($sections|Where-Object{$rva -ge $_.rva -and $rva+$field.size -le [uint64]$_.rva+$_.virtual_size})
    if($matches.Count -ne 1){throw 'Expected one virtual mapping'}
    $section=$matches[0];$delta=$rva-$section.rva
    $data=[byte[]]::new($field.size)
    if($delta+$field.size -le $section.raw_size){
        $offset=[int]($section.raw+$delta)
        if($offset -lt 0 -or $offset+$field.size -gt $bytes.Length){throw 'File bounds'}
        [Array]::Copy($bytes,$offset,$data,0,$data.Length);$kind='initialized file bytes'
    }elseif($delta -ge $section.raw_size){$offset=$null;$kind='PE virtual zero-fill; no file bytes'}else{throw 'Mixed mapping'}
    [ordered]@{name=$field.name;va='0x'+$field.va.ToString('x');size=$field.size;section=$section.name;file_offset=$offset;mapping=$kind;little_endian_hex=[Convert]::ToHexString($data).ToLowerInvariant()}
})
[ordered]@{target_sha256=$expected;method='Direct bounded read-only PE mapping after exact SHA check; not new REA Evidence';fields=$fields;scope='Image initialization before loader/CRT writes'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'initial-globals.json') -Encoding UTF8
$outputs=Split-Path -Parent $PSScriptRoot
$asmPath=Join-Path $outputs 'anadius-all-functions.asm.txt'
$pseudoPath=Join-Path $outputs 'anadius-all-functions.pseudo.c'
$asmHash=(Get-FileHash -LiteralPath $asmPath).Hash.ToLowerInvariant()
$pseudoHash=(Get-FileHash -LiteralPath $pseudoPath).Hash.ToLowerInvariant()
if($asmHash -ne '9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42' -or $pseudoHash -ne '26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'){throw 'Retained evidence identity mismatch'}
$asm=Get-Content -LiteralPath $asmPath -Raw
$pseudo=Get-Content -LiteralPath $pseudoPath -Raw
$excerpts=[Collections.Generic.List[string]]::new()
$functions=@(foreach($va in @('0x1800067e0','0x1800067f0','0x18004c1e0','0x18004c1f0','0x18004c1fc','0x18004c218','0x18004d2f8')){
    $am=[regex]::Match($asm,'(?ms)^; '+$va+' .*?(?=^; 0x|\z)')
    $pm=[regex]::Match($pseudo,'(?m)^/\* '+$va+' .*?Evidence (ev_[a-z0-9]+)')
    if(!$am.Success -or !$pm.Success){throw 'Missing retained evidence'}
    $excerpts.Add($am.Value)
    [ordered]@{va=$va;assembly_evidence=[regex]::Match($am.Value,'Evidence (ev_[a-z0-9]+)').Groups[1].Value;pseudocode_evidence=$pm.Groups[1].Value}
})
Set-Content -LiteralPath (Join-Path $PSScriptRoot 'assembly-evidence.txt') -Value ($excerpts -join "`n") -Encoding UTF8
[ordered]@{target_sha256=$expected;source='Retained REA/Ghidra plus separately labeled bounded PE reads';assembly_sha256=$asmHash;pseudocode_sha256=$pseudoHash;functions=$functions;globals='initial-globals.json';scope='Seven bounded normal-flow functions; opaque rebuilt head/node tokens and caller-owned qword cells';remaining='Native SList storage/interlocked effects and loader/TLS lifetime, SEH/unwind and runtime equivalence'}|ConvertTo-Json -Depth 7|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
'PASS: seven retained function records and initial globals recorded'
