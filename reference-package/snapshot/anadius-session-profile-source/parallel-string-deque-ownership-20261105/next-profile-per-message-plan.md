# Profile per-message ownership: read-only next-stage design

2026-11-05. Read-only audit of current selected sources. No production edit, compiler, tests, linker, DLL execution, REA query or ZIP operation. This additive plan is not part of the frozen stage evidence. The retained025480 → detached021770 →028d30/033d70 contract is reused; no new native absence or equivalence claim is made.

## Preserve existing identity; change ownership of active work

`SessionToken` and `JobToken` in profile.hpp already carry index/generation. `JobIdentity` adds the exact closure, session owner, function and control. `JobHandoffState` in profile.cpp already owns per-job response/request snapshot, offer and `Fresh → Processing → Prepared → Consuming → Consumed` message state. Keep those identities, its per-job busy guard, actual strong owner and exact Parent JobPermit. No replacement public token system is needed.

The current blockers are concrete: `JobHandoff::process_message` (profile.cpp711 onward) rejects any `active_calls`, its checkpoint requires exactly one, and it uses `Slot::processing` across the whole033d70 call. It also rejects `native->retained_prefix()` before/after processing, whose present definition includes any healthy active Invocation. These assumptions would reject both distinct-session and valid same-session B while A is blocked in UnknownMessage's modal diagnostic. A counter substitution or a Session mutex would preserve that defect.

In Concurrent mode, a stable per-message Profile call record should retain the exact JobIdentity, stable Job/Slot pointers, JobPermit, response/request references and its own checkpoint/admission state. Admission validates that record and aggregate phase; aggregate active_calls is observation/drain metadata, never a global exact-one permission. Own `Message::Processing` rejects duplicate execution of that job. Healthy other jobs on the same Slot do not reject; actual shared cipher/event/business roots remain shared. HOST retains its current guards.

## Registry publication cannot remain unsynchronized

`slot`, `slot_for`, `job`, `job_identity` and `match_handoff` (profile.cpp340–367) currently index/read fields without the State gate because Parent serialized the whole body. Stable deque element addresses do not make concurrent append/size/index access safe. Root `grow_sessions`/`grow_jobs`, handoff publication/reset, job phase changes and session job counts need short metadata transactions. Capture stable record pointers plus generations under that gate; validate Parent/JobPermit/checkpoints outside it; reacquire and revalidate before commit. Do not retain a deque iterator or replaceable registry-vector span across an opaque call.

`match_handoff` must protect the actual row's phase, closure pointer, handle/detach state, handoff shared_ptr and Slot job/logic identity, not only the claim's immutable numbers. Current `Mutation` rejects all active_calls, while profile_call/mutation are global lifecycle flags. Concurrent session greeting/publication and retirement must reserve the actual affected Slot/Job, with prepared metadata allocation and provider destruction outside the gate. Retirement still requires that Slot's actual jobs/invocations and native strong ownership to permit it; a different Slot's healthy call is not a failure. Whole-application close/drain still observes all calls and retained prefixes. It must not poison a healthy child merely because work remains.

`RegistryView`, `ports()` and Root frame/read/write/completion adapters can remain explicitly original-IOCP-worker APIs. The original worker consumes response receipts serially, so its `completing` continuation pointer need not become an ambient child authority or a universal new invocation framework. Child code must use its captured call and cannot consult that global Root-only pointer. `DispatchState::active` is an aggregate tombstone lifetime pin, not a source body lock; keep it.

## Small NativeProfile foundation before Profile admission

native.hpp/cpp requires more than replacing `invocation_`: each Invocation owns XML documents/printers/ciphers, locals, output/request, checkpoint and its **own immutable Dependencies copy**. Keep actual SessionOwner/cipher and shared business backing; do not clone them or swap Logic.deps_ around a call.

Minimal proposed interfaces are a separate `unknown_prefix() const noexcept` and `active_invocations() const noexcept`, while retaining legacy `retained_prefix()` semantics for old callers; normal return must prove retirement of the caller's own invocation. Concurrent Profile/Graph must use unknown-prefix admission, not reject every other healthy call. A concrete nonvirtual `process_native` overload accepting a per-call `Dependencies` value can preserve the current default overload while enabling Graph-owned call adapters. Shared pointers in that value retain the exact provider/code backing on unknown outcomes. Peer designs may settle equivalent names, but active work and unknown retention must remain separate thread-safe observations.

`Invocation::refresh` (native.cpp40–49) still copies Parent.memory.regions. Concurrent Parent intentionally has no union span, so ordinary heap messages would fail. Compose private per-invocation inline windows with the exact Parent byte resolver, preserving overlap/ambiguity rules without copying a global mutable catalog. Parent already provides the actor's actual ticket borrow. Private DOM/printer/state references and the composite context must outlive any retained source prefix.

## Coupled source group before enabling real messages

| Group | Concrete remaining work |
|---|---|
| Profile profile.hpp/.cpp | Per-message admission, synchronized Job/Slot publication/validation, no Concurrent Session-wide processing flag, exact owner retirement. |
| NativeProfile native.hpp/.cpp | Per-invocation dependencies/lifetime and composite byte lookup; active versus unknown distinction. |
| Graph graph.hpp/.cpp | Replace single weak job/job_binding and global active/router_active/in_opaque/writers with call-bound adapters/router/writer ownership. Preserve shared event/achievement/config/Online roots and only their actual native locks. |
| Content, Information, Licensing effects.hpp/.cpp | Their current mutable JobCall pointer/busy/checking/checkpoint state must become invocation-bound views over the same backing. Calls cannot locate authority through one global current job or ambient TLS. |
| Policy policy.hpp/.cpp | `cleanup_job_exit_waits` (173–207) currently reserves global active and invokes global source(). Concurrent children and Root lifecycle hooks need independent child cleanup reservations with the exact JobPermit. Keep the existing current-thread exit registry and its actual narrow native lock. |
| OwnedJobs jobs.hpp/.cpp | Replace the body-wide source_gate around acquire/process/return (154–156) and its reaper acquisition (231) with brief row reservations. Preserve actual offer, receipt work pin, logical return, physical FLS exit and outside-Parent reaping. Returned response completion already precedes physical reap. |

OwnedEnvironment still has one weak job/job_binding. Current child processing is033d70-only; Root later runs the prepared response/021770 tail. Concurrent explicit clone_environment or full028d30 dispatch on children also requires actor-scoped environment authority. That existing HOST split must remain explicit; it is not proof of the original child's complete ordering. OwnedAccept's retained body gate is another declared prerequisite where that route is exercised.

## Failure and validation bounds

Do not replay033d70 after Message::Processing or free request/response/closure/strong ownership on an unknown provider outcome. Keep each failed invocation independently retained; a healthy sibling ending must not clear another record or shared sticky poison. Retire only that call's opaque checkpoint/dependencies outside metadata gates, then validate admission before declaring success. Preserve the new last-use request handoff and current per-actor map/header ownership; their pins alone are not shared-payload synchronization.

The later meaningful test is two actual NativeJobs on the **same warmed plaintext session**, showLSX false: A reaches the measured UnknownMessage modal; B GetConfig produces its normal response/receipt before A is released. Distinct-session progress is additional coverage. Use no manual work-count drain, readiness barrier, foreign API self-wait assumption or Session strand. Foundations may be implemented in separate reviewed stages, but actual Original selection remains disabled until this coupled route is coherent.

Audited Profile SHA256: header `5c8f0a3e432b1f9c8ae0ab1738173acc496d098345c24cb5ed5554d72cf76808`; implementation `7413d158e406bdcdca2b308f495dbc4a71db265df313566f17e741f4cb892676`. Other named source selectors refer to the current retained implementation; this report does not modify or revalidate the historical native evidence.
