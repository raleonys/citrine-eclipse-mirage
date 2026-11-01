# VCRT API resolution and fallback source

`api.cpp` reconstructs resolver `0x1800509bc` and the five wrappers at
`0x180050b0c`, `0x180050b54`, `0x180050b9c`, `0x180050be4`, and `0x180050c38`.
This is independently compilable C++20 source, not original source or a complete DLL.
The input SHA-256 is `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

The resolver has three plain qword module cache cells at `0x1800a4ea8` and five
plain qword procedure cache cells at `0x1800a4ec0`. They are separate from the
21-module/31-encoded-procedure ACRT resolver in `anadius-crt-runtime-source`.
The three UTF-16 table entries at `0x180088f88` are
`api-ms-win-core-fibers-l1-1-1`, `api-ms-win-core-synch-l1-2-0`, and `kernel32`.
All four FLS wrappers use DWORD candidate indices `{0,2}`; critical-section Ex uses
`{1,2}`. `evidence.json` retains the bounded table/name region and digest after
verifying the complete input SHA-256. The assembly excerpt retains the original
REA evidence IDs and instruction addresses for all six functions.

Each load first uses `LoadLibraryExW(name, NULL, 0x800)`. A zero result retries with
flags zero only for error 87 and when the name does not begin with case-sensitive
`api-ms-`. This resolver has no `ext-ms-` exclusion. A loaded module lacking the
requested export stops the candidate scan. Missing modules and procedures cache
all-one qword sentinels. Successful competing loads use unconditional exchange;
if the old cell was nonzero, the code frees the newly loaded local handle and
then queries that same local handle. The reconstruction preserves this sequence,
including overwriting existing cache values, instead of substituting a corrected
compare/exchange algorithm.

The recovered wrapper return types follow SDK semantics and the observed untouched
return register: DWORD for allocation, signed 32-bit BOOL for free/set/critical
section, and pointer for get. Noncanonical BOOL return bits remain intact. The
assembly explicitly restores each original argument after calling the resolver;
Ghidra's void signatures and omitted arguments are not used. All external effects,
including fallback calls, remain mandatory abstract methods. Resolved addresses
are opaque rebuilt-provider tokens; this module never casts or invokes original
preferred addresses.

If FLS is unavailable, `FlsAlloc` calls `TlsAlloc` and discards its destructor
callback. Free/get/set use their TLS equivalents; critical-section initialization
falls back to `InitializeCriticalSectionAndSpinCount`, dropping only Ex flags.
There is no synthetic destructor emulation or claim that TLS provides FLS cleanup.

`State` uses new C++ atomic cells to avoid C++ data races while modeling the observed
MOV/XCHG accesses. Module names are immutable during resolution. Callers must
provide valid in-range indices, NUL-free names and external shutdown/unload
quiescence. This is not an ABI layout replacement. Native APIs, real loaders,
FLS/TLS lifetime, SEH/CFG and final DLL lifecycle remain unverified integration work.

Run `./build.ps1` to compile with the existing workspace LLVM toolchain and execute
the synthetic provider tests. The 185 checks cover positive/negative caches,
candidate ordering, exact prefix/error retry rules, loaded-but-missing exports,
deterministically interleaved exchanges, same-local free/query behavior, wrapper
argument widths, raw return bits and TLS callback loss. Neither target DLL,
native provider APIs nor the game are loaded or executed. `validation.json` records
the compiler, arguments and generated object/test executable hashes.
