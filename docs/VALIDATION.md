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
