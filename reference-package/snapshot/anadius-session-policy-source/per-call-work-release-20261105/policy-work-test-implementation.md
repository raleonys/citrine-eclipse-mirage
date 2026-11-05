# Policy work release component source tests

Status: authored source and static review only. No compiler, project test,
linker, reconstructed DLL execution, or ZIP operation was run.

Only owned source changed: `outputs/anadius-session-policy-source/policy_tests.cpp`.
Before SHA-256: `8f68c316ad3b666472914e4eefc205c55bd03bb77d5069df8b85cc7c8ff0b0f1`.
Final SHA-256: `831ddf43348a478bf922e5e338a8c7f56ed253833b3e504c048c5fc119450b76`.

## Preserved leaf checks

The existing nonfinal drop and oldcount1 stop matrix remains. It checks real
Policy leaf calls on its bound Parent: one decrement, stop before wake, preserved
upper32 bits, existing stop/wake suppressing post, exact zero-packet arguments,
false post with status0, and error-read counts. The error77 terminal keeps the
native system category, pqcs context, one formatting/free pair, and exception
value-copy/base-slicing text checks. Unknown post and pre-drop checkpoint errors
still distinguish consumed=true/count0/stop1 from consumed=false/count1/stop0.

These are explicitly component tests with controlled counters and injected SDK
effects. They do not assert real tracked Environment accounting, real sibling
thread overlap, physical reaping, or a complete Runtime pipeline. The controlled
FLS fixture is not extended into an invalid concurrent fake global state.

## New bounded assertions

- Successful copied checkpoint target retirement is exercised both before and
  after the fixture's observed original-worker return. Its destructor observes
  aggregate active=true and Bound, calls harmless original Parent metadata, and
  requires close(true) to return false while the exact WorkCall is published.
  After final validation, the healthy call has active=false. The target copy
  retires exactly once, the caller's own target remains intact, and the nonfinal
  decrement, flags, post/error counts and LastError are preserved.
- `workerror`, `workthrow`, and `workadmission` now expect Poisoned plus aggregate
  active=true for retained failed WorkCall membership. They require close and
  outside-Parent external_quiescent to return false without an opaque external
  proof callback. A fresh consumed=false caller cannot replay the retained
  operation: exact work/stop/wake/post/error/checkpoint and diagnostic counts
  remain unchanged.
- `workforeign` explicitly remains Bound/active=false with no admission record,
  counter/flag/post/error/checkpoint effect, preserving the distinction between
  a preflight mismatch and an admitted unknown prefix.
- New isolated `workretire` (expected terminal537) destroys an admitted checkpoint
  copy outside metadata while the call remains published. Its noexcept target
  destructor swallows a rejected same-actor ordinary Policy effect. The outer
  final validation still fails, preserves exactly the completed nonfinal
  decrement with no stop/wake/post, retains failed membership, blocks close and
  external proof, and rejects a fresh caller replay without another invocation
  or destruction of that admitted target.
- New isolated `workcopy` (expected terminal538) throws from the user-controlled
  checkpoint copy constructor. That copy can observe Bound/active=true and enter
  harmless Parent metadata without deadlocking. It proves the stable work record
  is published before opaque target copying. Failure retains admitted membership
  with no checkpoint invocation, decrement, stop/post, or invented completed
  target destruction. Fresh caller replay cannot copy the target again.

`WorkCheckpointTarget` marks only std::function copies as source copies. Moving
the initial temporary into the caller's std::function does not count as an
admitted copy, and the caller's target destructor is not mistaken for source
retirement. Destructor hooks catch all exceptions; observable failure is checked
by the enclosing test rather than throwing through a noexcept destructor.
Copy/retirement hooks are cleared before their synchronous local captures leave
scope. Failed source fixtures deliberately retain owners and terminate through
their existing isolated-process model; no unknown effect is cleaned up or replayed.

No authored assertion has been executed. Thirty-one isolated terminal branches
are now declared by the source driver, including the two new modes; that count
is source inventory, not an execution result. Actual Environment call adapters,
per-row tracked executor families, concurrent Runtime stop composition and the
full original native child response tail remain separate open work.
