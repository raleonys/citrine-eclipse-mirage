# Authored actual two-Scope Runtime composition

Status: source implementation and static review only. The scenario below uses
the actual reconstructed components, but NO compiler, project test, linker,
reconstructed DLL execution or ZIP update was run. Every runtime assertion is
UNEXECUTED; this is not a runtime PASS or native equivalence result.

Only owned source changed:
`outputs/anadius-application-graph-source/graph_integration_tests.cpp`.
Before SHA-256: `0d65bfb6d588b571eb4bdb62c8bda3fd0ec31c2eabbd588bbcdb19c96c4c9f23`.
Final SHA-256: `36a43bce9074211ba5f577ef7ac77c9b07cd8db37482962d8a38efd85b568ed8`.

## Exact selected composition and source lineage

New isolated mode `--prepared-scoped-runtime` is wired into the existing source
test subprocess driver. It creates one real Domain, actual NativeThreads
wrappers, one controller-owned PreparedMemory and actual prepared original
worker. Backend explicitly selects OriginalConcurrentActors for that SAME
borrowed Parent. Concrete Graph/Profile/Policy/Runtime and the real kernel IOCP
are used. Existing concrete business providers are shared by this one Graph.

One `Environment(1, OriginalDefault)` is passed through the new typed
`options.profile.environment_owner`. Automatic environment_capacity is0 to
prove the supplied object's own storage policy is selected. Profile performs
the object's actual bind/session/accept/release/close. The public source
projection's owner token must match that exact retained Environment object's
snapshot token; there is no duplicate default/shadow Environment.

An observation-only Jobs/PreparedJobs facade captures the actual Application
and Ports and forwards EVERY action to one real NativeJobs owner. The scenario
uses two MANUAL genuine PreparedMemory Scope actors, not native message jobs:
NativeJobs started/completed/detached/pending and receipt counts remain0. Its
normal production entry is not altered and no fake scheduler job pin is added.

prepare_session captures the actual given SessionView and prepares the injected
peer. Actors start later, in ordinary lifecycle.after, only once the actual
Session is Ready and its source receive loan is pending. The original worker
resolves one source through public registry().environments, matching the real
owner identity+0x78. Only that stable source address, SessionView and canonical
factory binding are captured; no copied growable span/iterator or test getter
becomes authority. An actual live SessionControl strong reference is retained
through both actors and released under a normal original Operation after their
source/native drain, while the pending receive still holds its own reference.

## Genuine tracked work and held post-decrement schedule

The original Parent prepares two REAL JobMemory offers. Each actual child
acquires its own offer, enters that Parent with the offer, obtains its genuine
JobPermit and enters the SAME Environment using its own owned checkpoint and
explicit immutable actor context. The checkpoint checks only its permit,
actual Parent and observation flags; it does not call an original-affine
Graph/Profile checkpoint or infer authority from a current-thread lookup.

A's actual canonical58 factory transforms the protected initial source into
one tracked temporary, retaining one real work pin. Scope.copy retains a second
pin; an explicit scoped move transfers that copy without another retain.
Moved-from empty cleanup is also performed. Immediately before destroying
the moved copy, A arms its checkpoint and samples the actual work counter N.
The original worker waits and B has not been launched, so the first N→N-1
observation can only follow A's real destroy → Runtime.finish_job → concrete
Policy.fetch_sub path. N>1 is required. The checkpoint holds ONLY after this
genuine nonfinal decrement and records the observed consumed prefix.

Only after that observation is B launched. A stays held and the original worker
waits for B's source-return event, so B's work deltas have no other writer. B
uses its own Scope on the same protected source/session and performs actual58
factory, tracked copy and move. It also calls the newly implemented explicit
Scope.prepare_40 on its OWN tracked row, adding one genuine retained pin through
the actual three-address prepare caller. Each actual extra/moved/tracked output
is destroyed with the matching real counter drop, its moved-from empty value
is cleaned, Scope closes, Parent Operation exits and its JobMemory retires.
The source event does NOT attest native/FLS exit.

Thus A contributes two real family retains/drops and B contributes three,
including prepare40. No work counter is assigned/reset, no synthetic retain
stands in for a native family pin, and no test capability replaces Runtime's
real EnvironmentWorkStop. The original's unrelated IO work remains separately
owned by its actual source pipeline.

## Physical progress, ordinary original work and natural drain

The lifecycle callback returns before any OS join/reaping. The existing
Dispatcher.get maintenance point runs outside ANY original Parent Operation.
It joins B's actual launch handle with normal exit code, thereby observing the
real wrapper/FLS exit, and calls actual Parent.reap_jobs. Outstanding jobs must
be exactly1 while A stays held. If ordinary maintenance already reaped B, a
later reap result0 is valid; no guessed exact reap count is required.

The original then enters a brief normal Parent Operation, queries the unchanged
protected Environment capability, resolves the SAME Ready Application session
and performs ONE actual Policy.cleanup_exit_waits call. Its counter increases
by1, Policy remains Bound, and A's foreign work release/offer remain live. This
is actual original work rather than only passive snapshot access.

Only then is A released. A resamples the genuine counter on return from its
held checkpoint, because unrelated original packet/IO work may have progressed
while it was held. Its remaining own retain is consumed from that CURRENT
value, not a stale global baseline. It closes Scope, leaves Operation, returns
its offer and exits through the actual native/FLS wrapper. A is joined and
reaped outside Parent; outstanding jobs becomes0. The original verifies the
source again and releases its real session strong hold. All borrowed source/
SessionView references are cleared BEFORE EOF can retire their owners.

The fixture withholds XML and EOF until this point. It then posts the real
pending receive's exact OVERLAPPED EOF packet through the owned kernel IOCP.
The final pending accept995 uses the existing typed packet/publication path.
These are injected SDK source completions, NOT real network cancellation or
real external network WSARecv proof. Actual source IO pins, Environment rows,
controls, Policy/worker state and Runtime work naturally drain to zero; the
SAME selected Environment closes. Final cleanup count is the one actual
original cleanup, never a reset. The controller observes original plus both
child native/FLS exits before releasing PreparedMemory and the one Domain.

## Preservation and limits

The existing prepared_profile_parallel, prepared_native_parallel,
parallel_receive/maintenance, prepared_native_profile and run_prepared_native
function bodies remain text-identical to the captured source (read-only source
comparison). Shared dispatcher/receive selectors add a branch only for this
new mode, including an initial-accept guard before a Connection exists. The
old native XML, held-dialog, error, early-stop and SDK scripts remain selected
for their original modes.

No production Concurrent opt-in, native job entry change, extra unload barrier,
handler/response scheduling permission, private production getter or ambient
actor authority was added. Actual Runtime oldcount1/Returned-CHILD stop remains
OPEN: this scenario deliberately holds real IO, so its child drops are nonfinal.
Full original native child response/021770 ordering, native ABI/EH equivalence
and runtime validation remain separate open work. Authored source composition
and static reviews must not be presented as executed completion percentages.
