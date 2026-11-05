# Independent source design review — environment reference ledger

Read-only design result: the vector-growth plan is appropriate for this private ledger, subject to the authority/publication checks below. This is not implementation approval or a fresh native-equivalence finding. Reviewed current environment.cpp/.hpp and the other source auditor's source-triage.md. No production/test edits, compilation, project tests, linking, DLL execution, packaging or REA/session calls occurred in this review.

## Storage and lock boundary

Control.rows (`environment.cpp:57–60`) publishes no pointer/span/reference to its own Row elements. Its address fields point to caller-owned IoHandlerEnvironment objects; executors, tables, protected JobState.source pointers and capability tokens are separate stable identities. A new vector may relocate Row metadata while preserving every current address/cap/state/Origin/job_epoch and all counters. No deque or external span-registration framework is needed.

Control.gate is nonrecursive. Current make/copy perform their complete nonallocating row commit under it (`:136–141,163–167`). A future resize or reserve at those sites would allocate under the gate and could deadlock a same-thread new-handler calling snapshot/query/perform. Prepare the replacement vector outside both Control.gate and the global Registry gate. Do not retain Row& or a row index as an authoritative free slot across that allocation.

Use a dedicated growing reservation set under Control.gate before the first new allocation. perform (`:105–113`) must reject and poison new entry while growing **before** it invokes any admission callback or increments active. Keep checking distinct: probe (`:91–95`) owns that flag while running ordinary checkpoints, so setting checking across growth would reject the outer transaction's own post-allocation admission.

The helper should use the same selected JobState captured by its admitted perform, not silently replace it with a fresh job.lock() selection after allocation. Internal admission may run while its own growing reservation is held; a blanket growing rejection in admission itself would require a narrowly identified internal reservation path. The small design is to guard new perform entry and let the owner run ordinary admission with its captured selection.

Capture expected active count when reserving. Do not require active==1: legitimate existing composed handler/response paths can nest admitted calls before this growth starts. New perform entry during growing must not change that captured count; require it unchanged at commit.

## Allocation-to-publication checklist

1. Validate Bound state, the current actor, nonempty active operation, current selected job identity/epoch/open state, source and destination predicates under gate; reserve growing and record old rows size/data identity. Do not copy old Row values yet.
2. Allocate an empty checked-doubled vector outside gate, preserving LastError for extra metadata work. Old rows and all caller environments remain authoritative.
3. Run existing Parent/actor/checkpoint/IO/backend admission outside gate using the same captured selection. A swallowed callback failure must still leave phase Poisoned or an invalid selection so this check/commit rejects.
4. Reacquire gate and revalidate phase/reservation/active count, table size/data identity, selected pointer/open/epoch, destination freshness and complete current source identity. Copy current Row values into prepared storage only now. Choose a currently empty/new row; do not use a cached pre-allocation Row pointer.
5. Commit vector swap, new row, output fields, refs/statistics together without opaque effects. Existing caps/states/origins/epochs and counters do not change because of metadata growth. Only the new projection obtains a fresh globally unique capability. If capability exhaustion can throw, avoid updating counters/output or moving old rows before that check succeeds; obtaining an unused monotonic cap before the final lock is safe if a later failed commit does not reuse it.
6. Destroy any retired or failed candidate vector outside gate. Keep growing set until that cleanup finishes, including rollback paths; clear it with a short gate scope on every exit. Existing perform exception policy handles source poison; do not synthesize temporary destruction, successful JobScope close or native cleanup.

## Per-route revalidation obligations

| Route | Additional proof needed because growth introduces a callback |
| --- | --- |
| construct(query), currently162 | Current code captures query_row(query) then passes only state/Origin into make. That loses the capability and actual source-address proof. Carry the original query/cap and source identity through the private call. After allocation re-fetch query_row(query), validate the actual environment at its registered address with full row checks, and compare cap/address/state/Origin/job_epoch. Same-address destroy/recreate or identical Origin with a different cap must not revive a stale query. |
| copy(out,from), currently163–167 | Re-run fresh(out,true) and row(from,true) after allocation. Compare the complete current source record against the captured cap/state/Origin/job_epoch; enforce allowed lineage again. Do not copy cached source fields into a new destination after the source or selected epoch changed. Preserve the caller's partial native copy prefix; do not substitute full/fresh(false) semantics for this path. |
| session(out,view), currently206–208 | Recheck Parent ownership before dereferencing the captured control, its live strong count, embedded owner identity and weak_self object/control. Preserve SessionToken/Origin and original-worker-only authority. Re-run fresh(out,false). An old session control cannot be assumed live merely because its values were copied before allocation. |
| accept(out,allocator), currently209 | Preserve exact server allocator token and original-worker-only authority; fresh(out,false) must still succeed. The new row remains the accept Origin/state, not a fabricated session. |
| all child make/copy | Revalidate the exact selected JobState/open/epoch and permit/checkpoint admission. Inspect the protected source at selected.source with full fields and compare its cap/Origin, in addition to validating the immediate query/copy source. Current JobState.check only validates the session/control/permit, so it does not by itself prove the protected environment's payload/table/schedule/executor fields remained intact. New temporary rows must use that selected epoch and exact Origin. |

fresh(out,partial) must run again even if raw output fields look unchanged: a callback may have registered that address meanwhile. For full source validation, current row(value,true) checks payload owner/cap, state, operations table, schedule and executor. For partial output copy, retain the existing fresh(out,true) predicate and preserve the caller-supplied prefix; do not silently overwrite an unrelated changed prefix or broaden the accepted domain. Move requires no new reference row and can retain its present locked path behind the growing admission barrier.

## Lifecycle and scope paths

enter_job (`:216–245`) already requires idle active/checking/binding/job state and reserves job_binding before its own JobState allocation. Growth must not bypass those checks. Its existing final full-source cap/Origin check remains necessary.

JobScope.close (`:247–272`) sets open=false before rejecting active/foreign admission. Thus a close attempt from a growth allocation callback can invalidate the selection even when active prevents the close from committing. The outer growth transaction must recheck selected open/identity/epoch and ordinary permit admission after allocation; merely observing unchanged rows or the same shared_ptr is insufficient. Existing close rechecks protected-source identity and scoped temporaries around checkpoint target retirement; keep those checks and do not keep Row references across that unlocked destructor boundary.

Environment.close (`:274–289`) and destructor already reject active work; the outer perform's active pin is the existing lifetime reservation. An early active-child close rejection currently throws before its later active/phase-poison branch. That rejection does not mutate the ledger, but do not claim every lifecycle rejection necessarily poisons unless an explicit growing check is deliberately placed first. There is no Environment::cancel API in the current header; no new cancel/unload protocol is needed. PreparedMemory cancellation is a separate capability layer.

release's empty-environment fast path is unregistered and does not mutate rows; active table destruction enters perform and should be covered by the new growth barrier. Global query_owner may read coherent old rows under its existing registry/control lock order, then releases both locks before perform. It must not hold either lock while candidate allocation or a source checkpoint executes.

## Recommended source review boundary

The other source-triage plan correctly keeps vector metadata, stable external objects, a separate growth flag, current-data copy after callbacks and source/destination revalidation. The most important extra review points are the same captured selected job across postchecks, active-count equality rather than a new single-call assumption, and explicit protected-source full-field validation rather than only JobState.check. Native evidence still determines the justified behavior domain before root chooses implementation.

Current reviewed source hashes (unchanged):

- environment.cpp: af5bb3c6c5b03714818d616759968b428afe96e6078e81d8b67b6324056be6dc
- environment.hpp: cbb3048100274a1710b458ea5f1aa8a73528a8cd9cccd6510db2134c5cac887f

No whole-DLL percentage increase or executable validation follows from this design audit.
