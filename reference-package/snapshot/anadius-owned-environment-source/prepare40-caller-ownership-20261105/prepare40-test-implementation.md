# Prepare40 caller ownership component source tests

Status: authored source and static review only. No compiler, project test,
linker, reconstructed DLL execution or ZIP update was run.

Owned changed sources:

- `outputs/anadius-socket-source/io_completion_tests.cpp`:
  before SHA-256 `7edbb145ca7239421077dcdc76f5083a10f59859991d1fe5b37175ccc60162b4`;
  final SHA-256 `9782579292f1075f9684e237f2cc7e87ecef8db1bd4cf253f31cf63caf1d0f8b`.
- `outputs/anadius-owned-environment-source/environment_tests.cpp`:
  before SHA-256 `654fb74592b875290cd563e06bfea06aa45228ca20a64f95637a77aa8150d3f4`;
  final SHA-256 `03e95083dcf76d2002d7794daa10eddbf8c880536fa97b97ca736ff55f07170e`.

## Actual helper boundaries

`preparation_caller_boundaries` covers the real public prepare40 helper with
default fallback, one admitted private-operations body, and one malformed hook
that runs the body then returns false. Hook entry sees the complete original
source and destination before executor/query or prefix effects. The actual body
temporary is exposed only by address before admission.

In both normal routes, the query destroys the original executor. The already
cached40 function must still construct the same actual body temporary. The
move callback changes the temporary's live destroy target and executor; the
conditional guard must read that target after the move's native state/executor
tail. Private query/factory/move/destroy operations remain inside the one held
owner claim. Both normal routes invoke exactly one query/factory/move/destroy.

The run-then-false hook supplies a valid executor for an attempted fallback;
the real body's replay guard must reject BEFORE a second query/factory. The
first completed output remains intact, with no replay or guessed rollback.
An explicit preparation refusal is also tested with null source table/executor:
the explicit callback must be selected first and reject before any source read,
body attempt or output mutation.

`preparation_unknown_prefixes` adds exactly four bounded exception boundaries:
query, factory40, move slot and live temporary destructor. Query/factory throws
leave the later output move uncommitted. A throwing move retains only the real
operations/schedule prefix and never supplies payload/state/executor success
tail. A throwing temporary destructor preserves the completed moved output.
Each case checks exact entered callback counts and absence of invented source
or output destruction. The test never dereferences the expired internal
temporary after the helper returns or throws.

The original IO move, owning-move, storage, direct helper, error/lifetime and
polymorphic receive assertions remain. Read-only comparisons confirmed the
selected old function bodies, including `polymorphic_receive`, are text-identical
to the captured source. Existing executor-deletion, live guard, early operation
cleanup and no-unwind expectations therefore remain available for the actual
consumers Root has connected to this helper. `handler_dispatch_tests.cpp` was
not modified by this task; its cached-function/deleted-executor source test is
preserved.

## One actual selected owned Environment

`root_prepare40_caller_boundary` exercises one Host EnvFixture Environment
through the actual public helper. A forwarding-only explicit Preparation
captures the body's real temporary ADDRESS and delegates the SAME body to the
actual owned source table hook. It creates no authority, row registry, alternate
environment or replacement algorithm, and does not inspect pre-admission
temporary fields.

The first nonmutating source callback sees unchanged source/output and the
distinct real temporary address, with active_calls1 and no factory/move yet.
The factory-publication phase leaves output unchanged. Subsequent callbacks
observe the full prepared output after actual factory and move, under that
same one admitted effect. All callbacks preserve the exact protected source;
they do not query the incomplete output. The source implementation's actual
three-row reservation is independently reviewed; passive callback observation
alone is not presented as proof of private claim metadata.

Normal return preserves LastError and the source capability, creates a distinct
prepared capability and exactly one reference/factory/move, changes no copy or
destroy accounting, and leaves no healthy active effect. Host preparation adds
no tracked work. Source and output then retire their actual projections and the
fixture closes normally. The canonical moved-from sentinel guard is skipped as
the real algorithm requires; the separate generic helper case exercises a live
non-guard temporary destructor while the admitted body is still held.

Existing 4100-projection growth/reuse, tracked executor families, OriginalDefault
classification, direct raw copy placeholder and actual message copy caller test
bodies remain text-identical to this stage's before snapshot (read-only source
comparison). No historical execution proof or delivery artifact was rewritten.

## Limits

These are bounded source/component scenarios. EnvFixture still has controlled
sole-worker FLS and controlled WorkStop; it cannot mint a genuine child permit.
No positive Scope/Runtime concurrency, actual two-Scope native/FLS/Parent reaping,
or actual Runtime oldcount1/Returned-child stop result is claimed. No fake pin,
counter reset, second Environment business root, ambient actor lookup, shared
context rebinding or production test getter was added. The synchronous prepare
API grants no handler/response scheduling or deferred task permission. All
authored checks remain UNEXECUTED; native ABI/EH and full original child response
tail equivalence remain separate questions.
