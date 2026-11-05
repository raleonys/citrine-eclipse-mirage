# Independent static review — owned environment ledger growth

Result: STATIC PASS for the final environment.cpp/.hpp diff. Reviewed against the exact stage before copies and the prior design/source-triage notes. No code edits, compilation, project tests, linking, DLL execution, packaging or REA/session calls were performed by this reviewer. Test-source validation is separate; this is not runtime PASS.

Root's fresh comparison establishes individual constructor/move/copy-caller/prepare-caller operations in023dd0,023cb0,028d30 and0229d0 without a4096 host reference-ledger gate in those examined bodies. The actual nonempty native virtual copy/prepare targets remain unknown. This review approves the bounded host-ledger correction and does not claim recovery of those unknown bodies, native STL/RTTI/vtable/EH equivalence or unlimited native capacity.

## Stable authority and source proofs

The private vector<Row> can relocate because no Row pointer/span is externally published. Every recorded IoHandlerEnvironment address remains the same external object; its opaque capability/state/Origin/job_epoch and all old reference/statistic counts are copied unchanged. No output/source environment is moved merely because metadata storage grows.

construct(query) now carries its queried Row value into make instead of discarding the source-cap/address proof. Copy similarly captures a Row by value. The new source_matches predicate compares registered address, cap, state, Origin and job epoch, and validates actual payload owner/cap, state, operations table, schedule and executor. It is checked before growth and again after allocation, so a same-address replacement or stale query cannot gain authority from equal state/Origin alone.

The selected JobState is captured before the first growth allocation and held through ordinary admission and final validation. There is no callback-bearing successful step between the existing perform selection and make's selection, while the active call already prevents JobScope replacement. The final check requires the same job.lock() selection, open state and Origin. Its immutable source_cap/epoch identity cannot silently change. The protected source is captured with full fields/cap/Origin and revalidated after allocation, in addition to the immediate query/copy source. New child temporaries receive the same selected epoch and Origin; existing destructive-source and original-only policy gates remain.

For session Origin, post-allocation code first verifies Parent ownership before reading the control strong count and embedded owner/weak_self identity. Accept Origin remains distinct. Output freshness is rechecked against both registration and the existing raw-prefix predicate. Partial copy retains its existing prefix semantics and writes only payload/state, while full make initializes the defined owned fields. No stale source Row reference spans an allocator call.

## Nonrecursive gate and coherent commit

A separate growing reservation is set under Control.gate, which is nonrecursive. The replacement vector allocation and retired-vector destruction occur outside that gate. New perform entry while growing marks phase Poisoned under gate, then constructs/throws its error after unlocking, avoiding recursive allocation-handler deadlock in that new rejection path. It does not invoke another admission callback or increment active. The existing checking flag remains independently owned by normal probes.

The growth transaction captures the existing active depth, not a new active==1 assumption. It runs ordinary post-allocation admission with the same selected JobState, then validates Bound/growing/binding/checking/job_binding, unchanged active depth and table size/data identity under gate. Snapshot/query lookup can inspect the coherent old ledger during allocation; mutating perform entry cannot publish partial state.

The final locked validation uses nonthrowing boolean predicates. On success it copies current rows, swaps the preallocated vector and publishes one new row/output plus refs/statistics without an opaque callback or storage allocation. Capability allocation/exhaustion is handled before entering that commit; copied rows do not receive new caps. Failed validation throws after unlocking. Candidate and retired vectors live outside the lock scope and are destroyed while growing remains set. Only then does the RAII guard clear the reservation; the enclosing perform postcheck still catches a swallowed cleanup-callback poison after publication.

Allocation failure before commit leaves old rows and source/output projections authoritative. A postpublication admission failure retains the existing coherent poisoned-prefix policy; it does not invent source destruction or a successful cleanup replay. Added metadata allocation/destruction preserves LastError in the growth helper without claiming original allocator/CRT side-effect equivalence.

## Lifecycle boundaries retained

JobScope.close can set its open flag false before rejecting an active close attempt. The new final check therefore tests selection/open state and ordinary permit admission, not just shared_ptr equality or unchanged rows. Existing enter_job idle/job_binding and close protected-source/temp-epoch checks remain. No new child session/accept/scheduling permission or provider-close condition was introduced.

Move/query/destroy paths remain their existing locked operations behind perform's new growth barrier. The unregistered empty-environment release fast path does not alter the active ledger. There is no Environment::cancel API to emulate. No new unload or CRT framework was added.

No unresolved blocking defect was found in this final two-file diff. Caller-owned environment lifetimes, Parent serialization, unknown-prefix retention and the unresolved native virtual target behavior remain material limits.

## Final independently rehashed source bytes

| Source under outputs/anadius-owned-environment-source | SHA-256 |
| --- | --- |
| environment.cpp | e7293bf8bfcb67ec566e8518b34f6c76719ac9810dcd25e3f3242b74183aff3f |
| environment.hpp | 48d78dc79ed9bad3fbdc5f8d8171db335c16c001403a75ebcf7cd11f08bf7d02 |

The author confirmed these bytes stable. No executable validation or whole-DLL completion credit is inferred.
