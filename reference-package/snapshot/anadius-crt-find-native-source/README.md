# Find API SDK bridge

`find_native.hpp/cpp` supplies typed Windows x64 imports for the three existing
find effects used by `search_06e984`. It is a new source bridge, not an additional
function recovered from the DLL or a reconstruction of an original C++ class ABI.

`Api` uses the SDK declarations of `FindFirstFileExW`, `FindNextFileW` and
`FindClose`, including `WINAPI`, 64-bit handles, 32-bit signed `BOOL`, and typed
`WIN32_FIND_DATAW`. `system_api()` binds the real imports. Tests replace all three
with typed fixture functions; they never enumerate native directories or load
the target DLL.

`Session::first` forwards the caller's live UTF-16 pattern pointer (including
NULL on conversion failure), info/search zero, filter NULL and flags zero.
Only the all-ones handle is invalid. Valid zero and high-bit handles are preserved.
`next` returns the complete signed BOOL rather than a low-byte or bool conversion.
`close` ignores BOOL and ends the record lifetime even when the API reports failure.
There are no attribute filters or `GetLastError` calls in this boundary.

Each active handle owns a stable, typed 592-byte record; the map node is allocated
before the first API call. `filename_storage` exposes only its 520-byte `cFileName`
array at offset 44. The same array is passed to subsequent calls and remains
allocated until explicit close or session destruction, so nested search flows
and multiple active handles do not overwrite each other's filename buffers.
Its contents may change on any subsequent API call for that handle, including
failed next calls whose output is unspecified. `contains`, `read_u8` and
`read_u16` restrict access to live filename arrays; unaligned word reads use
`memcpy`. A zero-byte span at the end is allowed. Borrowed spans expire on close
or destruction, even if a later allocation reuses the numerical address.

The destructor intentionally performs no `FindClose`: original normal-flow close
order belongs to the search core, and this bridge does not claim original SEH
or unwind cleanup. Production callers must execute explicit close on their
normal paths. Unknown/double handles and duplicate active handles are explicit
new `domain_error` domains; no synthetic return status or OS cleanup is invented.
Constructor API fields must all be non-NULL. Bookkeeping failures, SDK provider
exceptions and invalid caller pointers are outside original-equivalence claims.
Calls sharing one session need external serialization; independent sessions have
independent storage. No original address is dispatched as a host function pointer.

`evidence.json` and bounded `selected-assembly.txt`/`selected-pseudo.txt` preserve
hashes and REA evidence IDs. At `0x18006eaf4`, the first API is called via IAT
`0x18007f330`; at `0x18006ec2a`, next uses `0x18007f338` and full EAX is tested;
close uses `0x18007f320` at `0x18006ec6a` and `0x18006ec96`. The original record
base `RBP-0x20` and filename source `RBP+0xc` establish offset 44 independently
of the SDK assertions. Names are associated with these call sites by retained
pseudocode; the assembly determines argument and result handling.

Run `build.ps1` to compile the production object and execute the injected fixture
test. It checks ABI layout, exact arguments, NULL/invalid/zero/high-bit handles,
full BOOL values, UTF-16 including surrogate pairs, maximum filename length,
stable storage across next calls and 96 simultaneous records in two sessions,
failure lifetime, stale bounds, duplicate handles and no implicit close on faults
or destruction. `validation.json` records compiler identity, commands and hashes.
Passing tests establish these source contracts; original binary runtime equality
and complete DLL reconstruction remain unproven.
