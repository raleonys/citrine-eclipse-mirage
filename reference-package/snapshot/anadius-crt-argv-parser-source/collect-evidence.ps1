$ErrorActionPreference='Stop'
$outputs=Split-Path -Parent $PSScriptRoot
$sources=@(
    [pscustomobject]@{file='anadius-all-functions.asm.txt';sha='9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42';start='; 0x1800636c0 ';end='; 0x180063888 ';selected='selected.asm.txt';id='ev_ed00e9744f46469e25bb27fb3ebac38f987cac9513a6ef9442c3389e0e6d9aa1'},
    [pscustomobject]@{file='anadius-all-functions.pseudo.c';sha='26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb';start='/* 0x1800636c0 ';end='/* 0x180063888 ';selected='selected.pseudo.c';id='ev_339b48485a753b4766933230d35ec6871872e9333f98e60f454f384efd1b25bc'}
)
$records=@(foreach($item in $sources){
    $path=Join-Path $outputs $item.file
    $actual=(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
    if($actual -ne $item.sha){throw "Retained source digest mismatch: $($item.file)"}
    $all=[IO.File]::ReadAllText($path)
    $begin=$all.IndexOf($item.start,[StringComparison]::Ordinal)
    if($begin -lt 0){throw 'Selected function start absent'}
    $end=$all.IndexOf($item.end,$begin,[StringComparison]::Ordinal)
    if($end -lt 0){throw 'Selected function end absent'}
    $selected=Join-Path $PSScriptRoot $item.selected
    [IO.File]::WriteAllText($selected,$all.Substring($begin,$end-$begin),[Text.UTF8Encoding]::new($false))
    [ordered]@{file=$item.file;sha256=$actual;function='0x1800636c0';evidence_id=$item.id;selected=$item.selected;selected_sha256=(Get-FileHash -LiteralPath $selected).Hash.ToLowerInvariant()}
})
[ordered]@{
    target='anadius64.dll';target_sha256='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';provider='Ghidra 12.1.4';analysis='retained static evidence; no target opened or executed in this subtask';function_count=1;functions=@([ordered]@{address='0x1800636c0';recovered='parse_0636c0';direct_dependency='0x18006f9a8 signed-byte multibyte-lead predicate; mandatory external Effects provider';assembly_first='0x1800636c0';assembly_last='0x180063884'});retained_sources=$records;claims=@(
        [ordered]@{addresses='0x1800636ed-0x180063701';claim='character count reset before pointer count=1 and optional initial argv store'},
        [ordered]@{addresses='0x180063708-0x180063775';claim='program argument special quote stripping, pre-count/copy/predicate order, delimiter overwrite or copied NUL'},
        [ordered]@{addresses='0x18006372f-0x180063737;0x18006381c-0x18006381f';claim='both predicate arguments sign-extend an 8-bit byte to int32'},
        [ordered]@{addresses='0x1800637ac-0x1800637f9';claim='uint32 slash runs; odd/even slash quote rules; paired quote inside quote'},
        [ordered]@{addresses='0x180063812-0x180063842';claim='later byte write precedes predicate and count; trail count precedes trail write'},
        [ordered]@{addresses='0x180063847-0x180063864';claim='each NUL and final argv NULL counted; optional outputs independent'}
    );limitations=@('New std::span storage contracts and out_of_range guards are reconstruction choices for unsupported inputs.','Only normal flow is reconstructed; no SEH/unwind equivalence claim.','No original source, exact compiler/library version, target execution, native locale provider or complete DLL equivalence claim.');original_files_modified=$false
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
