# Combined stable IO source review

2026-11-05. **No remaining blocking finding in the final production bytes below. Static source review only; NOT RUN.** Reviewed the full IO implementation, seven socket resolver changes and the nine root consumer changes against captured baselines. `sdk-review-source-hashes.json` SHA256 `5ede615a753b4de66d6236df2d3100cd972544d3c6fcb0f92975b6a7b8f216cb` records exact before/current hashes for those18 production files and the two separately authored test sources. The neutral new header correctly has an absent-before record, not a fabricated baseline hash.

| IO production file | Final SHA256 |
| --- | --- |
| io_memory.hpp | `ba766c7d3f2eb1c8d4e84e43ecadab41e1002e6650b55701d6c759efcdcd0dc2` |
| io_memory.cpp | `812044f220b13cbb32d4d4e821879d80181801b6f1b10066b5a4f3e6ec3b763b` |

No production edit, compilation, project test/process execution, linking, original/reconstructed DLL execution, REA call, network or ZIP update was performed by this reviewer. The reviewer authored the two test files; their independent cross-review is root's separate responsibility, not an independence claim of this report.

## Owner, lock and callback lifetime

Concurrent mode stores actual storage/packet descriptors in stable shared Record owners. Registry replacement moves index ownership without relocating those descriptor addresses or rewriting concurrent external packet fields. Exact resolvers validate an already admitted Parent actor and return the owner descriptor, preserving outputs on Miss/Ambiguous. They do not mint Parent authority or a lifetime lease. Serialized retains its original public mutable vectors as the authoritative storage/packet tables; the legacy packet-tamper negative remains meaningful. Legacy first-match routing is preserved at the socket wrappers and Runtime's Host indexed branch.

Stack Effect reservations distinguish actual thread/fiber and a specific busy row. IO metadata locks are released before Record/vector preparation, Parent allocation/free/range-pin work, variant lifecycle operations and opaque recovered callbacks. Parent admission happens before acquiring IO metadata, avoiding the old IO-to-Parent lock inversion. Concurrent disjoint actors can progress while another effect or callback remains active. Replaced vectors, retired shared metadata and Parent pin destruction occur outside the IO mutex. Added metadata bookkeeping preserves LastError while native allocator/callback results retain their actual visible effects.

Call retains exact Record metadata and the starting generation, not the lifetime of a retired native allocation. Foreign access to that active generation rejects. Normal same-actor native cleanup may physically free, cache or reproject the raw storage, and a new generation is independent of the old Call. Call teardown removes only its own reservation, releases shared metadata outside the gate and does not inspect the old projection/slot/generation after callback-driven retirement. The actual posted/receive/send/task sources copy their required locals before cleanup/callback and do not subsequently access the retired source object. This preserves immediate physical free while keeping the bookkeeping safe.

## Corrected completion regressions

Review found a regression at initial implementation `3957a74e…`: observation detached pending owner/range ownership into a stack object before validating the non-last strong decrement. A failed count guard would poison but unwind the range pin and forget the pending prefix. Final code retains the actual pending row, owner pointer, packet and range pin until that atomic guard/decrement succeeds outside the IO gate. No callback or Parent checkpoint intervenes between success and the short exact commit. Routing/pending clear only at that commit; pin reset then occurs outside the mutex before the native callback. Failed count validation leaves the retained prefix intact. Unknown post-decrement lock failure stays poisoned and cannot replay the operation; no general native EH rollback equivalence is claimed.

The first correction moved the known duplicate-completion guard inside the outcome-poison catch, which would break the existing cached-send duplicate/reuse test. Final receive/send/posted wrappers perform `completed/unknown` preflight before Call/outcome entry. Known duplicate rejection therefore keeps healthy IO usable; real observation/body unknowns remain sticky. Existing tests were not weakened to accept the regression.

Root's final review also identified that a disjoint actor could poison IO during the unlocked atomic-decrement interval. After reacquiring the mutex, final observation checks metadata phase before clearing any loan/packet or entering the native callback. This introduces no Parent/admission callback after the decrement. A failed phase check retains the row/range/packet alongside the already-consumed, nonreplayable decrement prefix; it does not claim that strong reference was restored. This interleaving is source-reviewed without adding an artificial CAS hook or claiming an executed scheduling test.

## Consumer agreement

By-value Posted/IoCompletion environments and DispatcherState carry the exact owner resolver only under the explicit Concurrent Parent policy. Profile, both Policy Ports checks, Environment and Jobs use mode-aware actual-owner matching. CoherentIO phase checks no longer request forbidden union catalogs. Accept's Posted overlay, stable packet alias registration and retirement operate on the real mode-authoritative base packet; no stale descriptor lookup is needed after alias unregistration. Runtime validates the observed handle generation and operation/OVERLAPPED before opaque completion and reads no old packet afterward.

The audit also found Stop's remaining legacy catalog-size admission. It now uses `storage_capacity()` forwarding CURRENT registry coverage, as Runtime does, preserving receipt limits even when application binding grows IO. Using only initial constructor capacity would have been insufficient.

## Source validation limits

Authored IO tests are `4bfd141e524e1f140ce03855762d63bd9e16e12a542795b995a41e80d3e062b1`; handoff tests are `b676e53f01d240846fd0e0a3d268e38b8381110b2354d81525162b1a4fea1604`. They specify real PreparedMemory/NativeThreads A/B effects, stable guard references through sibling growth/free, known successful allocation pause, callback envelope retirement/reuse, exact generations and an isolated failed-strong retained prefix. They have not been executed. Concurrent cases use Posted software work; preserved receive/send cases separately cover kernel-loan/range/strong semantics.

Actual Original Parent selection remains Serialized. OwnedAccept's body gate, upper NativeJobs/Profile/Graph per-message ownership, same-session/distinct-session full message progress and arbitrary shared payload or foreign SDK synchronization remain separate work. No compiled concurrency result, complete native ABI or whole-DLL equivalence is established here.
