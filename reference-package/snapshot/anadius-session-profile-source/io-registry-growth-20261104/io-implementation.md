# Base IO registry growth — source implementation

Scope: `outputs/anadius-io-memory-source/io_memory.cpp` and `io_memory.hpp` only, with before copies retained beside this note. Source edits only; no compilation, tests, linking, target/reconstructed-DLL execution, REA query or packaging was performed by this task. Root separately owns persistent external view routing and SDK Accept storage/pointer integration.

## Native basis

Root's eight fresh records and `native-findings.md` are retained in this directory. The observed receive path requests0xe0 at `0x180021d46` and calls0269b0 at `0x180021d4b`; send requests0xd0 at `0x18002200e` and calls0269b0 at `0x180022013`. Cache allocator021820 inspects two reuse slots, then allocates the rounded requested extent plus marker through04b0bc at `0x1800218cf`. These examined paths have no64-record admission table. Native allocation/OS failure remains possible; no unlimited-resource or exact allocator/EH ABI equivalence is claimed.

## Storage and publication

Exhaustion now doubles all four metadata tables, bounded by the minimum of their actual max_size values. Fresh candidate vectors are allocated while the old records/generations/storage/packets remain complete and published. Parent admission, phase, shape and all live packet aliases are validated after the final allocator callback. Current metadata is copied only then. The commit moves existing unique_ptr owners into preallocated slots and swaps the four tables without allocating or invoking opaque callbacks. Record pointees, OperationStorage descriptors, variant projections, kernel OVERLAPPED addresses, handle indices/generations, submission sequence, control/buffer pins and loan state remain unchanged.

Every surviving Receive/Send projection's packet pointer is repaired, including completed-but-cached projections. Registered external packet fields are repaired at the same time. The two stable owner-held span fields are updated before old vector storage is destroyed. `storage_bindings()` and `dispatcher_bindings()` remain snapshots; the new `storage_view()` and `dispatcher_view()` return addresses of those live span fields for persistent by-value contexts. Snapshots expire on allocation growth. The view-field addresses survive for the OwnedIoMemory lifetime; consumers must not outlive that owner.

The selected Parent serialization and recursive gate remain. Existing Mutation spans the allocation transaction, so normal API reentry cannot access partial tables. A rejected Access during mutation now marks phase Poisoned, making a swallowed allocator callback rejection visible to the outer admission check. Snapshot and span-only queries may see the coherent old tables during candidate allocation; all-table commit itself has no callbacks. Added growth allocations and retired metadata destruction preserve Win32 LastError. No broad lock, concurrency or unload redesign was added.

## External packet alias contract

`bind_external_packet(OperationStorage&, DispatcherOperationBinding*&)` registers one stable external pointer field for an exact live storage row. The supplied field must already name the current packet row; registering a different field when one is already registered, a field address already registered to another row, a foreign pointer or altered field is rejected. A repeated bind of the same exact field is idempotent. The field's containing object must remain at the same address until unregistered or cleared by the base.

`unbind_external_packet(DispatcherOperationBinding*&)` looks up the exact registered field address without requiring an OperationStorage descriptor, which may already have been freed. A registered field that still names its current packet row is cleared and forgotten; a registered null field is also safely forgotten, so manual nulling cannot leave its address dangling after owner destruction. A registered nonnull foreign value is rejected. An unregistered null field is an idempotent no-op after base free; an unregistered nonnull field is rejected. `clear_binding` nulls and forgets the registered external field before projection/free/observed-completion cleanup; no callback is involved. SDK integration always calls alias-only unbind before destroying its stable row's variant/pointer field. The base stores no observer function or unrelated lifetime framework.

## Allocation failure prefix

No table iterator is held across an allocator callback. The current empty index is selected after any table growth; the new Record allocation is followed by Parent/phase admission before row installation. The source raw allocation retains its existing reserved Record/generation policy. On return, the known raw address/extent/descriptor are recorded before the new post-allocation admission check. Failure marks that Record failed and phase Poisoned; it does not replay the call or guess a free of the returned or preexisting raw objects. Existing loans and strong pins are untouched by metadata failure.

The generic injectable category-provider Packet& hazard described in `io-storage-triage.md` is outside the selected guarantee: selected Policy's category getter is a nonallocating identity/enum return. This patch does not claim arbitrary replacement providers or dangling/unregistered caller-owned pointer fields remain valid.

## Source fingerprints

| File | Before SHA-256 | After SHA-256 |
| --- | --- | --- |
| io_memory.cpp | 669a2b098f2496266ff330e1efccd4a6a29b8d55a10f01b752bec51b39a509fd | 5dc50d2a627c80f04c9f02e67c28741b255c6a1348b11c4c2c8f0d3ca898d3f8 |
| io_memory.hpp | 6106cfff8df2cafac2f2570b54f78ef921204ffb9f45c6943bf1894c776d2847 | 28ff3d3fd2503192a83f4a16cbf5f4216f89fbd4ed81a34f455e8e7e1df810c6 |

Independent static review and the other coordinated source layers remain separate; this note records implementation and evidence limits, not runtime PASS.
