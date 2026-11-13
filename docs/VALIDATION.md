# Validation classes

| Class | Evidence established |
|---|---|
| SOURCE_QA | Source hashes and scope, metadata/command ownership, flat package identity, and negative infrastructure checks |
| PORTABLE_UNIT | Portable application and host-Python infrastructure tests on Linux |
| WINDOWS_CROSS_BUILD | Windows x64 objects and actual DLL linking with LLVM-MinGW |
| WINE_SOURCE_TEST | Source/adapter tests of Windows executables under Wine; this is not native Windows execution |
| READ_ONLY_BINARY_INSPECTION | PE import/export/resource reporting and byte identity of the owner-supplied avatar |
| WINDOWS_MSVC / WINDOWS_NATIVE | Recorded only when actually executed on Windows with MSVC |
| GOLDEN_TARGET_ACCEPTANCE | Full pipeline against the actual product policy and exact DLL; fixture PASS is insufficient |
| CONSUMER_GAME | Owner execution of the named/versioned game and Consumer flow against an identified DLL; earlier owner results do not validate a new candidate |

Run `python -B scripts/run_source_qa.py`. CTest infrastructure tests are
registered in both the portable configuration and the Windows test group.
The CRT probe checks the compiler CRT's heap, exception unwinding,
container/thread, and TLS-destructor behavior. It does not guarantee custom
CRT equivalence or a cross-DLL C++ ABI.

Source QA retains partial stdout/stderr from a timed-out child in its
failure output and optional JSON report, decoding byte streams as UTF-8
with replacement. A timeout records exit code 124; a child startup error
records 127 and its diagnostic. Remaining checks still run, and either
failure keeps the overall result `FAIL`. The per-check deadline remains
90 seconds; use `--report build/qa/SOURCE_QA.json` to retain the report.

`tools/inspect_dll.py --report` rejects destinations resolving or hardlinking
to the input DLL, including input aliases through parent directories. Report
file symlinks and nonregular destinations are rejected. JSON is published by
private temporary file and atomic replacement, preserving unrelated output
hardlinks and the previous report on serialization or replacement failure.

The XML reader allocation contract retains earlier decoded attribute/text
views while injecting recoverable C++ allocation failures into later lazy
lookups and ordered-attribute extraction on the same document. It checks full
contents, pointer/wrapper identity, successful retry and isolation from later
parses. Allocation sites are discovered dynamically; MSVC checked iterators
remain enabled and excluded proxy-sized requests are reported separately.

The ordinary XML reader suite also injects one failed pugixml allocation at
a time during normal parsing and compatibility DOM construction. Long decoded
attribute/text payloads, attribute order and a following child must remain
complete on controls and retries. Failed parses preserve a separate earlier
document and its borrowed views; vendor live blocks return to their baseline
and the original allocation callbacks are restored after document destruction.

The bounded C++ type-identity check masks comments, quoted literals and
raw strings before scanning socket interface/adapter definitions and their
namespace marker. It preserves source offsets and line attribution while
retaining duplicate-definition and direct self-inheritance diagnostics. A
comment-only inventory cannot satisfy the twelve interface/adapter pairs.
This lexical check is not a C++ parser or preprocessor; the actual Windows
reference-object compilation remains separate authoritative evidence.

The PE checks adapted from Brownfield reject malformed optional headers, raw
extents/overlap, resource recursion/fanout, ordinal-only exports, prohibited
CRT imports, checksum errors, and SecurityCookie defects. Additional tests
reject incorrect/private manifests, source drift, duplicate or unsafe ZIP
paths, nested archives, symlinks, write failures, unowned metadata, missing
command arguments/exit codes, and premature Golden termination.

Historical VDF implementation/compiler results are in
[VDF_PARSER_STATUS.json](../metadata/evidence/VDF_PARSER_STATUS.json);
the adaptation scope is recorded in
[INFRASTRUCTURE_TRANSFER_MATRIX.json](../metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json).
Raw results of earlier infrastructure/source tests are retained; they do not
establish the same claims as the current reports.

Current runtime integration:
[RUNTIME_BINDING.md](RUNTIME_BINDING.md). The earlier vendor/parser test
counts here are historical snapshot evidence.
[RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json) describes
active bindings and retains older validation checkpoints; it is not an
execution ledger for new tests. Use the selected head's workflow results.

Current application C interface and separation of source DLL variants:
[PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md).
Historical interface validation:
[PUBLIC_ABI_STATUS.json](../metadata/evidence/PUBLIC_ABI_STATUS.json).

Production/reference target partitioning and per-file use checks:
[BUILD_PARTITION.md](BUILD_PARTITION.md). Historical comparative verification
is [BUILD_PARTITION.json](../metadata/evidence/BUILD_PARTITION.json);
earlier reports retain their original historical results.

The native size collector (`scripts/collect_size_evidence.py`) resolves
Visual Studio object paths for the selected configuration only. Inactive
`ItemGroup` ancestors and `ClCompile` conditions cannot substitute Debug
objects into Release evidence. Existing `IntDir`/`ObjectFileName` resolution
and unknown-macro refusal remain in effect. Portable mapping fixtures do not
establish comparable same-build native DLL/MAP/OBJ measurements.

Current hosted acceptance belongs to the selected commit's GitHub Actions
jobs and artifacts, not a repository-wide PASS/NOT_RUN flag in an older
snapshot. PR creation checks lightweight integrity; the later full-validation
gate requires all eleven source/portable/Windows/reference contexts. See
[GITHUB_WORKFLOW.md](GITHUB_WORKFLOW.md) for scheduling and exact-head checks.
Merge records, configured tests and skipped jobs do not substitute for
executed native or Golden evidence. Owner game acceptance remains separate.

Opt-in [runtime load measurements](RUNTIME_BENCHMARKS.md) use the real TCP service
and selected production components with diagnostic-only worker candidates,
plus the actual hook-disabled Application/router over plaintext and negotiated
encrypted LSX. Application measurements require complete response validation,
actual handler-state lock observations and final XML/INI checks for persistence.
Correctness failures reject a sample; timings are observations, not merge gates
or original-DLL/Consumer acceptance. The manual performance workflow selects
this suite explicitly and leaves ordinary PR scheduling unchanged.

Runtime evidence also requires a post-link source/executable receipt and the
committed offline binary/raw-sample/JSON/CSV auditor. Its consistency PASS is
not execution attestation; native claims require the identified successful
Windows run and artifact digest. See the benchmark guide's question/evidence
matrix for interpretation, instrumentation and coverage boundaries.

## Registry allocation-failure assertions

The `portable_registry_rollback` direct-fabrication assertion disarms its allocation
probe before constructing any diagnostic. It records the operation's bad_alloc
and requires an observed injected fault, then checks caller status/output and
exact ownership rollback. A diagnostic allocation cannot satisfy the operation's
expected failure. This controlled-effects fixture is separate from native Registry
hook/SDK and Consumer execution.

## Accepted-client publication rollback

`socket_retirement_contracts` includes a controlled failure after an accepted
Client is constructed but before its client-list insertion. The private source
fixture selects a throwing list; ordinary builds retain std::vector. This is
a tagged pre-insertion failure, not evidence of real allocator exhaustion. The
fixture requires unpublished Client/Handler destruction without a session slot,
normal-close admission rearm, and exact retained native ownership when close
would block. Repair must drain cancellation and close the retained socket once.
The complete fixture remains a Windows socket/IOCP test; a POSIX projection,
cross-build or syntax check does not establish native Windows execution.

## Prepared-response Asio post ownership

The `post-owner` case in `net_failure_contracts` uses an unopened actual Client
and polls its executor on the controller. Accepted submission, one failed
allocation and a successful retry must preserve counted response bytes and
settle each controlled receipt once. Failed initiation must release the final
response owner and propagate bad_alloc without executing its callback. No
listener or worker is started, and explicit callback settlement is not socket
delivery. The native fixture retains its Windows allocator/IOCP coverage; a
POSIX projection or cross-build cannot establish that native execution.

## Raw Config2 selection-copy order

`portable_config_application` runs the existing configuration application fixture
through the actual raw-byte parser, selected subtree copy and section application.
The literal Content IDs a/i and b/j collide at eight buckets but differ at 64,
so exact resulting order detects an omitted selection copy even when scalar
values and counts agree. The override must retain that same order. Memory-backed
input, BOM, path/log and failure checks remain active. Only the Windows
NativeFactory generated-file tail is excluded in this portable target; the
ordinary `config_application_contracts` target retains it. Portable results do
not establish native filesystem, original-DLL or Consumer execution.

## Config2 lookup allocation rollback

The Config2 hash fixture measures a successful borrowed-key lookup, then faults
each observed throwing data allocation in a fresh eight-child owner. Failure
must remove the provisional child and orphan traversal key while retaining
existing child/nested-child addresses and recursive payloads. An independent
list-splice oracle checks traversal before and after published 8-to-64 rehash;
a late failure may retain the larger valid table. Retry uses an owned key,
never the potentially invalidated borrowed view. Allocation counts are measured,
not fixed across libraries. Checked MSVC iterator-proxy bookkeeping remains
enabled and explicitly unfaulted, as in the existing assignment campaigns.
The same fixture remains registered in portable and native hash tests.

## Prepared-response dispatch refusal

The ordered Session composition fixture refuses response dispatch before the
controlled IO queue accepts ownership, across plaintext/controlled-encrypted
and isolated/coalesced requests. The original exception must propagate without
output, receive rearm or buffered-successor admission. Encrypted preparation
must cancel its event reservation once; a separate Session sharing the queue
retries the exact event, retaining it through partial output and acknowledging
it only after full delivery. Actual Session, bridge, message core, XML reader
and EventDelivery participate; cipher, queue, scheduler and transport effects
are controlled. This complements actual Asio post-allocation coverage and does
not establish socket, worker-pool, OS allocation or Consumer execution.

## Prepared-response send timer faults

`timer_failure_contracts` retains #265's direct-send controls and the three
receive/callback fault runs. Additional prepared-response runs receive a complete
request over loopback TCP, drive its owned handler job on the controller, and
arm timer cancel/wait exceptions before IO dispatch starts the response. They
require silent peer closure, one completion-boundary diagnostic, exact receipt
cancellation/destruction and drained socket/timer/output ownership. An unarmed
control requires exact counted bytes, one commit and receive rearm. The seven
fault runs exercise five existing timer boundaries; they are not seven distinct
new fault capabilities. Timer exceptions, Handler and receipt are controlled;
Windows IOCP/MSVC, worker-pool scheduling and actual event storage remain
separate evidence from a POSIX projection or cross-build.

## Asio owned-thread launch rollback

`asio_thread_launch_contracts` selects a private launcher that throws before
the first/second handler worker or IO runner constructs its std::thread. Earlier
workers are real threads. Pre-cleanup snapshots must show they already returned
and were joined before the selected exception escapes, with no listener,
readiness or Handler publication. Explicit stop remains idempotent. A separate
fresh two-worker owner executes 32 jobs; failed owners are never restarted.
Ordinary builds keep OwnedThread exactly std::thread. This controlled constructor
fault and any POSIX projection are separate from an actual OS launch failure
and native Windows thread/socket execution.

## Portable ordered Session flow

`portable_session_flow` compiles the same `tests/native/session_flow_tests.cpp`
as the native Windows `session_flow_contracts` target, with the actual Session,
message/XML bridge and pinned XML backend. Portable checks therefore cover
ordered delivery, receipt/buffer lifetime, silent-command continuation, invalid
dispatch and settlement refusal without copying fixture cases. Both
registrations retain a 30-second budget. Scheduling, transport, cipher and
effects are controlled; this does not establish real sockets, AES or Windows
acceptance.

## Native file identity fallback

`file_copy_windows_integration` source-includes the Windows copy adapter and
intercepts only its two metadata calls. Controlled FileIdInfo errors 50/87 must
reach the real legacy API on owned handles; other errors must not fall back.
Independent Win32 snapshots and known SDK cells check volume width, high/low
DWORD order and eight zero tail bytes in a prefilled identity. Legacy errors,
hard-link identity, skip-existing content preservation and unarmed real Win32
copy/metadata cases remain covered. Controlled faults, cross-compilation and
Wine are distinct from fresh native MSVC execution.

## Profile cache failures and explicit cleanup

The shared online unit fixture injects offline avatar-query/profile-read and
authenticated avatar-query/profile-write exceptions. It checks completed state
prefixes, restored error mode, retained owner identities and no readiness or
implicit cleanup. Explicit cleanup retires each identity once. A fresh worker
invocation follows owner cleanup and an explicit online-byte reset, then checks
replacement identities, cache state and readiness ordering. These cases run in
both existing portable and full unit configurations. Providers and I/O are
controlled; throwing before the write marker does not prove disk atomicity,
real DLL ownership, thread publication or an automatic restart protocol.

## Prepared response owner allocation failure

The common Session-flow fixture arms its executable-only allocation probe after
response bytes and receipt preparation. It rejects the shared PreparedResponse
owner allocation before IO dispatch for isolated and coalesced frames, while
allowing smaller checked-STL bookkeeping allocations. It requires one failure,
one cancellation/destruction, no commit/output/dispatch/rearm, and preservation
of an unadmitted successor. Later cancellation cannot settle the receipt again.
The probe is normally disabled. Both existing portable and native registrations
retain their 30-second budgets; caller-side Asio closure, real event storage and
OS-wide memory exhaustion remain outside this controlled unit case.

## Checked writer error retention across close

The common binary-IO fixture has an opt-in controlled Output whose close clears
its status. A prior checked-write failure must remain a failure, with exact
counted bytes and balanced state/write/close/state/destruction ordering. A
repaired independent Output from the same factory must succeed. This interface
stimulus does not assert that native fstream clears failed-write status, recover
a physical handle, or exercise disk I/O. The recovered initial-state-only writer
and the unarmed native cases remain unchanged.

## Portable prepared MessageHandler contracts

`portable_session_handler` reuses the unchanged native MessageHandler fixture
with the actual bridge, message core, OwnedStorage event queue and pinned XML
reader/printer backend. It continuously checks binary FIFO/reservation state,
receipt cancellation/commit, exception unwind and boolean encryption failures.
It complements the accepted ordered Session-flow registration; overlapping
failure detection does not replace this focused bridge matrix. Both native and
portable handler targets retain a 60-second budget. Cipher, effects and mutex
adapters are controlled; these tests do not establish native sockets, AES or
physical persistence.

## Portable Windows lexical path contracts

`portable_path` reuses the unchanged native `path_contracts` fixture and actual
`src/core/paths.cpp`, retaining the native 60-second budget. It checks drive,
UNC and device roots, decomposition/joining, trailing separators, counted UTF-16
and bounded self-alias behavior. Existing host-hook and online consumers also
compile this source but do not exercise the entire lexical matrix. These are
controlled string contracts; portable execution does not establish host
filesystem compatibility or native Windows file access.

## Worker image-reference acquisition failure

`cleanup_retry_contracts` loads the existing fixture DLL with a scoped environment
selection that rejects its one address-based GetModuleHandleExW acquisition
before obtaining any image reference. The fixture source-includes the unchanged
production entry once and forwards unarmed calls to the SDK. Require worker
termination with the original error, zero Application construction/cleanup,
idempotent public shutdown, cleared caller exclusion and physical unload after
the sole caller reference is released. A healthy reload retains the original
three-attempt cleanup case and checks release of both caller and worker leases.
The Application is a controlled counter implementation with no sockets or hooks.
Cross-compilation and SDK effect models do not establish real Windows loader,
thread or physical-unload acceptance; those assertions require native execution.

## Native status sharing recovery and search ownership

`status_contracts` source-includes the unchanged Windows status adapter and
controls only the initial attribute error plus selected search/close failures.
For normal and read-only owned files, ERROR_SHARING_VIOLATION must fall back
through NativeStats/UsedStats to real FindFirstFileW and FindClose, retaining
permissions and closing each acquired search handle exactly once. Failed
searches preserve their error without a fabricated close; unrelated attribute
errors do not enter the fallback. Missing files retain their classification.
A reported-close-failure control closes the actual handle before reporting
failure, preserving the reference ignore-result policy without leaking a handle.
Unarmed queries still work after the scoped injection. These assertions require
native Windows execution; SDK models and cross-compilation neither establish
real loader/filesystem acceptance nor induce a natural OS sharing violation.

## Completed event receipts before locked settlement

The existing event queue fixture crosses successful/cancelled completion with
last/nonlast binary events against actual OwnedStorage. A completed callback
must still exclude ordinary pop until a locked queue operation settles its
receipt. Refusal preserves the exact binary front and closed receipt; cancelled
fronts receive distinct retry tokens and reject stale callbacks. Settlement
retires only the delivered front, preserves a following event and permits an
ordinary pop afterward. The four cases add 40 deterministic assertions while
retaining the allocation-before-persistence matrix and all competing-completion
coverage. Full fixture counts vary with valid concurrent winners. Host runs use
real completion threads and controlled persistence callbacks; they do not
establish backend file I/O, natural Session misuse or native Windows acceptance.

## Binary input exception unwinding and retry

The shared binary-I/O fixture controls failures at input opening, initial state,
seek-to-end, position, seek-to-beginning, read before/after copying and close.
Each failure must retain its type, source and operation, stop the call sequence
at that operation and destroy an acquired stream once without inventing an
explicit close during unwind. Clearing the fault and retrying through the same
factory must preserve embedded NUL bytes, the compatibility zero-filled tail
on short read, full operation order and a fresh balanced stream lifetime.
Both existing portable and native registrations reuse these cases; the checked
writer's error-retention control and all unarmed native file cases remain.
Faults target controlled Input implementations, not the native filesystem;
portable and sanitizer execution do not establish native Windows fault behavior.

## Portable encoding source contracts

`portable_encoding` compiles the actual codec implementation and existing
encoding fixture. A target-local definition excludes only its Windows include
and native test group; the default native registration still invokes both
controlled and real Win32 groups. The controlled fixture checks codepage
selection, conversion flags, replacement refusal, invalid-flags fallback,
query/fill counts and failure/length behavior without changing conversion policy.
The controlled and native function bodies remain shared and unchanged. The
portable registration retains a 30-second budget and explicitly reports that
Win32 was not executed. Host execution and sanitizer results do not establish
native adapter/Windows acceptance, locale behavior or complete donor equivalence.
