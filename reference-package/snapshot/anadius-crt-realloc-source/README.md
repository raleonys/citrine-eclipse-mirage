# Recovered malloc/realloc base control flow

This independently written C++20 module recovers two additional local functions from the retained REA/Ghidra 12.1.4 static evidence for `anadius64.dll` SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. It reuses the existing `anadius-crt-heap-source` State, error slot, decoded new-handler, new-mode and free cores. Original preferred VAs are evidence labels, never dispatch destinations.

| Original VA | New function | Observed behavior |
|---|---|---|
| `0x18006a308` | `malloc_base_06a308` | Accept byte counts through `0xffffffffffffffe0` inclusive; normalize zero to one; allocation flags zero; retry under new-mode and a successful new-handler; otherwise write 12 through `05d724` and return null. |
| `0x18006e3a0` | `realloc_base_06e3a0` | Null input delegates to malloc even for zero bytes. Non-null input and zero bytes uses the existing free core and returns null. Positive accepted sizes use reallocation flags zero, retain the original input pointer across retries, and use the same failure policy. |

The unsigned maximum is `2^64 - 32` (`UINT64_MAX - 31`), not `UINT64_MAX`. `06a311..06a315` uses unsigned `JA`; `06e3cb..06e3cf` uses unsigned `JBE`. Both reload the heap global for each allocation attempt, allowing a handler to change it. No allocation failure calls GetLastError or the OS-error mapper; the zero-size free path may do so through `free_0668c0` if freeing fails.

Both callers test **EAX**, at `06a32a`, `06a336`, `06e3ee`, and `06e3fa`. The reused `0634b0` body tests the callback's full EAX at `0634dc`, uses `SETNZ BL` and `MOV EAX,EBX` to normalize its result, and returns zero when no decoded handler exists. Ghidra's `CONCAT71(extraout_var,bVar1)` is a decompiler typing artifact, not a recovered ABI requirement. Tests include callback values `0x100`, negative values, and `INT32_MIN`, so a low-byte-only interpretation would fail.

`Effects` extends the real existing heap Effects with a required `heap_realloc` provider. Providers own live storage, preserve the old allocation on failed reallocation, implement error/lock/callback behavior, and must bind rebuilt callbacks explicitly. This module neither installs a provider nor executes an original address. Successful malloc preserves whatever bytes the provider returns; unlike calloc, it requests no zero-memory flag.

Build and run the fixture-only tests:

```powershell
& .\outputs\anadius-crt-realloc-source\build.ps1
```

Validation: **800 checks**, using C++20 `-O2 -Wall -Wextra -Werror` and the existing LLVM-MinGW toolchain. Coverage includes maximum boundaries, all 31 rejected upper sizes, null/zero distinctions, nonzero high callback bits, heap/cache/mode mutations, lock ordering, error propagation, preserved fixture contents and provider exception propagation. Exceptions test the explicit source-provider boundary; no original exception-unwind equivalence is asserted. Only synthetic providers execute: no target DLL/game, HeapAlloc, HeapReAlloc, HeapFree or GetProcessHeap call. Fixtures do not request huge real allocations.

`evidence.json` and `selected-assembly.txt` bind observations to retained evidence. Runtime equivalence, binary ABI equivalence, original source/compiler identity and a complete independently rebuilt DLL remain unproven.
