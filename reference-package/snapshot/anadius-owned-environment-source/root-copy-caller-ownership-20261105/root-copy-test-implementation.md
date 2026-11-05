# Root message environment copy caller source tests

Status: authored source and static review only. No compilation, project test
execution, linking, reconstructed DLL execution or ZIP update was performed.

Owned changed sources:

- `outputs/anadius-socket-source/message_dispatch_tests.cpp`
  before SHA-256 `5980c32365669191912281ec39e7ccbc74dc9cddff27a53e43313504694f87e2`;
  final SHA-256 `edaaa9c3601fa6ee5f6e973ab959f8d3b9c73c6a46be6fb1c322d0751ce6a5f5`.
- `outputs/anadius-owned-environment-source/environment_tests.cpp`
  before SHA-256 `9ded4933d9eb592638113835228d0ee1257e9e3906f4243dd33ff15a3036f934`;
  final SHA-256 `654fb74592b875290cd563e06bfea06aa45228ca20a64f95637a77aa8150d3f4`.

## Message helper call boundaries

Seven bounded cases in `copy_caller_boundaries` exercise the new actual public
helper through the existing MessageDispatchContext overload. A local optional
HOST table hook and explicit stack slot bridge observe public arguments and
the real helper body; they do not emulate a production Environment authority.

- Hook entry precedes every source/destination native header write, lookup or
  slot effect. A throwing owner refusal preserves both complete original
  projections and never invokes a body or slot.
- A declined hook falls through to the ordinary body once. A handled hook runs
  that same body once with its stack bridge and returns true; no default
  fallback duplicates it. Trace order is enter/slot for fallback and
  enter/bridge/slot/leave for the handled route.
- The bridge receives the exact cached callback that was looked up after the
  native operations/schedule prefix. The raw slot observes that committed
  prefix and the uncommitted destination executor, copies payload/state and
  changes the live source executor. Only after slot return does the actual
  caller copy that changed executor to the destination.
- Missing, null and duplicate callback bindings reject after the native header
  prefix has committed, before any slot payload or executor-tail effect.
  Original payload/state/executor and the entire source stay unchanged.
- A throwing slot retains its real header/payload and changed source executor
  prefix; the destination executor remains its original placeholder. The
  caller never runs an invented success tail or cleanup after the throw.

All pre-existing normal/prepared dispatch, lifetime, failure, header, trampoline
and authoritative-header test function bodies remain text-identical to the
captured before source. Existing trace assertions still cover both original
message copy segments, live table/executor changes, exact residual cleanup and
the default table's no-hook route. This is preservation of source assertions,
not evidence that they have executed.

## Actual selected owned Environment component

`root_message_copy_caller_boundary` calls the same public helper directly over
the ONE selected Host EnvFixture Environment's exported copy bindings. The
helper's span overload avoids manufacturing an unused frame/API/context or a
second business root. This route executes the actual owned table hook and its
private slot bridge.

Its first nonmutating admission callback sees the original destination before
the caller writes operations/schedule. Every callback sees exactly one active
effect and the unchanged actual source. After payload/copy-counter publication,
the raw-slot checkpoint must still see the placeholder destination executor.
Only a later callback after the caller's live executor tail may see the actual
source executor; that later phase must follow the observed raw-slot phase and
still report active_calls1. The callback never queries an incomplete output.

On normal return the call is inactive, the copied capability is distinct, the
source capability remains exact, and one reference/copy has been added without
extra create/move/destroy accounting. Source and copied destination then drain
normally. This distinguishes the raw-slot callback boundary from full caller
completion and observes ownership through the actual tail rather than only
inside a raw callback.

The previously authored direct raw slot08 placeholder regression is unchanged;
it continues to require the caller-owned tail after raw slot return. The root
whole-move regression, 4100-projection growth/reuse, tracked family and
OriginalDefault classifier function bodies are also text-identical to this
stage's before snapshot, verified by read-only comparison. Existing known
unregistered versus registered-corrupt preflight and retained-prefix assertions
are preserved.

## Limits

The message fixture is a bounded helper/component harness. EnvFixture uses
controlled sole-worker FLS and controlled WorkStop; its finish_job intentionally
rejects because it has no genuine prepared child authority. No JobPermit,
work-count reset, fake pin, shadow Environment root or production test getter
was introduced. Positive Scope/Runtime concurrency, actual two-Scope progress
with real native/FLS/Parent reaping and actual Runtime oldcount1/Returned-child
stop composition remain OPEN. No source assertion here establishes a compiler
or runtime pass, native ABI/EH equivalence, or full native child response-tail
ordering.
