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
