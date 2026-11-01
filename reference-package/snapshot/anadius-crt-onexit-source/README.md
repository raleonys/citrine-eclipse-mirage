# CRT onexit table normal flow

Static reconstruction for `anadius64.dll` SHA-256
`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
`onexit.cpp` is independently compilable C++20 reconstruction, not recovered original source.

The 24-byte `Table` has encoded begin, used-end and allocation-end at offsets
0, 8 and 16. Encoding is `ROL64(address, cookie & 63) XOR cookie`, decoded with
`ROR64`. A cleared entry is the cookie (encoded null). Initialization returns
-1 for a null table, resets all three fields only when encoded begin equals
encoded allocation-end, and otherwise leaves every field unchanged.

Registration and execution use CRT lock **2**, including null-table paths.
The lock must permit same-thread reentry from callbacks. Registration grows
an empty allocation to 32 eight-byte entries, doubles capacities through 512,
then adds 512 entries. Primary allocation failure or unsigned growth overflow
falls back to old capacity plus four. Each `_recalloc_base` call is followed
by `free(nullptr)`, even on failure. New slots are explicitly filled with the
cookie. Failed allocation leaves the table untouched. The provider owns size
overflow rejection, allocation lifetimes and bounds checks.

Execution visits entries in reverse order and clears each slot before invoking
its decoded callback. Encoded-null entries are skipped. After each callback,
begin and used-end are decoded again; changed begin or end restarts the reverse
scan from the new end. Thus registrations from callbacks, including reallocating
registrations, are observed. Final storage is freed and all fields become the
current cookie. Initially decoded begin equal to zero or UINT64_MAX returns
success without clearing or freeing the table. `_onexit` routes to the CRT table
when the application's **raw encoded begin field** equals UINT64_MAX; it returns
the supplied callback only on successful registration. Consequently `atexit(0)`
returns -1 even if registering that null slot succeeds.

`Effects` is mandatory for memory, allocation, lock and callback operations.
Storage and callback addresses are opaque unsigned integers referring only to
rebuilt data and explicitly resolved rebuilt functions. No original address is
called, and no native CRT is substituted with a success stub. `State` deliberately
does not guess global initialization order or a security-cookie value. Cookie
rotation counts 0 through 63 are tested with coherent tables; arbitrarily changing
the process cookie while retaining old encodings is not a valid supported state.

This scope recovers the **normal return flow** of register/execute guarded calls.
MSVC SEH funclets, CFG/XFG, unwind behavior when a provider or callback throws,
invalid memory, concurrent cookie/table mutation, malicious callback addresses,
and the actual native CRT lock/heap ABI remain unproven. The C++ implementation
does not install speculative exception cleanup; exceptions can leave the required
provider lock held and are outside its normal-flow contract. Callback return values
are unused, as in the observed indirect call. No CRT table initializer or real
module loader is invoked by this module.

Build: `powershell -File .\build.ps1` with the existing workspace toolchain.
The test executable uses only synthetic storage and callbacks. It checks null
and sentinel inputs, initialization preservation, null callback handling,
cleared-before-call ordering, LIFO, all 64 rotations, both allocation failures,
overflow fallback, 1,200 callbacks across seven growth sizes, same-thread
registration with/without moving allocation, and application/CRT routing.
`validation.json` records the compiler, hashes and exact check count.

Evidence is retained in `../anadius-all-functions.pseudo.c` and
`../anadius-all-functions.asm.txt`; `evidence.json` maps addresses to both IDs.

Sonraki heap aşamasında `../anadius-crt-recalloc-bridge-source` gerçek malloc/realloc/msize/recalloc gövdelerini wildcard ve onexit akışlarına aynı heap state üzerinden bağladı;5.483 ortak kontrol geçti. Bu klasörün eski bağımsız testi/validation kaydı kendi sağlayıcısını kullanmayı sürdürür. Yeni bileşimde recalloc artık kopyala/free compatibility fixture değildir; alt Heap API sağlayıcıları sentetiktir ve gerçek Windows/SEH/ABI eşdeğerliği açık kalır.