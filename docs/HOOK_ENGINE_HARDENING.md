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
| PEGTL | `419ba27d27e80e33bf180932a9e4c483c4b8dd65` |
| pugixml | `f0a3faa76e04f35110257ca8713363dee3c890e8` |
| MinHook | `8af6b4acae5a9388fd742b56fa79ece89d96f823` |

These identifiers are evidence from SHA-pinned ZIP comments, not independently
fetched Git-object verification. The lock preserves that distinction. Archive
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
The current transform also selects `ACTION_DISABLE` for a single-hook disable
(D326-F0006 / PR #19). Current adapted patch script SHA-256:
`c55ddf218353acf8bf6994ef486771c1f807212c2f39772c298afd19a612cacb`.
Current patched `hook.c` SHA-256:
`0b586a3aa53d3d0b08f148784219da111375bc3557d52a195dd040c9125a56d1`.
These are the active values in `metadata/HOOK_ENGINE_PATCH.json`.

Historical ownership-only script SHA-256
`cd22a276907f4cd7dcaca6d091ebe4147e79d19c6577e5ac56164a735c614d4f`
produced `hook.c` SHA-256
`4d0fc88c91deed5eb97344e4daa9bd69d2fdca15d2fabefd911e08e27ef9fea3`.
That pre-single-disable-fix output is retained only as the named negative control,
not as the current shipping engine identity.

At configure time, `prepare_hook_engine.py` verifies the original tree, copies all
61 MinHook files under the selected project `build/` directory and applies this
script only to that copy. It verifies all staged members, the pinned script and
the expected patched `hook.c` SHA-256. Every build repeats read-only verification.
Both testing-enabled and testing-disabled application builds compile this copy.
The new transaction owner and tests are first-party adaptation, not claimed to be
byte-identical donor application code. Existing MinHook license files and source
license headers are retained in both original and staged trees.

Windows CMake's `file(WRITE)` emits CRLF for this generated source, while the
pinned output uses LF. The preparation helper canonicalizes only the newly
generated `hook.c`, and only when replacing CRLF with LF produces the exact
existing output hash. It then verifies every staged member as before. Unknown
content is not rewritten or accepted; read-only verification still rejects a
CRLF build copy. The vendor input, patch script and output hash pins are unchanged.
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
The manual `Windows MinHook single-disable acceptance` workflow builds
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

The same C++ fixture is also linked to a separate build-only historical control.
`hook_real_peer_control.py` accepts only the exact current hook SHA, reverses the
single action-selection correction, and requires the exact historical `4d0fc8…`
SHA before writing under `build/…/test_controls/`. It does not modify the verified
engine copy or vendor tree. `hook_real_peer_stale_control` succeeds only when
that unchanged fixture executable exits 42 for the precise stale real-IP
mismatch **and** confirms complete restoration/join/handle cleanup. A normal
success, unrelated failure, crash, timeout or incomplete cleanup is rejected;
CTest `WILL_FAIL` is not used to accept arbitrary errors.

Both new tests retain a 30-second budget and run with the existing injected
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
are recorded in the delivery evidence document after execution.

The separate application unload scanner now retains failed resume/close owners
as well; see [UNLOAD_RECOVERY.md](UNLOAD_RECOVERY.md). This is an application
correction and does not change the pinned MinHook patch or original vendor bytes.
