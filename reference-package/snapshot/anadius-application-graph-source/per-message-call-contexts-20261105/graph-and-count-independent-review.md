# Independent Graph and shared-count source review

Final status: no blocking source finding remains in the four exact files below. Reviewed full Graph implementation/header against stage before copies, root count/header test delta, and fresh native ordering findings. No compiler, project tests, linking, native/reconstructed DLL execution or packaging. CR-aware whitespace comparisons emitted no errors. `graph_tests.cpp` is unchanged and is not a changed/tested-file claim; the SDK integration fixture has a separate review.

The root registry owns stable MessageCalls; each call owns a private execution with invocation-local active/router/opaque/checkpoint state, writers and concrete Information/Content/Licensing views. The private execution aliases the SAME options/business roots, queue, vectors, native mutex and actual event/achievement gates. Raw root/call backlinks avoid a root→call→root strong cycle. Explicit Dependencies reach `process_native_call`; no global/TLS selected-job or dependency swap remains. Preparation precedes allocations, and publication persists through actual provider closes, writer/adapter retirement and checkpoint destruction. Unknown own prefixes remain registered and shared phase stays sticky; a catch does not clear another execution.

Review findings corrected before final freeze:

- Initial constructor changes had incomplete owned-storage/reference field declarations. Final declarations implement actual shared aliases, rather than copying mutexes/queue ownership; this was static review, not an observed compilation failure.
- Snapshot now observes a shared passive atomic event-hold indicator and initialized flag, without scanning racing private flags or acquiring the queue mutex just to observe it.
- Pending admissions capture exact Logic/SessionToken. Per-Logic destruction/retained checks include only their own pending/published calls, preserving unrelated Root lifecycle progress.
- Event producer `assign→admit→relinquish(slot)→admit→count++` ends only that actor's transient Parent view at the real ownership handoff, under the actual event lock. This avoids retaining an artificial foreign-reader pin until the producer's whole Operation ends. SDK/other-actor pins remain intact.

Lock ownership/waiting and native-pop validation are invocation-local; actual shared mutex acquisition still waits normally outside Graph metadata. Event structure/map access follows acquisition. A queue emptied between peek and acquisition is accepted, and pop/count/head are committed before unlock. Runtime's actual achievement mutex/platform and native count remain shared. Parent bind/source publication and passive row/backend/profile reads use brief root metadata protection; lease retirement runs outside it. Healthy active calls defer drain/external retirement with false, rather than poison merely for activity. No per-session message strand or new unload condition was introduced.

Root's lock-free relaxed count wrapper preserves one shared peek cell, numeric/value-copy aggregate compatibility, and mutations under actual queue ownership. Fresh native `0342fc→034317→034332/337` confirms pre-lock peek and post-lock empty recheck; `03451b/522/528→034573` confirms pop publication before unlock. The unchanged typed033d70 body retains that ordering. New unit assertions check count snapshots/assignment and actual event-pop state before controlled unlock Failure83, with document cleanup absent; previous cases remain. These source assertions are NOT runtime PASS.

This stage still selects Serialized Parent/Profile/NativeJobs behavior. General simultaneous configuration/token/map/cipher mutation, foreign SDK thread safety, native MSVC ABI/EH and full end-to-end parallel scheduling remain bounded future work. The atomic projection is a C++ memory-model mechanism, not a recovered ABI overlay.

| Final reviewed source | SHA-256 |
|---|---|
| `outputs/anadius-application-graph-source/graph.hpp` | `62938f990e41fed26dc8444969c75e53c90ae2308dc954d0b0ad4a8c10e0b6c2` |
| `outputs/anadius-application-graph-source/graph.cpp` | `7a5950a9b39d13ab51269a652b105c07e5f2e35d5b47de2e1ffce58cd464d9e2` |
| `outputs/anadius-socket-source/native_message.hpp` | `31fae1bb984981c2171de631fe0c29a3d2d288aac626cab64e4857bdaa4f753e` |
| `outputs/anadius-socket-source/native_message_tests.cpp` | `0dfe3a4cb187f6eb6429a3e6d914c8334ec3149bd43ec9a2beb674369cc2ac2d` |
