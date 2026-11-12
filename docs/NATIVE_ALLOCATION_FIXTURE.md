> **Historical checkpoint.** Use [VALIDATION.md](VALIDATION.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

# Native fault injection: Asio allocation path

In the Windows/MSVC logs supplied by the owner, Compile Only,
Production/Golden, Debug, and Static CRT profiles passed.
Native Acceptance was 204/205; only `net_failure_contracts` stopped
at the first scenario with
`accept initiation allocation failure was not injected`.
Static CRT passed 6/6. These logs do not report game acceptance.
There were no warnings in Production profiles; twelve warnings in
the native test compilation came from test fixtures and are not
claimed resolved by this narrow correction.

Original inputs and summary are retained in
[OWNER_LOG_REVIEW.json](../metadata/evidence/native-allocation/OWNER_LOG_REVIEW.json).
The final DLL SHA-256 printed by the Production log is
`5d75523d82a575277e1c5e91395637d301f0f981913c8358deee02c3c3cc24fc`.
The DLL itself was not provided with those logs; its hash was read
from the log. These are owner-environment results for the earlier
source snapshot, not native acceptance of the revised test.

## Cause and correction

The earlier test used a thread-local flag to force failures only
in global `operator new` / `new[]`. Pinned Asio
`detail/memory.hpp` uses `operator new` on MinGW but calls
`_aligned_malloc` on MSVC; Asio converts its null result to
`bad_alloc`. The test flag was never consulted on that path, so
accept initiation succeeded normally and the test reported that
no injection occurred. This is not evidence of an actual accept
failure or a stale-pending flag.

The revised test intercepts `_aligned_malloc` only inside its own
executable and only while processing Asio's memory-helper header.
Under fault injection it returns null, leaving the real Asio helper
to throw `bad_alloc`. Successful calls reach the original CRT allocator,
with corresponding aligned free preserved. Global scalar-new fault
injection is also retained.

The helper's standard/vendor-dependent headers are processed before
the macro is introduced, so the mapping cannot leak into STL/CRT
header bodies. A preprocessed helper or conflicting CRT macro mapping
causes a compile-time error instead of silently proceeding.
Vendor files, product `src/include`, Production compiler options,
listener, handler, rollback, and shutdown code remain unchanged.

The test asserts both which allocator path was consumed and that a
complete injection occurred. The post-failure scenario also verifies
the injection count; the stop-OOM scenario checks that neither of
two tracked allocation paths was consumed on the calling thread.
This is not a global proof of no allocation across all
`malloc`/`HeapAlloc` calls or all threads. An unsupported new
`std`/Boost aligned branch causes compile-time failure rather
than a misleading green result.

## Cross-platform regression

The normal `net_failure_contracts` target uses the native CRT
aligned path on MSVC and the ordinary compiler path on MinGW.
An additional `net_failure_msvc_memory_replay_contracts` test was
added on MinGW. It selects the MSVC allocation branch only while
processing `memory.hpp` and then removes the temporary macro.
All other Asio platform/compiler detection and actual Windows
IOCP/socket/timer operations stay unchanged. This is not MSVC
compiler emulation or native MSVC acceptance; it replays the pinned
helper's allocation path with a real Windows CRT.

In isolation, the previous test reproduced the same error on
that path. With the correction, accept, post, stop-OOM,
I/O-exception, and drain-retry passed on both ordinary and aligned
paths. Product `queue_accept` catch/reset behavior was not
changed. Assertions were not removed; no tests were skipped,
deadlines/budgets expanded, or Production Asio flags changed.

MSVC Native Acceptance retains 205 tests; the MinGW cross-suite
has 206 with the additional replay. Their separate counts and
execution classes are recorded in
[VALIDATION.json](../metadata/evidence/native-allocation/VALIDATION.json).
The previous Windows 204/205 result is not relabeled as a passing
205/205 result; native Windows rerun remains required.

The current contract is
[NATIVE_FIXTURE_REVIEW.json](../metadata/NATIVE_FIXTURE_REVIEW.json).
The external record
`surge-native-fixture-delivery-verification.json` covers build,
QA, and three DLL cycles from a cleanly extracted source package.
There is no new claim about game acceptance or the cause of
a VirusTotal label.

Local validation passed Release 206/206, affected Debug 9/9,
and source QA 17/17; vendor regressions passed 19/19 with no
skipped tests. Removing the pending-flag reset in an external
negative-control copy reproduced the expected stale-pending
error on both allocator paths. The 3,096 product/vendor/resource/
export files remained byte-identical to the parent package;
all 3,346 development-DLL API names were preserved.
