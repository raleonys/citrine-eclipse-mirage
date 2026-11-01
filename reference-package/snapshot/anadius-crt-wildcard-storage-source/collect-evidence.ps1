$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$records=@(
    @{file='anadius-all-functions.asm.txt';sha256='9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42';output='selected.asm.txt';start='(?m)^; 0x18006e800 ';stop='(?m)^; 0x18006e984 '},
    @{file='anadius-all-functions.pseudo.c';sha256='26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb';output='selected.pseudo.c';start='(?m)^/\* 0x18006e800 ';stop='(?m)^/\* 0x18006e984 '}
)
foreach($item in $records){
    $inputPath=Join-Path $root $item.file
    if((Get-FileHash -LiteralPath $inputPath).Hash.ToLowerInvariant() -ne $item.sha256){throw "Retained evidence digest mismatch: $($item.file)"}
    $contents=[IO.File]::ReadAllText($inputPath)
    $begin=[regex]::Match($contents,$item.start)
    $end=[regex]::Match($contents,$item.stop)
    if(-not $begin.Success -or -not $end.Success -or $end.Index -le $begin.Index){throw 'Bounded function markers missing'}
    $excerpt=$contents.Substring($begin.Index,$end.Index-$begin.Index)
    [IO.File]::WriteAllText((Join-Path $PSScriptRoot $item.output),$excerpt,[Text.UTF8Encoding]::new($false))
}
