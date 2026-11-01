param([string]$EvidencePath = (Join-Path $PSScriptRoot '..\version-reconstruction-input.json'))
$ErrorActionPreference = 'Stop'
$d = Get-Content -LiteralPath $EvidencePath -Raw | ConvertFrom-Json
$assembly = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'wrappers.asm') -Raw
$definition = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'version.def') -Raw
$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'export-evidence.json') -Raw | ConvertFrom-Json
$instructionCount = 0
foreach ($e in $d.pe.exports) {
    $symbol = if ($e.name -eq 'DllMain') {'ReconstructedDllMain'} else {"reconstructed_$($e.ordinal)"}
    $expectedExport = '{0}={1} @{2}' -f $e.name,$symbol,$e.ordinal
    if (($definition -split '\r?\n' | ForEach-Object {$_.Trim()}) -cnotcontains $expectedExport) { throw "Missing export $expectedExport" }
    if ($e.name -eq 'DllMain') { continue }
    $original = @($d.allAssembly | Where-Object name -eq $e.name)[0]
    $originalLines = @($original.text -split '\r?\n' | Where-Object {$_})
    $originalNameAddress = [regex]::Match($original.text, 'LEA RDX, \[(0x[0-9a-f]+)\]').Groups[1].Value
    $body = [regex]::Match($assembly, '(?ms)^' + $symbol + ' PROC FRAME\r?\n(.*?)^' + $symbol + ' ENDP').Groups[1].Value
    if (-not $body) { throw "Missing procedure: $symbol" }
    $rebuiltLines = @($body -split '\r?\n' | Where-Object {$_ -match '^    [A-Z]'} )
    if ($rebuiltLines.Count -ne $originalLines.Count) { throw "Instruction count mismatch: $symbol" }
    for ($i=0; $i -lt $originalLines.Count; $i++) {
        $op = $rebuiltLines[$i].Trim()
        $op = [regex]::Replace($op, 'L_([0-9a-f]+)', {param($m) '0x' + $m.Groups[1].Value})
        $op = $op.Replace("[name_$($e.ordinal)]", "[$originalNameAddress]")
        $op = $op.Replace('[__imp_GetProcAddress]', '[0x180006030]')
        $op = [regex]::Replace($op, '\[OuterState\+(\d+)\]', {
            param($m) '[0x' + (0x18000e000L + [long]$m.Groups[1].Value).ToString('x') + ']'
        })
        $op = [regex]::Replace($op, '\b0([0-9a-f]+)h\b', {param($m) '0x' + $m.Groups[1].Value})
        $expected = ($originalLines[$i] -split ': ',2)[1]
        if ($op -cne $expected) { throw "Instruction mismatch for $($e.name): expected '$expected', got '$op'" }
        $instructionCount++
    }
    if ([regex]::Matches($body, '(?m)^    \.endprolog\r?$').Count -ne 1) { throw "Invalid unwind prologue: $symbol" }
}
if ([regex]::Matches($definition, '(?m)^    .+ @\d+\r?$').Count -ne 198) { throw 'Export count mismatch' }
$inc = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'embedded_original_machine_code.inc') -Raw
$payload = ($inc -split '\r?\n' | Where-Object {$_ -notmatch '^//'}) -join ''
$hex = ([regex]::Matches($payload, '0x([0-9a-f]{2})') | ForEach-Object {$_.Groups[1].Value}) -join ''
if ($hex -cne ($d.embedded.bytes_hex -replace '\s','').ToLowerInvariant()) { throw 'Embedded bytes mismatch' }
if ($manifest.Count -ne 197) { throw 'Manifest count mismatch' }
$result = [pscustomobject]@{result='PASS'; exact_wrapper_instructions=$instructionCount; exports=198; opaque_bytes=($hex.Length/2); execution='none'}
$json = $result | ConvertTo-Json
$utf8 = New-Object System.Text.UTF8Encoding($false)
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'static-source-validation.json'), $json, $utf8)
$json
