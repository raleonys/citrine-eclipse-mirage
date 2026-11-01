param(
    [string]$EvidencePath = (Join-Path $PSScriptRoot '..\version-reconstruction-input.json')
)
$ErrorActionPreference = 'Stop'
$d = Get-Content -LiteralPath $EvidencePath -Raw | ConvertFrom-Json
$utf8 = New-Object System.Text.UTF8Encoding($false)
function Save-Text([string]$Name, [string]$Text) {
    [IO.File]::WriteAllText((Join-Path $PSScriptRoot $Name), $Text, $utf8)
}
if ($d.pe.exports.Count -ne 198 -or $d.allAssembly.Count -ne 198 -or $d.allPseudo.Count -ne 198) {
    throw 'Expected exactly 198 retained exports, assemblies and pseudocodes.'
}
$asms = @{}; $pseudos = @{}
foreach ($a in $d.allAssembly) { $asms[$a.name] = $a }
foreach ($p in $d.allPseudo) { $pseudos[$p.name] = $p }
$asm = New-Object System.Text.StringBuilder
$null = $asm.AppendLine('; Reconstructed from retained REA/Ghidra wrapper assembly. Do not edit generated file.')
$null = $asm.AppendLine('option casemap:none')
$null = $asm.AppendLine('EXTERN __imp_GetProcAddress:QWORD')
$null = $asm.AppendLine('EXTERN OuterState:QWORD')
$null = $asm.AppendLine('.const')
foreach ($e in $d.pe.exports) {
    if ($e.name -eq 'DllMain') { continue }
    if ($pseudos[$e.name].text -notmatch ('GetProcAddress\(.*?"' + [regex]::Escape($e.name) + '"')) {
        throw "Missing exact resolver name evidence: $($e.name)"
    }
    $null = $asm.AppendLine(('name_{0} BYTE "{1}",0' -f $e.ordinal, $e.name))
}
$null = $asm.AppendLine('.code')
$manifest = @(); $instructionCount = 0; $internalBranchCount = 0
foreach ($e in $d.pe.exports) {
    if ($e.name -eq 'DllMain') { continue }
    $a = $asms[$e.name]; $p = $pseudos[$e.name]
    if ($a.address -ne $e.address -or $p.address -ne $e.address) { throw 'Address mismatch' }
    $lines = @($a.text -split '\r?\n' | Where-Object {$_})
    $addresses = @{}
    foreach ($line in $lines) {
        if ($line -notmatch '^(0x[0-9a-f]+): (.+)$') { throw "Unexpected assembly: $line" }
        $addresses[$Matches[1]] = $true
    }
    $symbol = "reconstructed_$($e.ordinal)"
    $null = $asm.AppendLine("; $($e.name) @ $($e.ordinal); original VA $($e.address)")
    $null = $asm.AppendLine("; assembly $($a.evidence_id); pseudocode $($p.evidence_id)")
    $null = $asm.AppendLine("PUBLIC $symbol")
    $null = $asm.AppendLine("$symbol PROC FRAME")
    $prologue = $true
    foreach ($line in $lines) {
        $null = $line -match '^(0x[0-9a-f]+): (.+)$'
        $address = $Matches[1]; $op = $Matches[2]
        $null = $asm.AppendLine(('L_{0}:' -f $address.Substring(2)))
        if ($op -match '^J[A-Z]+ (0x[0-9a-f]+)$') {
            $destination = $Matches[1]
            if (-not $addresses.ContainsKey($destination)) { throw "Out-of-function branch: $line" }
            $op = $op.Replace($destination, ('L_' + $destination.Substring(2)))
            $internalBranchCount++
        }
        elseif ($op -match '^LEA RDX, \[0x[0-9a-f]+\]$') {
            $op = "LEA RDX, [name_$($e.ordinal)]"
        }
        elseif ($op -eq 'CALL qword ptr [0x180006030]') {
            $op = 'CALL qword ptr [__imp_GetProcAddress]'
        }
        else {
            $op = [regex]::Replace($op, '\[(0x18000e[0-9a-f]{3})\]', {
                param($m)
                $offset = [Convert]::ToInt64($m.Groups[1].Value.Substring(2),16) - 0x18000e000L
                if ($offset -lt 0 -or $offset -gt 0x88 -or ($offset % 8) -ne 0) { throw 'Unexpected global' }
                '[OuterState+' + $offset + ']'
            })
        }
        $op = [regex]::Replace($op, '0x([0-9a-f]+)', { param($m) '0' + $m.Groups[1].Value + 'h' })
        if ($op -match '\[018000[0-9a-f]+h\]') { throw "Untranslated absolute address: $line" }
        $null = $asm.AppendLine("    $op")
        if ($prologue -and $op -match '^PUSH ([A-Z0-9]+)$') {
            $null = $asm.AppendLine(('    .pushreg {0}' -f $Matches[1]))
        }
        elseif ($prologue -and $op -match '^SUB RSP, (.+)$') {
            $null = $asm.AppendLine(('    .allocstack {0}' -f $Matches[1]))
            $null = $asm.AppendLine('    .endprolog')
            $prologue = $false
        }
        elseif ($prologue) { throw "Unexpected prologue: $line" }
        $instructionCount++
    }
    $null = $asm.AppendLine("$symbol ENDP")
    $cacheSlot = $null
    if ($a.text -match 'MOV RAX, qword ptr \[(0x18000e[0-9a-f]{3})\]') { $cacheSlot = $Matches[1] }
    $module = if ($a.text.Contains('[0x18000e078]')) {'winmm'} elseif ($a.text.Contains('[0x18000e080]')) {'kernel32'} else {'version'}
    $manifest += [pscustomobject]@{
        name=$e.name; ordinal=$e.ordinal; original_va=$e.address; original_rva=$e.rva
        symbol=$symbol; module=$module; cached=($null -ne $cacheSlot); original_cache_slot=$cacheSlot
        instruction_count=$lines.Count; assembly_evidence=$a.evidence_id; pseudocode_evidence=$p.evidence_id
    }
}
$null = $asm.AppendLine('END')
Save-Text 'wrappers.asm' $asm.ToString()
$def = "LIBRARY version`r`nEXPORTS`r`n"
foreach ($e in $d.pe.exports) {
    $symbol = if ($e.name -eq 'DllMain') {'ReconstructedDllMain'} else {"reconstructed_$($e.ordinal)"}
    $def += ('    {0}={1} @{2}' -f $e.name,$symbol,$e.ordinal) + "`r`n"
}
Save-Text 'version.def' $def
$hex = $d.embedded.bytes_hex -replace '\s',''
if ($hex.Length -ne 2 * $d.embedded.size -or $hex -notmatch '^[0-9a-fA-F]+$') { throw 'Embedded byte length invalid' }
$bytes = New-Object byte[] ($hex.Length / 2)
for ($i=0; $i -lt $bytes.Length; $i++) { $bytes[$i] = [Convert]::ToByte($hex.Substring(2*$i,2),16) }
$sha = [Security.Cryptography.SHA256]::Create()
$embeddedHash = ([BitConverter]::ToString($sha.ComputeHash($bytes))).Replace('-','').ToLowerInvariant()
$null = $sha.Dispose()
$inc = New-Object System.Text.StringBuilder
$null = $inc.AppendLine('// OPAQUE ORIGINAL MACHINE CODE, NOT RECONSTRUCTED C++ SOURCE.')
$null = $inc.AppendLine(('// Evidence: {0}; outer VA {1}; SHA-256 {2}' -f $d.embedded.parentEvidence,$d.embedded.outerVA,$embeddedHash))
for ($i=0; $i -lt $bytes.Length; $i+=16) {
    $last = [Math]::Min($i+15,$bytes.Length-1)
    $null = $inc.AppendLine(('    ' + (($bytes[$i..$last] | ForEach-Object {'0x{0:x2}' -f $_}) -join ', ') + ','))
}
Save-Text 'embedded_original_machine_code.inc' $inc.ToString()
Save-Text 'export-evidence.json' ($manifest | ConvertTo-Json -Depth 6)
$summary = [ordered]@{
    scope='Outer version.dll reconstruction only; embedded PE is original machine code, not recovered source'
    wrapper_count=$manifest.Count; export_count=$d.pe.exports.Count
    winmm_wrappers=@($manifest | Where-Object module -eq 'winmm').Count
    kernel32_wrappers=@($manifest | Where-Object module -eq 'kernel32').Count
    cached_version_wrappers=@($manifest | Where-Object cached).Count
    translated_wrapper_instructions=$instructionCount; verified_internal_branches=$internalBranchCount
    embedded_bytes=$bytes.Length; embedded_sha256=$embeddedHash
    embedded_evidence=$d.embedded.parentEvidence; dllmain_evidence=$d.entry.evidence_id
    pe_header_evidence=$d.pe.headerEvidence; pe_rdata_evidence=$d.pe.rdataEvidence
    binary_compilation='Generation does not compile; see build-llvm/build-record.json for separate compilation evidence when present'
    runtime_validation='Not performed: original and reconstructed DLLs were not loaded or executed'
}
Save-Text 'validation.json' ($summary | ConvertTo-Json -Depth 6)
$summary | ConvertTo-Json -Depth 6
