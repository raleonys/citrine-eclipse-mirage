$ErrorActionPreference='Stop'
$expected='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
$target='C:\Users\Skysect\Downloads\anadius64.dll'
if((Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected){throw 'Target identity changed'}
$bytes=[IO.File]::ReadAllBytes($target)
function U16([int]$o){[BitConverter]::ToUInt16($bytes,$o)}
function U32([int]$o){[BitConverter]::ToUInt32($bytes,$o)}
$pe=U32 0x3c
if((U32 $pe) -ne 0x4550 -or (U16 ($pe+24)) -ne 0x20b){throw 'Not PE32+'}
$sectionStart=$pe+24+(U16 ($pe+20))
$sections=@(for($i=0;$i -lt (U16 ($pe+6));$i++){
    $at=$sectionStart+40*$i
    [pscustomobject]@{name=[Text.Encoding]::ASCII.GetString($bytes,$at,8).TrimEnd([char]0);rva=(U32 ($at+12));virtual_size=(U32 ($at+8));raw_size=(U32 ($at+16));raw_offset=(U32 ($at+20));characteristics=(U32 ($at+36))}
})
$queries=@(
    @{name='raw_DllMain_hook_088040';rva=0x88040;length=8},
    @{name='static_UCRT_flag_0a5a78';rva=0xa5a78;length=4},
    @{name='TLS_callback_slot_0a5a70';rva=0xa5a70;length=8},
    @{name='startup_state_0a4790';rva=0xa4790;length=4},
    @{name='startup_lock_0a4798';rva=0xa4798;length=8},
    @{name='DLL_and_onexit_flags_0a47a0';rva=0xa47a0;length=2},
    @{name='attach_count_0a47ec';rva=0xa47ec;length=4}
)
$facts=@(foreach($q in $queries){
    $matches=@($sections | Where-Object {$q.rva -ge $_.rva -and $q.rva+$q.length -le $_.rva+[Math]::Max($_.raw_size,$_.virtual_size)})
    if($matches.Count -ne 1){throw 'Unmapped/ambiguous range'}
    $section=$matches[0];$delta=$q.rva-$section.rva
    if($delta -lt $section.raw_size){
        if($delta+$q.length -gt $section.raw_size){throw 'Cross-boundary query'}
        $offset=$section.raw_offset+$delta
        if($offset+$q.length -gt $bytes.Length){throw 'File read out of range'}
        $value=$bytes[$offset..($offset+$q.length-1)];$source='file-backed';$fileOffset=('0x{0:x}' -f $offset)
    }else{$value=New-Object byte[] $q.length;$source='PE zero-filled virtual tail';$fileOffset=$null}
    [ordered]@{name=$q.name;va=('0x{0:x}' -f (0x180000000L+$q.rva));rva=('0x{0:x}' -f $q.rva);bytes=$q.length;section=$section.name;section_characteristics=('0x{0:x}' -f $section.characteristics);source=$source;file_offset=$fileOffset;initial_hex=([BitConverter]::ToString([byte[]]$value).Replace('-','').ToLowerInvariant())}
})
foreach($fact in $facts){if($fact.initial_hex -notmatch '^0+$'){throw 'Expected zero static field'} }
[ordered]@{target_sha256=$expected;source='Supplemental bounded read-only PE parser, not new REA evidence';facts=$facts;limitations=@('Initial image values only. Runtime writes, injection, callbacks and races are not observed.','Retained assembly mentions 0a5a78 once (read only) and no direct write; this is not a proof against arbitrary indirect modification.')} | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'static-image-facts.json') -Encoding UTF8
$pseudoPath=Join-Path $PSScriptRoot '..\anadius-all-functions.pseudo.c'
$asmPath=Join-Path $PSScriptRoot '..\anadius-all-functions.asm.txt'
$pseudo=Get-Content -LiteralPath $pseudoPath -Raw
$asm=Get-Content -LiteralPath $asmPath -Raw
$addresses=@('04b418','04b454','04b488','04b4a0','04b4c8','04b540','04b570','04b584','04b5c0','04b6e4','04b708','04b9e0','04ba30','04bb48','04bbc8','04bcfc','04bfd8','03af80','04c1e0','04c1f0','04c1fc','04c218','04c220','04c25c')
$functions=@(foreach($rva in $addresses){
    $va='0x180'+$rva
    $pm=[regex]::Match($pseudo,'/\* '+$va+' ([^\r\n]+); Evidence (ev_[a-f0-9]+) \*/')
    $am=[regex]::Match($asm,'; '+$va+' ([^\r\n]+); Evidence (ev_[a-f0-9]+)')
    if(!$pm.Success -or !$am.Success){throw ('Missing retained evidence '+$va)}
    [ordered]@{va=$va;name=$pm.Groups[1].Value;pseudocode_evidence=$pm.Groups[2].Value;assembly_evidence=$am.Groups[2].Value}
})
[ordered]@{target_sha256=$expected;analysis='Retained REA/Ghidra pseudocode and assembly plus bounded initial-image fields. No target execution.';pseudocode_sha256=(Get-FileHash -LiteralPath $pseudoPath).Hash.ToLowerInvariant();assembly_sha256=(Get-FileHash -LiteralPath $asmPath).Hash.ToLowerInvariant();functions=$functions;anchors=@('04ba52/04ba84/04bab3: AL success checks; 04baaa and argv 04b474: EAX checks','04bb55..04bb6e: signed-positive count, decrement before startup validation','04bb15: wrapping 32-bit count increment after TLS callback','04baf1..04bb12: validate SLOT address then reload callback; invoke reason2','04bc62..04bc79: application FALSE attach -> application detach -> CRT detach with reserved bool','04bcb4..04bcc4: detach result replaces application return and becomes 1','04b5c6..04b638: already-initialized bypasses module-kind validation; static mode six all-ones words','04b570 ECX=0, then late ACRT and VCRT cleanup; return ignored','04c220/04c25c: actual start==end callback ranges empty');limits=@('Static-UCRT mode only; no invented general-UCRT startup locking or onexit-table initialization','Normal flow only. Native SEH filters, unwind/finally handlers, CRT EH metadata and XFG ABI not implemented','Remaining CRT lifecycle, table walkers, SList, stdio, environment, cookie, TLS validation/invocation and application callback are mandatory Effects','Application onexit Table borrowed from the actual onexit owner; lifetime and caller synchronization required','No DLL loaded, actual DllMain exported, native loader registration or native CRT backend invoked')} | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
'Recorded '+$functions.Count+' retained function evidence pairs and '+$facts.Count+' initial-image facts'
