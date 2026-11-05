# Source-stop admission receipt to worker cancellation — source only

Status: **implemented, not compiled or executed**. The user's explicit rebuild
instruction is required before compilation, tests, linking or binary execution.
The previous full-build snapshot and ZIP remain unchanged.

The previous Worker Settings change exposes a controller-owned Stop and binds
that exact pointer to the server Runtime. This addition connects that Stop's
prerequisite to the callback-first stopper returned by install_host. It is an
explicit source composition, not newly recovered native behavior or a complete
production DLL entry path.

## Selection

```cpp
// Existing complete providers/settings and resources.stop_sources are required.
auto host = host_platform::install_host(resources, host_inputs);
accept_stop::Options options;
options.lifetime = actual_cancellation_and_work_proof_backing;
options.scope = accept_stop::Scope::SessionIo;
options.other_work_drained = real_independent_admission_and_raw_job_proof;
resources_factory::select_worker_stop(inputs, host, options);
resources_factory::install(resources, inputs);
```

Selection is passive and must happen on the original Worker controller/fiber,
outside loader lock and before Inputs consumption. The supplied callbacks,
receipt and Inputs must name the exact same callback and memory Domain. A
missing receipt, mismatched Domain or already selected Stop is rejected. Stop
creation still validates its existing imports, scope, capacity and timeout.
Selection never invokes the source stopper or work proof, cancels IO or waits.
Assembly and the caller's Options are not consumed. Stop owns a composed proof
which retains the receipt, actual supplied backing and independent callback.

The selector temporarily reserves Inputs during construction, including opaque
callback copying and local capture destruction. Nested install or selection
rejects before provider effects. Exceptions before the final commit leave the
worker slot unchanged and release the reservation. Inputs and Assembly must
remain serialized; arbitrary direct mutation of their public fields from a
callback is outside the contract. A caught nested preflight rejection does not
represent an unknown SDK prefix or consume Inputs.

## Receipt and ordering

bind_source_stop_with_receipt allocates receipt, shared stop state and callable
before swapping the source donor into the owner. A construction allocation
failure therefore leaves the donor intact. The original two-argument
bind_source_stop API remains as a wrapper. All copies share one stop state.

SourceStopReceipt can only be constructed/published by that stop state. It
retains the exact Foreign Domain and exposes read-only identity and acquire
completion observations. Completion is published with release only after:

1. New callback admission was permanently sealed by Domain.pause().
2. The caller's existing source stopper returned true, memoized as before.
3. A final Domain.pause() reported no active admitted callback body.

Known false leaves the receipt incomplete and permits the existing retry.
Exceptions and swallowed reentry retain the existing poison/terminal-destruction
policy. After completion, repeated stops do not call the opaque stopper again.
The source capture is destroyed before its receipt/Domain owner. The new
selector's independent proof capture retires before actual backing, then the
receipt/Domain. The caller must also retain executable code and any aliases.

The selected other_work_drained callback is the conjunction of receipt
completion, the caller's original work proof, and a second receipt observation
after that opaque proof. It preserves the raw callback's LastError outcome,
including exceptions; the unopened receipt path preserves incoming LastError.
It supplies no always-true fallback. AcceptOnly still requires its stronger
receive/send/session-work prerequisite; SessionIo keeps its existing narrower
external-admission/raw-job requirement. Caller scope is preserved.

SourceBackend still orders stop_sources, Worker.request_stop, native callback
pause and worker joins. Source stopping must therefore release any active
callback body independently of cancellation that only the later Worker request
can trigger. Waiting for that later cancellation inside stop_sources, or
requiring it before callback-body drain can finish, would create a cycle. This
composition does not solve such a caller-specific cycle.

## Bounded meaning and outstanding work

A completed receipt is **not** proof of native Binding lease/entry epilogue
drain: those can finish after the callback body leaves. Existing native-context
pause/active-entry checks and external hook exclusion remain necessary. It also
does not prove unregistered SDK callbacks, raw/deferred jobs, kernel completion,
provider retirement, image unload safety, or compiler CRT thunk lifetime.
The mandatory caller proof must establish independent admission/work drain;
nonnull backing tokens alone do not establish it. Public Assembly fields can
still be dishonestly recombined or bypassed by direct callers; this helper
checks exact identities and depends on the documented lifecycle/ownership.

Assembly gains a shared receipt field, changing its HOST layout. Rebuild all
consumers together when authorized. This adds no translation unit and changes
no export or native ABI. It does not define either production loader hook.

Required future validation, **not run**:

1. Receipt false before stop, false source/active-body retries, true memoization,
   no publication on throw or swallowed reentry; correct LastError propagation.
2. Allocation/callable construction failure before donor swap; donor emptied
   only on success and source capture retained through Domain lifetime.
3. Exact callbacks/receipt/memory Domain rejection; null proof/backing, occupied
   slot and consumed Inputs rejection; passive selector and preserved scope.
4. Throwing/reentrant functor copy and destructor, reservation through retirement,
   failed create leaving the slot unchanged, later normal install permitted.
5. Incomplete receipt short-circuits raw proof; complete receipt plus raw false
   blocks request, raw throw poisons existing Stop, true admits only a wake.
6. Actual Resources/Runtime stop and own IOCP packet/acknowledgement drain with
   the selected pointer, native-entry epilogue overlap and pre-unload retries.
