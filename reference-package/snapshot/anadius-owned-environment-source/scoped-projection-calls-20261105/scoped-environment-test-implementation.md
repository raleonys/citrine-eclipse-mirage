# Scoped Environment change: bounded root component tests

Status: authored source and static review only. No compiler, project test,
linker, reconstructed DLL execution, or ZIP operation was run.

Only owned source changed: `outputs/anadius-owned-environment-source/environment_tests.cpp`.
Before SHA-256: `0777c3614d7f8cdb96bfe3c02546a737f29557ac1b809bc7061dfea59f84c734`.
Final SHA-256: `ea5da8221b329e74f5990e07d71ebfa840ac6b4966280f6103fd9c77755ee654`.

## New public root boundary checks

`root_move_admission_boundary` exercises the existing three-argument
`move_io_environment_023cb0` on one valid registered owned projection, with
undefined destination payload/state/executor fields. The first opaque admission
callback must observe BOTH the full original source and the original destination
before any native header mutation, with exactly one active admitted effect.
Metadata snapshot and the actual original Parent context are callable there.
The completed move must preserve LastError, transfer the same capability,
install the native moved-from sentinels, increment moved once, preserve the one
existing reference and all other creation/copy/destruction counters, and leave
no healthy active effect. Normal projection/owner/Runtime cleanup remains.

The isolated `executor` source branch is updated for the new whole owned move
preflight. A forged executor now rejects the FULL source before the native
prefix. Source and destination defined fields and move/create/destroy counters
stay unchanged; the provider is Poisoned with its original reference retained,
but no admitted effect is published (active_calls0). This replaces the former
partial-header expectation. The native slot-helper success order and its live
executor reload are checked separately by the socket leaf tests owned by Root.

New isolated `move-probe` (expected terminal312) swallows a rejected same-actor
source query from inside whole-move admission. The outer call still fails before
committing any source/destination header. It retains exactly one admitted
unknown effect and the original projection/counters. A repeated call cannot
invoke a new checkpoint, publish a second effect, or mutate either prefix.
The driver now declares eleven isolated terminal modes; this is source inventory,
not an execution result.

## Truthful retained-prefix expectations

Existing checkpoint-close reentry, type-probe reentry, unknown admission,
tracked-before/after work-drop failures, and scheduled handler failures now
expect active_calls1 for their exact retained failed effect, replacing the old
transient active_calls0 expectation. This aggregate reports retained membership,
not a still-running callback. The existing before/after consumed distinctions,
one finish/decrement, source reference, consumed caller slot and no-replay checks
remain. A full-source move preflight rejection is kept distinct from these
already-published unknown effects.

The existing 4100-projection growth/reuse test, OriginalDefault classification
test and tracked executor family transformation test bodies are byte-for-byte
unchanged from the captured source snapshot (read-only text comparison). They
retain stable-address capability checks, fresh generations on reused addresses,
expired capability rejection, exact copy/factory/move accounting and source
family/work checks. The existing self-deleting handler still releases its source
and deletes itself from the callback; it is not weakened to forbid that native
ownership handoff. Existing successful scheduler and natural close checks remain.

## Explicit coverage limits

EnvFixture uses controlled sole-worker FLS and a controlled WorkStop. Its
finish_job intentionally rejects because it has no actual prepared child
authority. This change does not fabricate JobPermits or add concurrent threads
to those global fixture counters. No positive JobScope/operations/copy test in
this fixture is represented as an actual child/Runtime result.

Real two-Scope factory/copy/move/drop progress over one selected Environment
Control, same business roots/Parent/Runtime capability, with A held after a real
decrement and B's actual native/FLS/Parent reaping before A release, remains
OPEN. Actual full Runtime oldcount1/Returned-child stop composition also remains
OPEN. No counter reset, fake work pin, second shadow Environment root or
production test getter was introduced. The source-only component assertions
above do not establish runtime success, native ABI/EH equivalence, or full
native child response-tail ordering.
