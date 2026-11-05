# Scoped child-worker deque growth — source only

Status: **implemented, uncompiled and unexecuted**. No compiler, tests, linker
or binary was run. Explicit user rebuild authorization is still required.
Existing build receipts and ZIP apply to their previous source snapshot.

This step connects a live Profile JobHandoff to existing026670 growth of its
exact session output queue. It does not execute a message body, clear the job
claim, free its closure/strong owner or manufacture source completion. Default
InlineJobs and Environment/Policy/Graph worker checks remain unchanged.

## Per-Operation authority

An explicit child Parent Operation may mint a copyable JobPermit. It holds
Parent metadata and a unique shared seal with immutable actor thread/fiber and
an atomic active bit. The Parent stores only a raw active seal identity; there
is no shared ownership cycle. Repeated minting within one Operation reuses the
same seal, including after a reentrant allocation handler publishes it first.

Operation destruction release-invalidates the seal before clearing active
scope/actor and releasing the JobScope pin. Copies can retain metadata, but
cannot extend permission. check rejects another Parent, another thread or an
expired seal before waiting on the Parent gate. Under that gate it then checks
the exact currently published seal, explicit job scope, actor, phase and actual
RootScope PTD/fiber permission. Same-thread fiber changes reach the existing
root poison policy. A new Operation cannot reactivate a previous seal.

Normal original-worker Operations cannot mint this permit. The Parent must be
shared-owned so copied metadata remains safe. Keeping an expired permit is no
provider lease, native thread join, active source permission or drain proof.
Operation's HOST layout grows again; rebuild all allocating/passing consumers
together when the user authorizes compilation.

## Concrete source path

Deques has an explicit grow overload taking JobPermit. It verifies permission
before queue access and copies the proof into its growth descriptor. The copy
is rechecked before and after opaque checkpoints and after checkpoint-target
retirement. No Operation pointer or stack scope reference is retained. The
existing typed Parent-owned queue/map checks and actual026670 algorithm remain.

Default grow still requires the original worker, as do construction and close.
The new proof grants growth under the exact current Parent Operation only; it
does not grant lifecycle control or arbitrary higher-provider access.

JobHandoff.grow_output(extra) pins the dispatch target, owns the exact child
Parent Operation and a Profile active call, revalidates the complete live claim
and session/control identity, then selects that session's output.fields. Its
checkpoint captures shared handoff state and stable Application implementation,
never a stack Call guard. The new deque overload runs actual026670 there. No
message Logic, Environment, Policy or Graph route is invoked by this method.
Unknown outcomes retain/poison the affected source prefix as before.

## Callable retirement correction

Static review found that multiple by-value std::function moves could leave
internal small-buffer donors that retired after success publication. Public
grow now passes its sink by reference through the helper into Call construction.
All backing fields initialize first, then checkpoint.swap(donor) consumes that
sink. No internal callable donor remains after construction. The retained target
retires while Growing/active; permit/provider checks run afterward before Ready.
Caller-held copies and external plain-moved donors remain caller lifetime
obligations; this API cannot consume objects outside its own parameter sink.

## Unexecuted validation source

child_deque_permit was added to the existing real single-Domain handoff fixture:

1. Two prepared Parents in one Domain; normal-worker mint and wrong Parent deny.
2. A distinct actual worker rejects before waiting on another actor's held gate.
3. Default grow/close still deny the child; the explicit route describes two
   actual026670 typed-map growths, 8 then16 slots, preserving typed ownership.
4. Copied proof expires at Operation exit and stays invalid in a later scope.
5. Original worker performs actual owner/map disposal and provider close, while
   backing still waits for independent actual child thread/FLS exit.

These are source scenarios, **not executed passes**. Opaque checkpoint/capture
retirement, allocation reentry, mid-call scope loss and SDK/allocator/PTD failure
runtime checks remain pending. Positive full prepared-Profile handoff/growth
integration is still absent from current test sources and unvalidated.

Next work is scoped Environment access using live authority, then matching
Policy/Graph and Profile message execution admission. Source/kernel/foreign
finality, native EH/ABI/image/CRT lifetime and both loader hooks remain open.
No overall DLL percentage is inferred from this change.
