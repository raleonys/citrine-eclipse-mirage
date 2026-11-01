# Source cleanup and measured optimizations

This document records historical measurements from the first
optimization pass. The current 60/78 Production/reference split
and subsequent narrow cleanup/TLS work are documented in
[BUILD_CLEANUP.md](BUILD_CLEANUP.md). Earlier test counts,
translation-unit totals, and export counts below are not
the current totals.

This review covered all 138 first-party CPP files and
139 headers observed in compiler dependencies. The split
remained 64 Production and 74 retained reference CPP files.
No CPP file or general API function was removed.
Unused private storage and an unreachable branch were
removed; XML processing and Release diagnostic calls
were improved.

The previously owner-accepted DLL SHA-256 was:
`4a0f37c9328ecf31d0815a9e9b85634bc481e77e2f07b636d4ea65bb4783f26b`.
Its Windows Native 193/193 PASS and owner-reported game
acceptance remain historical evidence for that DLL.
The source changes below had not received a new native
Windows/Golden or game rerun at this checkpoint.
Local results were LLVM-MinGW/Wine and Linux-sanitizer
evidence, not MSVC or game-performance measurements.

## Review and decisions

Using the actual Windows x64 compile database, all 138 CPP
files were analyzed with Clang-Tidy 21.1.6 and Clang AST;
there were no compiler errors. Selected checks covered
analyzer core/cplusplus/deadcode, bugprone, performance,
misc-unused, and redundant groups. Vendor-header warnings
were not counted as first-party findings and no vendor
source was patched.

A total of 1,779 explicit AST definition records were
extracted. This includes template instances and constructor
records; it does not mean 1,779 independent logical functions.
No direct DeclRefExpr/MemberExpr reference was observed for
969 records. This does not mean "969 dead functions":
virtual dispatch, Windows callbacks, constructors/destructors,
CRT/TLS startup, and development-DLL export roots cannot be
fully proven by that graph. Of the records, 738 were
virtual/override. All 3,347 development-DLL export names
were preserved before and after this checkpoint.

All 53 distinct first-party Clang-Tidy diagnostics were
recorded with individual dispositions. For example,
applying a `c_str()` suggestion in `messages.cpp` could
change the first-NUL boundary; Asio `error_code`
overloads cannot be labeled "unchecked errors" simply
by inspecting return values. No automatic fixes were
applied. A separate follow-up concerned the possibility
that calling `stop()` from the Asio destructor could
throw during exceptional allocation/join failures.
No native fault injection covered that path; the
warning was neither declared resolved nor safety-proven.
Exceptions were not swallowed in ways that risked live
thread ownership.

Per-file old/current hashes and scope:
[FILE_MATRIX.csv](../metadata/evidence/optimization/FILE_MATRIX.csv).
Function records and retention decisions:
[FUNCTION_MATRIX.csv](../metadata/evidence/optimization/FUNCTION_MATRIX.csv).
All diagnostics and individual decisions:
[STATIC_DIAGNOSTICS.csv](../metadata/evidence/optimization/STATIC_DIAGNOSTICS.csv).
Compiler scope and boundaries:
[COMPILER_SCOPE.json](../metadata/evidence/optimization/COMPILER_SCOPE.json).

## Changes implemented

| Change | Rationale and preserved behavior |
|---|---|
| Remove staging string before XML parse | TinyXML2 already copies the owned terminated buffer. Preserve the first-NUL boundary of a counted view, empty input, truncated views, complete ErrorStr, and document ownership. |
| Store the XML Element wrapper in the map node | Remove a second heap allocation per node. `unordered_map` rehash preserves node addresses; the vendor document is destroyed after its wrappers. |
| Inline Debug gate for internal diagnostic calls | With Golden LTO disabled, empty Release log bodies in other translation units did not remove the caller's strings. Release calls are now eliminated within their own translation unit, while existing out-of-line development API logging symbols remain. |
| Clean up private Environment fields | Remove the unused `xml_` reference. `log_mutex_`, used only in Debug, is absent from the Release class. Retain the XML constructor input for actual Storage use. Retain `clock_`/`formatter_` because they are used. |
| Remove zero receive-size branch | After the prior `size>=maximum` guard, maximum-size, 65,536, and `max(spare,512)` are positive; their minimum cannot be zero. Preserve helper API and full-buffer behavior. |

Hooks, shutdown/rollback ownership, FIFO/event commits,
atomic file writes, fixed SDK/port settings, four immutable
vendors, PNG/resource/PE metadata policy, and the three
C exports/ordinals were unchanged. Compiler/linker
settings remained unchanged, including the existing
`/LTCG:OFF` Golden contract. Public C++ source API
signatures were preserved, but internal Environment
layout changed; old C++ OBJ files must not be reused
without recompiling the sources.

## Measurements and regressions

A standalone probe compared four variants using identical
LLVM-MinGW Release settings and the same non-XML core
library: old code, staging-buffer change only, wrapper
change only, and both together. Each variant ran five
rounds in rotated order, with three scenarios per round
and 500 operations per scenario. The Production allocator
was not modified; the allocation counter existed only
in the separate test executable.

| Scenario | Allocations/operation, before → after | Requested heap bytes/operation, before → after |
|---|---:|---:|
| Small LSX request | 13 → 9 | 9482 → 9370 |
| 64 KiB text | 8 → 6 | 140200 → 74648 |
| 256 child elements | 539 → 281 | 83808 → 79184 |

Allocation counts and requested bytes exclude allocator
metadata. The p50 medians improved, but p95/p99 did not
improve in every scenario; Wine timings varied. For
example, the p95 median for the many-child scenario
increased from 255,900 to 291,900 ns. These measurements
do not establish native speed or queue-latency gains.
All variants and samples are retained in
[XML_BENCHMARKS.json](../metadata/evidence/optimization/XML_BENCHMARKS.json).

The new `xml_reader_allocation_contracts` test fails on
the older code due to a second input-sized allocation
and passes on the new code. It also verifies that
cache lookup performs no new allocation and that the
wrapper can be retried after an allocation failure
without losing the document. No brittle time-based
test threshold was added.

The persistent `xml_reader_corpus_contracts` test stores
the same 12,087 inputs and the old code's complete
expected outputs under `tests/fixtures/xml_reader`,
rechecking them at every Native Acceptance run.
XML unit tests also cover counted/unterminated spans,
subsequent modification of the caller buffer, and
rehashing/pointer identity with 1,025 children.
Server tests verify receive-size positivity and
maximum boundaries, including SIZE_MAX.

Old and new XML readers were additionally compared
over 12,087 normal, malformed, truncated, NUL-bearing,
UTF-8/random-byte, and depth-boundary inputs using
Linux ASan+UBSan. Error codes/flags, full ErrorStr,
and all element/attribute/text outputs matched
byte-for-byte; no sanitizer errors were reported.
This finite corpus does not prove whole-program
equivalence or game acceptance. Results and corpus/
output hashes are in
[XML_CORPUS_PARITY.json](../metadata/evidence/optimization/XML_CORPUS_PARITY.json).

At this checkpoint, Release 196/196, affected Debug
21/21, and source QA 17/17 groups passed. The XML
unit test passed 649 checks and the persistent corpus
passed 12,087 inputs under ASan+UBSan. After clean
ZIP extraction, a Production build with tests and
development DLL disabled passed alongside three
actual load/start/profile/stop/unload cycles.
The same LLVM Release output shrank from 1,081,856
to 1,077,760 bytes, a reduction of 4 KiB.

Current build/test results, actual before/after DLL
section sizes, and pending native tasks are in
[VALIDATION.json](../metadata/evidence/optimization/VALIDATION.json).
The reduction in the LLVM output must not be
extrapolated from the owner's previous MSVC DLL;
different toolchains cannot establish a comparative
improvement.

## Collecting actual Windows size evidence

After a normal Production Build, from the project root:

```powershell
python -B scripts/collect_size_evidence.py --build-dir build/golden_msvc --dll dist/preloader_l.dll --output build/qa/native-size-evidence.zip
```

If `dumpbin` is available on PATH within a Visual
Studio Developer environment, add `--with-symbols`.
The tool verifies the current source manifest;
it does not execute the DLL, modify compilation/link
options, or promote/publish files from dist.
It does not overwrite an existing report.
It creates a ZIP and a companion SHA-256 file.
Unknown/LTCG object formats produce PARTIAL,
not a misleading complete evidence claim.

The tool collects source contracts, final/raw DLL
sections/exports, the Production MAP, and sections
from ordinary/BigObj COFF OBJs in the current
CMake target inputs. Stale objects on disk are
excluded if they do not belong to the current
graph. Fourteen infrastructure tests check these
limits, Visual Studio object-path resolution,
malformed COFF, and overwrite prevention.
The actual LLVM Production graph successfully
read 145 current OBJs.

The total OBJ bytes are not their final-DLL
contribution: the linker may discard or merge
unused/identical COMDATs. A MAP public-symbol
range is also not an exact function size.
A proper before/after native investigation
requires MAP/OBJ/DLL evidence from two versions
rebuilt under the same MSVC/SDK/Release settings.
The earlier accepted native DLL and logs exist;
its matching original native OBJ/MAP records
were not provided. The new collector enables
gathering such missing evidence for future builds.

## Acceptance criteria for subsequent optimizations

Before deleting any candidate, evaluate its
actual Production link contribution, development
DLL/test/virtual/callback/lifetime roots, and
error paths. Measure changes independently and
run controlled regression against prior behavior,
affected Debug and full Release tests, source QA,
clean Production build, and native/game acceptance.
Do not silently change vendor, ABI, or product
policy to reduce size. This review does not
guarantee that no other bugs or dead code remain.
