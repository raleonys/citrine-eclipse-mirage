$ErrorActionPreference='Stop'
$outputs=Split-Path -Parent $PSScriptRoot
$asm=Join-Path $outputs 'anadius-all-functions.asm.txt'
$pseudo=Join-Path $outputs 'anadius-all-functions.pseudo.c'
$expectedAsm='9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42'
$expectedPseudo='26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb'
$inputPath='C:\Users\Skysect\Downloads\anadius64.dll'
$inputHash='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c'
if((Get-FileHash -LiteralPath $inputPath).Hash.ToLowerInvariant() -ne $inputHash){throw 'Original DLL identity changed'}
if((Get-FileHash -LiteralPath $asm).Hash.ToLowerInvariant() -ne $expectedAsm){throw 'Retained assembly identity changed'}
if((Get-FileHash -LiteralPath $pseudo).Hash.ToLowerInvariant() -ne $expectedPseudo){throw 'Retained pseudocode identity changed'}
$asmBody=[regex]::Match([IO.File]::ReadAllText($asm),'(?ms)^; 0x18006e984 .*?(?=^; 0x|\z)').Value
$pseudoBody=[regex]::Match([IO.File]::ReadAllText($pseudo),'(?ms)^/\* 0x18006e984 .*?(?=^/\* 0x|\z)').Value
if(!$asmBody -or !$pseudoBody){throw 'Bounded search evidence missing'}
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'selected-assembly.txt'),$asmBody,[Text.UTF8Encoding]::new($false))
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'selected-pseudo.txt'),$pseudoBody,[Text.UTF8Encoding]::new($false))
[ordered]@{
 artifact='anadius64.dll';sha256=$inputHash;original_input_verified=$true;image_base='0x180000000';
 method='Bounded static reuse of retained REA/Ghidra assembly and pseudocode. This module is a new Windows x64 SDK binding, not an additional recovered binary function or original class ABI.';
 retained=[ordered]@{assembly_sha256=$expectedAsm;pseudo_sha256=$expectedPseudo;assembly_evidence='ev_6cea0a61974294918c3ea08f45c00073e8eac70ff3dc2c8d4bacd2c87cb5cc92';pseudo_evidence='ev_9d4e1a037001a99805dd5060b51fa408366c82fd37bce78d84abd290a4f8c5bf'};
 selections=[ordered]@{assembly_sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'selected-assembly.txt')).Hash.ToLowerInvariant();pseudo_sha256=(Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'selected-pseudo.txt')).Hash.ToLowerInvariant();function='0x18006e984';end_inclusive='0x18006ecd0'};
 callsites=@(
  [ordered]@{address='0x18006eaf4';iat='0x18007f330';api='FindFirstFileExW';evidence='06eadb selects record at RBP-0x20; 06eadf tests complete conversion status; 06eaeb selects NULL pattern on conversion failure; 06eae1/06eae6/06eaef/06eaf2 zero flags/filter/search/info; 06eafd compares handle to -1'};
  [ordered]@{address='0x18006ec2a';iat='0x18007f338';api='FindNextFileW';evidence='06ec23 selects the same RBP-0x20 record; 06ec27 moves full RBX handle; 06ec33 tests full EAX'};
  [ordered]@{address='0x18006ec6a';iat='0x18007f320';api='FindClose';evidence='06ec67 moves full RBX handle after sort; return BOOL ignored'};
  [ordered]@{address='0x18006ec96';iat='0x18007f320';api='FindClose';evidence='06ec93 moves full RBX handle on append error; return BOOL ignored'}
 );
 layout_evidence='Original record base is RBP-0x20, filename source is RBP+0xc at 06ebc0: difference44. Windows SDK static assertions enforce WCHAR2, BOOL4, HANDLE8, record592, cFileName44/520.';
 source_choices=@('Stable owning typed record per live handle, with preallocated map node before first API call','Only cFileName storage is exposed; new bounded host reads use memcpy','No filename output published on failed first or next','Destructor frees host records only, never implicitly calls FindClose','Only INVALID_HANDLE_VALUE (-1) rejected; zero and high-bit handles preserved','Unknown/double handles and duplicate active handles throw explicit host domain_error without invented OS cleanup');
 limitations=@('Bookkeeping/address-domain exceptions and class layout are new source choices, not original behavior or SEH reconstruction','No actual native enumeration, target DLL execution, or runtime equivalence measurement','Single Session requires externally serialized access; independent active records and sessions support nested usage','Caller supplies a live UTF16 WCHAR pattern pointer, or NULL as observed on conversion failure; no pointer validation or pathname policy added','No GetLastError, attribute filtering, path normalization, sorting, or lexical-order policy in this boundary')
} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence.json') -Encoding UTF8
