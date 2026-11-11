# Current DLL C interface

Under the owner's latest instruction, Production returned to Brownfield's
three-export/ordinal layout: **DllMain@1, SURGE@2, DllShutdown@3**.
`DllWaitReady@4` and `DllServerPort@5`, their implementations, and public
declarations were removed. C++ source APIs remain in the separate
`citrine_eclipse_mirage.source.dll` development target.

The Windows loader invokes `DllMain` through the CRT entrypoint; tools do
not call it manually. `SURGE` is a RET-only branded import marker.
`DllShutdown` reports the actual worker/I/O/hook/provider cleanup result
through the existing status codes 0..4. The caller must not release its
`LoadLibrary` reference before a successful shutdown.

The runtime test host does not need the two removed management exports.
Instead, it observes the DLL-provided `EALsxPort` environment variable
and the actual LSX Challenge/GetProfile flow, exercising three
load/start/socket/profile/shutdown/unload cycles. Incorrect or missing
initial network responses cannot count as PASS. The C17 consumer links
against the three-export import library and checks real worker shutdown
and idempotence.

Name-based lookup uses the export name; ordinal-based lookup uses the numeric
identifier. Tools can display the name table alphabetically, so displaying
DllMain, DllShutdown, SURGE alongside 1, 3, 2 is compatible with the
Brownfield layout. The `.def` entries are written in ordinal order.
Neither display order nor `.def` line order specifies DLL startup or hook
activation order.

The original Anadius `CDX@1` / `RUNE@2` / `anadius@3` markers target the
same RET body; these unwanted names are not restored. Automatic compatibility
with the original consumer import `anadius64!anadius` is not claimed.
The earlier 548-byte ABI assertion was withdrawn because supporting source
evidence was absent. Earlier owner game-startup reports and the I/O/ABI
correction's pending Windows rerun belong to their own checkpoints. Assess
Windows/native and game acceptance against the selected commit and DLL using
[VALIDATION.md](VALIDATION.md), rather than inheriting those old statuses.

The complete contract is in `metadata/PUBLIC_DLL_ABI.json`.

The source-level status type is `DllShutdownStatus`. Its constants are
`DLL_SHUTDOWN_OK`, `DLL_SHUTDOWN_CONCURRENT`,
`DLL_SHUTDOWN_TIMEOUT`, `DLL_SHUTDOWN_WAIT_FAILED`, and
`DLL_SHUTDOWN_CLEANUP_FAILED`. Their values remain 0..4, respectively;
these are not additional DLL exports.
