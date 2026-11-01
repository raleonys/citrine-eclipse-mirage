param(
    [string]$EvidencePath = (Join-Path $PSScriptRoot '..\version-reconstruction-input.json'),
    [string]$BuildDirectory = (Join-Path $PSScriptRoot 'build-llvm'),
    [string]$OutputDirectory = (Join-Path $PSScriptRoot '..\version-build-verification')
)
$ErrorActionPreference = 'Stop'
$d = Get-Content -LiteralPath $EvidencePath -Raw | ConvertFrom-Json
$bytes = [IO.File]::ReadAllBytes((Join-Path $BuildDirectory 'version.dll'))
function U16([int]$o) { [BitConverter]::ToUInt16($bytes,$o) }
function U32([int]$o) { [BitConverter]::ToUInt32($bytes,$o) }
function U64([int]$o) { [BitConverter]::ToUInt64($bytes,$o) }
function CString([int]$o) { $end=$o; while ($bytes[$end]) {$end++}; [Text.Encoding]::ASCII.GetString($bytes,$o,$end-$o) }
$pe = U32 0x3c
if ((U16 ($pe+4)) -ne 0x8664 -or (U16 ($pe+24)) -ne 0x20b) { throw 'Not PE32+ AMD64' }
$sectionCount = U16 ($pe+6); $optional = $pe+24
$imageBase = U64 ($optional+24); $entryRva=U32 ($optional+16)
$sections=@(); $sectionTable = $optional+(U16 ($pe+20))
for ($i=0;$i -lt $sectionCount;$i++) {
    $o=$sectionTable+40*$i
    $sections += [pscustomobject]@{name=([Text.Encoding]::ASCII.GetString($bytes,$o,8).Trim([char]0));rva=(U32 ($o+12));virtual_size=(U32 ($o+8));raw_offset=(U32 ($o+20));raw_size=(U32 ($o+16));flags=('0x{0:x}' -f (U32 ($o+36)))}
}
function RvaOffset([long]$rva) {
    foreach ($s in $sections) {
        if ($rva -ge $s.rva -and $rva -lt ($s.rva+[Math]::Max($s.virtual_size,$s.raw_size))) {
            return [int]($s.raw_offset+$rva-$s.rva)
        }
    }
    if ($rva -lt (U32 ($optional+60))) {return [int]$rva}
    throw "Unmapped RVA $rva"
}
$exportRva=U32 ($optional+112); $exportSize=U32 ($optional+116); $ed=RvaOffset $exportRva
$ordinalBase=U32 ($ed+16); $functionCount=U32 ($ed+20); $nameCount=U32 ($ed+24)
$eat=RvaOffset (U32 ($ed+28)); $names=RvaOffset (U32 ($ed+32)); $ordinals=RvaOffset (U32 ($ed+36))
$exports=@(); $byName=@{}
for ($i=0;$i -lt $nameCount;$i++) {
    $name=CString (RvaOffset (U32 ($names+4*$i))); $index=U16 ($ordinals+2*$i)
    $rva=U32 ($eat+4*$index)
    $row=[pscustomobject]@{name=$name;ordinal=($ordinalBase+$index);rva=('0x{0:x}' -f $rva);address=('0x{0:x}' -f ($imageBase+$rva));forwarder=($rva -ge $exportRva -and $rva -lt $exportRva+$exportSize)}
    $exports+=$row; $byName[$name]=$row
}
if ($functionCount -ne 198 -or $nameCount -ne 198) {throw 'Export table is not exactly198 named entries'}
foreach ($e in $d.pe.exports) {
    if (-not $byName.ContainsKey($e.name) -or $byName[$e.name].ordinal -ne $e.ordinal -or $byName[$e.name].forwarder) {throw "Export mismatch $($e.name)"}
}
$imports=@(); $importRva=U32 ($optional+120); $descriptor=RvaOffset $importRva
while ((U32 ($descriptor+12)) -ne 0) {
    $module=CString (RvaOffset (U32 ($descriptor+12)))
    $lookup=U32 $descriptor; $iat=U32 ($descriptor+16); if (-not $lookup) {$lookup=$iat}
    $index=0; $thunk=U64 (RvaOffset $lookup)
    while ($thunk) {
        if ($thunk -ge 9223372036854775808uL) {throw 'Unexpected ordinal import'}
        $symbol=CString ((RvaOffset $thunk)+2)
        $imports += [pscustomobject]@{dll=$module;name=$symbol;iat=('0x{0:x}' -f ($imageBase+$iat+8*$index))}
        $index++; $thunk=U64 ((RvaOffset $lookup)+8*$index)
    }
    $descriptor+=20
}
$expectedImports=@($d.pe.imports | ForEach-Object {($_.dll+'!'+$_.name).ToLowerInvariant()} | Sort-Object)
$actualImports=@($imports | ForEach-Object {($_.dll+'!'+$_.name).ToLowerInvariant()} | Sort-Object)
if (($expectedImports -join '|') -cne ($actualImports -join '|')) {throw 'Import set mismatch'}
$symbols=@{}
foreach ($line in (Get-Content -LiteralPath (Join-Path $BuildDirectory 'version.map'))) {
    if ($line -match '^([0-9a-fA-F]{8,16})\s+[0-9a-fA-F]+\s+\d+\s+(.+?)\s*$') {
        $symbols[$Matches[2]]=$imageBase+[Convert]::ToUInt64($Matches[1],16)
    }
}
if ($symbols['ReconstructedDllMain'] -ne $imageBase+$entryRva) {throw 'PE entry does not equal reconstructed DllMain'}
$instructions=@{}; $order=0
foreach ($line in (Get-Content -LiteralPath (Join-Path $BuildDirectory 'disassembly.txt'))) {
    if ($line -match '^([0-9a-fA-F]+):\s+(.+)$') {
        $va=[Convert]::ToUInt64($Matches[1],16)
        $instructions[$va]=[pscustomobject]@{order=$order;text=$Matches[2]}; $order++
    }
}
function Canonical([string]$op) {
    $op=$op.ToLowerInvariant()
    $op=$op -replace '^jz\s','je ' -replace '^jnz\s','jne '
    $op=$op -replace '\*0x1\b',''
    ($op -replace '\s','')
}
$wrapperResults=@();$total=0;$branches=0;$memoryRefs=0
$getProcIat=[Convert]::ToUInt64(($imports | Where-Object name -eq 'GetProcAddress').iat.Substring(2),16)
foreach ($e in $d.pe.exports) {
    if ($e.name -eq 'DllMain') {continue}
    $a=@($d.allAssembly | Where-Object name -eq $e.name)[0]
    $lines=@($a.text -split '\r?\n' | Where-Object {$_})
    $firstOrder=$null;$lastOrder=$null;$matched=@()
    foreach ($line in $lines) {
        if ($line -notmatch '^0x([0-9a-f]+): (.+)$') {throw 'Invalid retained instruction'}
        $originalAddress=$Matches[1]; $expected=$Matches[2]
        $nativeAddress=$symbols['L_'+$originalAddress]
        if (-not $nativeAddress -or -not $instructions.ContainsKey($nativeAddress)) {throw "Missing compiled instruction $originalAddress"}
        $native=$instructions[$nativeAddress];$actual=$native.text
        if ($null -eq $firstOrder) {$firstOrder=$native.order}
        $lastOrder=$native.order
        if ($expected -match '^J[A-Z]+ (0x([0-9a-f]+))$') {
            $destination=$symbols['L_'+$Matches[2]]
            $expected=$expected.Replace($Matches[1],('0x{0:x}' -f $destination)); $branches++
        }
        elseif ($expected -match '^LEA RDX, \[0x[0-9a-f]+\]$') {
            $destination=$symbols["name_$($e.ordinal)"]
            if (-not $destination -or (CString (RvaOffset ($destination-$imageBase))) -cne $e.name) {throw "Resolver string mismatch $($e.name)"}
            $expected='LEA RDX, [0x{0:x}]' -f $destination
        }
        elseif ($expected -eq 'CALL qword ptr [0x180006030]') {
            $expected='CALL qword ptr [0x{0:x}]' -f $getProcIat
        }
        else {
            $expected=[regex]::Replace($expected,'\[(0x18000e[0-9a-f]{3})\]',{
                param($m)
                $offset=[Convert]::ToUInt64($m.Groups[1].Value.Substring(2),16)-0x18000e000L
                '[0x{0:x}]' -f ($symbols['OuterState']+$offset)
            })
        }
        if ($actual -match '#\s*(0x[0-9a-fA-F]+)') {
            $resolved=$Matches[1]
            if ($actual -notmatch '\[rip[^\]]*\]') {throw "Unknown relative reference $actual"}
            $actual=[regex]::Replace($actual,'\[rip[^\]]*\]',('['+$resolved+']')); $memoryRefs++
        }
        $actual=($actual -split '#',2)[0] -replace '\s*<[^>]+>',''
        if ((Canonical $expected) -cne (Canonical $actual)) {throw "Mismatch at original0x$originalAddress / new$nativeAddress : expected '$expected'; actual '$actual'"}
        $matched += [pscustomobject]@{original_address=('0x'+$originalAddress);rebuilt_address=('0x{0:x}' -f $nativeAddress);normalized_instruction=(Canonical $actual)}
        $total++
    }
    if ($lastOrder-$firstOrder+1 -ne $lines.Count) {throw "Inserted/missing instruction in $($e.name)"}
    if ([Convert]::ToUInt64($byName[$e.name].address.Substring(2),16) -ne $symbols["reconstructed_$($e.ordinal)"]) {throw 'Export/symbol start mismatch'}
    $wrapperResults += [pscustomobject]@{name=$e.name;ordinal=$e.ordinal;result='PASS';instructions=$matched}
}
$embeddedSymbol=@($symbols.Keys | Where-Object {$_ -eq '(anonymous namespace)::kOriginalEmbeddedPE'})
if ($embeddedSymbol.Count -ne 1) {throw 'Embedded symbol is ambiguous'}
$embeddedOffset=RvaOffset ($symbols[$embeddedSymbol[0]]-$imageBase)
$embeddedHex=([BitConverter]::ToString($bytes,$embeddedOffset,$d.embedded.size)).Replace('-','').ToLowerInvariant()
if ($embeddedHex -cne ($d.embedded.bytes_hex -replace '\s','').ToLowerInvariant()) {throw 'Compiled embedded bytes mismatch'}
$resourceEvidence=Get-Content -LiteralPath (Join-Path $PSScriptRoot '..\version-resource-evidence.json') -Raw | ConvertFrom-Json
$originalResource=$resourceEvidence.normalized_result.resources[0]
$resourceRva=U32 ($optional+128)
if (-not $resourceRva) {throw 'Missing RT_VERSION resource directory'}
$resourceRoot=RvaOffset $resourceRva; $directory=$resourceRoot
$resourceIds=@(16,1,1033)
for ($level=0;$level -lt 3;$level++) {
    if ((U16 ($directory+12)) -ne 0 -or (U16 ($directory+14)) -ne 1) {throw 'Resource tree differs from original single RT_VERSION/id1/lang1033 path'}
    if ((U32 ($directory+16)) -ne $resourceIds[$level]) {throw "Resource ID mismatch at level $level"}
    $target=U32 ($directory+20)
    if ($level -lt 2 -and ($target -band 0x80000000L) -eq 0) {throw 'Expected resource subdirectory'}
    if ($level -eq 2 -and ($target -band 0x80000000L) -ne 0) {throw 'Expected resource data leaf'}
    $directory=$resourceRoot+($target -band 0x7fffffffL)
}
$payloadRva=U32 $directory; $payloadSize=U32 ($directory+4);$payloadOffset=RvaOffset $payloadRva
$resourceCodePage=U32 ($directory+8);$resourceReserved=U32 ($directory+12)
$sha=[Security.Cryptography.SHA256]::Create()
$payloadHash=([BitConverter]::ToString($sha.ComputeHash($bytes,$payloadOffset,$payloadSize))).Replace('-','').ToLowerInvariant()
$sha.Dispose()
if ($payloadSize -ne $originalResource.payload.location.bytes -or $payloadHash -cne $originalResource.payload.sha256) {throw "RT_VERSION payload differs: size=$payloadSize SHA256=$payloadHash"}
if ($resourceCodePage -ne $originalResource.code_page -or $resourceReserved -ne $originalResource.reserved) {throw 'Resource code page/reserved fields differ'}
$null=New-Item -ItemType Directory -Path $OutputDirectory -Force
$utf8=New-Object Text.UTF8Encoding($false)
function Save([string]$name,$value) {[IO.File]::WriteAllText((Join-Path $OutputDirectory $name),($value | ConvertTo-Json -Depth 10),$utf8)}
Save 'compiled-pe-tables.json' ([ordered]@{image_base=('0x{0:x}' -f $imageBase);entry_rva=('0x{0:x}' -f $entryRva);sections=$sections;exports=$exports;imports=$imports})
Save 'compiled-wrapper-comparison.json' $wrapperResults
Save 'compiled-resource-comparison.json' ([ordered]@{
    result='PASS';type=16;id=1;language=1033;code_page=$resourceCodePage;reserved=$resourceReserved
    rebuilt_payload_rva=('0x{0:x}' -f $payloadRva);rebuilt_payload_file_offset=$payloadOffset
    original_payload_file_offset=$originalResource.payload.location.offset
    payload_bytes=$payloadSize;sha256=$payloadHash;comparison='Exact payload length and SHA-256 equality with retained original resource evidence'
    source='version-rebuild/version.rc';evidence_id=$resourceEvidence.evidence_id
})
$summary=[ordered]@{
    result='PASS';compiled_exports_and_ordinals=198;compiled_imports=7
    compared_wrappers=197;compared_wrapper_instructions=$total;compared_branch_targets=$branches
    compared_resolved_rip_memory_references=$memoryRefs;compiled_embedded_bytes=$d.embedded.size
    version_resource_exact_payload_match=$true;version_resource_bytes=$payloadSize;version_resource_sha256=$payloadHash
    dll_sha256=(Get-FileHash -LiteralPath (Join-Path $BuildDirectory 'version.dll') -Algorithm SHA256).Hash.ToLowerInvariant()
    entry_equals_exported_dllmain=$true
    normalization='Address remapping through retained original-VA labels/linker map; RIP addresses resolved from llvm-objdump comments and matched to IAT/state/name symbols; JE=JZ and JNE=JNZ; whitespace/case ignored; original registers/widths/immediates/branch destinations retained.'
    limitations=@('Static wrapper equivalence only; compiler-generated C++ DllMain was not proven instruction-equivalent.','PE layout, section RVAs, function addresses, exception/unwind metadata, and binary bytes can differ.','Embedded component remains original machine code, not recovered C++ source.','Original and rebuilt DLLs were never loaded or executed; full runtime equivalence remains untested.')
}
Save 'verification-summary.json' $summary
$summary | ConvertTo-Json -Depth 5
