# Recovered CRT lock banks

Seven original bodies are expressed in one C++ production unit: ACRT lock
initialization `1800625b0`, lock `1800625f8`, cleanup `180062614`, unlock
`18006264c`; VCRT initialization `18005093c`, cleanup `180050984`; and the
small ACRT API resolver wrapper `18006707c`.

The ACRT bank has 14 slots at target VA `1800a4f40`, with the counter at
`1800a5170`; VCRT has one at `1800a4e78`, counter `1800a4ea0`. All slots have
40-byte stride. These are source evidence addresses, never runnable pointers.
`LockRef` carries a bank and signed index. It does not manufacture a native
`CRITICAL_SECTION` from byte storage. An eventual native provider must map it
to a real, live, correctly typed, caller-owned object with suitable alignment
and bounds. This module invokes no native critical-section operation.

Both initializers use spin 4000 and flags 0, check the low 32 bits of the
provider result, increment the current global counter after each successful
call, and roll back through the recovered cleanup body on a zero result.
The AL result is represented as `bool`; incidental upper RAX bits are outside
the source interface. A value `0x100` succeeds despite AL zero, while
`0x100000000` fails despite a nonzero 64-bit value.

Cleanup snapshots the initial count to a local index, calls Delete in reverse
order, and decrements the current global counter after each callback. Callback
mutations cannot change the traversal. Counter arithmetic wraps at 32 bits.
Neither initialization nor cleanup invents SEH or C++ exception rollback.

The source's supported initialization domain requires an initially empty bank.
Cleanup supports initial counts 0..14 for ACRT and 0..1 for VCRT. Unsupported
initial counts throw `std::domain_error`; this is an explicit reconstruction
boundary, not a claim that the original checks counts. Callback mutations that
make a later cleanup exceed the physical bank are likewise outside that domain.
Lock/unlock reproduce MOVSXD ECX and signed index-times-40 even for negative
and out-of-bank indexes, without doing pointer arithmetic. Passing such a
reference to a native bank requires the caller to establish a valid view;
the original's invalid-memory behavior is not simulated as success.
State access requires external serialization, matching the startup contract.

ACRT `06707c` uses the previously recovered encoded procedure cache and loader
resolver at `0669b0`: procedure slot 15, name `InitializeCriticalSectionEx`,
module candidates `[7,16]` (API set synchronization then `kernel32`). A resolved
procedure receives lock, spin, flags; the absent procedure path calls the
required `InitializeCriticalSectionAndSpinCount` provider with lock and spin.
The wrapper forwards the provider result. XFG/CFG indirect-call metadata and
SEH behavior are not reconstructed. VCRT uses the distinct `050c38` wrapper,
recovered in `anadius-vcrt-api-source`; this module requires its provider and
does not substitute ACRT's cache for VCRT's.

Run `build.ps1` with the workspace LLVM-MinGW toolchain. The synthetic tests
cover all 14 failure positions, partial cleanup, both bank counters, low EAX
versus AL, reverse traversal under callback mutation, wraparound, signed
index limits, throwing providers, resolver cache/fallback paths, and physical
bank domain boundaries. Only our generated test executable runs. No original
or rebuilt DLL, game, native CS, actual module loader, or hook runs.

`evidence.json` binds the seven retained REA/Ghidra pseudocode and assembly
records and the bounded 8-byte candidate-table read to the original SHA-256.
Retained pseudocode is supporting evidence, not original source code.
