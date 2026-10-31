# Recovery of the original general-purpose file copy helper

The owner asked whether this part could be recovered from the original DLL.
The prepared RE `content_handlers` source defined only the
`copy_overwrite` (mode 2) effect. The original DLL dump contained
the general body 048d20, the file-identity helper 04895c, and the
last-write-time query 0489f4. This pass reconstructed the observed
decisions in all three bodies directly from pseudocode and assembly.

| Option (`options & 0xF`) | Observed behavior |
|---|---|
| 0 | First call `CopyFileW(fail_if_exists=TRUE)`, returning its result and error code. |
| 1 | If the target exists, open two handles (`share=READ + WRITE`) and compare identities. Different files give skip/false/0; the same file gives false/32. |
| 2 | Call `CopyFileW(fail_if_exists=FALSE)` without entering the other option branches. |
| 4 | If the target exists, open it with `share=READ` and compare signed 64-bit last-write times. If the source is newer, first close the target and source handles, then overwrite. Otherwise continue to the identity check. |
| Other low-nibble values | Do not reject them: follow the original `share=READ` identity branch. Ignore high option bits. |

The file-identity helper first invokes
`GetFileInformationByHandleEx(FileIdInfo=0x12,24 bytes)`.
Only errors 50/87 trigger the `GetFileInformationByHandle`
fallback. In the legacy result, the 32-bit volume is widened
to 64 bits, indexHigh and indexLow each occupy four bytes,
and the remaining eight bytes are zeroed. Other errors are
returned unchanged. The time helper reads `LastWriteTime`
from the 0x28-byte `FileBasicInfo` query result.

Handles are closed target before source. A `CloseHandle`
failure ends with `abort`, as in the original. Short-circuit
ordering and cleanup are retained for source/target open,
time-query, and identity-query failures. The
`INVALID_HANDLE_VALUE` with `GetLastError=0` edge case
is not normalized into a newly invented error.

The implementation is in
[file_copy.cpp](../src/platform/file_copy.cpp) and the
actual SDK boundary in
[file_copy_windows.cpp](../src/platform/file_copy_windows.cpp).
The [source interface](../include/preloader_l/platform/file_copy.hpp)
carries explicit boolean/error fields. It does not claim to
reproduce three undefined padding bytes of the original
compiler or its former C++ binary ABI. Private CRT/heap
sources were not imported; other standard filesystem/fstream
paths and the four vendors were preserved.

Instead of directly shortcutting to `CopyFileW`, the avatar
adapter calls the general body in mode 2. Normal avatar
behavior remains the original mode-2 branch. All other options
are available in source, and the legacy file-identity call is
connected to the real helper. The current Release development
DLL has verified static imports of
`GetFileInformationByHandle`,
`GetFileInformationByHandleEx`, and `CopyFileW`.
This does not force a different identity query on every
avatar copy or fabricate imports.

Controlled tests cover option masks, unknown bit combinations,
signed timestamp boundaries, all identity bytes, 50/87
fallback and other errors, short-circuit ordering, and fatal
close ordering. A Windows test executable covers Unicode/
binary files, skip/overwrite/update, the same file,
hard links, and actual metadata queries. Results are recorded
in `metadata/evidence/RE_COPY_RECOVERY.json`.
Wine/cross-build, actual Windows/MSVC Golden, and the owner's
game acceptance are separate evidence classes.
