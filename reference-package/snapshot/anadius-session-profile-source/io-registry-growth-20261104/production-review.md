# Independent static review — coherent IO registry growth

Result: STATIC PASS for the final four base/accept producer files and their integration with the ten consumer changes recorded in `consumer-independent-review.md`. That separately retained consumer report SHA-256 is `659b9758e7e70fb5e22767d0b19584535ff8535ab339d5d395c3218f3e9dce90`; it contains the exact ten consumer source fingerprints. This review changes no production/test source and performs no compilation, project tests, linking, DLL execution or packaging. Test additions are outside this production result.

Read root's eight-record native basis:021d10 request0xe0 at `0x180021d46`→0269b0 at `0x180021d4b`;021fd0 request0xd0 at `0x18002200e`→0269b0 at `0x180022013`;021820 cache miss allocation at `0x1800218cf`. The examined two-slot cache is reuse metadata, not an observed64-operation admission ceiling. Source changes remove that reconstruction ceiling, not native allocation/OS limits or original ABI/EH uncertainty.

## Base tables and stable identities

The four replacement vectors are prepared while old tables remain authoritative. Record ownership is held by unique_ptr, so moving the metadata owners does not move OperationStorage, variants, OVERLAPPEDs, kernel operands or pin/loan state. Existing slot generations, packet errors/transferred values, submission sequence and cancellation/completion flags are copied unchanged. New empty rows receive default generation0 and no operation identity.

After allocation, Parent/phase/shape checks and packet-alias validation run before copying current metadata. Commit moves unique_ptr owners into already empty prepared elements, swaps all four arrays, repairs every live receive/send packet pointer plus registered external fields, and updates stable live span fields before old table storage retires. The commit has no opaque callback or storage allocation. Completed-but-cached receive/send projections are included; repair is not limited to pending IO.

Existing recursive gate/Parent serialization is retained. Mutation covers the full allocation transaction and now makes rejected normal Access reentry sticky. A swallowed allocator callback rejection therefore prevents the outer postcheck from accepting the result. Raw span/snapshot getters can inspect complete old or complete new tables; they are not permission to mutate incomplete metadata. This does not claim allocation occurred outside all inherited recursive-lock levels or establish generic arbitrary concurrent allocator/new-handler safety.

The Record allocation is checked before installation. When the raw parent allocation returns, its known raw address, extent and descriptor are retained before the final admission check. A failure then marks that Record failed and the owner poisoned; it does not replay the effect, invent a free or release prior SDK loans. Allocation failure before commit leaves existing metadata intact. Metadata-only allocation/destruction preserves added LastError effects in the new growth helper; exact native allocator/CRT LastError equivalence remains unproved.

## External packet field ownership

The base records one exact external pointer-field address per live Record. Bind validates current row pointer, rejects a second field for that row, and rejects the same field address registered to another row. The registered field must have a stable containing object.

Alias-only unbind locates the field by address rather than dereferencing a potentially retired OperationStorage descriptor. A registered current pointer or registered null is cleared/forgotten; a nonnull foreign value rejects. An unregistered null is idempotent after base projection/free already cleared registration. This handles manual nulling without leaving a dangling registered field. clear_binding nulls and forgets the field before changing the projection or releasing its storage.

Accept now uses deque<Row>, preserving old AsyncAcceptOperation, Task and raw OVERLAPPED addresses, including the registered op.packet field through additional SDK activity. Its index helper lazily covers new base indices by checked doubling. A growth reservation is set before deque resize; normal Access and snapshot reentry reject/poison before inspecting partially appended state. Parent admission, adapter state and exact base slot/generation are rechecked afterward. Failure follows the retained poison policy without dropping an earlier pending pin.

Accept project installs packet metadata and op.packet before exact registration. Retire always calls alias-only unbind after pending/pin checks and before destroying the variant; it does not skip unbind merely because the public pointer is null. Base.project_posted can already have cleared that registration, for which null unbind is safe. Release then performs the existing base free. No stale storage descriptor is required merely to unregister a dead field. Initial accept row coverage no longer has to match the base's current allocation length because index grows to cover actual live slots.

## Consumer closure and bounded limitations

Runtime creates PostedEnvironment and whole-run DispatcherState with base-owned live span-field pointers. Existing by-value environments, const Ports and transient copies preserve those pointers; legacy aggregate callers with null optional fields retain fixed-span behavior. All selected lookup/matching consumers resolve current views. Dispatcher packet references are consumed before opaque completion, and Runtime Router reacquires packet identity after completion_owner. The retained consumer report details and fingerprints these ten changes.

The generic injectable `publish_send_status_028b50` category callback can retain a Packet& across an opaque provider. The actual selected Policy category getter inspected here is a nonallocating identity/enum return, so it cannot grow these tables at that site. This review does not extend the guarantee to arbitrary replacement providers; stale caller-owned unregistered pointers or expired borrowed snapshots remain outside the contract.

No unresolved blocking defect was found in the reviewed selected route. Pending IO, exact kernel completion, buffer/control pins and physical thread finality are still governed by their existing mechanisms. No new safe-unload or CRT framework was added, and no runtime PASS or whole-DLL completion percentage follows from this static result.

## Final producer source SHA-256

| Source under outputs | SHA-256 |
| --- | --- |
| anadius-io-memory-source/io_memory.cpp | 5dc50d2a627c80f04c9f02e67c28741b255c6a1348b11c4c2c8f0d3ca898d3f8 |
| anadius-io-memory-source/io_memory.hpp | 28ff3d3fd2503192a83f4a16cbf5f4216f89fbd4ed81a34f455e8e7e1df810c6 |
| anadius-accept-source/owned.cpp | ec4beab122e4402f8163ce2d8a95c9a4970bccac3d6a9a3575883a15bbed9edc |
| anadius-accept-source/owned.hpp | b77f2418372bab73dd313a62f4a92b0686e622cf575fbcd44a05c0e542686d13 |

Both source authors confirmed these files final; reviewer independently rehashed them. Full native replies and exact before copies remain in this stage.
