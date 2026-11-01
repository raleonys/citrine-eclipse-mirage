$ErrorActionPreference='Stop'
$parent=Split-Path -Parent $PSScriptRoot
$binary='C:\Users\Skysect\Downloads\anadius64.dll'
$expectedBinary='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
$expectedPseudo='26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'
$expectedAssembly='9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42'
$pseudoPath=Join-Path $parent 'anadius-all-functions.pseudo.c'
$assemblyPath=Join-Path $parent 'anadius-all-functions.asm.txt'
foreach($pair in @(
    [pscustomobject]@{path=$binary;expected=$expectedBinary},
    [pscustomobject]@{path=$pseudoPath;expected=$expectedPseudo},
    [pscustomobject]@{path=$assemblyPath;expected=$expectedAssembly}
)) {
    $actual=(Get-FileHash -LiteralPath $pair.path).Hash.ToLowerInvariant()
    if($actual -ne $pair.expected){throw "Evidence hash mismatch: $($pair.path)"}
}
$pseudo=Get-Content -LiteralPath $pseudoPath -Raw
$assembly=Get-Content -LiteralPath $assemblyPath -Raw
$addresses=@('180063888','1800638e8','18005d724','18006ecd4')
$records=@()
$selectedPseudo=@()
$selectedAssembly=@()
foreach($address in $addresses){
    $pseudoMatch=[regex]::Match($pseudo,"(?ms)^/\* 0x$address .*?(?=^/\* 0x|\z)")
    $assemblyMatch=[regex]::Match($assembly,"(?ms)^; 0x$address .*?(?=^; 0x|\z)")
    if(-not $pseudoMatch.Success -or -not $assemblyMatch.Success){throw "Missing retained evidence: $address"}
    $selectedPseudo+=$pseudoMatch.Value.TrimEnd()
    $selectedAssembly+=$assemblyMatch.Value.TrimEnd()
    $records+=[ordered]@{address="0x$address";role=$(if($address -in @('180063888','1800638e8')){'reconstructed'}else{'dependency-only'});pseudo_evidence=[regex]::Match($pseudoMatch.Value,'Evidence (ev_[0-9a-f]+)').Groups[1].Value;assembly_evidence=[regex]::Match($assemblyMatch.Value,'Evidence (ev_[0-9a-f]+)').Groups[1].Value}
}
$selectedPseudo -join "`r`n`r`n" | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'selected-pseudo.c.txt') -Encoding UTF8
$selectedAssembly -join "`r`n`r`n" | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'selected-assembly.txt') -Encoding UTF8
[ordered]@{
    artifact='anadius64.dll';sha256=$expectedBinary;image_base='0x180000000';
    method='Retained hash-bound REA/Ghidra decompilation and assembly, inspected statically; assembly resolves ordering and misleading __doserrno label';
    retained=[ordered]@{pseudo_sha256=$expectedPseudo;assembly_sha256=$expectedAssembly};
    functions=$records;
    globals=@(
        [ordered]@{address='0x1800a5180';role='260-byte module filename buffer'},
        [ordered]@{address='0x1800a59e0';role='program name pointer'},
        [ordered]@{address='0x1800a59e8';role='argc DWORD'},
        [ordered]@{address='0x1800a59f0';role='argv pointer'},
        [ordered]@{address='0x1800a5a00';role='command line pointer'}
    );
    decisive_instructions=@(
        '06389b/06389e: pointer count must be strictly below 0x1fffffffffffffff',
        '0638a6/0638ac: unsigned DIV element size and strict character multiplication guard',
        '0638bc/0638bf: reject character bytes >= complement(pointer bytes)',
        '0638c9/0638d3: calloc(total bytes,1), then free(NULL), even when calloc fails',
        '063910/063918/06391a: 05d724 cell <-22 before invalid-parameter handler',
        '05d73b: errno cell is PTD+0x20; decompiler __doserrno name is misleading',
        '06393d/063949: ignore module filename return, publish module buffer as program name',
        '06397a/063990/0639cc: count parse, contiguous allocation, actual writing parse',
        '0639dc/0639e3: mode 1 publishes argv before argc (low DWORD count-1)',
        '06ecd4: JMP 06e5bc; wildcard expansion is a required unresolved lower provider',
        '063a03..063a13: mode 2 failure frees expansion output then parse allocation',
        '063a26..063a43: successful expansion counts terminated pointers before publishing argc then argv',
        '063a4a/063a56: successful mode 2 free(NULL), then original parse allocation'
    );
    scope='Two reconstructed function bodies. Mode 2 wildcard algorithm remains required provider; module path, multibyte initialization, allocation, free and invalid parameter are explicit effects. No real DLL/process/native API execution.';
    limitations=@('Zeroed State defaults are new C++ choices, not an original image initialization claim.','Malformed provider buffers, unterminated spans and DIV-by-zero are explicit unsupported source domains, not original error returns.','Only observed normal control flow is reconstructed; no inferred SEH or cleanup on provider exceptions.','Repeated configure calls overwrite published argv without inventing release of an earlier published allocation.','Pointer/counter low DWORD behavior does not prove full binary ABI or complete DLL equivalence.')
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
Write-Output 'PASS: retained argv evidence and original SHA-256 verified; two bodies and two dependency excerpts.'
