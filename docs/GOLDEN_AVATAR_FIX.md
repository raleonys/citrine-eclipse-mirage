> **Historical checkpoint.** Use [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

> Historical snapshot: subsequent corrections for hook 11/lifecycle and
> metadata are in
> [Bluefield alignment](HOOK_ALIGNMENT.md) and
> [Brownfield metadata audit](REFERENCE_METADATA_AUDIT.md).

# Avatar, import, and Golden corrections

Work item: `YF-GOLDEN-AVATAR-20261029`. The earlier
`yellowfield-msvc-native-lookup-fix-source.zip` package is retained unchanged.

The owner's explicit selection, translated: "It should not be a PE resource;
it should replace the existing Anadius avatar data." PNG RCDATA 101 was
removed. The same 3,653-byte SURGE PNG is converted at build time into
a private, read-only C++ array and replaces the RE profile-avatar data
at `0x1800809d0`. No external PNG is required at runtime, and existing
configuration avatars or provider artwork are not overwritten. The PNG
file in the source tree serves as a build input.

The separate default PNG at `0x180082710` is not replaced with the
same artwork. The original 3,882-byte fallback was recovered directly
from the supplied `anadius-rea-evidence.json` memory data; all PNG
chunk CRCs were verified. No Brownfield/Bluefield application code was
copied. Hash, address, and evidence records:
[AVATAR_DATA_CONTRACT.json](../metadata/AVATAR_DATA_CONTRACT.json).
`version.rc` bytes and the VERSIONINFO profile did not change.

`OLE32.dll` was not an export; its import resulted from
`SHGetKnownFolderPath` and `CoTaskMemFree` in the earlier adapter.
RE `FUN_180039bc0` invokes `GetEnvironmentVariableW` in increments of
260 UTF-16 units. `LocalAppData` and `ProgramData` were restored to
that original behavior, and the `ole32` link input was removed.
The names/ordinals of the DLL's five exported C functions were unchanged
at that checkpoint.

The latest owner-provided Native log passed the prior `unexpected()`
error but encountered C2589 for `std::min` in an encoding unit test.
NOMINMAX preventing the Windows `min` macro was set on an object
library but did not propagate to core consumers through
`$<TARGET_OBJECTS>`. `citrine_eclipse_mirage_core PUBLIC NOMINMAX` corrected
this propagation. Vendor sources were not modified and tests were
not skipped.

The four missing Golden targets were also completed under earlier
owner authorization. Compiler identity is retained; only the checksum
is finalized in the copy. Fixed fixture timestamp/linker values are
not promoted to product policy. See
[Golden product targets](GOLDEN_PROTOCOLS.md).

Test results and actual binary observations from this phase are
retained in
[GOLDEN_AVATAR_FIX.json](../metadata/evidence/GOLDEN_AVATAR_FIX.json).
LLVM/Wine tests do not substitute for actual Windows/MSVC/game
acceptance. Native and Golden profiles require reruns on Windows
with the changed source. A SHA-256 sidecar using the standard
`hash  filename` format is generated beside the new source ZIP.

Local results: Release 173/173, affected Debug 5/5, and source QA 15/15
passed. Both actual LLVM DLLs contained only the VERSIONINFO resource;
both avatar images were in read-only `.rdata`, and there was no OLE32
import. Five C exports were preserved. These results are not actual
MSVC Golden or game acceptance.
