# Stable IO owner bindings and concurrent transactions

2026-11-05. SOURCE ONLY. This author changed only `outputs/anadius-io-memory-source/io_memory.hpp` and `io_memory.cpp`; actual before copies were captured by root in this stage. Root/other agents own the socket interfaces, concrete consumers and test sources. No compiler, project tests, linker, DLL execution or packaging ran.

This is the next disabled prerequisite after `actor-catalog-foundation-20261105`. IO mode derives the immutable Parent `operation_policy()`. The actual production composition remains Serialized. These changes do not close or claim execution of either the distinct-session or same-session UnknownMessage/GetConfig progress trace.

## Public contract

- `storage_resolver()` and `dispatcher_resolver()` return passive, stable typed owner identities without admitting an actor. Actual exact identity/OVERLAPPED/CancelOperation lookup checks the explicitly admitted Parent Operation. Lookup returns the socket agent's Miss/Found/Ambiguous result and changes output only on Found.
- `registry_capacity()` reports the CURRENT index size under a brief metadata gate and preserves LastError. It is not the constructor's initial size and returns no borrowed catalog. This preserves Runtime/Stop's existing current-capacity relationship after Application.create has already grown IO storage.
- `packet_binding(OperationStorage&)` returns the exact mode-authoritative mutable packet. Serialized returns the existing public legacy vector row; Concurrent returns the stable embedded Record row. It supports OwnedAccept's deliberate public Accept packet overlay over an otherwise inert Posted reservation, not an assumption that every public packet is the base private Posted operation.
- Existing legacy `storage_bindings/dispatcher_bindings/storage_view/dispatcher_view` remain authoritative and meaningful in Serialized mode, including public packet tamper checks. Concurrent rejects those unguarded borrowed union interfaces and requires exact owner lookup. No concurrent vector growth rewrites packet addresses or registered external fields.

## Transaction contract

The shared IO mutex protects short metadata sections. Access first validates Parent outside the IO gate. A nonallocating stack Effect node reserves the actual thread/fiber and optional exact row; it prevents allocator/lifecycle reentry by that actor while permitting independent sibling row transactions. Per-row busy state prevents competing reuse. Allocating a Record/control block, preparing/growing index vectors, Parent byte allocation/free, range lease preparation/destruction, control-count release and native callbacks all occur after releasing the IO gate.

Each Record has stable shared host metadata containing its OperationStorage descriptor, storage binding and dispatcher binding. Index vectors move owner pointers, not Records. Concurrent bindings therefore keep their address across index growth. Shared ownership here retains the host sidecar across an entered source effect; it does **not** defer physical byte free, create a new native strong, or grant ownership to an arbitrary returned descriptor.

Record preparation reserves an index/generation before native allocation. A row is not live to resolver consumers until its private returned prefix is published Ready. Raw/descriptor fields of a Reserved row are not read by live traversal. On a clean no-return Parent allocation failure, only that attempted IO row can be removed; a healthy sibling's allocations are unrelated. Returned/unknown prefixes stay retained and failed/poisoned. No global allocation-count rollback is used.

Index growth allocates replacements outside the gate, rechecks current size/free slots, copies current generations and owner pointers during a callback-free commit, and releases replaced vector capacity outside the gate. If another actor grew/claimed the prepared size, preparation repeats before entering the source effect; it never holds a registry mutex while waiting for another body. Serialized legacy packet aliases are patched only within its existing serialized contract. Concurrent embedded packet addresses are never patched.

Projection transitions reserve their particular row, clear the prior routing metadata, construct/destroy the concrete host variant outside the gate, then publish the next generation/packet under the gate. Actual operation/cache owners still supply the normal source lifetime while initializing returned projections; the descriptor is not a generic concurrent-write permission.

Free reserves the exact row, removes its public routing, destroys its typed projection and immediately calls Parent sized free outside the gate. A completed physical free is not repeated after a failed postcheck. Failed prefixes remain retained/poisoned. Successful removal can free/reuse the index while an old callback's host sidecar is still privately retained.

## Loans and callback lifetime

Arming validates actual typed OVERLAPPED, kind, generation, handler control, socket and range. Parent range pin and extra strong are acquired outside IO gate; completed ownership is committed to that exact reserved row. Issuing/Kernel/Software/Unknown and submission counters retain their existing distinctions. Throwing SDK publication retains the pin/strong/row and records Unknown; cancellation does not consume a loan.

Terminal observation validates the exact row/handler under a short gate while leaving its pending owner/range pin retained. It then performs the non-last strong decrement outside the IO gate. A failed count>=2 guard poisons without dropping the original pending row, owner pointer or byte-range pin. Only after a successful decrement does a callback-free brief metadata commit detach the range pin, clear pending/routing and mark completion once. Pin destruction runs outside the gate **before** native completion. No Parent/admission callback is inserted between CAS and recording that completed prefix; unknown relock outcomes remain poisoned/retained, never replayed. Existing native handler/control ownership still survives to the callback.

This ordering corrects the first review candidate, which detached the pin before the strong-count guard and could unpin on that failure. The earlier implementation note is preserved as `io-implementation.round1.md`; it is historical, not a pass. The independent test author is adding a retained child case for that exact failed observation prefix, without executing it.

Native completion/task invocation owns a separate nonallocating Call token and a private shared host Record capture. Call ownership is keyed by **Record plus generation**, not by all storage reuse forever. The source callback can free/cache/reproject its own envelope; another row or a legitimately transferred new generation may progress while the old handler remains opaque. This is necessary because recovered completions release/cache the envelope before invoking the handler. A body-wide IO mutex or permanent per-Record body strand would reintroduce the confirmed progress gap.

After the native call there is no old projection/slot/generation access. Call teardown removes only its own token/global active count and releases its private host metadata. If the callback destroyed the operation and reused the index, teardown cannot overwrite the successor row. Metadata active counts include entered lifecycle Effects so drain cannot mistake an in-progress allocation/free for idle; these counts do not make B wait for A's body.

Actual source ownership, pending loans, cache ownership and generation reservations provide lifetime. Resolver pointers alone do not pin a row; no operation-wide Parent view pin is added to an IO envelope, which could incorrectly prevent legitimate cache transfer before an outer handler returns. Arbitrary same-object mutation, forged external pointer fields and foreign provider thread safety are not claimed safe.

## Compatibility and metadata details

Serialized mode continues to use public packet vectors for AsyncReceive/Send.packet and SDK arm checks. The existing negative mutation of `dispatcher_bindings()[slot].overlapped_identity` therefore still changes the authoritative binding and is not hidden by a detached Record copy. Exact owner lookup validates storage identity/data/extent against private raw ownership. Null/stale/missing rows do not fall back to an unrelated catalog; duplicate exact live matches are Ambiguous.

Snapshot output capacity is prepared outside the gate, then current rows are copied without allocation during a short section. Native callbacks, source effects and potentially allocating exception text do not execute while the nonrecursive IO gate is held; the private rejection helper unlocks first. Extra metadata/guard/deleter work locally preserves LastError. Allocation/free wrappers capture the actual Parent/Heap error after the source effect and restore it after their extra host sidecar destruction, rather than restoring the pre-call value across the effect.

Close still requires actual rows/loans/calls/effects drained and releases its Parent lease outside IO gate. This adds no worker-readiness, SDK serialization, unload barrier or physical-exit claim.

## Native provenance and remaining work

Existing fresh capture proof and retained source bodies are indexed in `anadius-session-memory-source/parallel-frame-admission-20261105` and `handler-admission-overlap-20261105`. Recovered027710/027a20/027120 and owned task bodies already explicitly free/cache their envelope before invoking the later handler, and must not be kept alive by an invented body lock. This stage changes logical host storage/owner projection, not the native ABI, atomic/EH implementation, CRT or target binary.

Per-message Profile/NativeProfile/Graph/provider contexts, authoritative Strings/Deque/registry consumers and removal of NativeJobs.source_gate remain separate required integration. The same Parent must ultimately support warmed same-session as well as distinct-session A/B progress, preserving actual shared business/Online/cipher identities. Production enabling remains withheld until that coupled route is coherent. Test-source cases authored by the independent agent have not run.

Frozen review candidate SHA256:

- `io_memory.hpp`: `ba766c7d3f2eb1c8d4e84e43ecadab41e1002e6650b55701d6c759efcdcd0dc2`
- `io_memory.cpp`: `9bdd429c21b8c00ea0f0e699313ac774554b02494b8907f971c4317f80c79050`

Independent reviews and any concrete successor correction are recorded separately; none of these source statements is a compiler or runtime pass.
