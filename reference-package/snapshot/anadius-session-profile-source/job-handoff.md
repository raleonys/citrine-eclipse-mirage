# Profile job / closure / session authority — source only

Status: **implemented, uncompiled and unexecuted**. An explicit user rebuild
instruction is required before compilation, tests, linking or binary execution.
Existing build receipts and ZIP apply to their prior source snapshot.

This step supplies nonexecuting identity inspection of a pending source job.
Default Profile/Graph/Policy/Environment/Deque worker checks and InlineJobs are
unchanged. Neither a numeric snapshot nor another same-Domain capability grants
message execution.

Subsequent source checkpoint: `grow_output(extra)` now routes existing026670
growth for this exact session under a per-Operation permit. See
`../anadius-owned-deque-source/job-deque.md`. Default deque growth/close still
require their original worker. This is unbuilt/unexecuted and still grants no
native message-body execution.

Subsequent source checkpoint: `cleanup_exit_waits()` routes the concrete
Policy's existing04a47c for this child's own registered waits, under the exact
live Parent permit and job claim. No request/closure/job completion occurs;
the021770 child bridge is not yet wired. See
`../anadius-session-policy-source/job-exit-cleanup.md`. No positive prepared
Profile/Policy test source or runtime validation is claimed.

## Exact source binding

Application.prepare_job runs on the original worker under a whole normal Parent
Operation. It accepts only a retained Submitting/Pending job and captures job
and session generations, exact live closure/binding, owner/function token,
retained strong control, live Parent control ownership and weak-self identities.
The message function token must be 0x180028d30. The live strong count and session
job pin are checked before owner access.

The move-only JobHandoff holds the prepared Parent JobMemory and immutable
identity. The source row also retains its shared state. Preparing/bound rows
cannot run via the legacy runner or recycle. The existing source strong pin is
not duplicated or released by inspection; no closure cleanup or job-count
completion is manufactured.

Bookkeeping allocation precedes Parent.prepare_job. A reservation blocks the
same legacy job and keeps Profile lifecycle active through preparation. After
the backing offer exists, noexcept moves adopt it and its row claim before
postchecks. A failed postcheck poisons/retains that prefix. Earlier known failure
or an unsupported Parent route leaves no armed claim and selects no fallback.

## Explicit nonexecuting round trip

```cpp
// Original worker, existing whole normal Parent Operation:
auto handoff = application.prepare_job(job_token);
// Caller transfers owning handoff to its actual native job, serializing access.
// Distinct same-Domain native job worker:
handoff.acquire();
auto identity = handoff.inspect(); // numbers only; no mutable source context
handoff.return_memory();
// After acknowledgement and ownership transfer back, original worker inside
// its normal whole Parent Operation:
application.release_job(handoff);
// The original source job is still unexecuted; its old runner is eligible again.
```

The class launches no thread and performs none of those ownership transfers
automatically. Caller retains the facade, backing code and Application/source
lifetimes and serializes every method/transfer. Canceling an unacquired offer
requires excluding pending users and independently joining any launched thread.

acquire uses the existing actual sibling-worker JobMemory protocol. inspect
first pins the Application via its dispatch tombstone, then enters Parent with
the exact memory capability. No dispatch gate is held across Parent entry.
A separate identity-only path revalidates claim, generations, closure binding,
owner/function and live control under that whole Operation. Original start must
have returned and source detach succeeded. It calls no message Logic, Policy,
Graph, Environment or Deque method. Ordinary Profile entry stays worker-bound.

identity() alone returns captured numbers; it is no live permission/finality
proof. return_memory only returns child memory authority. It neither executes
the source job nor clears its claim. Original release_job revalidates identities
and either cancels Offered memory or requires a Returned child. Acquiring,
active or poisoned claims cannot release. Physical thread/FLS exit remains an
independent PreparedMemory obligation.

## Failure and lifetime

Every call pins shared state and reserves its lifecycle. A dispatch active pin
prevents Application retirement through the call; no facade address is retained
across work. An unresolved facade destructor poisons its live Profile when the
target can be pinned and preserves the source claim/closure/strong owner. Idle
memory offers are abandoned into existing root poison. During a busy call,
memory abandonment is deferred until Parent scope teardown, preventing dangling
permission pointers. Reentry is sticky failure; an inspect mismatch retains
instead of granting a different job. An unknown underlying memory root cannot
be canceled into success.

No existing facade/Dependencies layout, base virtual slot or export changed.
Private job storage grows; new types/methods need authorized compilation. No
translation unit or recovered native procedure was added.

## Pending validation and remaining execution work

The controlled sole-owned Parent test source now covers stale generation,
wrong closure owner/function, unsupported prepared-route rejection twice,
unchanged Profile/job/session/LastError state, and original deferred execution
after known preflight failure. **These tests were not run.**

A positive actual prepared-Parent/Profile child acquire/inspect/return/release
round trip is not covered by those test sources and is uncompiled/unexecuted.
Future validation needs that single-Domain prepared fixture plus wrong claims,
generations/control, facade movement/destruction, allocation/reentry, partial
acquisition and tombstone/lifecycle overlap. These are requirements, not passes.

Native message execution still needs explicit Profile source-call admission and
matching Graph/Policy/owned Environment/Deque permissions. Their affinity checks
must not simply be removed. Actual message/thread-exit cleanup, source/kernel/
foreign drain, executable-image/CRT lifetime and both loader hooks remain open.
No overall DLL percentage is inferred from this source checkpoint.
