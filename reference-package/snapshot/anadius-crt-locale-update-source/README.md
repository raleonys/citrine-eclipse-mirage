# Locale update constructor and pair refresh helpers

Reconstructed normal-flow C++ for three `anadius64.dll` routines, from the retained exact-input REA/Ghidra assembly and pseudocode. This is recovered source, not the original vendor source or a complete CRT locale implementation. Run `build.ps1` for the standalone production object and synthetic fixture test executable; `validation.json` records the successful run and source hashes.

| Original VA | Recovered function | Key evidence |
| --- | --- | --- |
| `0x180058b04` | `locale_update_058b04` | `0x180058b13` clears ownership byte first; `0x180058b23–38` snapshots/stores a pair; `0x180058b3e–6d` acquires PTD and calls both helpers; `0x180058b72–88` reloads flags and acquires bit 2 only if absent. |
| `0x180064ef0` | `update_locale_info_064ef0` | `0x180064ef6–f11` compares current global locale and then the PTD/mask; `0x180064f13–18` refreshes and stores the returned address. |
| `0x180064f5c` | `update_multibyte_info_064f5c` | `0x180064f62–7d` performs the equivalent multibyte checks; `0x180064f7f–84` refreshes and stores. |

Target SHA-256 is `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. `evidence.json` includes the three bounded assembly/pseudocode records, original evidence IDs and complete retained-file hashes. Assembly resolves sequencing and partial writes where decompiler expressions are less explicit.

`Context` aliases the existing byte-classification `LocaleContext`. The explicit-locale and global-fast paths preserve its PTD word: the original constructor does not initialize that word on those paths. Pair reads precede both stores, preserving overlapping pair semantics under the provider's serialized access contract. Two abstract reads do not reproduce atomic SIMD memory access or concurrent racing behavior.

The changed-locale path reads PTD `+0x90` then `+0x88`, updating output words in the original order. It calls the real reconstructed helper functions. The first helper receives the initially acquired PTD; the second and final flag guard reload the context PTD. Each helper reads its current global before comparing the output field, then reads flags before mask. Refresh return values, including zero, are stored directly. The final ownership byte becomes one only after writing thread bit 2; no constructor cleanup is added.

All lower dependencies are required pure virtual effects. Native locale/PTD ownership, thread refresh implementations, locking, table refcounts, allocation, exception/SEH cleanup, and locale object lifetime remain integration responsibilities. The nearby indexed routines at `0x180064f24`/`0x180064f90` are intentionally outside this module because their index/array domains have not been established. No success-return placeholders or target DLL loading is used.

The fixture run passes **399,644 checks**: exhaustive low-byte flags/mask combinations for both helpers, full DWORD high-bit checks, exact call order, explicit/global fast paths, aliasing snapshots, nested construction, updater mutation, context PTD reloads, zero refresh results and partial-write ordering. Fixture exceptions delimit effects only; they do not assert original SEH behavior. All memory addresses are fixture identities bound to owned C++ map storage or live context fields.
