# Scoped Environment bounded root component tests v2

Status: authored source and static review only. No compilation, project test
execution, linking, reconstructed DLL execution or ZIP update was performed.

Only owned source changed: `outputs/anadius-owned-environment-source/environment_tests.cpp`.
Captured before SHA-256: `0777c3614d7f8cdb96bfe3c02546a737f29557ac1b809bc7061dfea59f84c734`.
Final SHA-256: `02b5350e6359e276391e76a895f52cac9fe47eaffcaa270a53c9452c92e3bc7f`.

This report supersedes `scoped-environment-test-implementation.md` for this
final source. That report remains unchanged with SHA-256
`c77e4cdb7ca920475db929b55e3ba5f7df934bfe8436f7e700877a3bee51c0b6`;
its preceding source identity was
`ea5da8221b329e74f5990e07d71ebfa840ac6b4966280f6103fd9c77755ee654`.

## Added raw slot08 executor-tail regression

`root_copy_executor_tail_boundary` uses the existing Host EnvFixture with one
real registered root source and a fresh destination containing a defined
placeholder executor. The actual `copy_environment` helper sets operations and
schedule, calls the exported raw copy slot08, and owns the source-executor load
after that slot returns.

Its nonmutating admission callback detects actual payload publication from the
destination's defined capability fields plus the public copied counter. At
every such post-copy checkpoint, the destination MUST still contain the
placeholder executor. Payload/state and caller-installed operations/schedule
must already match the copied source projection, while the full source remains
unchanged and active_calls is exactly1. The callback performs only snapshot,
defined-field observation and harmless Parent metadata; it never queries the
incomplete destination or mutates the source/destination.

After the raw slot returns, the caller's tail must install the actual source
executor. Only then does the test query the completed destination. It checks a
distinct copied capability, unchanged source identity, exactly one added
reference and copied counter, no extra create/move/destroy, and no healthy
retained effect. Both projections then retire once through normal source
cleanup. The public late-write assertion catches an executor assignment moved
inside the callee before post-copy admission, even when the final bytes would
otherwise look identical. It does not invent a positive JobScope test for
Scope.copy or claim a runtime pass.

## Preserved first revision checks

`root_move_admission_boundary` still verifies the actual three-argument whole
owned root move: its FIRST opaque callback observes the full untouched source
and original destination before any native header mutation, with one admitted
effect. Successful return preserves LastError, transfers the same capability,
installs moved-from sentinels, increments moved once without changing the
existing reference count, and retires the healthy call.

The `executor` isolated branch now checks full-source preflight rejection of a
forged executor before any native prefix: source/destination defined fields and
create/move/destroy counters remain unchanged, Poisoned retains the original
reference, and active_calls0 distinguishes preflight failure from a published
effect. New `move-probe` (terminal312) swallows nested source-query rejection
during admission; outer validation still fails before prefix mutation, retains
one exact unknown effect, and rejects another callback/effect or field mutation
on retry. Eleven isolated terminal modes are declared by the source driver;
that number is source inventory, never execution evidence.

Existing checkpoint/type-probe/unknown admission, tracked-before/after work
drop and scheduled-handler unknown paths now expect active_calls1 for retained
failed membership. Their actual consumed distinctions, one decrement/finish,
source projection, consumed caller slot and no-replay assertions remain. The
self-deleting handler still releases its own source and deletes itself from
the callback without an outer task/environment reread.

The 4100-projection growth/reuse, OriginalDefault classification and tracked
executor family transformation test bodies remain byte-for-byte equal to the
captured source, verified by read-only text comparison. Stable-address source
fields, fresh generations at reused addresses, expired capability rejection,
family flags, work accounting and natural cleanup are preserved.

## Explicit limits

EnvFixture has controlled sole-worker FLS and controlled WorkStop; finish_job
intentionally rejects because this fixture has no actual prepared child
authority. No JobPermit was fabricated and no parallel use of its global
fixture counters was added. Scope.operations/Scope.copy positive child
coverage, real two-Scope tracked factory/copy/move/drop progress over one
selected Environment Control with A held after a real decrement and B's actual
native/FLS/Parent reaping, and actual Runtime oldcount1/Returned-child stop
composition remain OPEN. No fake work pin, counter reset, second shadow
Environment root or production test getter was introduced by these changes.
These bounded component sources do not establish native ABI/EH equivalence,
full native child response-tail ordering, or compiler/runtime success.
