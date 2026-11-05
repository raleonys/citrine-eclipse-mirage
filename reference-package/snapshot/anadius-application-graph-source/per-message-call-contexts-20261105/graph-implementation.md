# Graph per-message call contexts — source implementation

Only Graph graph.hpp/.cpp were authored. No compiler, project test, linker, executable, DLL, ZIP or REA operation ran. Root owns the shared native-message counter/header/tests and native evidence; the SDK agent owns integration-test changes. Actual Parent/Profile/NativeJobs concurrency selection remains unchanged.

| Source | Captured before SHA256 | Final SHA256 |
| --- | --- | --- |
| graph.hpp | `de539b9a711a527a824ff55d9b22ac67c6098fb3657ab3d0ee49bc0c0d853aaa` | `62938f990e41fed26dc8444969c75e53c90ae2308dc954d0b0ad4a8c10e0b6c2` |
| graph.cpp | `fc90cf952eb5bb9230421d8dc97981ed6c7b60cf94d5b18f0cbd695ea0fdaf84` | `7a5950a9b39d13ab51269a652b105c07e5f2e35d5b47de2e1ffce58cd464d9e2` |

Root captured both before files before GO. This note freezes the author's reviewed candidate; any later concrete correction requires a successor/addendum rather than rewriting recorded proofs.

## Explicit message execution and singular backing

Existing np::JobLogic::process_native_job is still the real child entry. Its process_job now registers a stable MessageCall carrying the exact Parent/Strings/Logic/session/control/JobPermit/actor/prefixes/checkpoint. Stack Admission reserves actor/prefixes and exact Logic+token before metadata allocation. Healthy independent calls, including the same session, are not rejected merely because another native Logic prefix is active. Unknown Logic health uses failed_prefix; broad retained_prefix retains its historical observation role.

The implementation reuses existing Graph adapter bodies in a private execution Impl. Its Options, event-values/map/queue/native-mutex, actual event/achievement gates, initialized publication and achievement counter are **references to the singular creator backing**. Its flags, writers, guarded Clock/Random and provider views are private execution state. Unused private owned-storage members remain empty metadata; no business configuration, Runtime, Online, token/attempt/auth state or actual shared queue/mutex is cloned. The private constructor neither constructs Parent/Strings/Deques nor reacquires the provider lease.

Every child Information/Content/Licensing call selects the completed concrete CallView factory and immutable exact CallContext. It does not install a root with_job/checkpoint/EffectScope, look up a global current job, use ambient TLS authority or swap shared Dependencies. Router references are rebuilt for each dispatch against the same business roots. Actual licensing control/diagnostic, Content, writer, random/time and event adapters use this explicit execution.

## Lifetime and failure prefixes

Root registry strongly owns MessageCall; MessageCall links back to Root nonowningly and owns its private execution. Execution links back to its MessageCall nonowningly. The existing Logic owner/local executing pin retains actual Root. Dependencies are local to process_job and copied into NativeProfile: the call does **not** own its Dependencies, avoiding a registry/call/Root or call/deps/call strong cycle.

Call membership, Root phase/rows, exact token/Logic and bound Parent/Strings are revalidated under the Root metadata gate; Parent/permit/control/source checks and callbacks run outside it. Root source-state fields are captured under Root metadata before querying Parent/Profile/backend. Newly introduced rejection exceptions allocate after unlocking. Metadata preserves LastError locally, without wrapping SDK/opaque message effects.

The call stays published through NativeProfile normal return, writer retirement, explicit private-provider closes, chosen Dependencies/adapter destruction, closed-wrapper retirement, private execution destruction and source checkpoint-target destruction, followed by exact final validation. Only then is the row unlinked by nonallocating splice and empty metadata retired outside the gate. First unknown/partial outcome poisons shared Graph health and retains the actual row/prefix; no guessed unlock/free/provider close or repeated source completion was added.

Pending Admissions participate in retained(token) and the exact Logic destructor guard. A different Logic can still be created/retired while another call is healthy; there is no unrelated-session global job veto. Fresh/reset output storage remains required by NativeProfile's native overwrite contract.

## Actual locks and committed event state

Actual event and bound Runtime achievement mutexes remain shared. Held/waiting flags and native pop old-count/offset/head observations belong to the current execution, so another actor can wait on the actual mutex rather than fail a global ownership boolean. Event structure/map/offset access occurs only after true event acquisition; initialized is an atomic shared publication. An already-initialized queue getter returns the actual queue without a structure scan or pre-peek event lock.

Root's NativeMessageCount change supplies one actual shared relaxed, lock-free count at all typed read/write sites. Graph neither clones that count nor delays native pop commit until unlock. A native waiter may acquire after another actor has emptied the queue: old_count0 validates the no-pop/no-head-touch path without unsigned underflow. Nonempty pop checks the actual already-committed count/offset/front before releasing the same mutex. Unknown unlock/postcheck retains that committed prefix.

Shared atomic event_observed records the same logical lock/unlock prefix for passive Root snapshot; it does not scan another actor's private booleans or grant authority. Snapshot reads count and initialized atomically without adding an event-lock wait.

At producer last use, push_raw performs Strings.assign, admission, Strings.relinquish(slot), admission, then count publication while holding the real event lock. This drops only that producer's whole-ticket temporary view (SSO/Serialized no-op), retaining SDK/foreign pins. A consumer may therefore pop/free a heap event after unlock without waiting for the producer's unrelated Operation to end. It adds no deferred physical free or whole-catalog pin.

## Root progress and lifecycle

Creator passive getters/snapshot/retained, unrelated make_logic/greeting and normal source progress no longer depend on a singleton child-job weak pointer. Session row growth uses Root metadata and generation checks; calls re-look up rows instead of keeping relocated row spans. Creator configuration update rejects live calls before mutation. Drain/external proof return false for healthy live/pending calls and do not discard their leases; final retirement still requires the actual existing lifecycle. Known returned first-bind leases are adopted before a post-allocation health check, and Root lease target destruction occurs outside metadata locks. Backend/profile/bound-source publication uses Root metadata.

## Review and limits

Manual review traced full adapter selection, source controls, Root/private ownership, exact context/row admission, native lock ordering, queue-prefix effects and target retirement. Independent review caught initial incomplete owned/reference declarations, passive lock-observation scope, pending-Logic observation and producer view transfer; the final hashes include all corrections. No compiler failure or runtime success is asserted.

Actual Parent/Profile/NativeJobs remain Serialized in this stage. The existing execution mutex's plain pre-SRW owner_48 reads remain a documented pre-enable prerequisite, as do general shared business string/map/scalar mutations. No body strand, readiness barrier, native class/string/EH ABI claim or additional unload condition was added. The SDK fixture must distinguish real new private-provider selection from a claim that the whole application already runs messages concurrently.
