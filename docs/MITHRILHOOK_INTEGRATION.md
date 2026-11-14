# MithrilHook integration and ownership

The selected application engine is the owner's **MithrilHook 1.0.4** archive.
Its 104 payload files are installed unchanged under `third_party/mithrilhook`.
Archive SHA-256: `e2c1248ed063beb23cb6b5dc63bd6cef8cf9dfc5adbf8fd333729cfac97b76b3`.
The ZIP comment records commit `10203713702b7d9349eb2212bf1bf2e4314a5068`;
this is distinct from independent upstream Git-object verification.
See [the selection contract](../metadata/HOOK_ENGINE.json) and
[vendor lock](../metadata/VENDOR_LOCK.json).

## Vanilla dependency and application adapter

The project no longer contains the previous standalone MinHook vendor tree,
`MH_*` application calls, compatibility aliases or the old source-copy patch
script. MithrilHook's original CMake project builds the exact pinned source.
Windows staging makes a byte-identical 104-member copy inside the owned build
root so the unmodified package can keep native control outputs in its own
build/ subtree. Configure and build-time checks verify every source byte and
reject drift without silently repairing it; only generated build/ output is
excluded. Neither the installed vendor nor its staged source is patched.
The dependency is statically linked, with a separate engine instance per DLL.
MSVC's development C++ DLL rebuilds those same immutable C sources using its
shared CRT; it does not reuse the shipping static-CRT archive.

`NativeApi` calls `MTH_*` directly. Existing application statuses -1 through 12
are preserved; status 13 represents same-thread recursive entry refusal.
The eleven selected hook targets and callback ABI stay the same. A refused
target or failed queue/apply aborts startup through the existing retained-owner
rollback path; it does not silently omit a required hook.

There is no external decoder, NT-header or additional runtime DLL dependency.
The engine uses C17 and normal compiler CRT support. The application remains
C++20. Original BSD/HDE and separately authorized SAFE-FREEZE notices remain
inside the GPL-3.0-only combined MithrilHook distribution; do not remove or
rewrite those notices to eliminate historical library names.

## Freeze, completion and retirement

MithrilHook supplies strict peer preparation, physical-state IP reconciliation,
retryable protection/cache completion, generated-code retirement and supported
x64 unwind metadata. These mechanisms are in the vanilla package, not a local
patch or a selectable weaker mode. Consult its
[safety contract](../third_party/mithrilhook/docs/SAFETY.md) and
[unwind limits](../third_party/mithrilhook/docs/UNWIND.md).

The application still queues the complete set and applies once. A failed batch
can have physically committed targets; it is not an all-or-nothing transaction.
Shutdown disables, drains original/callback users, then uninitializes. Failed
disable, drain or cleanup retains the engine, original slots, context and image
for explicit retry. The original-call admission must remain closed through
successful retirement; never call a retired trampoline.

Same-thread recursive engine entry is refused before its internal lock.
Cross-thread host-lock inversion, uncoordinated thread births, arbitrary foreign
code changes and universal game/unload safety are not solved by this selection.
Persistent OS failures can leave cleanup pending and resources/peers retained.
Engine operations must remain outside DllMain/loader lock.

## Meaningful regression coverage

| Previous coverage | Selected-engine coverage |
| --- | --- |
| Patched-copy SAFE-FREEZE and retained cleanup | Pristine package fault/cleanup/churn tests plus five exact cleanup negative controls |
| Physical suspended-peer mapping | Package real-peer fixture and exact disable-reconciliation omission/cleanup control; project host tests also reject false success in that vanilla oracle |
| Live trampoline calls and thread churn | Package live-call/churn fixtures, completion journal and retirement tests |
| Upstream-only fake Win32 comparison | Replaced by actual selected-engine decoder/relocator, completion, reentry, retirement, unwind and exact-refusal controls; no upstream comparison motor remains linked |
| Application lifecycle and original slots | Existing application/transaction fixtures plus `hook_backend_contracts` using the real `NativeApi`, real patched leaf calls and original trampolines |

The old Freeze-action mutation is not reused: the selected engine reconciles IPs
from actual written state at completion. The package counterexample
omits the disable-side FindOldIP reconciliation in a build-only mutant and
requires the real worker mismatch plus complete cleanup, never an arbitrary crash.

The project-specific backend fixture tests external ownership refusal, staged
batch activation, preserved original slots and real target restoration. Its
post-apply error return, pre-uninitialize error return and callback-drain result
are explicitly injected; these observations are not reported as actual OS faults.
The existing application runtime fixture continues to exercise its configured
eleven-hook setup, Registry A/W/provider paths and shutdown.

`laureondo_hook_acceptance` builds the package's native tests and these project
fixtures. The existing manual hook workflow runs them under genuine MSVC x64
Debug and Release, recording source, executable hashes and JUnit evidence.
Local Linux builds exercise the actual portable decoder/relocator, not Windows
suspension, target patching or loader/unload behavior. Each result belongs to
its exact tested commit. PR opening still requests integrity only; later full
synchronization and owner Consumer acceptance follow existing policy.

Earlier MinHook build-copy observations remain a
[retired checkpoint](HOOK_ENGINE_HARDENING.md). They are not fresh acceptance
of this MithrilHook integration, and archived copyright/source records remain
unchanged.
