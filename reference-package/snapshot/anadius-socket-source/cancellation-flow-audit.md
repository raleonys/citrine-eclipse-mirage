# Independent cancellation queue and wake flow audit

Scope: retained static analysis of `anadius64.dll`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. No original code was loaded or executed. No network calls or private user configuration/achievement files were read. This report establishes normal control flow, not native layout or runtime equivalence.

The full retained pseudocode and assembly listings were rehashed. Their SHA-256 values are respectively `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb` and `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

## `0x180026aa0`: cancellation extraction and delivery

Evidence: pseudocode `ev_d4f71cf8b783578dcbaae1a45c68bba3e631d5381f8102b4c3d6ec82db60b419`; assembly `ev_5263c2b7208ef28e3ec671a729ee180d773d5dcd6ba45d16972e7ebef04cba21`.

- `026ad1..026bb7`: visit exactly four consecutive maps at engine+`0x68`, stride `0x38`, in increasing order. Skip zero bucket count. Hash is unsigned socket key modulo live bucket count (`026af0..026b01`). The bucket contains first and last node pointers. The end is captured as `last->next`, not last itself (`026b08..026b1d`). Scan forward until this exclusive end, accepting only the first equal 64-bit key. Sentinel is separately excluded both before scanning and on match (`026b0c`, `026b38`).
- `026b50..026b9c`: copy all sixteen bytes of the supplied error representation to each pending operation at native+`0x38`; pop the node FIFO at native node+`0x18/0x20`, clearing the removed operation's next at native+`0x20`. Clear node tail only when the popped FIFO becomes empty. Append removed operations to a single aggregate FIFO, preserving per-map and per-node FIFO order. These native offsets describe evidence only; typed reconstruction must not overlay SDK OVERLAPPED with the original ASIO object.
- `026b9e..026bac`: call required recycler `0283c0(map,node)` even if its operation FIFO was already empty. Set matched=true in either case. Thus a matched empty node still causes the final wake. A mismatch does not recycle or set matched. Subsequent misses preserve earlier matched=true.
- `026bbd`: scheduler identity is read from engine+`0x28` **after all four maps and recycler calls**. Cache that scheduler for the delivery phase. Its port handle is read separately for every attempted post (`026c08`), allowing a provider effect to change a later post's handle.
- `026be0..026c0c`: detach aggregate head and clear its next; set its queued dword at native+`0x30` to **1 only immediately before its attempted post**. Call `PostQueuedCompletionStatus(port,0,0,operation)`. Any nonzero BOOL means success. Do not sample last error or retry a failed post.
- `026c23..026c76`: first zero BOOL enters the cached scheduler's live gate. Append the failed operation to the scheduler FIFO, using the current scheduler head/tail; append all remaining aggregate operations after it and set final tail accordingly. Those unattempted operations retain their previous queued values. Publish actual atomic exchange of dispatch flag with 1 (`XCHG` at `026c6e`), then leave the gate. No further posts occur. Existing scheduler FIFO order is preserved.
- `026c87..026c90`: when any matching map node existed, call `028850(engine+0x58)` once, after posting or fallback enqueueing and gate leave. A complete miss does not wake.

The apparent callback cleanup at `026c9d..026ce7` is **unreachable on all normal paths**: no work starts with RDI=0; successful exhaustion sets RDI=next=0; failed-post fallback reaches `026c84` which zeroes EDI. The test at `026c98` therefore always skips the block. Ghidra also warns that it removed this block. Reconstructing this as an ordinary callback would invent behavior and double-deliver completions.

## `0x180028850`: one-byte wake

Evidence: pseudocode `ev_9a1fe4c4ceb75c56791c10e35bd439a1423da6615f0d3dc8d9e7476e88ec498b`; assembly `ev_095d9853e46b5d31f95097518a09129ccf9b9050adc51fa79c2110be97dd4856`.

`028854..028891` reads the wake socket at wake object+8; creates one WSABUF with len=1 pointing to a local NUL byte; initializes transferred DWORD=0; calls WSASend with count=1, flags=0, overlapped=NULL and completion=NULL. The result and transferred count are ignored. `028897` calls category getter `028620`, then `02889c` samples WSAGetLastError **unconditionally, including send success**. Error 64 or 1234 calls category getter once more (`0288a2..0288ae`); all other values do not. No remapped error escapes and no retry or socket-state test occurs. The native adapter must preserve this order and avoid hidden API calls or temporary cleanup between send and error sampling.

## Required boundaries and context

`0283c0` is a mandatory recycler, not a pure erase: it updates bucket first/last pointers, clears key, adjusts active/free counts, splices a node into a recycle list and decrements map size. Evidence: pseudocode `ev_9f53a74fdc3280f360d1ce5770b00ecf0d27721d1f88cc316eeb303a6ea950b8`, assembly `ev_1c51eada2fac2491b445eaad1ca3641ce117107dbab6a3a71dd6a7392dc453bc`. A required injected recycler may retain this boundary; its full native body must not be counted as newly recovered.

Caller `026fb0` caches socket before entering engine gate, constructs cancellation error 995 through category getter, calls `026aa0` under that gate, then leaves it before close. Evidence: pseudocode `ev_4ffe8b68e27a849ee81d94fa3520491aa59f139532cc5e31294d1bee89c8c4b4`, assembly `ev_949e26c04afda34fe45f482346b1b127456e1debcf6ba15190922608977a89d9`. The new bridge should respect that borrowed engine/gate boundary rather than add a second engine lock.

Category getter `028620` has lazy-init side effects and remains a required provider. Evidence: pseudocode `ev_fd2098bcb37a61e83ff7f76f095ba6d0fe598933164420bda64f6c6ef3920095`, assembly `ev_73535f6dec0ad639cf35346f8021a5b39597a421eb724714a6aec9986996b9f6`.

Unresolved: original ASIO layouts/allocators, recycler body, callback execution by a real scheduler, global state construction, general concurrent lifetime correctness, native exception/unwind ABI, DLL entry/export wiring and actual IOCP/socket behavior. Typed SDK adapters and injected fixtures do not establish those properties. Two normal-flow original procedures are in scope; provider bodies/callers add zero recovered procedures.
