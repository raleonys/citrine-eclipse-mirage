# Independent source inventory mapping audit — 23 October 2026

Status: canonical/build verification complete; final generated inventory sample review pending.

Scope: retained static REA/Ghidra listings, recovery metadata, C++ source and existing compiled objects. Original DLL/game/network execution was not performed. Original files and production source/build configuration were not modified. Real configuration, token and achievement files were not read. This report does not change the legacy rough completion estimate or claim native behavioral equivalence.

## Verified canonical scope

`anadius-recovery-index.json.pseudocodeIndex`, full pseudocode headers and full assembly headers contain the same 2,076 unique local function entries. Of these, 2,031 are not thunks and 45 are local thunks. The broader procedure inventory contains 170 external entries; its total of 2,246 is not the local function denominator. Reproducible checks and SHA-256 values are in `inventory-independent-probe.ps1/json`.

`version-all-functions.pseudo.c` covers 198 outer version.dll local functions. The embedded preloader function CSV contains another 107 functions and is a different image. Numeric VA `0x180003e80` is present in both the preloader and anadius canonical inventories. All mappings require an artifact/digest namespace; a bare numeric VA is insufficient.

## Verified compiled source scope

The PASS common build record contains 177 main production unit compilations: 173 C++ and four C. A further four MinHook synthetic compatibility compilations must not be counted again as main production units. Tests, independent archives and address references are not original function bodies.

`llvm-nm --defined-only --demangle` was run only against the 177 existing generated source objects. The retained `inventory-object-symbols.txt` has 814 address-tagged defined symbol lines, 459 unique six-digit tags and 435 tags matching canonical native entries. These are cross-check counts only: adapters, abstract models, methods and native body implementations can share a tag, while inline code can have no separate object symbol. None of these counts is an implemented/native-equivalent function total.

## Mapping pitfalls and bounded samples

- `queued-write-evidence.json.functions[]` identifies newly recovered `0x18002a500`; its `context_functions[]` contains previously recovered bodies and must not add implementations. Pseudocode evidence: `ev_74e713f6d63b5db2c8594c89965baf991c280377ff6f5607819938f99daa863f`; assembly evidence: `ev_7b4a354660d536700feb53ad52ca0a2f961ca2042dbeb3a635143135bd1fe188`.
- `anadius-native-source/evidence.json.functions[].original_va` records SDK adapters around previously reconstructed code. Its explicit mapping states "no new original function claim". A schema field named functions does not make every entry a recovered native body.
- `composed_write.cpp` defines `composed_write_0261d0` and the `RecoveredComposedWriteEffects::send_body_0261d0` bridge. Existing `server.cpp` also defines `Session::send_completed_0261d0` as a coarser model. This is one canonical native entry with several source projections, not three native functions. The same overlap occurs for `025a70` and `02a500`.
- A pure virtual declaration such as the required `receive_body_025c40` provider, a call site, an address constant, a comment, a contextual decompiler listing or a test fixture is not a C++ definition of that native body. Definition detection must exclude these classes.
- Some anonymous/helper/adapter methods use data/global-address suffixes, such as `0a3838`; these are not canonical function entries.
- `anadius-crt-pair-leaves-source/evidence.json.raw_gap_leaves[]` explicitly documents four new raw-gap bodies outside the 2,076 canonical Ghidra entries. Examples `0x1800704e8` and `0x1800644f0` must remain an annex/exclusion, rather than silently inflating or being lost in the canonical denominator. The 24 noncanonical object tags are not all such bodies.

## Required interpretation

Address-tagged definition presence is a searchable lower-bound source mapping, not proof of complete recovery. Evidence-only module associations should retain their weaker status. An unmapped canonical entry means the current mapper did not prove a matching compiled production definition; it does not prove that no semantically corresponding helper exists. Rebuildable standalone DLL readiness, native ABI/global initialization/lifetime closure and runtime equivalence require separate evidence. A count of passed test groups or production objects is not a native function completion percentage.
