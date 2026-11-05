# Per-call concurrent deque ownership

2026-11-05. SOURCE ONLY, NOT RUN. This author changed only `anadius-owned-deque-source/deque.hpp` and `deque.cpp`; root captured their before bytes. No compiler, project tests, linker, DLL execution, fresh REA query or ZIP operation was performed.

The existing constructor and two grow overloads remain. The immutable Parent policy selects the implementation: Serialized retains the global Growing/active/single-retained-call branch, including its existing close/reentry failure semantics. Concurrent keeps healthy phase Ready and gives each exact actor/queue invocation its own Call, checkpoint sink, optional JobPermit, byte/slot adapters and growth context. Snapshot.active aggregates running calls and preparing admissions; allocated_maps is cumulative after Parent returns each typed map, and successful_growths increments only after source and checkpoint retirement succeed.

A nonallocating stack Admission reserves its exact queue and thread/fiber before compiler Call/list allocation. Prepared list metadata and Call construction occur outside the nonrecursive gate; publication is a no-allocation splice. Same-actor recursive growth is sticky poison, matching the explicitly nonmutating checkpoint contract. A foreign actor requesting the same exact queue gets a nonpoisoning ownership rejection. Different queue/actor calls do not wait for one another's source body. This is not a Session/Logic strand, an external queue mutex or a claim that arbitrary simultaneous mutation of one native queue is safe.

Concurrent maps use the exact Parent output-map resolver instead of a borrowed union catalog. Initial validation uses the new Parent `owns_output_map(span)` predicate for the exact full allocation; a bounded prefix/subwindow is insufficient. That predicate is intentionally not applied from every later admission: recovered026670 frees the old map at02682c before its026831 owner-size/pointer publication. Revalidating old-map ownership in the post-free callback would reject that legitimate prefix. Actual growth arithmetic, slot ordering, aligned header checks and old-map free/new-map publication remain in the existing recovered body.

Each Call rechecks its registry/actor, optional exact JobPermit, caller checkpoint, Parent admission and live queue owner outside the provider gate. Compiler preparation and callback retirement locally preserve LastError; the native Parent allocation/free result is not hidden by a whole-body restorer. The native caller must retain the actual control/queue lifetime; resolver identity and Call descriptors do not add a fabricated strong reference or pin arbitrary payload mutation.

On success, the Call remains active while its checkpoint target is swapped out and destroyed outside the gate. Exact permission and phase are checked afterward; only that row is unlinked, its success counted and its empty metadata destroyed outside the gate. Unknown outcomes retain separate failed descriptors and native prefixes, without overwriting another child's context or replaying the recovered algorithm. A bound-original Concurrent close returns false while any call/admission is live, without poisoning a healthy child. Idle close reserves retirement, checks Parent admission and releases only the provider lease; it frees no maps and invents no drain/cancel barrier.

This is a disabled lower prerequisite. Actual Original production selection, upper per-message providers and native response-queue caller scheduling proof remain separate work. No full same-session progress, executed fixture result or native ABI equivalence is claimed. Both source diffs passed whitespace inspection only.

| Final file | SHA256 |
|---|---|
| `outputs/anadius-owned-deque-source/deque.hpp` | `cf38e757e6189ca3a6a2cd3c9016c504401c6941b7a50cff10c1f225aa3cb35e` |
| `outputs/anadius-owned-deque-source/deque.cpp` | `35bcec259f98a2f65e136f6d257b36cdf1be699dba689d6b814b50cc7474d10d` |

Before bytes are `before/anadius-owned-deque-source__deque.hpp` and `__deque.cpp`, with hashes recorded in this stage's `before-manifest.json`.
