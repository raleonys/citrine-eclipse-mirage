# Write completion independent retained-evidence audit

Independent reviewer: `write_finish_audit`, 2026-10-23. Complete retained pseudocode and assembly for `0x180025a70`, `0x1800261d0`, and `0x18002a500` were read. Only `025a70` is a newly recovered normal body in this stage. This is static inspection: original DLL/game/network execution is none; real configuration, tokens, achievement files and other user data were not read. CRT production remains unchanged.

Target `anadius64.dll` SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

| Function | Complete pseudocode Evidence | Complete assembly Evidence | Counting |
|---|---|---|---|
| `0x180025a70` | `ev_f7448c3646f8693c49a161e2449f559db548111bdf4c3a9cacfa5295f5e963b8` | `ev_55d30b85e7b6ed50c63ac8a363324c343cf0045dfef90f8fc377ed5e9e01b4d7` | One new normal body |
| `0x1800261d0` | `ev_45b2f55e8f0ef15562ee5d5978c239a8bd7dbe150a53979f20633e3f28880e61` | `ev_5500451b02e60551ba95436e9c19673671b10b219573052f2e88f4d15d4a4e75` | Existing caller context |
| `0x18002a500` | `ev_74e713f6d63b5db2c8594c89965baf991c280377ff6f5607819938f99daa863f` | `ev_7b4a354660d536700feb53ad52ca0a2f961ca2042dbeb3a635143135bd1fe188` | Required provider context; not counted |

## Caller and tail view

`0261d4` writes the incoming flag before reading error code at `0261d8`. Its noninitial terminal branch copies the complete native error into a private local at `026210`–`02621c`, passes the native handler's tail address `handler+30` at `026213`, then calls `025a70` at `026221`. The existing logical `SendHandler.word_88` projects this tail's first pointer. This member is the completion owner, distinct from the composed-write stream member at native `handler+8`. Neither a native pointer overlay nor source class ABI is established by that projection.

## Success: deque selection, destruction and advancement

`025a94` caches signed32 error code before the branch. For zero, `025a9e` saves the owner from the incoming tail pointer. `025aa1`–`025ab9` select exactly `owner.map_d0[(owner.map_size_d8-1) & owner.offset_e0]`, where subtraction and masking are unsigned64; the map element is a pointer to a 32-byte native string. This is not `std::deque::pop_front` inferred from a host implementation.

`025abd`–`025ac5` read saved string capacity and skip deallocation for capacity at most15. Otherwise `025ac7` increments capacity to size argument modulo64 and `025aca` loads the saved data pointer. For size at least4096 (`025acd`–`025ad4`), `025ad6` adds39 modulo64 to the size argument, `025ada` reads the allocation header at `data-8`, and `025ade`–`025ae9` reject unsigned `(data-base-8)>31` via nonreturning `05c974` at `025c34`. The call at `025af2` is to actual `04b0f8`, with RCX selected data/base and RDX calculated size. The pseudocode suppresses the size argument, so assembly is authoritative for this call-site behavior. This report does not newly reconstruct the CRT thunk's implementation or claim its downstream size use.

Only after deallocation returns, `025af7`, `025aff`, `025b07` write the saved string's size0, capacity15, then first union byte0. Callback-driven changes to the owner map must not redirect these writes to another string. `025b0a` decrements the saved owner's count at+e8 modulo64. A resulting zero writes saved owner offset+e0=0 (`025b14`); otherwise its live offset increments modulo64 (`025b2c`). `025b1f` or `025b33` reloads the incoming tail's owner pointer after these actions before calling `02a500`. Therefore callbacks can make the next provider receive a different owner, while queue cleanup stays on the original saved owner/string.

## Error: category identity and log-call ordering

The nonzero branch calls miscellaneous-category provider `0285b0` at `025b40` before loading the incoming error's live category object pointer at `025b45`. The error code remains the previously cached value. `025b49`–`025b56` compare category **object fields+8** for equality, and suppress output only if equal and code is2. Existing logical `Category.name_identity` is used as that opaque comparison projection; neither category object pointer equality nor rendered category name equality is the native test. Error paths do not mutate/dequeue success state or start the next write.

For unsuppressed error, `025b66`–`025b74` call existing `006880` with global stream identity `0x1800a4130` and prefix identity `0x1800845d8`. The previously loaded category object remains saved across this call. Only after it returns, `025b7c`–`025b8e` reload that object's live vtable and message slot+10 and invoke it with cached signed32 error code, producing the local string. A replacement of `error.category` by the prefix callback must not replace this cached object, but mutation of its message callback must affect invocation.

`025b92`–`025bab` select inline buffer for capacity<=15 or heap data otherwise, then call `007990` with the local string's complete size. Its returned stream may differ from the stream produced by `006880`, and becomes the stream retained by `025bb0`. `025bb3`–`025bba` read a **signed int32** displacement from its vtable+4 and add the sign-extended value to the returned stream. `025bbd`/`025bbf` call `01e510(adjusted_stream,10)`; `025bc4` narrows its result to low8 for `00fe90(returned_stream,byte)` at `025bca`; then `00e1e0(returned_stream)` flushes at `025bd2`. A begin-stream pointer, unsigned displacement, full result value, or newline literal bypass would change observed native call arguments/order.

## Local message cleanup and normal-body boundary

After flush returns, `025bd8`–`025c0c` read the local message string's live capacity and data. Capacity<=15 skips cleanup. Otherwise the same unsigned64 cap+1, size>=4096 allocation-header selection, size+39 and unsigned data/base gap<=31 rule applies. Invalid metadata calls `05c974` at `025c2e`; valid cleanup calls actual `04b0f8` at `025c0c`. No writes resetting local string fields occur after this deallocation. There is no success-owner access on the error branch.

This stage reconstructs the normal instruction prefix and explicit cleanup order. It does not invent host RAII cleanup for exceptions thrown by a supplied effect, nor claim native unwind/SEH/security-cookie equivalence. Providers must maintain coherent live objects and memory at each callback boundary. Native string storage, header reads, category object/message dispatch, ostream helpers and `02a500` remain typed required effects, not native ABI overlays.

## Next-write provider context remains open

`02a513`–`02a51a` skip initiation if owner count+e8 iszero. Otherwise `02a52c`–`02a556` acquire the owner's strong reference using atomic compare-exchange; expired/null ownership reaches `026860`/INT3 at `02a67d`. `02a573`–`02a59f` inspect the same masked queue element and choose inline or heap data, with zero-size yielding null. `02a5a7`–`02a627` construct the composed-write handler and at most65536-byte first chunk; `02a62c` calls existing `021fd0`. `02a632`–`02a668` release the strong/weak pair after initiation. These existing-context instructions explain why the completion calls this provider, but none of this body is newly implemented or counted in the current stage.

## Evidence integrity and limits

The full retained listings were hashed during review and match their baselines: pseudocode SHA-256 `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; assembly SHA-256 `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`. The exact prefix string's text and any real user data were not read. Original DLL equivalence, native factory/tables/ABI, receive025c40, next-write02a500, concurrency and exception behavior remain unproven. No original file was loaded, executed or changed.
