> **Historical checkpoint.** Use [RE_COPY_RECOVERY.md](RE_COPY_RECOVERY.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

> This research examined a source snapshot before the full copy
> helper was integrated. Current implementation:
> [RE copy recovery](RE_COPY_RECOVERY.md).
> The probes and JSON files mentioned below belong to the separately
> delivered `surge-api-import-investigation.zip` research package.

# SURGE / Anadius: source and linkage analysis of six API differences

October 29, 2026. Product code was not modified. This report
narrows uncertainties in the previous import comparison; it does
not validate priorities based solely on import names.

## Were four apparently missing APIs omitted during the port?

These four imports do not imply four forgotten application features.
For two, the implementation of an older library changed; for the
other two, unused branches of the original general-purpose helpers
produce imports even though the observed application call does
not select those branches.

| API | Original location | Current source and finding |
|---|---|---|
| GetFileInformationByHandle | File-identity helper `FUN_18004895c`, called by certain options of general copy helper `FUN_180048d20`. | The avatar call site shows `0x180031b4c MOV R8D,2` and `0x180031b52 CALL048d20`. Mode 2 directly enters `CopyFileW(...,FALSE)` without calling the identity helper. Our `content_handlers` `copy_overwrite` contract and Environment adapter execute the same branch. Since the entire multi-option standard helper was not linked at this checkpoint, the import was unnecessary. No porting omission was found for this observed use. |
| InitializeCriticalSectionAndSpinCount | Legacy Asio helper `027be0` and old VCRT/UCRT compatibility thunks `050c38` and `6707c`. | The RE native adapter still exists in `include/citrine_eclipse_mirage/net/transport_service_constructor_native.hpp`. The active network implementation is `src/net/asio_service.cpp` with newly vendored Asio, whose `win_mutex` uses SRWLOCK. The examined modern static UCRT imports InitializeCriticalSectionEx, not the older SpinCount fallback. Not every body in the old native adapter must remain live in Production. |
| TryAcquireSRWLockExclusive | Try/timed branches of old general mutex helper `04a6d4`. | Observed direct caller `04a6ac` supplies `deadline=nullptr` via `XOR EDX,EDX`, taking the blocking AcquireSRWLockExclusive path. The provided `anadius-execution-source` recovers only that untimed path, and current `execution.cpp` has the same contract. In the examined MSVC 14.51 `_Mtx_lock`, blocking Acquire is used; the Try API appears only in try/timed bodies. New Asio also defines `try_lock`, but this alone produces no import without active use. No missing try-lock call was found on the observed application path. |
| VerifyVersionInfoA | Legacy IOCP constructor `024990`, selecting wait behavior by OS version. | The RE native adapter remains in `include/citrine_eclipse_mirage/net/transport_iocp_constructor_native.hpp`. Active modern Asio `get_gqcs_timeout` directly returns INFINITE when `_WIN32_WINNT>=0x0600`. Brownfield's chosen `0x0601` target selects that path. The separate application VerifyVersionInfoW host/hook check is different. |

There is more than one Asio difference: lock type and legacy
OS-version checking are two distinct changes, and the removal
of a critical-section compatibility fallback in the modern CRT
also matters. The try-lock and file-identity examples distinguish
a helper's presence in a binary from actual use of a particular
branch. This does not establish full DLL behavioral equivalence.

## CreateFile2: concrete origin

The owner log identifies MSVC 19.51.36260 and the
`Tools/MSVC/14.51.36231` path. The published Microsoft 14.51
CRT x64 Desktop package was downloaded. Its package SHA-256
matched the URL/metadata pin, and it contained `libcpmt.lib`
under the 14.51.36231 path. This does not mean its bytes were
compared against the owner's local LIB.

In `libcpmt.lib(filesystem.obj)`, direct
`__imp_CreateFile2` relocations were observed in
`__std_fs_remove`, `__std_fs_open_handle`, and related
bodies. The application's `fs::remove` uses this standard
filesystem family. An isolated link with `__std_fs_remove`
reproduced the CreateFile2 import. The cause was the modern
MSVC STL's file-opening backend, not a first-party
CreateFile2 call or residual Gulrak dependency.

Microsoft STL PR 5434 (April 29, 2025) documents the change
from the older desktop CreateFileW path to CreateFile2 and
removal of some legacy fallbacks. It would nevertheless be
incorrect to infer that modern MSVC contains no
GetFileInformationByHandle: the examined 14.51 archive still
contained the corresponding helper. In this investigation,
the verified explanation for the missing product import
was selecting the observed mode-2 branch instead of linking
the general-purpose copy helper.

Defining `_WIN32_WINNT=0x0601` for Asio does not rebuild
precompiled `libcpmt.lib` and cannot reverse that API
selection. CreateFile2 was introduced in Windows 8;
neither a build macro nor a PE header alone proves
Windows 7 runtime compatibility.

## HeapQueryInformation: CRT dependency through Asio aligned allocation

The MSVC branch of vendored Asio `memory.hpp` uses
`_aligned_malloc`/`_aligned_free` for
`aligned_new`/`aligned_delete`. In the examined official
Windows SDK 26100.7627 static `libucrt.lib` archive:

1. `align.obj` contains aligned allocation/free and aligned
   reallocation bodies in the same non-COMDAT `.text$mn` section.
2. The reallocation branch in that section has a relocation
   to `_expand`.
3. `expand.obj` implements `_expand`/`_expand_base` and
   contains a `__imp_HeapQueryInformation` relocation.
4. In Microsoft's official CRTSource 22621.3 `expand.cpp`,
   failure to shrink a small heap block triggers a query
   for LFH status through HeapCompatibilityInformation.
   The query does not modify heap settings.

The isolated `_aligned_malloc`/`_aligned_free` link log
shows `align.obj` loading `expand.obj` through `_expand`,
with HeapQueryInformation in the final import table.
An ordinary malloc/free link against the same CRT does
not import it. This reproduces the concrete dependency
chain, rather than merely guessing that it may originate
from the CRT. The linker can include the import because
of a shared object/section dependency even if aligned
reallocation never executes in the application.

## Seven controlled linkage experiments

| Experiment | Result |
|---|---|
| malloc/free | No HeapQueryInformation. |
| _aligned_malloc/_aligned_free | HeapQueryInformation present. |
| _Mtx_lock | AcquireSRWLockExclusive present; TryAcquireSRWLockExclusive absent. |
| _Mtx_trylock | TryAcquireSRWLockExclusive present. |
| __std_fs_remove | CreateFile2 present; GetFileInformationByHandle absent. |
| CopyFileW(...,FALSE) | CopyFileW present; GetFileInformationByHandle absent. |
| General `__std_fs_copy_file(...,2)` | GetFileInformationByHandle present because unused option branches are linked too. |

Each experiment used actual official static CRT/STL archives.
The compiler was Clang 21 targeting MSVC COFF; the linker
was LLD. This was not native `cl.exe`/`link.exe` execution.
`/O2 /MT /GS`, explicit static library inputs, and
`/OPT:REF` were used. The `/Zl` probe flag only removes
default-library directives; CRT libraries were supplied
explicitly. No experiment forced unresolved symbols or
used fake API stubs. All seven DLLs linked successfully;
inspection of their import tables confirmed 7/7 expected
results. The DLLs were not executed, and this was not
SURGE Golden or game acceptance.

## Scope and remaining certainty limits

The final MSVC DLL/MAP used in the owner's comparison
was not supplied in this investigation. Consequently,
every call-site RVA in the target DLL was not matched
definitively. Nevertheless, the RE call arguments,
existing adapters, real MSVC library objects, and
comparative link experiments identified concrete
mechanisms behind these import differences.
Product code, existing ZIPs, vendor contents,
export/ordinal/Golden policies remained unchanged;
no speculative import was added as though a missing
feature had been established.

Download note: the returned Visual Studio channel-manifest
bytes did not match its declared SHA-256/length, and this
was recorded. The manifest was used only to discover
the official payload URL. The examined CRT payload's
own SHA-256 matched its pin, while the metadata length
differed; length was not trusted over the cryptographic
hash. Byte equality with the owner's local library
was not independently verified.

## Primary sources

- [Microsoft STL PR5434](https://github.com/microsoft/STL/pull/5434)
- [Microsoft STL filesystem source, pinned commit](https://github.com/microsoft/STL/blob/b6f01e438b68ee497f9c08313e49d12bb2f0d8e0/stl/src/filesystem.cpp)
- [Microsoft STL mutex source, pinned commit](https://github.com/microsoft/STL/blob/b6f01e438b68ee497f9c08313e49d12bb2f0d8e0/stl/src/mutex.cpp)
- [Microsoft CRTSource 22621.3 package](https://www.nuget.org/packages/Microsoft.Windows.SDK.CRTSource/10.0.22621.3)
- [Microsoft SDK CPP x64 26100.7627 package](https://www.nuget.org/packages/Microsoft.Windows.SDK.CPP.x64/10.0.26100.7627)
- [Microsoft _expand documentation](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/expand)
- [Microsoft HeapQueryInformation documentation](https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapqueryinformation)

Complete import results are in `LINK_PROBE_RESULTS.json`,
call evidence in `RE_CALL_PATHS.json`, and library/object
hashes in the corresponding JSON records. Files
with .c/.map/log extensions under Probes are diagnostic
evidence; Microsoft libraries and DLL binaries are not
bundled in the research report package.
