# Selected owned Environment and scoped Runtime independent review

2026-11-05. Independent read-only production/test source review; only this new report was written. No compiler, project tests, linker, reconstructed DLL, ZIP update or new REA target was run by this reviewer. No blocking defect remains identified in the bounded six-file change. Runtime assertions described below are AUTHORED, NOT EXECUTED; this is not a runtime PASS, native equivalence result or function-completion claim.

## Exact frozen sources

All six files were independently read and SHA-256 checked after author freeze. Paths are relative to the project root.

| File | SHA-256 |
| --- | --- |
| `outputs/anadius-session-profile-source/profile.hpp` | `838a0f0acd22ef5c23973d3479d767cacbfa43ba754c3372dc82b332c7291f9c` |
| `outputs/anadius-session-profile-source/profile.cpp` | `c9efdbd1cbe629891b41250236a38106ced83946003242fc781467a17889a9d5` |
| `outputs/anadius-session-profile-source/profile_tests.cpp` | `91d359de8fc878baffcac3b8bdeec7570b292cfc1724d1324cd4d99967f3d4ba` |
| `outputs/anadius-application-graph-source/graph.hpp` | `7c0b23301f15f50d153e4ef7df8f7ea13a7561194e9b03d8a625225fdedb1c77` |
| `outputs/anadius-application-graph-source/graph.cpp` | `609d171d7c526dea6f50e5fef91b5e6896e7ef0f326f0396a378a3e53de9ceb1` |
| `outputs/anadius-application-graph-source/graph_integration_tests.cpp` | `36a43bce9074211ba5f577ef7ac77c9b07cd8db37482962d8a38efd85b568ed8` |

Related author reports: `selected-owner-implementation.md` SHA-256 `bf75833107b37e50c158417cc7b582a829c1440d8b7098ef2c5a0a03b52e120d`; `scoped-runtime-test-implementation.md` SHA-256 `a8faa3039f6eaf6be2ff12cac53deac9a829a6193d9083facbaa4a9c6b7b01ae`.

## Production selection and lifecycle

Profile Dependencies and Graph ProfileSettings append an optional typed shared `environment_owner`. A forward declaration avoids the Environment/Profile header dependency cycle. Existing aggregate fields retain their order, and the null option preserves automatic and legacy selection. Graph copies the SAME selected object into Profile dependencies. Its create-time automatic environment-capacity validation is conditional on no selected owner; other capacities and required providers retain their existing validation. A supplied object's deliberate classifier/capacity is not overridden by automatic-default flags or an unused automatic capacity.

Profile uses the existing exhaustive legacy-empty predicate: types, all four sentinels, query/factory/handler/response/copy/accept registries must all be empty when an owner is supplied. Conflict rejects before configuration, default provider allocation or bind. The owner is moved out of the Dependencies slot into the existing State-owned provider path, so there is no second selected business root or duplicate Dependencies owner slot. The existing configuration Fresh-only check rejects Bound/Closed reuse without changing that provider's phase. Existing bind/session/accept/release/close paths act on this exact selected object.

This adds no getter, fake root, wrapper Policy, ambient scope selection or implicit Parent/child permission. Retained public Environment access still needs the existing actor, exact session/origin, genuine JobPermit and Parent Operation checks. Configuration is NOT an exclusive reservation: two Profiles can export a single Fresh object before either binds, and the existing second bind can poison that object. The new header contracts explicitly require one Profile/Graph binding per supplied provider. The caller must not concurrently take over its bind/close lifecycle. This documented single-owner contract was retained rather than adding a general reservation barrier or use_count heuristic.

The production change introduces no new strong backlink from Environment to Graph/Profile. Existing provider leases, State ownership, failure-prefix retention and close order remain responsible for lifetime. Production Parent defaults remain Serialized; only the new isolated fixture explicitly selects OriginalConcurrentActors.

## Profile component source checks

`profile_selected_environment_owner` is a controlled component fixture. Three separate single legacy fields (scalar sentinel, query registry and copy registry) reject mixed ownership and leave the supplied provider Fresh with no bind/session effects. A Host/capacity1 provider is selected despite automatic OriginalDefault=true and automatic capacity0. Export identities establish the same selected provider, and the real Profile owned path binds it, creates a greeting/session projection, reaps that projection and closes it. Bound and Closed constructor reuse reject while preserving the existing provider/application/lease state.

This component fixture is not the separate Graph/Runtime concurrency scenario and was not executed. Its identity/accounting assertions are stronger than merely keeping a shared pointer alive, but do not alone establish overlapping actor progress.

## Authored selected Graph/Runtime two-Scope scenario

The isolated `--prepared-scoped-runtime` scenario composes actual DomainStorage, NativeThreads wrappers, controller-owned PreparedMemory, prepared original worker, selected Graph/Profile/Policy/Runtime and real kernel IOCP. SDK socket operations remain injected token-based operations, not live network activity. One explicit Environment(1, OriginalDefault) is injected with automatic capacity0. The public source registry's token must match that SAME retained Environment snapshot; no shadow provider, replacement Runtime Ports or test getter supplies ownership.

prepare_session only captures the given SessionView and prepares the peer. Actor launch occurs later from lifecycle.after after Application.session verifies Ready and the actual source receive loan is pending. The original captures an exact stable environment address from the public registry, not a retained growable span/iterator. A genuine existing session strong reference protects the owner through both actors. Borrowed source and SessionView references are cleared before EOF can reap them.

Two MANUAL genuine PreparedMemory offers are acquired by two actual native wrappers. Each child enters its own Parent Operation, obtains its own permit, enters the same selected Environment with an owned checkpoint, and closes Scope before Operation exit/offer retirement. The checkpoint validates that child's permit and actual Parent rather than using the original-affine Graph/Profile callback. The test does not fabricate a JobPermit or infer authority from global current-job state.

A performs canonical58 initial-to-tracked construction, tracked copy and move. Its moved-copy destruction uses actual Environment -> Runtime.finish_job -> concrete Policy work release. It arms immediately before that call, then holds only after observing actual N-to-N-1 with N>1. B has not been launched and the original is waiting, so this first observed drop belongs to A's real release, not an invented pin. B starts only after that event. While A remains held and the original waits for B's SOURCE-RETURN event, B uses its own Scope on the same protected source for canonical58/copy/move and explicit Scope.prepare_40, then destroys each genuine retained output, closes Scope, leaves Operation and retires its offer. No scheduler work counter is reset or manually incremented by this new actor route.

The event is not treated as physical exit. Dispatcher maintenance runs outside ANY original Parent Operation, joins B's actual native handle through normal exit/FLS completion, calls Parent.reap_jobs and requires outstanding_jobs==1 while A stays held. A normal original Operation then queries the exact unchanged source capability, resolves the Ready session and performs one actual Policy.cleanup_exit_waits call. Root and sibling progress are therefore tested by actual ordinary APIs, not only snapshots.

Only after those checks is A released. Its resume counter is resampled because the original packet/IO work may have changed while A was held; its remaining own drop compares against that genuine current value rather than a stale global baseline. A Scope/Operation/offer and native/FLS exit then drain, and another outside-Parent join/reap requires outstanding_jobs==0. No exact reap return count is guessed when prior ordinary maintenance may already have reaped a returned row.

The original then releases its real session hold while the receive retains its own reference, clears raw borrows and posts EOF on the exact pending receive OVERLAPPED. The final typed pending accept receives scripted995 only after both actor offers and the session drain. Actual source work/IO/control accounting must reach zero through normal Runtime/Policy closure; the SAME Environment, Profile and Graph must become Closed. These are authored expectations over injected SDK outcomes, not observed real-kernel cancellation results.

The observation-only Jobs/PreparedJobs facade forwards to a real NativeJobs owner for normal binding/teardown, but the scenario's bodies are MANUAL Scope actors. Final assertions require NativeJobs started/completed/detached/pending and message receipt counts all remain zero. It proves no NativeJobs message-body/JobHandoff integration. Existing message-job fixtures remain separate.

## Remaining limits

All tests and their timing/counter/lifecycle assertions remain unexecuted. This stage closes the source-composition gap for retaining the legitimate selected provider and authors meaningful real-component overlap coverage; it establishes no executed concurrency success. It makes no child-old1 claim: A's observed drop is explicitly nonfinal and its remaining drop requires other actual IO work. Natural final stop belongs to subsequent original IO drain.

NativeJobs message bodies using these Scope operations, child handler/response publication, response20/88 caller integration, full028d30/021770 tail and native ABI/EH/runtime equivalence remain separate open work. This source-only stage queried no new native evidence or function coverage. The coordinator's fresh response021420 query belongs to a future stage and is not validation of this freeze. Original binaries, historical build/link evidence and ZIP contents remain unchanged.
