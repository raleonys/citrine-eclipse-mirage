# Independent counter audit — 2026-11-03

This read-only audit collected fresh REA/Ghidra observations in its own isolated session for `C:/Users/Skysect/Downloads/anadius64.dll`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`, using Ghidra 12.1.4. The isolated child initially had no open target; the three complete error replies are retained. After the parent authorized a separate immutable open, the target identity matched and all requested queries succeeded. The child session was closed successfully. No source edits by this auditor, build, test, link, original/derived DLL execution, binary mutation, or ZIP change occurred.

## Native observations

| Address | Observed instruction |
| --- | --- |
| 0x18000fdc0 | MOV EAX, dword ptr [0x1800a3888] |
| 0x18000fdc6 | INC EAX |
| 0x18000fdc8 | MOV dword ptr [0x1800a3888], EAX |
| 0x18000fdce | CMP EAX, 0xf |
| 0x18000fdd1 | JZ 0x18000fe09 |

The complete 19-byte read is `8b05c23a0900ffc08905ba3a090083f80f7436`; there is no LOCK prefix or indivisible read-modify-write instruction. The threshold compares the incremented register value, without rereading the shared cell. The initialized analysis-memory dword at 0x1800a3888 is `00000000`. These are static observations, not runtime measurements.

Complete structured Evidence is retained in the adjacent success JSON files:

- Instructions: `ev_839077f99c02612bc8d409358a72a00b4078cf9a201c931361fd8e1c4381c349`.
- Bytes: `ev_2c123225d73c1d6658e7511d29381e3e29841c9cbc05314f94b00a96a0b2d5f4`.
- Initial counter: `ev_bbcb055b706e65599e7e17c9d99baf4f7085db8bc0e0768db452874ab08a2d85`.
- Exact xrefs: `ev_49d332bd5c806367f1a57004291fc820b4ab5beb0a5e6832598e99841dccea68`.
- Typed incoming references: `ev_3507636da5f2b51a5c219e0f319b7c1218851bf24520f3391108fb0108255989`.

Ghidra reports exactly two references to the counter address: READ at 0x18000fdc0 and WRITE at 0x18000fdc8. The typed reference query corroborates both. This is the boundary of the negative search: unknown aliases, computed writes, overlapping wider stores, and external runtime writers have not been excluded. No other direct analyzer-visible counter writer was observed.

## Active source sites

The source search covered C++ .cpp/.hpp/.h files under outputs and work, excluding toolchain/node_modules content. The only active increment was `connect.cpp:27` before the parent's change, followed by a source reread at line28. Both `connect_00fd00` in that file and the early redirect path at `native_callbacks.cpp:107` call the same helper.

`native-context/context.hpp:19` owns the plain uint32 cell in CallbackSeed. `context.cpp:31` binds its reference into StateRefs. `initial-cells/defaults.cpp` constructs a zero seed before publication. Other matched accesses were test setup/assertions, not another active production writer. The existing ownership contract excludes retained external writers. The foreign callback Domain still rejects overlapping body admissions in `foreign.cpp:257`; a counter correction alone must not be described as granting concurrent full-body admission.

All six pre-edit source hashes are recorded in `audit-manifest-pre-edit.json`.

## Source-model finding and review

The minimal faithful defined C++ model is a lock-free atomic_ref<uint32_t> relaxed load, unsigned low32 increment into a cached uint32 value, relaxed store, and comparison of that cached value against15. A fetch_add, CAS loop, mutex, or locked increment would make the combined update indivisible and eliminate allowed lost-update behavior. For example, two threads may both observe14, both store15, and both take the terminal branch.

The parent's reviewed final source implements this split sequence and scopes atomic_ref to the load/store block; the cached uint32 value survives into the threshold test. It adds is_always_lock_free and required_alignment<=alignof(uint32_t) static assertions and documents that live plain external accesses require quiescence. No review defect remains in this counter change.

Reviewed snapshot SHA-256:

- connect.cpp: `73659fea38e311c87dd91e3311321f421483cf0f0ea9061d7e266f5469867f9d`.
- connect.hpp: `7e0bdcc5afc378c9ac2ec394092a794d16da3c8344ef82b4bf63dc228aefc890`.

The installed libc++ atomic_ref.h expresses required alignment using at least sizeof(T) for power-of-two sizes up to16 bytes (lines112–118), computes lock freedom using the compiler builtin (124–125), and uses distinct __atomic_store and __atomic_load builtins (133–160). Its SHA-256 is `56f0b4fc7cbe41552d83787122ade0c3352093ec8660d02ca5507ac94f9ca444`. This is static header evidence; the static assertions were not compiled.

Required limits: the uint32 object's lifetime and stable address must span every atomic_ref access; each live conflicting access must use compatible atomic operations or be excluded by quiescence. The assertion rejects implementations requiring stronger alignment than an ordinary valid uint32 object. The source model does not establish emitted machine instructions, all x86 interleavings, safety of live server-port/config/stream accesses, a completed concurrent callback ABI, or any runtime validation. Native INC flag differences are unobservable in this local sequence because CMP overwrites the branch-relevant flags.
