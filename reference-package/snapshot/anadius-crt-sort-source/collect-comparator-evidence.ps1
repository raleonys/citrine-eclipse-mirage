param([string]$Target='C:\Users\Skysect\Downloads\anadius64.dll',[string]$ToolchainRoot=(Join-Path $PSScriptRoot '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64'))
$ErrorActionPreference='Stop'
$expected='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
if((Get-FileHash -LiteralPath $Target).Hash.ToLowerInvariant() -ne $expected){throw 'Input identity mismatch'}
$bytes=[IO.File]::ReadAllBytes($Target)
$nt=[BitConverter]::ToInt32($bytes,0x3c)
$imageBase=[BitConverter]::ToUInt64($bytes,$nt+48)
$count=[BitConverter]::ToUInt16($bytes,$nt+6)
$optional=[BitConverter]::ToUInt16($bytes,$nt+20)
$va=[uint64]0x18006e41c;$length=18;$rva=$va-$imageBase
$mappings=@(for($i=0;$i -lt $count;++$i){
    $at=$nt+24+$optional+$i*40
    $sectionRva=[BitConverter]::ToUInt32($bytes,$at+12)
    $rawSize=[BitConverter]::ToUInt32($bytes,$at+16)
    $virtualSize=[BitConverter]::ToUInt32($bytes,$at+8)
    if($rva -ge $sectionRva -and $rva+$length -le [uint64]$sectionRva+[Math]::Min($rawSize,$virtualSize)){
        [ordered]@{section=[Text.Encoding]::ASCII.GetString($bytes,$at,8).Trim([char]0);offset=[int]($rva-$sectionRva+[BitConverter]::ToUInt32($bytes,$at+20))}
    }
})
if($mappings.Count -ne 1){throw 'Expected one raw section mapping'}
$offset=$mappings[0].offset
if($offset -lt 0 -or $offset+$length -gt $bytes.Length){throw 'Raw range bounds'}
$raw=[byte[]]::new($length);[Array]::Copy($bytes,$offset,$raw,0,$length)
$hex=[Convert]::ToHexString($raw).ToLowerInvariant()
if($hex -ne '483bca730483c8ffc333c0483bca0f97c0c3'){throw 'Comparator byte identity mismatch'}
$tool=(Resolve-Path (Join-Path $ToolchainRoot 'bin\llvm-objdump.exe')).Path
$arguments=@('-d','--start-address=0x18006e41c','--stop-address=0x18006e42e',$Target)
$disassembly=& $tool @arguments
if($LASTEXITCODE -ne 0){throw 'Bounded disassembly failed'}
Set-Content -LiteralPath (Join-Path $PSScriptRoot 'comparator-offline-disassembly.txt') -Value $disassembly -Encoding UTF8
$hasher=[Security.Cryptography.SHA256]::Create()
try{$rawHash=[Convert]::ToHexString($hasher.ComputeHash($raw)).ToLowerInvariant()}finally{$hasher.Dispose()}
[ordered]@{target_sha256=$expected;method='Direct bounded offline byte read/disassembly after exact SHA check; not a REA Evidence record';function_inventory_scope='06e41c is absent as an entry in the retained Ghidra-defined function inventory; no new Ghidra function count claimed';va='0x18006e41c';end_exclusive='0x18006e42e';bytes=$length;section=$mappings[0].section;file_offset=$offset;hex=$hex;range_sha256=$rawHash;tool=$tool;tool_sha256=(Get-FileHash -LiteralPath $tool).Hash.ToLowerInvariant();arguments=$arguments;observation='CMP RCX,RDX; unsigned JAE/SETA; returns EAX -1/0/1; neither argument is dereferenced';target_execution='none';limitations='Normal valid inputs; no instruction-level execution, CFG/ABI/runtime equivalence claim'}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'comparator-offline-evidence.json') -Encoding UTF8
'PASS: 18 comparator bytes and bounded offline disassembly recorded'
