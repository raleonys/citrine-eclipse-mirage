# Hook engine ownership and provenance

The owner selected the Brownfield SAFE-FREEZE implementation and vendor commit
traceability. Application behavior and the three production exports retain their
existing contracts. This document records the scoped lifecycle addition; it does
not assert native MSVC Golden or game acceptance.

## Immutable input and compiled copy

All four trees under `third_party/` remain byte-exact owner archive material.
`VENDOR_LOCK.json` now records each archive's 40-digit commit comment alongside its
existing archive SHA-256 and complete member hashes:

| Archive | Commit recorded in the supplied archive |
| --- | --- |
| Asio | `190a8d8daf1cc62909b4e0c72ff491ba264ca98e` |
| PEGTL | `b60f6110abd37e502d202d13a6c565410c552c33` |
| pugixml | `f0a3faa76e04f35110257ca8713363dee3c890e8` |
| MinHook | `8af6b4acae5a9388fd742b56fa79ece89d96f823` |

These identifiers come from SHA-pinned owner ZIP comments. For the selected
PEGTL 4.x snapshot, the complete archive member set additionally matches the
independent upstream Git tree. Other records retain their stated verification
limits. The lock preserves that distinction. Archive
installation checks the comment as well as archive/member bytes; offline installed
tree verification remains possible without shipping the original ZIPs.

The patch script `cmake/PatchHookFreeze.cmake` derives from
`scripts/build_orchestration/PatchMinHookSafeFreeze.cmake` from the supplied
Brownfield archive `HF20_REGISTRY_B_ARCH_B_STAGE03_ISSUER_SCOPE_FULL_SOURCE_CANDIDATE_NOT_RELEASE.zip`.
Donor archive SHA-256:
`2f32febfcd7ed330cb84b7fcf876ae4bdc8037ce4d7826c921f775c7b6dc6e67`.
Unmodified donor patch script SHA-256:
`0c6a6f72d558b2114a02fccec6e25e0d360714a9ac9bf9a6a4f18e42af928fe2`.

One five-line ownership correction is added to the donor script: if allocation of
the thread array and both subsequent handle closes fail together, the donor's
helper stored only the first handle. The second could disappear from ownership
tracking. The helper now uses the existing second emergency slot as well. A new
test failed on the exact donor output and passes on the corrected copy. No success
path, enumeration policy or public API is changed by this supplement.
The transform also selects `ACTION_DISABLE` for a single-hook disable
(D326-F0006 / PR #19) and retains the public mutation cleanup barriers below.
The completion/retirement addition described below is first-party hardening,
not an unchanged donor patch. `metadata/HOOK_ENGINE_PATCH.json` is the current
authority for the entry script, completion module and all three generated
member hashes. Before that addition the script/output identities were
`b99c83d085ba2af558c2c0536c1cb6e2ce5245c7ca369ce3f6f8bd310f8372cb`
and `c915c65764c72d30e022a60696d916542a31de2964d57cbf032e81740383194e`.

Before these barriers, the active script/output hashes were
`a95b5f1e263c3bac4ff5c048054fc913e0c0469da7fb95b26f54446c06a0eda9`
and `9e7c83c6b944d098f59f53f4b2005dc7ee7c948cf9c64909a442d79de2c68439`.
The older `c55ddf218353acf8bf6994ef486771c1f807212c2f39772c298afd19a612cacb`
and `0b586a3aa53d3d0b08f148784219da111375bc3557d52a195dd040c9125a56d1`
values previously printed as current here predated the project identifier
transformation. They remain historical identities, not accepted alternative pins.

Historical ownership-only script SHA-256
`cd22a276907f4cd7dcaca6d091ebe4147e79d19c6577e5ac56164a735c614d4f`
produced `hook.c` SHA-256
`4d0fc88c91deed5eb97344e4daa9bd69d2fdca15d2fabefd911e08e27ef9fea3`.
That pre-single-disable-fix output is retained only as a historical identity,
not as the current shipping engine or the current build-only counterfactual.

At configure time, `prepare_hook_engine.py` verifies the original tree, copies all
61 MinHook files under the selected project `build/` directory and applies this
script only to that copy. It verifies all staged members, both pinned scripts and
the expected patched `hook.c`, `buffer.c` and `buffer.h` SHA-256 identities.
Every build repeats read-only verification.
Both testing-enabled and testing-disabled application builds compile this copy.
The new transaction owner and tests are first-party adaptation, not claimed to be
byte-identical donor application code. Existing MinHook license files and source
license headers are retained in both original and staged trees.

Windows CMake's `file(WRITE)` emits CRLF for generated sources, while the
pinned outputs use LF. The preparation helper canonicalizes only the three
declared generated members, and only when replacing CRLF with LF produces the exact
existing output hash. It then verifies every staged member as before. Unknown
content is not rewritten or accepted; read-only verification still rejects a
CRLF build copy. That newline canonicalization does not alter vendor inputs or
relax the current patch-script and output hash pins.
Mismatch diagnostics now identify missing, extra and changed member paths with
expected/actual hashes. Reproduction and fresh checks are recorded in
[HOOK_BUILD_COPY_PORTABILITY.json](../metadata/evidence/HOOK_BUILD_COPY_PORTABILITY.json).

## Lifecycle contract

The existing `hook_engine::Api` / `BatchApi` boundary exposes application-owned
integer status codes without including MinHook or Windows headers. `NativeApi`
owns the MinHook include and compile-time checks for all native-to-application
status values. Method signatures, numeric diagnostics and the selected native
engine remain unchanged. Native tests that call MinHook directly include its
header explicitly; source-included shutdown fixtures include it before their
function interception macros.

`portable_hook_transaction` runs the same real transaction source and fake-engine
regressions on Linux and Windows without linking MinHook. It checks staging,
queued activation, failed rollback, retained ownership, drain and cleanup retries.
This proves the transaction boundary is independently buildable; it does not
exercise SAFE-FREEZE, native thread suspension or live patches. Existing native
engine and lifecycle tests retain those separate responsibilities.

| Requirement | Concrete path |
| --- | --- |
| NtGetNextThread SAFE-FREEZE | Patched MinHook enumerates stable duplicated peer handles, checks suspend/context operations and retains failed resume/close/heap owners for retry. Missing Nt API fails closed; no Toolhelp fallback. |
| One queued activation | Application stages all selected callbacks with queue-enable, then calls apply once. No individual enable in application installation. |
| Loader safety | DllMain schedules the worker only. Windows serializes thread DLL notifications; the worker additionally acquires a module lease through GetModuleHandleEx before constructing Application. Freeze/patch runs after the loader boundary. |
| Failed installation | Transaction disables the owned set, drains calls and releases the engine. Failed rollback enters RollbackIncomplete and retains original slots, callback context and engine ownership. An externally initialized engine is never adopted or uninitialized. |
| Trampoline lifetime | Disable precedes in-flight binding-lease drain and the existing external-thread module guard; uninitialize occurs only after both succeed. |
| Retryable teardown | CleanupIncomplete retains the same owner. Application retries hook quiescence; the actual DLL worker also accepts a subsequent shutdown request if Application::stop throws. It retains its image lease and owner until success. |

SAFE-FREEZE addresses ownership and failure handling. Its name does not guarantee
that the OS will always permit thread suspension or that cleanup always finishes.
An unrecoverable OS failure may keep the DLL loaded; timeout retains resources
instead of freeing a live trampoline.

## Verification

### Public mutation cleanup barriers

The Brownfield SAFE-FREEZE upstream candidate at owner-fork commit
`9c4753f771b437898025bd75912f2ac1f063cd07` exposed two paths which bypassed
Freeze: empty `MH_ApplyQueued` could report success while a suspended-thread
owner remained unresolved, and disabled `MH_RemoveHook` could free its trampoline
without first recovering that owner. The existing application NativeApi does
not expose RemoveHook, but its selected engine must preserve this API contract.
Application teardown still disables all hooks, drains callers and retries engine
uninitialization; the backport does not replace that transaction protocol.

CreateHook, RemoveHook, EnableHook, QueueHook and ApplyQueued now retry retained
resume/close/heap cleanup immediately after acquiring the existing engine lock.
Persistent cleanup failure returns the existing MH_ERROR_MEMORY_PROTECT status
before changing hooks, pending intent or caller output. A successful retry can
continue the original operation. The patch requires exactly one entry/lock
anchor for each insertion and rejects ambiguity before writing the build copy.
That backport added only those five guards. The current completion/retirement
addition is separate. SAFE-FREEZE remains enabled; upstream's optional default-OFF
selection and benchmark-only dependencies are not imported.

The inherited injected fixture retains its original 53 checks and
the upstream candidate's extended cases, then exercises all five entry barriers for resume, close and
backing-array release failures. It verifies unchanged output, hook/trampoline
ownership and queued intent, followed by successful public-API cleanup retry.
Five separately hash-pinned build-only controls remove one guard each. The
unchanged functional fixture must exit 1 with exactly the selected assertion;
success, crashes, timeouts and unrelated failures cannot satisfy these controls.
Host-Python tests separately verify mutation identity and rejection oracles.

`hook_live_calls_contracts` calls actual WINAPI leaf targets and original
trampolines while an owned caller runs during 800 direct/queued transitions over
20 sessions. It joins callers before removal/uninitialization and checks exact
target restoration, return values and zero residual handles. This does not prove
arbitrary deep-stack return safety or application unload correctness.

`hook_thread_churn_contracts` runs 50 bounded eleven-hook transactions while 16
handshaken workers remain live and another worker creates, joins and closes
transient threads. An expected strict preparation refusal must preserve physical
bytes and pending intent and fully release peer owners. Every refusal is logged;
churn is then joined, the same transaction is explicitly rolled back and retired,
and exact handle balance is required before a new transaction. No failed process
is silently retried. A passing run with zero natural refusals does not establish
coverage of that branch; injected failure cases remain separate.

The existing manual `Windows MinHook safety acceptance` workflow builds and runs
these tests, all five cleanup controls, the real-peer/stale-action controls and
the unmodified upstream fixture in MSVC Debug/Release. It records executable
SHA-256 identities, the exact source commit, generated/control sources and JUnit.
It remains workflow_dispatch-only; ordinary PR opening does not start this suite.
The generated target graph classifies the two real-API additions as production
tests and the five single-guard controls as vendor comparisons; none is a
shipping dependency. The same tests are in the full Native CTest inventory. Current-project native
results must be recorded separately; the fork's earlier native PASS and benchmark
ratios are not current-project acceptance or an old/new project speed comparison.

`hook_freeze_contracts` compiles the actual verified patched `hook.c` and injects
OS-boundary failures for enumeration, suspend, context, resume, close and heap
release. It checks retained owners, cleanup retry and preservation of the engine
heap while a peer remains unresolved. `hook_transaction_contracts` checks one
activation for eleven staged hooks, partial staging, failed apply/rollback, live
call drain, failed uninitialize and external ownership. `cleanup_retry_contracts`
uses the actual DLL worker with a test-only Application that fails cleanup twice;
the third shutdown must destroy the same single owner.

Single-hook disable selects `ACTION_DISABLE` so suspended peer IPs are moved
from trampoline instruction boundaries back to the target before unpatching.
The build-copy transform requires exactly one single-hook action anchor;
missing or ambiguous input fails before writing output. The source-linked
Python regression checks the transform and exact output identity. The native
`hook_freeze_contracts` fixture creates a real engine hook on executable test
bytes (never called), verifies target-to-trampoline enable relocation, rejects
disable with an unreadable peer context, then verifies trampoline-to-target
relocation and original-byte restoration on retry. Native Windows execution is
separate required evidence; source QA does not establish that result.
The manual `Windows MinHook safety acceptance` workflow builds
`hook_freeze_contracts` and the unchanged `hook_engine_fixture_tests` target
with genuine MSVC in Debug and Release, and retains the CTest and patched-source
artifacts. These two tests do not establish full native/Golden or game acceptance.

## Controlled real suspended-peer acceptance

`hook_real_peer_contracts` links the actual verified engine library without
intercepting thread enumeration, suspension, context, resume or close APIs.
It creates an owned Windows worker, waits for its startup handshake, externally
suspends it and saves its real `CONTEXT`. While that external suspension remains,
the controller places its real `Rip` at known executable-fixture boundaries and
calls public MinHook operations. The engine must add/release only its own
suspension and leave the external count at one. Native `GetThreadContext`
observes each resulting IP; process handle counts reject cursor/thread leaks.

Four owned hook targets cover two NOP prologues, a short hotpatch function and
an expanded relative CALL. The 13 mapping points include a nonidentity offset
(original +5 to trampoline +16). Single enable/disable, queued-single,
queued-all, direct ALL, x64 relay IPs, the hotpatch prelude, enabled and disabled
RemoveHook and active Uninitialize verify physical IPs and target bytes.
The fixture restores the worker's original context before resuming it, signals
and joins it, checks its exit code, closes its handles and restores the initial
process handle count. Executable fixture bytes are never called. Failed cleanup
retains unresolved resources until the isolated test child exits.

The same C++ fixture is also linked to a separate build-only stale-action control.
`hook_real_peer_control.py` accepts only the exact current hook SHA, reverses the
single action-selection correction while keeping all five cleanup barriers, and
requires the exact `021f5d42c3687eaaa2229a4a7ae893315ab50993dbb742e8256e43567f697337`
counterfactual SHA before writing under `build/…/test_controls/`. This is a pinned
one-action control, not the byte-exact older historical engine. It does not modify the verified
engine copy or vendor tree. `hook_real_peer_stale_control` succeeds only when
that unchanged fixture executable exits 42 for the precise stale real-IP
mismatch **and** confirms complete restoration/join/handle cleanup. A normal
success, unrelated failure, crash, timeout or incomplete cleanup is rejected;
CTest `WILL_FAIL` is not used to accept arbitrary errors.

Both real-peer tests retain a 30-second budget and run with the existing injected
failure and upstream-comparison tests in the manual Debug/Release hook workflow
and the full Native inventory. Focused Windows/MSVC results, source identity,
patched/control hashes and retained JUnit/LastTest logs are recorded in the PR.
Wine execution is a separate evidence class. The controlled, externally held
worker does not establish every naturally occurring scheduling race, arbitrary
instruction form, hostile-thread case or Consumer/game behavior. Existing
injected-failure fixtures retain their independent OS-error/ownership coverage.

The older `hook_engine_fixtures` intentionally compiles the immutable upstream
source as a comparison baseline. Its stock failure expectations do not describe
the hardened production engine; the new patched-source tests cover that engine.

Existing real-hook application, callback, public ABI, runtime and Debug/Release
logging checks remain separate from these injected-failure tests. Fresh run results
belong to exact-head workflow artifacts and PR descriptions; older delivery
records remain evidence for their own snapshots.

The separate application unload scanner now retains failed resume/close owners
as well; see [UNLOAD_RECOVERY.md](UNLOAD_RECOVERY.md). This is an application
correction and does not change the pinned MinHook patch or original vendor bytes.

## Upstream port and limits

### Patch completion and generated-code retirement

`PatchHookCompletion.cmake` extends the same verified build-copy pipeline.
Vendor inputs, MinHook's public header/status numbers, production exports and
the application adapter ABI remain unchanged. The generated buffer header is
internal: its release functions return a status and expose an internal address
membership check to the generated hook translation unit.

The engine records each affected page's original protection independently,
including five-byte and seven-byte patch-above ranges crossing a page boundary.
Once bytes are written, enabled/queued bookkeeping reflects those physical
bytes even if cache/protection completion fails. An incomplete record retains
the exact pages and pending operations. New mutations retry completion before
acquiring or resuming peers. If completion fails while a peer set is frozen,
the engine keeps that suspension ownership until completion and resume succeed;
persistent OS failures can therefore leave peers suspended and shutdown pending.
The mechanism does not promise continued process availability under such failures.
Generated trampoline/relay code is flushed before its original pointer is
published. A failed unpublished-buffer release retains its own retry record.

Target bytes can remain unchanged after a failed preparation/write even though
an earlier peer IP was redirected into a trampoline. Consequently retirement
does not rely solely on enabled bits or the application's callback counter.
Before removing a slot or destroying all blocks, the engine freezes peers and
checks full integer/control registers plus stack words against the generated
slot/block ranges. Reads are bounded to 64MiB per stack; query, read, short-read,
context and preparation failures defer retirement. Existing external suspension
counts are preserved. The application still disables hooks and drains callback
leases plus its module-range guard; that protocol and the engine's generated-code
retirement guard cover distinct resources.

VirtualFree failure preserves a linked, unrecycled block/slot. Partial engine
retirement enters a shutdown-only state: remaining blocks and the private heap
stay owned, other mutations fail, and uninitialize retries only unresolved work.
HeapDestroy is checked and occurs after frozen-array cleanup; its success also
reclaims the hook-entry array. Failure does not discard the heap handle.

`hook_completion_contracts` includes the actual generated hook/buffer sources
with projected peer contexts and injected completion/release failures. It checks
repeated failures, no-op mutation barriers, publication, two-peer preparation,
partial batches, stack-read failures and separate real page protections for both
ordinary and patch-above ranges. Targets and projected trampoline IPs are never
executed. `hook_retirement_peer_contracts` separately creates a real suspended
Windows thread and edits/restores its actual context and stack to verify busy
IP/register/return-address retirement barriers and foreign suspension counts.
It restores all state before the peer executes; it does not execute arbitrary
relocated functions or prove general host-stack safety. Both are in the focused
manual MSVC Debug/Release workflow and the native inventory. Configured tests
are not execution receipts; results belong to their exact tested commit.

The seven-byte/five-byte transforms do not provide an atomic all-hook commit,
instruction-decoder completeness, exception/unwind equivalence, or a universal
barrier against threads born after enumeration. Target code must remain mapped
and must not be concurrently patched by an unrelated owner. Caller-held original
pointers remain subject to their documented lifetime. These limits are not
removed by the new resource ownership and conservative retirement checks.

The separate [upstream MinHook proposal #179](https://github.com/TsudaKageyu/minhook/pull/179)
uses owner-fork candidate `9c4753f771b437898025bd75912f2ac1f063cd07`.
That candidate offers an optional strict mode; this project's verified build
copy already selects SAFE-FREEZE and does not import the optional default-OFF
switch. PR #352 backported the five additional cleanup barriers and tests.
Upstream review or benchmark results are not acceptance of this project's DLL.

Stable thread handles do not eliminate threads being born during enumeration,
and sequential multi-hook patching is not an atomic all-hook transaction.
Trampoline callers and DLL unload still require the application's lease/drain
and external-thread contracts. Controlled tests and comparative timings do not
establish superiority over every hook engine or safety for arbitrary host stacks.
