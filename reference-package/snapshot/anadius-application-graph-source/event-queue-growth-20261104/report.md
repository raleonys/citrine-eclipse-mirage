# Grow the actual application event queue — 2026-11-04

The production Graph event queue now grows instead of failing at its configured initial capacity. Previously the default64-slot ring could fail on the64th valid plaintext ShowIGO/ShowIGOWindow request: each handler appends one DOWN event, the lazy queue has one initial DOWN, and plaintext responses consume no events. This is a concrete production behavior correction, not a new missing-handler or whole-native-ABI claim.

## Fresh native basis

`native-findings.md` records the exact addresses and seven complete fresh REA6.8.0/Ghidra12.1.4 records for034660,026670,02c400,032b10 and033d70. The original hash and server identity were verified; full replies and Evidence bundle retained; session closed. No target execution.

Native034660 grows when map_size <= count+1 at0x180034681/3468c, before masking offset at34698.026670 doubles with minimum8 and preserves wrapped FIFO pointer order.032b10 appends DOWN through034660;02c400 copy-initializes the initial DOWN. Original allocation/SEH/CRT behavior remains outside the HOST vector projection.

## Source correction

`graph.hpp` describes event_capacity as an initial power-of-two allocation; default64 remains. `graph.cpp` grows at the native one-spare-slot threshold instead of throwing on full. It prepares both enlarged vectors and pointers, then rechecks source admission before touching live ownership. It transfers only the trivial ReceiveBufferString prefixes in logical FIFO order to new slots beginning at the normalized existing head. Heap payload/header addresses stay unchanged. Old prefixes reset; no throwing call occurs between ownership transfer and coherent vector/map/span publication. A postpublication checkpoint retains existing source semantics.

Growth occurs only under the existing Semantic event lock. It cannot relocate a head borrowed under the Native event-pop lock. The queue object stays stable; NativeProfile Invocation::refresh rebuilds its inline-region mapping after lower calls. Parent-owned large string headers track payload addresses, so metadata relocation does not require payload copies or second ownership. Wrapped-head mapping preserves FIFO without silently compacting the logical head to0.

Allocation failure before transfer preserves the old queue metadata and payload ownership. This is not a promise of an unpoisoned retry: existing Graph failed()/retained-lock behavior still applies. Postcommit checkpoint failure retains a coherent enlarged projection under the existing unknown-prefix rules. No lifecycle, readiness or unload requirement was added.

## Unexecuted regression source

`graph_tests.cpp` adds two existing-fixture pipelines:

-70 valid plaintext alternating ShowIGO/ShowIGOWindow requests from initial64,71 queued events, successful response-frame order, then ordinary backlog retirement.
-13 encrypted requests with an initial8-slot queue, three real event pops before further enqueues, wrapped growth, exact FIFO through SSO/16-byte/4094/4095/5000-byte/embedded-NUL payloads, independent client wire decryption and allocation retirement checks.

The obsolete expected overflow failure and child exit605 were removed from the active test source/main. Historical work/b46-core-proof.py and old registration/proof records still describe their original B46 inputs and were left unchanged. No active common build expectation for overflow605 was found. graph_integration_tests.cpp was not changed; its much larger native-child runner was not duplicated. These sources use actual Native033d70/Policy/Profile through controlled SDK boundaries; they have not been compiled or run.

One subagent implemented Graph growth, another updated meaningful regressions, and another independently reviewed the combined source. Root collected fresh native Evidence and reviewed ownership/refresh semantics. Before copies, implementation notes, independent review and final hashes are retained here.

## Limits and delivery

Two production files and one test file changed in this stage;13 canonical source/header files now differ from the last actual full build. No compiler, project tests, link, reconstructed DLL execution or ZIP update occurred. Existing DLL/ZIP and frozen build/link/package proofs retain their historical bytes.

This removes the default64-event rejection. It does not establish unlimited end-to-end native queues: the separate NativeProfile projection still rejects maps above1,048,576 slots, and existing string-header/profile resource limits remain. Initial host allocation settings and exact MSVC map/node allocator ABI are also separate. No numeric whole-DLL completion percentage is assigned from this local correction.
