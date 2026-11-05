# Independent source review — socket ownership boundary and authored IO cases

2026-11-05. Root independently reviewed these sources authored by the socket and SDK subagents. Inspection only; no compiler, project test, native/reconstructed target execution, linker or ZIP operation. The IO producer and root consumer/code changes have separate independent reports; these nine hashes are not self-review of root's own tests or production changes.

## Socket boundary

The neutral memory_lookup header moves the existing enum to a common dependency rather than including write_finish into completion_dispatcher and creating a cycle. Namespace/type identity stays the same. The new trailing optional fields defaultnull, leaving current_storage/current_operations and existing aggregate prefixes valid. Posted/handler cache loops and the dispatcher now resolve through typed wrappers. A nonnull owner is authoritative on Found/Miss/Ambiguous; output is committed only on Found and no oldspan fallback occurs. Null owner retains the old FIRST matching legacy row, including its historical duplicate acceptance. No payload permission/lifetime is minted by returning a descriptor.

The actual cache source still reads capacity/alignment, unlinks its selected slot before use/free and writes the requested marker in the same order. Dispatcher still selects packetmetadata by key, performs its queued-CAS and calls the cached operation. The wrapper addition does not introduce a later access to an operation/binding that the callback may destroy. New owner provenance must be validated by the integrating actual IO owner; root's consumer report covers that wiring.

## Real lower-actor source cases (NOT RUN)

Two fresh handoff fixtures use the existing real same-Domain NativeThreads/PreparedMemory/JobScope actors and a Parent explicitly selecting Concurrent mode. No manual PTD/work/readiness synthesis, alternate Parent or provider cloning is used. One case holds A inside the actual complete_posted moved functor only AFTER its envelope is physically freed on the selected no-cache lower; its old slot/generation can be reused by B. A separate live guard row supplies the retained storage/packet pointers. B performs70disjoint projections, actual own completion, physical frees, stale-generation checks and real wrapper/FLS join/reap before original releases A. Failure terminates after finite waits without releasing A and pretending delayed B was concurrent progress.

The second case holds A's exact113-byte physical HeapAlloc after owned heap publication outside the test ledger mutex. B's same disjoint growth/lookup/complete/free/native-exit/reap must finish before A is released. This catches IO metadata gate ownership across Parent/native allocation. Actual guard resolver identity, original descriptor/packet contents, external alias and generation survive growth. Source asserts all rows/providers/Prepared ownership drain normally afterward. These are direct Posted/lower-source cases, not actual Runtime/Graph/GetConfig or concurrent kernel-receive coverage.

The IO unit case verifies a stable owner resolver's current capacity1→2, copied environment exact descriptor lookup, actual Serialized mutablepacket identity, growth relookup, typed CancelOperation match and Miss output preservation. Existing HOST pending kernel strong/range loans, forged packet identity and duplicate cached send rejection remain meaningful and registered.

The additional isolated consumed-strong case arms an actual PinnedReceive loan (strong3/rangepin1), then deliberately supplies invalid count1. Terminal observation must throw before callback/environment transfer/decrement and retain Poisoned pendingrecord/allocation/rangepin/exactpacket. It exits through the existing self-child mechanism after assertions, without fake repair/destruction of an invalid prefix. Default source main registers both original unknown-SDK and new consumed-strong children; no build-script change or execution was needed to author them. This case exposed the producer's observation ordering regression; existing cached-send duplicate case exposed the later known-preflight catch regression. Neither expected result was weakened.

No blocker found in the inspected nine final source bytes. Actual production remains Serialized until stable Strings/Deque/header consumers and per-message upper ownership/NativeJobs integration are complete. Native instruction/callback/cache-free qualifications are documented separately. Assertions are unexecuted, ABI/fault behavior and arbitrary same-operation concurrent writes/foreign SDK behavior remain unproved.

## Exact reviewed source bytes

- `anadius-socket-source/memory_lookup.hpp` SHA256 `9daeaaf4e5431039a178288a52ee6ee6dd19775fef12e288bb6618d8f8c0c851`
- `anadius-socket-source/write_finish.hpp` SHA256 `fae0e0f9d5bb38c782f3665eafe92c337c9edb5854981fb30520ab9696832a5d`
- `anadius-socket-source/posted_operation.hpp` SHA256 `dcbdadf100a1f89a421d7787f03862edf6ca2d7e4bda991506f79d34f832c56d`
- `anadius-socket-source/posted_operation.cpp` SHA256 `e73a1612c4c1ac9aa2ba6da1212ffaad3385c1fb0a70f978fe450dc29cf3775d`
- `anadius-socket-source/completion_dispatcher.hpp` SHA256 `bf93f1e45b8850e7f76e8589283de076dc42ae79621bd43eb1b136aa9185b835`
- `anadius-socket-source/completion_dispatcher.cpp` SHA256 `d31a11ea940bd77a442feae3dda84a4dbc87e5128ad3161a637195aae849108f`
- `anadius-socket-source/handler_dispatch.cpp` SHA256 `42e33a86b4f8bcc6bf0df085045857e5fd9b287bf84a69b4d34335e183bebd9d`
- `anadius-worker-handoff-source/handoff_tests.cpp` SHA256 `b676e53f01d240846fd0e0a3d268e38b8381110b2354d81525162b1a4fea1604`
- `anadius-io-memory-source/io_memory_tests.cpp` SHA256 `4bfd141e524e1f140ce03855762d63bd9e16e12a542795b995a41e80d3e062b1`
