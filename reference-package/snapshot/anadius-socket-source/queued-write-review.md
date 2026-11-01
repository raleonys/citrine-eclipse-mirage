# Queued write independent source review

Reviewer: `queued_write_audit`, 2026-10-23. Result: **PASS; zero material findings** within the bounded typed normal-body model. Both complete production files were independently read against complete retained `02a500` pseudocode/assembly and complete `025a70`, `021fd0`, `0261d0`, `027a20` caller/callee context. Existing `WriteFinishOwner`, `SendHandler`, `IoReference`, `ComposedWriteContext` and actual `start_async_send_021fd0` were checked. The reviewer did not edit production.

| Frozen production file | SHA-256 |
|---|---|
| `outputs/anadius-socket-source/queued_write.hpp` | `7a205cbc34118c9181f829feeb474478dedd10eb21d37780af966018e3f6cc7a` |
| `outputs/anadius-socket-source/queued_write.cpp` | `8aec35f671804e86544701065a8c510b0b3c5632b0baf13cf31f6679fbd88211` |

## Normal body and bridge

Exactly one new normal original body, `0x18002a500`, is implemented. `RecoveredQueuedWriteApi.next_write_02a500` is final and calls that actual source with its default actual `021fd0` callee. This closes the prior completion `025a70` next-write provider at the source level. Existing sources and CRT production are unchanged. The bridge's other logger/free interfaces and `QueuedWriteTerminal.bad_weak_ptr_026860` remain mandatory, not invented implementations.

Owner lookup rejects absent/null/ambiguous projections without executing a native callback. The logical sidecar and queue pointer are host representations. Once the count is found to be zero, the code returns without reading control, string map or stream, and without any terminal, allocation or send effect. A missing owner/queue is an unsupported host projection, not a newly inferred native branch.

For nonempty queues the acquisition pointer is cached once. Null and observed strong0 invoke only the required nonreturning terminal. Actual atomic uint32 compare-exchange increments a nonzero expected value modulo32; a failed CAS reuses the standard updated expected value, checks zero and retries the same cached counter. `0xffffffff` wrapping to0 is preserved. Object and control are reloaded from owner after successful acquisition. No cached acquisition pointer is substituted into the handler's reloaded pair. Static inspection and synthetic tests do not establish native race/lifetime synchronization, and unsynchronized host non-atomic fields must not be raced by providers.

Map indexing is unsigned64 `(map_size-1)&offset` and selected map elements are bounded string pointers. Zero size returns data0 before reading capacity. Nonzero size selects inline first-byte host address for capacity<=15 or decodes the first8 bytes as a little-endian heap address token. The full64 size is preserved in the handler while the first buffer is limited to65536. The code creates stream identity `owner+10`, allocator member address `owner+78`, owner word88, progress0, flag1 and the reloaded owned pair. These literal identity calculations preserve native address-token wrap on the x64 host; they are distinct from any mutable binding metadata tokens.

Actual `021fd0` is called directly, flags0, with the selected service/implementation and initial buffer; there is no `0261d0` shortcut. Pure stream/context projection checks do not add external effects. The optional explicit function-pointer boundary is confined to direct caller testing (for residual cleanup and fault prefixes); the production recovered API bridge always chooses actual source. Tests using the injected boundary are not evidence that an alternative native callee exists.

## Lifetime and residual release

Existing actual `021fd0` copies the owned pair into the operation and clears the source pair before invoking the environment constructor. Thus normal initiation consumes the local pair. After this callee returns, the source reads only local `handler.control_98`; there are no owner, queue, selected string, stream, context or operation reads. Publication may end those lifetimes without a subsequent modeled access in this caller.

If the residual local control is nonnull, `release_residual` saves the pointer by value, decrements its uint32 strong counter atomically, and calls disposal only for old1. Only after disposal does it decrement the same saved control's uint32 weak counter and call the live delete slot only for old1. It does not reread the handler/owner or prematurely cache a delete callable. Counter wrap for old0 and separation of strong/weak effects match the normal instruction sequence. The control must remain alive until weak release under the existing `IoReference` contract; slot8 may delete it, after which the code performs no access.

Host `SendHandler` defaults initialize convenient logical placeholders, but native undefined bytes/padding and a handler prefix/vptr are not recovered or counted. Neither exception cleanup nor an automatic ownership guard was introduced. Unsupported projection exceptions and injected effect exceptions expose only the modeled completed prefix; no native unwind, SEH, security-cookie or full-DLL equivalence is asserted.

## Evidence and limits

The companion `queued-write-flow-audit.md/json` gives instruction addresses, all five context evidence pairs and freshly matched full retained listing hashes. Review JSON binds those audit hashes and both frozen production hashes. Source review is distinct from independent synthetic tests, compiler checks and the common build; this review does not assert unobserved test results.

No original DLL/game or real network was executed, no original file modified, no real user configuration/token/achievement contents read, and no CRT production modified. Remaining required native tables/factories/category/message/global stream/free effects, terminal026860, receive025c40, ABI, allocation storage, concurrency/lifetime and exception/unwind equivalence prevent any complete-DLL claim.
