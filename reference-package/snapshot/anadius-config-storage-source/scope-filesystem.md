# B8 configuration filesystem scope

`ConfigurationFilesystem` binds exactly three existing `configuration::Dependencies` methods: directory creation, directory enumeration and removal. It owns a shared `Filesystem` helper and an owned lower provider for all other methods. The same helper can be supplied to achievement persistence. B5 and B7 outer wrappers preserve their thirteen routes; these three yield sixteen selected routes before the independent PNG/storage additions. This is source/SDK composition, not a newly claimed native body or original MSVC ABI closure.

## Directory creation

The normal prefix/error flow follows retained `018e00` and `048fd8`. Empty input reports path error3. The existing recovered root-name parser chooses the initial prefix, including the extended-root drive-component case. Each subsequent component is passed to `CreateDirectoryW`; trailing separators cause the retained extra final-prefix call. A root-only path has no create call. `ERROR_ALREADY_EXISTS` invokes existing bounded `UsedStats` (flags3) to distinguish an existing directory from a file, including its sharing-violation and reparse-target paths.

The retained error-selection rule is preserved: errors2/3/53/123 do not replace the remembered non-ignorable error; a final failure selects the last remembered non-ignorable error when present, while final success returns success despite earlier prefix failures. This is not a generic stop-on-first-error implementation or compiler `std::filesystem` substitute.

## Enumeration

The retained `017a40` checks nonempty input and compares its C-string length to stored length. Empty paths or embedded NUL therefore fail with error2 before the SDK. Other filesystem routes retain native c_str truncation while their source lexical processing still sees the full stored path. Enumeration appends `*` with the existing recovered join algorithm; it uses `FindFirstFileExW` with `FindExInfoBasic`, `FindExSearchNameMatch`, no filter and options0. Only exact `.` and `..` entries are skipped. Returned paths follow SDK order without sorting. Error18 means normal end; other errors propagate with the retained constructor/operator++ operation label.

An important existing source-interface limit remains: `Dependencies.directory_entries` returns a vector, so this adapter eagerly gathers the collection before configuration starts removing avatar files. Original `01c870` has a live iterator and may already have removed earlier entries when a later enumeration error occurs. The vector interface does not reproduce that partial-side-effect prefix or provide a filesystem snapshot. Original iterator/shared-pointer/STL object layout, native allocation failure prefixes, races and concurrent mutation are not claimed.

## Removal

The normal flow follows retained `04948c`, `049434` and `048a48`: open with access0x10180/share7/OPEN_EXISTING/flags0x02200000, then access0x10000 retry only after access denied. The first disposition request uses class21 and flags0x13. Unsupported results1/50/87 use flags3, followed by the retained one-byte legacy FileDispositionInfo request when needed. With full access, fallback access-denied reads FileBasicInfo and may clear readonly, retry and restore readonly only for repeated access-denied. Other retry errors retain the native readonly side effect. Missing-name errors2/3/53/123 from the initial open are remove-no-op; errors from the access-denied retry are not normalized. No recursive deletion is introduced; ordinary OS path and reparse behavior still applies.

## Ownership, errors and deliberate boundaries

All SDK write buffers use actual `wchar_t` and typed SDK structs. Per-call owners release file/find handles on normal return and C++ unwinding. Failed close is a compiler `abort` terminal policy; full BOOL bits are tested. There is one explicit stronger cleanup boundary: retained raw049104 ignores the sharing-fallback FindClose result, while B8 makes a known failed close terminal rather than discarding ownership. Unknown throwing acquisition outcomes are also terminal; SDK callbacks must obey the nonthrowing Win32 outcome contract.

Selected `std::system_error` and compiler allocation/destruction are rebuilt policies, not original MSVC filesystem_error/EH/SEH ABI. The last recorded SDK LastError is preserved through local bookkeeping and exception construction; calls restore that state before entering the next SDK boundary. Close calls are SDK calls too. No implicit permission elevation, guessed path fallback, sorting, cancellation or DLL unload support is added. Methods have only per-call state, but OS filesystem races and injected API lifetime/synchronization remain caller responsibilities.

No previous source file was edited. No original DLL/game/rebuilt DLL was loaded or executed, no original bytes were changed, no real user CFG/token/achievement contents or blocked version artifacts were read. Strict production compilation with the existing C++20 compiler and `-O2 -Wall -Wextra -Werror` passed. Independent tests and whole-build results belong to the B8 owner; compilation alone does not establish them.

## Retained evidence

Target SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
Full pseudocode SHA-256: `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`.
Full assembly SHA-256: `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.
No fresh decompilation or original input read was needed for this source composition.

| Address | Pseudocode Evidence ID | Assembly Evidence ID |
|---|---|---|
| `0x180018e00` | `ev_aa91855c8f9a5cb7fc35e491b954dcb179e2fadbcad6947f765d1d8919c42aff` | `ev_c6c55d80f26c20c8a545576572b5bd9ce5ef777e51a5eaf971c2284a97bb0b5a` |
| `0x180048fd8` | `ev_ec2405434788aadef0cbc5881a31c3033b65cda584484ca8b02f2245de8f276c` | `ev_92f41332945d6575b5aeb3c071a6fa18d3c66f57143de347cc9e251d02632bc4` |
| `0x180049104` | `ev_005bb220a469430c8acd184c5ceaace3b27a578b843f9774e9842a86fcdb9122` | `ev_5c914d480dd2ba111f50296dae9a6904e1fc7b536ca1f76e0d57f5d3ea3b24fc` |
| `0x18004948c` | `ev_e72194d20aa35a443ed071ea2568e1157254a50f348d17fb16fc5638fff78b0d` | `ev_f466f245bd1484fd956c8dee43b3d19147868172fa2580f7ddc06039e39ff800` |
| `0x180049434` | `ev_55280ecfb3c34265727e88d7f2e18fcda8dc52b6f97e2bc017c934ac978d90b4` | `ev_7b745f5e76486d9818cd43f80a889202153ab46ab790662f974a21f9ab121c1b` |
| `0x180048a48` | `ev_83cbb2f8ee62f066318ab6451a975c49dcf1c2792c5f53a864b19ae81a00cc72` | `ev_5baee9bfa31c7624534f1964cbe75f3f6b45b318e9d5b20498ea34f76e9e5bc0` |
| `0x180011a40` | `ev_ce0c5dec45c662c9f2baac77b5e61e47cb04a6ad1950c85f881ed16e13b2a781` | `ev_b6a386f368816e9c88abd16557006ead0cdbd676e44597ed950e8d426c704dce` |
| `0x180017a40` | `ev_43d76c5092901a642bc14870de0f639acd0a7988b96aa1b64e002477da21b4a7` | `ev_073421e7009ee6649c0534af37f1fe40035a04ddfc4a8e9723c4f4bbc033f747` |
| `0x180018020` | `ev_d532a7075a66b9f651d5bb641f3f891753c1a39cd132f3cc392bdbc437e11713` | `ev_3908d9cb30feee1d07d5a1d41292b7d56375e463003c98ffe9d68c330988ba01` |
| `0x180017110` | `ev_b73cfdf1cde3954d962ee0f307723dae4ede3cb6ce29cbe6c2092083fd65cb47` | `ev_47c64d27e32e48a3c5bd00e18a8048d29f8b97869ceb18d26aa73c29f086ba57` |
| `0x180017e80` | `ev_aabdd47ad28fda81aba724b13dc93508b9f396aa91e73e4ff09953209501fa2e` | `ev_7587a6b3adbb997ddbffeb0fe567b20f06a43087f095e23a897c74658e5f5fcf` |
| `0x180049090` | `ev_77de3b02f24450f016f913598b06f9c4b2430d28fc397ca2f66214ad72ee00c0` | `ev_884a18cda4c9376da3bc0364e8d7beafae8e29b85d269fc044eb892813fafc26` |
| `0x180049050` | `ev_79d8bde589d46498589382b7ad7fcd6393a1d33c3dedc7d44e5676cfe0fb3196` | `ev_903f1816da43db010ffe952a406e509e3941b4223c563a8961a3ab3033783330` |
| `0x180049070` | `ev_68e0cf0d5ebe35359569ce91801d9e71db632e1701f28c670aaf740c69141256` | `ev_e7c6921d71ee74b642c3b6ba3ae8a6729379866c535f4b822d42338e387fd21f` |
