# CRT heap core: bounded C++ reconstruction

Six static functions from anadius64.dll are reconstructed in one production
translation unit (`heap.cpp`). This is a heap control-flow layer with mandatory
providers, not a complete CRT allocator or an independently loadable DLL.

| Original VA | Recovered function | Observed behavior |
|---|---|---|
| `0x180066848` | `calloc_base_066848` | Maximum product `0xffffffffffffffe0`; zero product becomes one byte; `HeapAlloc(heap,8,bytes)`; optional new-handler retry; final error slot value 12. |
| `0x1800668c0` | `free_0668c0` | Null is a no-op. `HeapFree(heap,0,p)` failure reads LastError, maps through `05d650`, then writes `05d724`. |
| `0x1800634b0` | `call_new_handler_0634b0` | Query handler, invoke outside its lock only if nonzero, normalize any nonzero signed result to true. |
| `0x1800634f0` | `get_new_handler_0634f0` | Lock index zero, XOR encoded handler with security cookie, rotate right by cookie low six bits, unlock, return snapshot. |
| `0x18007301c` | `query_new_mode_07301c` | Return the module's 32-bit new-mode global. |
| `0x1800704cc` | `initialize_heap_0704cc` | Store GetProcessHeap result, including null; return whether nonzero. |

`State` contains the four corresponding globals. `Effects` requires heap APIs,
error-slot and error-mapping access, CRT lock operations, and invocation of an
explicitly bound rebuilt handler. There are no production success stubs and no
calls through original binary addresses. `calloc` rereads the current heap on
each retry, and successful allocation leaves the error slot unchanged. Handler
exceptions are not translated into allocation failure: no catch was observed in
these function bodies. The Ghidra pseudocode labels the `05d724` accessor
`__doserrno`; the assembly address is the contract here, without relying on that
library-name inference.

The provider must honor HEAP_ZERO_MEMORY and return suitable allocation storage.
Thread `Context` and PTD C++ object lifetime establishment, alignment, destruction,
and storage ownership are still explicit integration responsibilities. This
module does not cast raw bytes into live Context/PTD objects, initialize the
security cookie or encoded-handler global, rebuild heap locking, or establish
DLL CRT lifecycle/SEH/XFG equivalence. Callers must provide the original-equivalent
state synchronization and initialization order. No reallocation/malloc/operator
new bodies are included in this bounded module.

`build.ps1` compiles with C++20, optimization and warnings as errors using the
workspace LLVM-MinGW toolchain. Tests use only owned byte arrays and synthetic
providers: division/overflow boundaries, zero-byte normalization, 40/968-byte
allocation domains, provider zeroing, mode and callback retry/failure paths,
all 64 pointer-rotation counts, snapshot-before-unlock behavior, error order,
heap replacement on retry, and escaping provider exceptions. No original DLL,
rebuilt DLL, HeapAlloc/HeapFree/GetProcessHeap operation or game is executed.
`validation.json` records the actual test result and tool/object/executable hashes.

Evidence is retained REA/Ghidra static output bound to the original DLL hash;
`evidence.json` lists exact parent Evidence IDs and the reused file hashes.
Static correspondence and synthetic checks do not establish runtime equivalence.
