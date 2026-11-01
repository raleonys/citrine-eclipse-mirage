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
| CONSUMER_GAME | Actual named/versioned game and Consumer flow; currently NOT_RUN |

Run `python -B scripts/run_source_qa.py`. CTest infrastructure tests are
registered in both the portable configuration and the Windows test group.
The CRT probe checks the compiler CRT's heap, exception unwinding,
container/thread, and TLS-destructor behavior. It does not guarantee custom
CRT equivalence or a cross-DLL C++ ABI.

The PE checks adapted from Brownfield reject malformed optional headers, raw
extents/overlap, resource recursion/fanout, ordinal-only exports, prohibited
CRT imports, checksum errors, and SecurityCookie defects. Additional tests
reject incorrect/private manifests, source drift, duplicate or unsafe ZIP
paths, nested archives, symlinks, write failures, unowned metadata, missing
command arguments/exit codes, and premature Golden termination.

Current VDF implementation/compiler results are in
[VDF_PARSER_STATUS.json](../metadata/evidence/VDF_PARSER_STATUS.json);
the adaptation scope is recorded in
[BROWNFIELD_TRANSFER_MATRIX_V2.json](../metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json).
Raw results of earlier infrastructure/source tests are retained; they do not
establish the same claims as the current reports.

Current runtime integration:
[RUNTIME_BINDING.md](RUNTIME_BINDING.md). The earlier vendor/parser test
counts here are historical snapshot evidence. New application tests are
recorded in [RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json).

Current application C interface and separation of source DLL variants:
[PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md).
Current validation:
[PUBLIC_ABI_STATUS.json](../metadata/evidence/PUBLIC_ABI_STATUS.json).

Production/reference target partitioning and per-file use checks:
[BUILD_PARTITION.md](BUILD_PARTITION.md). The latest comparative verification
is [BUILD_PARTITION.json](../metadata/evidence/BUILD_PARTITION.json);
earlier reports retain their original historical results.
