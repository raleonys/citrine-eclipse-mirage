# OwnedEnvironment concrete native-family row plan

Read-only implementation design for the expanded reconstruction scope selected by root. No production/test edits, REA calls, original access, compilation, tests, link, DLL execution or packaging. Interface agreement and the next evidence stage precede code changes. Host remains unchanged; OriginalDefault gains concrete initial/tracked semantics through its existing exported bindings, rather than unused standalone helpers.

## Representation and actual consumers

Keep vector<Row>, current capabilities/Origin/job_epoch and stable caller-owned environment addresses. Add private Family {Host, Initial, Tracked}, uint32 flags and Pin {None, Owned, ReleaseEntered}. The last state is an entered release effect whose outcome must never be retried after exception. A context-presence/identity field may be needed to distinguish a genuinely null concrete context from a moved-from projection; any nonnull context must resolve the exact bound Parent/scheduler, never an arbitrary native pointer. Existing initial sources resolve the bound scheduler and flags0.

Host roots and all their helpers use Family::Host and existing behavior. OriginalDefault roots use Initial/flags0/no pin. Its current023dd0 optimization and message-response flags1 posting stay unchanged. The expanded contract now makes the exported factory58 meaningful: invoking initial factory58 creates a tracked row; subsequent recovered023dd0, copy/move/destroy, prepare and021420 calls consume that family through existing interfaces. Every new helper must be installed in these bindings or invoked by their row dispatch; no disconnected native leaf module is needed.

Use two native schedule/type identities: initial queries return0a25c0; tracked queries return0a2690. Keep owner token in payload separately from schedule identity so owner()/capability resolution still works. Add a second stable IoHandlerExecutor identity and corresponding constructor/response executor bindings. Generic callbacks can share a private transition helper, but validate expected family and full current source before acting. Host keeps its existing query/schedule/executor configuration. Source type comparator handles only this evidenced concrete domain; it is not general04d2d0 RTTI compatibility.

The host IoHandlerEnvironment remains a defined-field/capability projection, not a native layout overlay. Its state_18 must not be mistaken for the DWORD flags at native payload+0xC. Query remains a checked capability resolving the exact row;006180's pointer identity is represented logically, not by uintptr casts. A future raw ABI class is outside this change.

## Bound transition table

| Existing binding / body selector | Initial | Tracked |
| --- | --- | --- |
| copy+8 /0228c0,0228a0 | Preserve family/flags; no work retain | Preserve family/flags; one retain if context exists |
| move+10 /0228c0,0231b0 | Transfer ledger address/cap without work change | Transfer the same pin with address/cap; clear moved-from context ownership; no retain/release |
| destroy+0 /017a30,0229a0 | Retire host metadata only | Enter one validated finishWork effect for its pin, then retire metadata on normal return |
| prepare40 /023520,023310 | Initial, flags &~1, no retain | Tracked, flags &~1, one retain |
| factory58 /023580,023360 | Tracked, flags unchanged, one retain | Tracked, flags unchanged, one retain |
| response20 /023400,023220 | Initial, flags OR1, no retain | Tracked, flags OR1, one retain |
| response88 /0234c0,0232c0 | Initial, flags &~2, no retain | Tracked, flags &~2, one retain |

Both native-family schedules use owned10->022f90->022e40 and borrowed18=null. OriginalDefault response publication uses the actual final row flags. Existing022e40 handles matching-TLS inline versus post;028ca0 alone adds the posted-operation work pin and ignores the incoming bit1 hint. Do not count that posted pin again in the family transition.

Initial default0 -> response20 |1 -> response88 &~2 still produces1, with no family pin. Thus the already closed ordinary response proof remains true. Preserve unspecified flag bits through the exact bit operations. No arbitrary flag-setter API is needed merely for tests; if nonzero initial/native flags need admission later, bind them through an evidenced constructor input rather than modifying private metadata through a debug escape.

## Ledger commit and pin ownership

Extend existing source_matches/protected-source/growth snapshots to compare family, flags, context and pin state as well as address/cap/state/Origin/epoch. row() validates family-selected schedule/executor fields. Reject use of a ReleaseEntered row as copy/query/move/destroy source; the protected child source cannot be moved/destroyed regardless of family. Initial and tracked temporaries retain the existing selected job's Origin/epoch.

For allocating transitions, capture source proof and selected actor/job, reserve growth when necessary, allocate and retire candidate metadata outside the nonrecursive gate, and revalidate current source/output/Parent/permit. Issue the new capability before the locked no-throw commit. Copy CURRENT rows only after allocator/checkpoint callbacks. Any required retain is an atomic DWORD update after all validations and allocations and immediately before publication, with no callback/throwing work between it and the committed row. This prevents an allocation exception from silently losing an already acquired pin. Natural DWORD/native counter semantics remain explicit; do not invent an unrelated capacity limit or silently claim overflow equivalence.

Each published tracked row with nonnull context owns exactly one family pin. A copy/factory owns a new pin; a move transfers the existing pin; a metadata-vector relocation owns no new pin. Failure before commit leaves old rows/counters/pins unchanged. Failure during postcommit retired-vector callback retains the published prefix, including its pin, under the established poison policy. Do not roll back a real atomic retain after unknown callback effects.

## Proposed finishWork boundary and failure prefix

Assume the separately designed small typed capability accepts the exact scheduler plus an optional pointer/reference to the captured JobPermit. Suggested logical call shape is finishWork(bound_scheduler, selected ? &selected->permit : nullptr); the final type/name belongs to root/Runtime design. It must validate exact Parent/scheduler and either real root actor or that admitted child permit **before** decrement, then own atomic decrement and old-count1 invocation of actual02a190. The callback, not Environment, should combine validation/decrement/stop so a foreign actor cannot consume a pin before rejection. Its owners/code outlive all rows and active effects through the existing provider lifetime.

Environment destroy snapshots the exact row/cap and captured selected JobState, marks Pin::ReleaseEntered under gate, then unlocks before calling that capability. Keep a bounded terminal-effect reservation while it executes, so reentrant same-row mutation or metadata growth cannot consume/retype the row. New native-family perform entry during this reservation may reject with sticky poison after unlocking, analogous to growing; snapshot may inspect the retained prefix. This is a metadata effect transaction, not an unload/concurrency policy and need not change Host behavior.

On normal return, reacquire gate, verify phase/actor/job/open/source/proof and exact row/cap, clear its pin and retire it exactly once. No old Row reference is retained across the callback. On exception, retain ReleaseEntered and poison; do not assume whether validation failed before decrement or stop failed after decrement. Never retry the callback, increment back, report successful destruction or fabricate work drain. A richer consumed marker is unnecessary if no recovery/replay is supported, though the Runtime capability may expose a precise prefix if its existing policy requires one. Preserve callback error state; only host metadata operations use LastError guards.

## Actor and source authority

The same captured JobState/permit is used through source allocation, transition and release. JobState.check validates live Parent control/permit but not protected environment fields, so recheck full protected source fields/cap/Origin/family/flags/pin after callbacks as in current growth. Existing active depth may be nested; compare its captured value, not active==1. JobScope close must see no scoped temporary rows; it cannot release their work pins implicitly.

Current Ports has no rawWorker/WorkerRunEffects and admitted dispatcher/Policy are original-worker bound, which is why root is assigning a separate typed finishWork capability. Do not route a child last release through those original-only wrappers or suppress its02a190 branch. Balanced NativeJobs temporaries often retain another job work pin, but that is not a universal proof that last-release child behavior is unreachable. The new capability resolves this concrete actor gap without imposing new unload conditions.

## Minimum coordinated scope and validation

Production scope is Environment row/configuration/transition bodies, its bind contract to receive the typed capability, and the small Profile/Runtime wiring owned by the other task. Existing023dd0/023cb0/021420/022e40/028ca0 and memory ownership algorithms remain the consumers; change them only if a concrete integration defect appears. Header comments must change OriginalDefault's previous initial-sources-only limitation to describe supported initial/tracked families while preserving its initial fast classification.

Source tests must exercise initial factory58 -> tracked through exported bindings; tracked type selection into actual023dd0; tracked copy retain; move without retain and no source double release; prepare bit0; both response transformations and post pin versus family pins; ordinary/final destroy; root and admitted child last release; foreign permit/scheduler rejection; callback failure before/after consumed release prefix; growth with tracked rows; and Host/default-initial regressions. Track scheduler work, family-owned rows, host refs and session strong refs independently. Do not claim bit1-nonzero coverage without an admitted nonzero-flags input or add a test-only mutation API.

No implementation starts until the finishWork contract and fresh-stage scope are agreed. This plan supersedes the earlier default-only deferral because root explicitly selected full concrete tracked-family support.
