# Bounded allocation-free hook path inspection

The owner explicitly adopts Brownfield's bounded, allocation-free path scanning
for hook11. The supplied reference archive's src/platform/crypt_observer.cpp uses
kMaxPathUnits = 32768. The existing protected-read and suffix decision are adapted
without importing the reference's second direct caller-memory suffix read.

The scan permits at most 32768 UTF-16 code-unit reads, including the terminating
NUL: a valid path can therefore contain at most 32767 non-NUL units. A terminator
at the final permitted unit is accepted; a terminator just beyond it is not read.
No CFG, provider or environment override is introduced.

Only a 15-unit circular suffix buffer for the `preloader_l.dll` compatibility
suffix is retained. This suffix is independent of the current shipping DLL name. Native inspection and selection
perform no C++ heap allocation, no whole-path copy and no UTF-8 conversion. The
original CryptQueryObject API and unrelated hook infrastructure may allocate;
this is not a claim that the entire API invocation is allocation-free. The callback
uses ReadProcessMemory for each observed unit and checks the entire two-byte
address range before pointer arithmetic. Validation checks surrogate pairs inline.
A valid, terminated stream is required before accepting the retained suffix.

The result contains validity, suffix-match and examined-unit count, not an owned
path string. The selector never rereads caller memory after protected inspection.
This is an observed stream, not an atomic snapshot of concurrently modified memory.
Unreadable, malformed, unterminated or over-limit input follows the original-object
branch; the real API remains responsible for its own subsequent argument handling.

Non-file objects, null-file selection, case sensitivity, ASCII suffix matching
without a separator requirement, companion identity, the real BOOL result, all
other arguments/output pointers and incoming/outgoing LastError retain their
existing contracts. Missing-trampoline policy, static CRYPT32 import, hook engine,
rollback/drain and the three production exports are unchanged.

The old CryptQueryHost read_path/u16string output boundary is replaced by a
noexcept inspect_path returning a PathProbe. This is an internal/source C++ API
adaptation, not a new production export. The owner-selected numeric SDK identity,
listener range, transport budgets, atomic persistence and immutable vendors remain
separate policies.

Validation includes ring rotations, Unicode/surrogate cases, embedded NUL,
exact/over-limit termination, read failure, integer address overflow, mutation at
the terminating read, inaccessible native pages, and the longest valid protected
Windows read. A test-only global C++ allocation observer counts requests during
inspection; input strings and fixture storage are prepared outside that scope.
The native callback suite separately checks forwarding, output pointers, LastError
and active-call lifetime. Current evidence: metadata/evidence/HOOK_PATH_SCAN.json.

The `portable_crypt_path_probe` CMake check compiles the unchanged scanner and
the existing controlled-reader fixture without a Windows SDK. It covers the ring
buffer, UTF-16, scan/read bounds, allocation observer and integer address guards
under the same 30-second test budget. Its target-local definition excludes only
the native reader and reports that exclusion in its output. Default Windows
`crypt_path_probe_contracts` retains the full protected-memory reader, including
partial code units and read-only pages; portable success does not establish those
native results. The selector interface uses standard C++ types, while the Windows
host adapter and native fixture own their SDK includes.

This adoption resolves the bounded-scan advantage identified in the hook11 source
comparison. That comparison identified no further required hook11 transfer; it is
not an exhaustive assertion that the whole application has no remaining issue.
LLVM-MinGW/Wine validation is separate from native MSVC Golden and game acceptance.
