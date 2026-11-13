> **Historical checkpoint.** Use [HOOK_PATH_SCAN.md](HOOK_PATH_SCAN.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

> Later explicit owner decision: unbounded/heap-based path reading was
> superseded by the [HOOK_PATH_SCAN.md](HOOK_PATH_SCAN.md) contract.
> The limit of 32,768 UTF-16 units, fixed suffix buffer, and protected reads
> apply. The unload scanner's later retained-handle correction is documented in
> [UNLOAD_RECOVERY.md](UNLOAD_RECOVERY.md); its body is no longer token-identical
> to the original Bluefield scanner. Descriptions of the older reader and
> body identity below are retained
> as historical records within that scope.

# Bluefield hook 11 alignment

The owner selected the Bluefield implementation as the correct reference.
The previous RE UTF-8/UD2 selector and independent unload policy were
inconsistent with that decision and were removed. The transferred scope
covers `CryptQueryObject` and the shared native hook lifetime management
required by it. Other application modules were not copied from Bluefield.

| Area | Recorded behavior / source |
|---|---|
| Selector | Bluefield `crypt_query.cpp`; include/namespace adaptation only; normalized body unchanged |
| Filename reading | UTF-16 unit reads via `ReadProcessMemory`; pointer-overflow check; no fixed path bound in the original version |
| Unreadable/invalid UTF-16 | Fall back to the original object; no UD2/heap UTF-8 conversion |
| Matching | Case-sensitive `preloader_l.dll` compatibility suffix; companion `EAAntiCheat.GameServiceLauncher.dll`; Bluefield null/non-file rules |
| Original call | Preserve all SDK arguments/output pointers and the result |
| LastError / observation | Preserve incoming/outgoing LastError in Bluefield order; thread-local phase observation |
| Missing trampoline | Bluefield failure log and fail-fast; no fabricated silent return |
| Target | Direct SDK `&CryptQueryObject` and mandatory static CRYPT32 import |
| Activation | Queue all native hooks, release the configuration lock, then apply once |
| Quiescence | Disable, wait a bounded period for admitted calls, check external threads/registers/stacks, then uninitialize |
| Unload guard | Bluefield body with only the include name changed; header byte-identical |
| Shutdown caller | Exclude the public shutdown thread from the guard scan, as in Bluefield |

The implementation adapter uses the existing Hooks/NativeApi/Binding
boundaries; the entire Bluefield application class was not copied.
The protected reader was moved to a separate host boundary, and lifetime
cleanup was connected to the existing worker. Source hashes and exact
adaptations are in `metadata/evidence/HOOK_ALIGNMENT_STATUS.json`.
SPDX GPL-3.0-only notices are retained.

Validation covers real MinHook queue/apply, disable/uninitialize, original-call
arguments, LastError, invalid UTF-16, unreadable and page-boundary path reads,
live external-thread guards, and DLL startup/shutdown/unload tests.
Full test records are associated with the same status file. Wine tests do not
replace actual Windows/MSVC/game acceptance; no in-game call trace was supplied.

The separate metadata correction is recorded in
[the Brownfield field table](REFERENCE_METADATA_AUDIT.md).
