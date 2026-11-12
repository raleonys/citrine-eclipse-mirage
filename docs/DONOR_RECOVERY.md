> **Historical checkpoint.** Use [REMAINING_WORK.md](REMAINING_WORK.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

# Donor retirement and validated failure paths

This report records the recovery/comparison checkpoint before the XML vendor
migration. References below to Yellowfield using TinyXML describe that earlier
state. The current application uses pugixml with first-party compatibility and
depth policy; TinyXML2 remains only in the external diagnostic control. See
[VENDORING.md](VENDORING.md) for the current dependency contract.

**Brownfield and Bluefield have not been retired.** The earlier
retirement proposal was withdrawn. This pass corrected three proven
failure paths and continued the comparative review; it does not
claim that no further defects remain.

## Corrections implemented

| Failure path | Change | Persistent regression |
|---|---|---|
| Allocation failure when initiating `async_accept` | Roll back `accept_pending`; do not swallow the exception. A restart is no longer blocked by a stale flag. | `net_failure_contracts`: actual acceptor, allocation failure, and acceptance retry with a real loopback peer. |
| Cleanup-post allocation also fails after a handler error | The worker only writes an atomic stop marker. The I/O loop observes it and closes its own listener/client resources. | Real worker and socket; verify a healthy session under an ordinary handler error and a cleanup-post allocation failure. |
| Loss of companion-DLL record when `FreeLibrary` fails | Clear successful slots; retain failed slots for the same owner's next cleanup attempt. Reserve the ownership slot before loading. | Real inert fixture DLL, two leases, mixed success/failure, consecutive failures and retry; no native load under reservation-allocation failure. |

Transport stop no longer requires a new `asio::post`
allocation. The I/O runner observes the stop marker in
100 ms `run_for` slices. This is neither a new CFG option
nor a protocol timeout. The 100 ms interval is the
observation interval, conditional on the executing callback
returning; it is not a total shutdown-duration guarantee.
The Windows Asio `io.stop()` path can also throw during
its IOCP wake-up, so the allocation-failure correction
does not rely exclusively on `io.stop()`.
Sockets are not closed from a worker thread.
Once the I/O runner closes the sockets, the control
thread joins it, then drains active handlers.
If a cancellation callback throws after the join,
a later `stop()` can drain the remaining callbacks.
There is no forced termination of user/provider callbacks
or general bounded-shutdown guarantee.

The reference `libraries::unload_035ab0` body was
not changed: its original behavior of ignoring the
`FreeLibrary` result remains in the reference model.
Retry ownership was fixed in the concrete
`ExtraLibraries` Application adapter. Since ownership
is reserved before native loading, an unsuccessful
vector allocation cannot force an unsafe potentially
failing FreeLibrary rollback.

## Test evidence and limits

Two persistent test executables contain eight independent
failure scenarios. The same test sources were compiled
against the old and corrected Production CPP files:
all eight scenarios FAILED on the old code and PASSED
on the corrected code.
[RED_GREEN.json](../metadata/evidence/recovery/RED_GREEN.json)
and raw case logs are retained. Fault injection is
restricted to the test executables; no allocation
or Win32 API overrides were added to Production.

The initial Debug run exposed a pre-existing test-oracle
race in `network_policy_contracts`: the I/O thread may
close the socket before the worker increments its
error counter. The revised test observes both closure
and the asynchronous error record within a time bound;
neither oversized response acceptance nor the expected
error was weakened. The initial failed log remains in
[DEBUG_INITIAL_TESTS.log](../metadata/evidence/recovery/DEBUG_INITIAL_TESTS.log).
Final runs, repetition counts and source QA are in
[VALIDATION.json](../metadata/evidence/recovery/VALIDATION.json).

The three Production C exports/ordinals, four vendors,
TinyXML depth 500, fixed SDK/port, connect scope,
PNG/VERSIONINFO and Golden linker/metadata policy
were preserved. No public header or function signature
was changed. The development DLL's automatically
emitted C++ symbols were compared under the same
compiler: compiler-generated inline destructor
emission and new private methods differ. This table
is not the fixed Production C ABI; details are in
[SOURCE_API_DELTA.json](../metadata/evidence/recovery/SOURCE_API_DELTA.json).

LLVM-MinGW/Wine results are not native Windows/MSVC,
Golden, or game acceptance. The previously accepted
DLL SHA-256 was
`4a0f37c9328ecf31d0815a9e9b85634bc481e77e2f07b636d4ea65bb4783f26b`;
that DLL is not represented as the current modified
source. New native Windows/Golden and game acceptance
remained pending. Local output was not promoted to dist.

## Expanded comparative review

The hash/role inventory of 1,805 first-party donor
files remains preserved. In this pass, 30 additional
files covering XML, Config2, route bindings/selectors,
startup and IOCP backpressure were reviewed at the
level of selected branches: **67 reviewed records total**.
This did **not** close the complete semantics of all
67 files; 1,738 files still lacked individual manual
review records. These files also include tests, tools
and documents, not only Application CPP files.
Lexical function and similarity indexes are not
proof of semantic equivalence.

| Family | Reviewed difference | Decision |
|---|---|---|
| XML framing / RequestView / child index | Brownfield uses literal LSX framing, one envelope/command and owned DOM spans/indexes. Yellowfield has NUL framing and RE document/dispatch contracts. | Stricter acceptance was not silently imported; ownership/boundary test ideas were tracked separately. |
| Entity/UTF-8/response formatter | Brownfield distinguishes decoded/source data and uses checked-size formatting with strict XML character validation. Yellowfield runs TinyXML and the RE printer. | Do not import the donor codec/allocator wholesale. Early serializer validation and aggregate memory budget remain open candidates. |
| TinyXML adapter | Bluefield limits implementation-local depth to 100 and uses staging and wrapper allocations. | Preserve the owner's depth-500 decision and measured allocation improvements. |
| Config2 / settings | Donor uses raw quoted spans, depth 33, staged visitor, section/key authority and different trailing-comment rules. | Preserve RE escape actions, partial state and error positions. Do not restore SDK/port CFG overrides. |
| Path / store | Brownfield uses module-relative ghc helpers and private model/arena storage. | Preserve EXE-relative configuration and standard filesystem; do not reintroduce Gulrak, custom CRT or a 548-byte ABI. |
| Route inventory / authentication / selectors | Donor distinguishes primary, compatibility, extension and local-social routes; certain selector/default and authentication-lease boundaries differ. | Do not import new commands or commit policies as presumed omissions from RE. Full route/output differential campaign remains open. |
| Startup / teardown | Donor has stop-before-initialize and stop-before-listener checks and separate logical/physical owner retirement. | Continue phase-specific fault comparison with the existing DLL worker/hook owner; do not import singleton/model architecture. |
| IOCP backpressure | Donor uses a 32 MiB non-reading peer and native IOCP compile gate. Yellowfield has actual AsioService slow-greeting/response/deadline tests. | Existing test counterparts were identified; Wine execution does not prove real Windows IOCP acceptance. Complete completion/cancellation/fault matrix remains open. |

File hashes and reviewed branch records:
[MANUAL_REVIEW_LEDGER.json](../metadata/evidence/recovery/MANUAL_REVIEW_LEDGER.json),
[ALL_DONOR_FILES.csv](../metadata/evidence/recovery/ALL_DONOR_FILES.csv),
[CANDIDATES.json](../metadata/evidence/recovery/CANDIDATES.json).
The candidate list preserves the initial findings'
historical status; read it alongside the corrections
applied for DC01–DC03 and remaining work above.

## Outstanding work before retirement

Native Winsock ownership and pending-callback retirement
under socket/listener close failures; cached deadlines
under timer setup/cancel failures; unbounded hook
quiescence waits; registry original-close/handle-reuse
provenance; aggregate serializer, event and configuration
memory budgets; console/environment side effects; and
startup phases remained open. Remaining source, test
and tooling families require manual comparison.
Every candidate needs concrete source/test evidence for
one disposition: implemented, already covered,
intentional policy difference, not applicable or open.
Updated source then needs native Windows/Golden and
game acceptance. Until these are closed, neither
"no further improvements" nor "donor obsolete"
is justified.

The source parent identity is the exact hash of
the immediately preceding delivered source ZIP:
`1f2a6581a08dbafa940031353c8723e1947e00f0a4681f3bceb1d9f65e3d509e`.
The accepted binary/log ZIP or a later working
copy with acceptance documents must not replace
that parent. Reviewed changed files and byte pins
are in [REVIEWED_CHANGES.json](../metadata/evidence/recovery/REVIEWED_CHANGES.json).
