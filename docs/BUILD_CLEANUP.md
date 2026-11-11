> **Historical checkpoint.** Use [BUILD_PARTITION.md](BUILD_PARTITION.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

# Production linkage and diagnostic cleanup

The owner requested that the confirmed dead-code audit decisions
be implemented together with the TLS diagnostic correction.
Four source files were moved out of Production linkage into
the retained reference/source-API group without deletion.
At that cleanup checkpoint the partition was **60 Production and 78 reference
CPP files, 138 total**. The current partition is maintained in
[BUILD_PARTITION.md](BUILD_PARTITION.md) and its machine policy. Public headers, four vendors, source
API signatures, three Production exports, and Golden
compiler/linker policy remain unchanged.

| Audit item | Implementation |
|---|---|
| DC-001–004 | Move `config/order.cpp`, `config/config_startup.cpp`, `net/session_routing.cpp`, and `platform/windows_effects.cpp` exclusively to the reference/API/test group. |
| DC-005 | Remove the obsolete `load_codemodel` Python reader unused by the active CLI; retain the generation-time graph gate and CMake file-API query. |
| DC-006 | Remove only seven unused import bindings across six files. |
| DC-007 | Remove an unused-result SHA-1 calculation in the avatar test; retain PNG/fallback byte assertions and the hash API. |
| DC-008–011 | Simplify a provably redundant token-length subcondition, an alias guard in a different-root branch, a repeated ClassicIso EOF check, and an identical QPC ternary. |
| Crypt diagnostics | Move `crypt_observed` outside NDEBUG builds, while retaining the Debug reentry guard and existing `debug_log` call. Preserve virtual method layout, hook selection, original invocation, LastError, and lifetime contracts. |

The earlier `config/startup.cpp` was renamed to
`config/config_startup.cpp` to avoid sharing the same
`startup.obj` name in the MSVC MAP with the Production
`runtime/startup.cpp`. Its body was byte-identical;
headers and C++ symbol names were unchanged.
No source body was deleted by that cleanup; all 138 checkpoint CPP files
were retained. Subsequent additions are outside this historical count.

The former model's 1,440 `config_hash` comparisons
were moved to a separate reference test, as was the
former bootstrap composition of achievement storage.
The actual hash/storage tests still run on the
Production-only core. `config_order`, session
integration, and WindowsEffects model tests were
classified as reference and retained without changing
their bodies or assertions. The portable hash test
was separated from the model dependency, with a
separate portable differential target added.
A Production test is not represented as passing merely
by linking the reference library.

Public representation/metadata/ownership designs and
mixed translation-unit proposals DC-012–019 were
outside this narrow cleanup. In particular,
`leases_` reservations, retry after failed native
release, quarantine for uncertain socket results,
pending I/O counters, buffer/receipt captures, and
the image lease remain intact. The generic unload
helper remains in source-API/test roots; it does not
replace checked close.

The TLS guard existed in the preceding source as well.
This cleanup does not establish the cause of a
VirusTotal label or predict that the label will
disappear. Section/TLS/raw-DLL/development-export
differences under the same toolchain are reported
separately in final validation. Cross-build
differences are not presented as native MSVC gains.

The checkpoint's reviewed decisions are in
[BUILD_CLEANUP_REVIEW.json](../metadata/BUILD_CLEANUP_REVIEW.json);
validation is in
[VALIDATION.json](../metadata/evidence/build-cleanup/VALIDATION.json).
Native Windows/MSVC/Golden and game acceptance must
be repeated for the changed source; earlier acceptance
cannot be transferred. Donor retirement remains under
review.

## Local results and limits

Release 205/205, affected Debug 23/23, and portable
21/21 tests passed. Source QA passed in 17/17 groups;
portable and infrastructure subchecks overlap, so
these totals must not be added together.
The Production/reference MAP boundary passed;
all 3,346 source-API export names matched exactly
before and after. Twenty new partition regressions
check that the four model objects do not reappear in
the Production-named archive and that startup
object filenames remain distinct.

Using the same LLVM-MinGW toolchain, the raw Release
DLL shrank from 1,083,392 to 1,070,080 bytes, a
13,312-byte reduction. Of that difference, 6,144
bytes were PE raw sections and 7,168 bytes were
out-of-section COFF tables/padding; this is not
13 KiB of machine code or an MSVC improvement.
`.text` VirtualSize fell by 3,600 bytes; the TLS
template fell by eight bytes, while the TLS raw
section remained 512 bytes. No independent size
or speed gain was attributed to each narrow edit.
The actual target's Release OBJ has no diagnostic
TLS; Debug callback instructions match the earlier
source.

These are LLVM-MinGW/Wine and Linux portable results,
not MSVC/Golden or game execution. For a full ZIP
clean-extract build with tests/source APIs disabled
and three actual DLL cycles, use the external final
record `surge-build-cleanup-delivery-verification.json`.
Build/log evidence is bound to that exact source
snapshot.
