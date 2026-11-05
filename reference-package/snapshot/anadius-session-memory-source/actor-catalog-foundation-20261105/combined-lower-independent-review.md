# Combined lower implementation review

2026-11-05. Read-only independent static review. No blocking finding remains in the final lower implementation below. No compilation, test/process execution, linking, reconstructed/original DLL execution, REA query, network or ZIP update occurred. The separate lower test source was authored by this reviewer and cross-reviewed by root; this report does not present that authorship as an independent test review or a runtime result.

| Production file | Final SHA256 |
| --- | --- |
| anadius-session-memory-source/memory.hpp | `9dabf8e664583ee13973522382d8023a8dd31b57b85c7274e66e2e9f8f523c71` |
| anadius-session-memory-source/memory.cpp | `ad221cc300d5bfdf1c2b17391fc7b999e86513b3c818b05fdd6cca846f91abb0` |
| anadius-worker-handoff-source/handoff.cpp | `eab095d8eb74f251b6bbf4fc33af255c87d069c038fcc5ad4772f117e8cc9931` |

`independent-implementation-source-hashes.json` records exact before/current hashes for these three, the separately reviewed Domain allocator change, eight socket resolver files and the authored handoff fixture. The final socket-only review uses manifest `a8179a98807f19b72673e29400d2f7a826a6c3fedd277a09b5fee6c683682b7c`.

## Actor and storage conclusions

Serialized remains the default and retains its separate recursive body mutex. Explicit Concurrent mode uses stable per-Operation actor records with actual thread/fiber, captured Domain FLS slot/PTD and, on the prepared route, the exact still-pinned JobScope. Operation admission reserves its actual thread using a nonallocating stack-linked pending record before metadata allocation. Reentrant admission cannot publish a second operation during allocation-handler preparation. Wrong-thread/fiber `Operation::job_permit()` and copied permit checks reject before attempting another actor's PTD validation. A foreign actor probe therefore cannot poison a healthy A merely by validating A's PTD on B.

Mutation, construction and temporary view preparation belong to the exact actor. Buffer fill/copy can perform its own internal pure exact lookup while its mutation reservation remains active. That does not grant an unadmitted caller access: resolver use first locates a live explicitly entered thread/fiber record. Concurrent mode does not expose the replaceable union byte/map catalogs.

Each exact lookup selects one live stable ticket under the metadata mutex and temporarily pins it before preparing a borrow node outside the mutex. Another actor cannot free that ticket during preparation. Commit adds one borrow for the requesting actor, without pinning unrelated allocations. Explicit free rejects sibling/SDK borrows before typed destruction, consumes only the freeing actor's own old view, reserves the ticket and immediately performs the existing destroy/free sequence outside the metadata gate. Operation exit drops only its own views, inactivates only its own seal and releases its moved-out JobScope after unlocking. Copied permits keep inactive metadata/root ownership, not an active scope pin; B exit/reap cannot invalidate A.

Reserved allocation fields are private until the locked Live publication; concurrent snapshots/resolvers do not inspect mutable Reserved payload fields. Known allocation failure uses that attempt's returned prefix, PTD and backing tracking state, without comparing a global allocation count that a healthy sibling may change. Unknown effect/PTD/tracking prefixes remain retained and poisoned, with no replay or guessed native EH rollback.

## Reaping correction and error boundary

Review identified that Prepared reaping allocated its vector snapshot while holding its nonrecursive State mutex. The final implementation reserves lifecycle membership/count under that mutex, allocates capacity outside it, copies at most that capacity under lock using nonallocating shared-pointer copies, polls/closes actual handles outside the mutex and splices retired list nodes for destruction outside it. Returned/Canceled rows are the only candidates; active sibling bodies are neither waited on nor treated as exited.

A known snapshot metadata allocation failure happens before the native-outcome try region and leaves Prepared Borrowed with its reservation cleared. Parent propagation now validates the exact original-worker Borrow identity/current PTD and healthy backing tracking outside its own mutex. It preserves an already healthy Parent only when that lower capability remains healthy; it never clears existing poison. Actual wait/close unknowns poison Prepared and consequently fail this validation, retaining Parent poison too. Exception class and global counts are not used to infer the outcome. Historical invalid-argument exception construction elsewhere under Prepared State lock was not broadly rewritten; this is the healthy snapshot/effect-path correction.

## Validation limits

The frozen handoff source fixture `23fedea9c99480b33ef8e8c5b776effa9ef168c0d12e109b22c3239bad837f30` uses actual PreparedMemory/NativeThreads children, not manual PTD or scheduler-work changes. It specifies A/B overlap, exact borrowed-range rejection, B allocation/free/catalog growth during known A failure, independent permit expiry, original further-offer preparation and physical B reaping before A release. It has not run. Snapshot-reserve failure specifically is source-reviewed, without a global compiler-allocator injection test.

Actual Original assembly remains unenabled for concurrent actors. Per-message Profile/NativeProfile/Graph/provider ownership, IO resolver integration and same-session/distinct-session actual NativeJobs/Graph overlap remain pending. The prepared route proves explicit JobScope authority; the public owned-ApplicationMemory constructor retains its preexisting explicit-enter/ready-PTD semantics and is not represented as prepared child authority. Arbitrary shared payload mutation, foreign SDK synchronization and native ABI fidelity are not established by this lower foundation.
