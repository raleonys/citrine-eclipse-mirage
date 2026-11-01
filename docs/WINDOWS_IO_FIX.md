# Restoring Windows I/O and removing an unused vendor

The owner requested correction of the Windows filesystem difference and
complete removal of Gulrak, which had no other active use. This changed the
earlier five-vendor target list by explicit owner instruction.

| Area | Current implementation |
|---|---|
| NativeFactory input/output | `std::filesystem::path` plus `std::ifstream/ofstream`, as in the supplied RE adapter |
| Directory creation/enumeration/removal | Runtime alias `fs=std::filesystem`; Windows/MSVC compiler-owned filesystem |
| Debug logging | `std::ofstream` with retained `#ifndef NDEBUG` guard |
| Gulrak usage | No longer present in application, includes, targets, or test backend |
| Gulrak vendor | `third_party/filesystem`, CMake interface target, include dependency, and current lock entry removed |
| I/O tests | Unicode/binary/rename/remove/error checks preserved with standard filesystem |
| Remaining vendors | Asio, MinHook, PEGTL, TinyXML2; original member hashes and TinyXML2 limit 500 unchanged |
| Asio diagnostics | Brownfield `_WIN32_WINNT=0x0601` / `WINVER=0x0601` definitions propagated to consumers |

The owner's original Gulrak archive and prior source packages remain preserved
outside the current product snapshot. The current product has no vendor payload
for it; historical metadata comparison records may retain its name for
provenance purposes.

This change selects the Windows/MSVC standard filesystem implementation.
It does not claim full import/byte/edge-case equivalence with the historical
Anadius CRT. LLVM/Wine tests do not replace actual Windows testing of the
Microsoft STL implementation.

## Production exports and logging

The Production `.def` now contains the same three C exports as Brownfield:
`DllMain@1`, `SURGE@2`, and `DllShutdown@3`. The implementations and ordinals
4/5 for `DllWaitReady` and `DllServerPort` were removed. C++ source APIs
were moved to a separate development DLL. The runtime host verifies readiness
and the port through existing `EALsxPort`/LSX greeting and profile replies
rather than additional public APIs.

Exporting `DllMain` is not required for Windows loader startup: the Windows
CRT entrypoint calls it. Export ordinals are numeric ABI identifiers;
alphabetic display order and `.def` line order do not determine call,
startup, or hook order.

The `surge.log` writer is compiled only into non-NDEBUG Debug builds.
Release creates no new log and does not append to a pre-existing Debug log;
user files are not deleted automatically. Mere presence of an existing log
does not prove that Production wrote it. An integration regression checks
this distinction in both build configurations.

The owner confirmed game startup with the earlier startup-build-fix package.
Actual Windows/MSVC and game reruns remain required for the new I/O/build
configuration. Current results are in
`metadata/evidence/WINDOWS_IO_FIX.json`.

## Required import DLL order

The owner required the final list to follow the original Anadius order:
CRYPT32, WS2_32, KERNEL32, USER32, SHELL32, MSWSOCK. This is unrelated to
export ordinal order. The finalizer only permutes complete
`IMAGE_IMPORT_DESCRIPTOR` entries; it preserves their RVA/name/thunk/IAT
values and guards all other bytes. It fails closed if the module set differs.
The null descriptor is preserved, the checksum recomputed, and CORVEXON
separately verifies final order.

This order applies only to `dist/preloader_l.dll` after Golden finalization.
The raw `build/preloader.unsigned.dll` retains linker order. The LLVM
development DLL includes dynamic CRT modules and cannot count as shipping
acceptance; runtime load/LSX/shutdown/unload tests of the same permutation
helper are separately evidenced.
