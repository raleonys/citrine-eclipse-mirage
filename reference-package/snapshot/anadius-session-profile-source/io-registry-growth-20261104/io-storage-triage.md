# OwnedIoMemory storage triage — source only

Read-only audit of selected current source on 2026-11-04. No production/test edits, original binary access, REA queries, compiler, tests, linker, DLL execution or packaging. This establishes a source ceiling and implementation dependencies; it does **not** establish a discrepancy from native behavior. Fresh native comparison is pending with root.

## Current inventory

`outputs/anadius-io-memory-source/io_memory.cpp:18-26` defines a heap-owned Record containing the OperationStorage descriptor, raw allocation/extent, projection generation, variant of typed operations/tasks, submission sequence, pending/completed/failure state, and strong/buffer pins. Four equal-length vectors at lines38-41 are initialized once at line44:

| Table | Meaning | Address stability needed |
| --- | --- | --- |
| records: vector<unique_ptr<Record>> | slot ownership; null means available | Record pointees, their storage descriptors and variant-held live projections must stay put; unique_ptr metadata itself need not |
| generations: vector<uint64_t> | monotonically increasing slot generation across projection/allocation reuse | preserve values by index; no pointer to its entries found |
| storage: vector<OperationStorageBinding> | identity-to-descriptor routing | external borrowed spans; each pointee already belongs to a stable Record |
| packets: vector<DispatcherOperationBinding> | operation/OVERLAPPED identity plus packet error/transferred bytes | external spans **and** typed operation.packet pointers into individual vector entries |

The constructor validates a positive initial count <=65536 (`io_memory.cpp:119-121`). Allocation scans records and throws bad_alloc immediately when no null entry exists (`:124-129`); there is no growth. Selected OwnedServerBackend defaults to64 rows (`outputs/anadius-owned-server-source/server.hpp:112-113`), so the 65th simultaneous live allocation fails even if the parent allocator has room. Cached allocations still occupy rows: free releases the Record only after the explicit source free path (`io_memory.cpp:137-143`), whereas native cache release may retain its descriptor (`outputs/anadius-socket-source/io_completion.cpp`, release_io_storage_026a00).

Projection changes increment the existing generations[i], clear packet routing and rebuild the variant (`io_memory.cpp:67-75`). get/index validate slot, generation, live ownership and descriptor shape (`:53-65`). A growth operation must not change any current generation, submission sequence, raw address, strong/buffer pin, cancellation or completion flag. Live OVERLAPPED and CancelOperation pointers are embedded in stable variant projections; relocating Record objects would invalidate kernel operands. Keeping the existing heap-owned Records avoids that problem entirely: converting records to deque is unnecessary.

## Internal live-pointer dependencies

AsyncReceiveOperation.packet (`outputs/anadius-socket-source/async_receive.hpp:30-33`) and AsyncSendOperation.packet (`composed_write.hpp:27-31`) are initialized to &packets[i] in `io_memory.cpp:156,160`. Any packets-vector replacement must repair **every currently live Receive/Send variant**, including completed-but-cached projections, before the old vector is destroyed. Repairing only pending operations misses valid typed projections. No Record/variant reconstruction or generation increment belongs in this repair.

Source start paths reload operation.packet at use sites after environment construction and SDK calls: receive021d10 (`async_receive.cpp:22-70`) and send021fd0 (`composed_write.cpp:55-104`). Thus a fresh pointer after callback-driven growth can serve their later accesses. However, simple pointer repair is insufficient for an already borrowed packet reference:

`publish_send_status_028b50` (`composed_write.cpp:37-41`) takes DispatcherOperationBinding& metadata, calls errors.system_category_028620(), then writes through that reference. A generic category provider that grows IO storage could invalidate the captured reference even when operation.packet has been repaired. **The selected production provider does not do so:** `outputs/anadius-session-policy-source/policy.cpp:126` installs v.native as errors; NativeApi::system_category_028620 (`native.cpp:33`) delegates to Catalog::get_028620 (`policy.cpp:33`), which returns category(1). That noexcept category method (`policy.cpp:32`) only computes a stable identity and enum. It allocates nothing and invokes no callbacks. Consequently this generic injectable-provider hazard does not require a selected production helper change. Do not expand the growth guarantee to arbitrary replacement providers. Retaining retired metadata alone would not solve that generic hazard, because writes to a retired entry would not update current routing.

`completion_dispatcher.cpp:94-106` resolves a packet entry after guard callbacks, consumes/copies its metadata, then invokes effects.complete; it does not use that entry again after the callback. It therefore only needs its DispatcherState.operations view to have been republished before lookup. The separate agent is mapping those external context spans.

## Smallest coherent growth plan

1. Keep vector<unique_ptr<Record>> and the existing stable Record layout. Prepare four zero/default candidate vectors of a checked doubled size (minimum current+1; natural max_size across all four), plus any new Record payload, without retaining table iterators or references across allocator calls. Preserve LastError around added metadata allocation/destruction only.
2. Under the metadata gate reserve a bounded growth/allocation transaction before the first allocator callback, retaining the selected Parent serialization and recursive-lock model. Public mutation during incomplete tables must reject; if the desired failure policy requires swallowed allocator rejection to remain sticky, existing Access only throws when mutating (`io_memory.cpp:45-52`) and needs an explicit poison/attempt marker. Recheck parent Operation admission, phase and old table shape after all candidate allocations. Snapshot-style read access may observe the old coherent inventory, but cannot receive partially appended new rows. Existing Mutation already supplies the transaction reservation for allocate; no new global barrier framework is necessary.
3. Copy current generations/storage/packets only after the final callback and admission check. Move the current unique_ptr records into already allocated candidate rows only in a no-throw commit section; no existing Record moves or is destroyed. Swap all four tables coherently, repair every live typed packet pointer, and publish registered external span destinations before retiring old table storage. No opaque effect belongs between these steps. Candidate allocation failure leaves old rows/tables intact; existing live IO/pins are retained.
4. Select/reselect the current empty slot after allocations. Preserve the existing source-allocation failure prefix: the reserved Record/generation is installed before the parent raw allocation effect; that failure marks its Record failed and phase Poisoned rather than guessing a raw free or releasing preexisting loans. Do not hold an old vector iterator across a growth or raw allocator callback.
5. Preserve close/active_calls admission and existing observed-completion rules. New table length is metadata, not evidence of completed IO. Fresh slots start with generation0 and empty bindings, and only normal projection/publication gives them packet identities.

## Gate scope and bounded reentry

Access owns a recursive_mutex lock for its entire method (`io_memory.cpp:45-51`). Allocation currently invokes both make_unique and the parent allocator while holding it (`:125-135`). More subtly, complete_receive_with_memory, complete_send_with_memory, complete_posted_with_memory and invoke_task (`:220-249`) retain their Access locks across source callbacks that can legitimately allocate further operations. Releasing only the nested allocator's own lock therefore does **not** place metadata allocation outside the gate: an ancestor completion still owns a recursion level.

For this selected Parent-serialized source, inherited recursive locking does **not** by itself prevent coherent growth. The existing Mutation barrier can remain active throughout the candidate allocation, validation and commit. Allocator callbacks that use normal Access must reject before observing incomplete state; snapshot() and the raw span accessors may return only the intact old inventory while allocations occur. No allocation/callback occurs during the multi-table commit and packet-pointer repair, and old table destruction follows complete publication. close() already refuses mutating, and note_unknown_submission may poison during a transaction, which the post-allocation check must observe. Thus retaining the recursive gate is the smallest selected implementation; broad completion lock redesign is not required to remove this ceiling.

Allocation outside the gate remains a preference, not a proven native or selected-behavior requirement. If a later change explicitly chooses that stricter property, all ancestor locks must be addressed as above; unlocking only allocate would not establish it. Do not introduce that larger change solely for this growth correction. Existing completion code's no-old-record-after-callback rule (`:224`) remains necessary regardless. No new unload or concurrency policy is proposed.

## Native selectors for root's next evidence pass

Existing modeled function names identify bounded queries: allocate_cached021820 (cache miss requests raw allocation), allocate_io_storage0269b0, start_async_receive021d10 (requested0xe0), start_async_send021fd0 (requested0xd0), release_io_storage026a00, complete_receive027710, complete_send027a20, raw allocator04b0bc and free04b788. Posted/task/response requests also share this storage owner; their minimum projections in io_memory.cpp are0x49/0x79/0x71. These are selectors for comparison, not newly observed native evidence or proof of any native unbounded policy.

## Source fingerprints

| Current audited file | SHA-256 |
| --- | --- |
| outputs/anadius-io-memory-source/io_memory.cpp | 669a2b098f2496266ff330e1efccd4a6a29b8d55a10f01b752bec51b39a509fd |
| outputs/anadius-io-memory-source/io_memory.hpp | 6106cfff8df2cafac2f2570b54f78ef921204ffb9f45c6943bf1894c776d2847 |
