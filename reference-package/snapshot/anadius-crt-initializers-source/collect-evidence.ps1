param([string]$Target='C:\Users\Skysect\Downloads\anadius64.dll')
$ErrorActionPreference='Stop'
$expected='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
if((Get-FileHash -LiteralPath $Target).Hash.ToLowerInvariant() -ne $expected){throw 'Target identity mismatch'}
$bytes=[IO.File]::ReadAllBytes($Target)
$nt=[BitConverter]::ToInt32($bytes,0x3c)
$sectionCount=[BitConverter]::ToUInt16($bytes,$nt+6)
$optionalSize=[BitConverter]::ToUInt16($bytes,$nt+20)
$imageBase=[BitConverter]::ToUInt64($bytes,$nt+24+24)
$sections=@(for($index=0;$index -lt $sectionCount;++$index){
    $at=$nt+24+$optionalSize+$index*40
    [ordered]@{name=[Text.Encoding]::ASCII.GetString($bytes,$at,8).Trim([char]0);rva=[BitConverter]::ToUInt32($bytes,$at+12);virtual_size=[BitConverter]::ToUInt32($bytes,$at+8);raw_size=[BitConverter]::ToUInt32($bytes,$at+16);raw=[BitConverter]::ToUInt32($bytes,$at+20)}
})
function FileOffset([uint64]$va,[int]$length){
    if($va -lt $imageBase){throw 'VA below image'}
    $rva=$va-$imageBase
    $matches=@($sections|Where-Object{$rva -ge $_.rva -and $rva+$length -le [uint64]$_.rva+[Math]::Min($_.raw_size,$_.virtual_size)})
    if($matches.Count -ne 1){throw 'Not exactly one initialized source mapping'}
    $offset=[int]($rva-$matches[0].rva+$matches[0].raw)
    if($offset -lt 0 -or $offset+$length -gt $bytes.Length){throw 'File range out of bounds'}
    return $offset
}
function RangeHash([int]$offset,[int]$length){
    $hasher=[Security.Cryptography.SHA256]::Create()
    try{return [Convert]::ToHexString($hasher.ComputeHash($bytes,$offset,$length)).ToLowerInvariant()}finally{$hasher.Dispose()}
}
$outputs=Split-Path -Parent $PSScriptRoot
$asmPath=Join-Path $outputs 'anadius-all-functions.asm.txt'
$pseudoPath=Join-Path $outputs 'anadius-all-functions.pseudo.c'
$asm=Get-Content -LiteralPath $asmPath -Raw
$pseudo=Get-Content -LiteralPath $pseudoPath -Raw
$symbols=@{}
foreach($match in [regex]::Matches($pseudo,'(?m)^/\* (0x[0-9a-f]+) (.*?); Evidence (ev_[0-9a-f]+)')){$symbols[$match.Groups[1].Value]=$match.Groups[2].Value}
function Descriptor([uint64]$va){
    $formatted='0x'+$va.ToString('x')
    [ordered]@{va=$formatted;symbol=$symbols[$formatted];binding='Metadata identity only; must be resolved to rebuilt code before invocation'}
}
$tableDefinitions=@(
    @{name='acrt_initializer_pairs';va=[uint64]0x18008aa60;count=16;stride=16},
    @{name='c_initializers';va=[uint64]0x18007f6d8;count=7;stride=8},
    @{name='cpp_initializers';va=[uint64]0x18007f5d0;count=32;stride=8}
)
$tables=@(foreach($definition in $tableDefinitions){
    $length=$definition.count*$definition.stride
    $offset=FileOffset $definition.va $length
    $entries=@(for($k=0;$k -lt $definition.count;++$k){
        $entryOffset=$offset+$k*$definition.stride
        $value=[BitConverter]::ToUInt64($bytes,$entryOffset)
        if($definition.stride -eq 16){
            [ordered]@{index=$k;initialize=(Descriptor $value);uninitialize=(Descriptor ([BitConverter]::ToUInt64($bytes,$entryOffset+8)))}
        }else{[ordered]@{index=$k;callback=(Descriptor $value)}}
    })
    [ordered]@{name=$definition.name;va='0x'+$definition.va.ToString('x');file_offset=$offset;bytes=$length;sha256=(RangeHash $offset $length);entries=$entries}
})
[ordered]@{target_sha256=$expected;method='Direct bounded read-only PE mapping after exact SHA check; this is not new REA Evidence';tables=$tables}|ConvertTo-Json -Depth 9|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'initializer-tables.json') -Encoding UTF8
$addresses=@('0x1800704f4','0x180070588','0x180064614','0x180064658','0x180064594','0x1800645cc','0x1800645a8','0x1800645bc','0x180064604')
$functions=@(foreach($address in $addresses){
    $am=[regex]::Match($asm,'(?ms)^; '+[regex]::Escape($address)+' .*?(?=^; 0x|\z)')
    $pm=[regex]::Match($pseudo,'(?m)^/\* '+[regex]::Escape($address)+' .*?Evidence (ev_[a-z0-9]+)')
    if(!$am.Success -or !$pm.Success){throw ('Missing retained function evidence '+$address)}
    [ordered]@{va=$address;assembly_evidence=[regex]::Match($am.Value,'Evidence (ev_[a-z0-9]+)').Groups[1].Value;pseudocode_evidence=$pm.Groups[1].Value}
})
$extract=@(foreach($address in $addresses){[regex]::Match($asm,'(?ms)^; '+[regex]::Escape($address)+' .*?(?=^; 0x|\z)').Value}) -join "`n"
Set-Content -LiteralPath (Join-Path $PSScriptRoot 'assembly-evidence.txt') -Value $extract -Encoding UTF8
[ordered]@{target_sha256=$expected;source='Retained REA/Ghidra function evidence plus separately labeled bounded PE table reads';scope='Nine normal-flow bodies; AL/EAX contracts; new source bridge does not add original functions';functions=$functions;table_file='initializer-tables.json';assembly_sha256=(Get-FileHash -LiteralPath $asmPath).Hash.ToLowerInvariant();pseudocode_sha256=(Get-FileHash -LiteralPath $pseudoPath).Hash.ToLowerInvariant();remaining='Actual full callback bindings, VCRT and other CRT bodies, native loader/SEH/unwind/ABI equality'}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
'PASS: hash-bound initializer tables and retained function evidence recorded'
