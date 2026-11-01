param([string]$InputDll='C:\Users\Skysect\Downloads\anadius64.dll')
$ErrorActionPreference='Stop'
$expected='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
if((Get-FileHash -LiteralPath $InputDll -Algorithm SHA256).Hash.ToLower() -ne $expected){throw 'Original DLL identity mismatch'}
$parent=Split-Path $PSScriptRoot -Parent
$assembly=Join-Path $parent 'anadius-all-functions.asm.txt'
$pseudo=Join-Path $parent 'anadius-all-functions.pseudo.c'
$asmLines=[IO.File]::ReadAllLines($assembly)
$pseudoLines=[IO.File]::ReadAllLines($pseudo)
$functions=@('0x180035b00','0x180035b60','0x180035bd0','0x1800366f0','0x18003cf00','0x18003cd30')
$records=@(foreach($va in $functions){
  $a=@($asmLines | Where-Object {$_ -like "; $va *"})
  $p=@($pseudoLines | Where-Object {$_ -like "/* $va *"})
  if($a.Count -ne 1 -or $p.Count -ne 1){throw "Nonunique function evidence $va"}
  [ordered]@{va=$va;assembly_evidence=([regex]::Match($a[0],'ev_[0-9a-f]{64}')).Value;
    pseudocode_evidence=([regex]::Match($p[0],'ev_[0-9a-f]{64}')).Value}
})
$anchorNotes=[ordered]@{
 '0x180035b11'='status flags 3';'0x180035b36'='error kind-minus-one mask excludes 1 and 9';
 '0x180035b3d'='return regular-file kind 2';'0x180035b69'='worker wait_steps(1)';
 '0x180035b74'='online flag gates normal cleanup';'0x180035b8c'='destroy object at virtual slot 0';
 '0x180035baa'='clear module after free returns';'0x180035bb5'='finish flag address a3c3c';
 '0x180035c09'='suppress error mode';'0x180035c18'='filesystem codepage';
 '0x180035c46'='literal conversion byte count 0x13';'0x180035cd1'='root-name helper in parent calculation';
 '0x180035d84'='path append';'0x180035d8d'='load copied path';
 '0x180035d92'='store module';'0x180035e26'='restore mode normal path';
 '0x180035e62'='GetProcAddress version_check';'0x180035e8e'='required version 2';
 '0x180035f2d'='mismatch dialog flags 0x10';'0x180035f60'='GetProcAddress create_online';
 '0x180035fac'='factory copied cookie argument';'0x180035fb1'='store factory object';
 '0x180036002'='status virtual+8';'0x18003600d'='online flag before all profile callbacks';
 '0x180036039'='username virtual+0x10';'0x18003606c'='persona virtual+0x18';
 '0x180036098'='user ID virtual+0x20';'0x1800360c5'='live branch cookie file test';
 '0x1800360d8'='live branch avatar file test';'0x180036107'='save profile flag';
 '0x18003632d'='write profile return ignored';'0x18003645a'='status 1 adds authentication failed fragment';
 '0x180036488'='one flush after offline log fragment';'0x18003648d'='cached profile flag';
 '0x1800364a1'='cached profile requires cookie regular file';'0x180036517'='read binary profile';
 '0x18003651d'='split limit 0';'0x18003652b'='split helper call';
 '0x180036540'='only exactly three fields';'0x18003656a'='assign field 0 persona';
 '0x180036594'='assign field 1 user';'0x1800365be'='assign field 2 username';
 '0x180036703'='general cleanup virtual slot 0';'0x180036705'='clear object after destroy';
 '0x18003671c'='general cleanup FreeLibrary';'0x180036721'='clear module after FreeLibrary'
}
$anchors=@(foreach($entry in $anchorNotes.GetEnumerator()){
  $line=@($asmLines | Where-Object {$_ -like "$($entry.Key): *"})
  if($line.Count -ne 1){throw "Nonunique instruction $($entry.Key)"}
  [ordered]@{va=$entry.Key;instruction=$line[0];observation=$entry.Value}
})
$bytes=[IO.File]::ReadAllBytes($InputDll)
$literals=@(foreach($rva in @(0x86400,0x86418,0x86430,0x86440,0x86460,0x86488,0x864b0,0x864c0,0x864e0,0x864f8,0x86510,0x84698)){
  $offset=$rva-0x7f000+0x7d800
  if($rva -lt 0x7f000 -or $rva -ge (0x7f000+0x21e00)){throw 'Out of rdata'}
  $end=$offset
  while($end -lt (0x7d800+0x21e00) -and $bytes[$end] -ne 0){$end++}
  if($end -eq (0x7d800+0x21e00)){throw 'Unterminated bounded literal'}
  [ordered]@{va=('0x180{0:x6}' -f $rva);raw_offset=('0x{0:x}' -f $offset);
    length=$end-$offset;ascii=[Text.Encoding]::ASCII.GetString($bytes,$offset,$end-$offset)}
})
[ordered]@{artifact='anadius64.dll';sha256=$expected;preferred_base='0x180000000';evidence_type='retained REA static assembly and pseudocode plus hash-bound rdata';
  sources=@([ordered]@{file='../anadius-all-functions.asm.txt';sha256=(Get-FileHash -LiteralPath $assembly -Algorithm SHA256).Hash.ToLower()},
    [ordered]@{file='../anadius-all-functions.pseudo.c';sha256=(Get-FileHash -LiteralPath $pseudo -Algorithm SHA256).Hash.ToLower()});
  rdata=[ordered]@{va='0x18007f000';raw_offset='0x7d800';size='0x21e00';evidence='ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082'};
  functions=$records;anchors=$anchors;literals=$literals;
  conclusions=@(
    [ordered]@{kind='observed';finding='Four main application function bodies control load, profile cache and normal cleanup';boundary='listed functions and anchors'},
    [ordered]@{kind='observed';finding='The used split path is semicolon delimiter, unlimited limit=0, exact-three-fields gate and explicit byte lengths';boundary='03cd30,03cf00 and 036506..0365be'},
    [ordered]@{kind='inference';finding='names username/persona/user ID are semantic field labels from downstream uses; provider virtual ABI is represented by required Effects';boundary='virtual+10,+18,+20 calls and globals'},
    [ordered]@{kind='unknown';finding='Actual online provider implementation, network behavior, global initialization, MSVC CRT/EH/object ABI and original runtime equivalence remain unresolved';boundary='provider module not analyzed or executed'});
  execution='No original target, online DLL or rebuilt target DLL executed';user_data='No real cookies, profile identities or credentials read';
  source_domain='valid object; valid NUL terminated callback strings; stable caller-owned shared state; finite representable strings; no provider retention of temporary factory argument'} |
  ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
Write-Output "Recorded $($records.Count) functions, $($anchors.Count) instruction anchors and $($literals.Count) bounded static literals"
