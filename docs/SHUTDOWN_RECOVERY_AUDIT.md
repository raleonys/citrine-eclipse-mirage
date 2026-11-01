# Hook shutdown, timer containment and socket-close review

**Donor retirement remains BLOCKED.** This follow-up review
contains one verified Production correction, three persistent
test executables and an ownership gap reproduced with
Brownfield's own Asio snapshot. The socket-close finding
was unresolved at this historical checkpoint. The earlier
[DONOR_RECOVERY.md](DONOR_RECOVERY.md) retains its
historical scope.

## Applied hook-cleanup correction

`Application::Impl::stop` previously retried failed
hook quiescence indefinitely inside its own
`while`/Sleep loop. If MinHook disable or release
failed persistently, the worker could not leave
this call and reach the DLL worker's existing
`ERROR_BUSY` and retry-on-next-shutdown protocol.
Although a `DllShutdown(timeout)` caller could
time out, internal cleanup continued retrying.

Each stop request now makes one quiescence attempt.
If incomplete, an exception reaches the DLL worker
while preserving the same Application/Hooks/Binding
owner. The worker records cleanup failure and waits
for another stop request. A completed disable is not
repeated; drain or engine release resumes where it
stopped. The owner is destroyed only after the
binding and trampoline ownership are released.
The three C exports, ordinals and existing shutdown
codes 0–4 remain unchanged. The former pending-timeout
result may be replaced by worker-reported
`DLL_SHUTDOWN_CLEANUP_FAILED`; no new status code
was introduced.

The 100 ms value is only the budget for the
existing hook-drain attempt. This change does
not provide a total shutdown guarantee that
preempts native APIs, provider/handler joins
or all Application shutdown work. Caller timeout
and ownership-preservation contracts remain.
The external audit's wider shutdown-liveness
question was not declared closed.

## Before/after evidence on the actual ownership chain

`application_shutdown_failure_contracts` runs the
actual `dll_entry.cpp` worker, `DllShutdown`,
Application, hook transaction and binding.
The fixture wrapper supplies only isolated Options
and counters; lifecycle operations go to the real
Application. MinHook API and external-thread scan
boundaries are controlled, so this fixture does
not perform actual hook patching or thread suspension.
The native-call drain scenario holds an actual
callback `Lease` open; its count is not fabricated.

| Failure | Previous source | Expected and observed in corrected source |
|---|---|---|
| Hook disable fails | Internal retry loop never exits. | Cleanup failure after one attempt; retain the same binding/owner. |
| Real in-flight Lease remains | Internal drain retries continue. | Keep Lease and engine alive; finish on a later request after Lease release. |
| Engine uninitialize fails | Internal cleanup loop repeats. | Retry only release without repeating a previously successful disable. |
| External unload scan rejects | Internal quiescence loop continues. | Preserve owner; close the same owner on a later request when the scan permits. |

The same final test sources were used for all four
scenarios: FAIL with the preceding source and PASS
with the corrected source. Failure fixtures also
clear the injected fault and shut down the DLL
worker; no blind FreeLibrary is performed on a
live trampoline/owner. The preceding source was
built from a manifest-verified clean extraction
of the delivered archive, not from a modified
working tree used as a baseline. Build receipts
bind harness and Application hashes and the
same old core-archive hash for both variants.
[PINNED_RESULTS.json](../metadata/evidence/shutdown/PINNED_RESULTS.json).

## Timer hypothesis: tested callers fail closed

`refresh_deadline` updates the cached deadline
before initiating a new wait. Examining that
method alone suggests potentially stale cache
state after an exception; the effect on a live
session depends on how its caller handles it.

The new `timer_failure_contracts` runs three
paths through real Asio socket/IOCP timers
and Production Client callbacks: wait-initiation
failure during receive, exception at the
cancellation boundary during receive, and
reschedule failure inside a timer callback.
In all three cases the session closes, the
cached deadline is cleared and the peer
sees closure. The preceding source also
passes the tests. No unnecessary vendor
or timer runtime-policy change was made.

Fault injection is limited to the timer
boundary in the test executable. Successful
operations use the real Asio timer; controlled
exceptions occur before the operation.
This does not measure real-Windows `cancel()`
failure frequency or prove coverage of every
partial OS failure. For example, Asio's
completed-queue fallback following IOCP
completion-post failure is not automatically
a C++ cancellation exception.

## Socket close: an open issue not resolved by donor transfer

On Windows IOCP, `win_iocp_socket_service_base::close`
returns `socket_ops::close` while setting the
implementation socket to **invalid_socket
even on error**. Therefore, `is_open()`
and another Asio `close()` cannot alone
prove native-handle retirement. A second
Asio call may perform no Win32 close attempt.

`socket_close_semantics_contracts` injects a
controlled native-close error without closing
the socket at a test-only Win32 boundary.
The subsequent Asio code and socket are real.
Four scenarios cover normal close, successful
internal retry after initial would-block,
twice-withheld would-block and withheld
network-down. In the last two, the raw
socket remains valid, Asio `is_open()` is
false, and the second Asio close makes no
new native attempt. The test ultimately
closes the descriptor itself. A PASS in
this characterization test does not mean
the product recovered from the failure.
Real-Windows occurrence rates for these
controlled combinations were not measured.

Brownfield's `AsioListenerOwner::apply_stop`
returns on close failure; however, a subsequent
check or cancelled-accept completion may see the
Asio record as closed and advance to reporting
quiescence. A probe with actual donor source
and the donor archive's own Asio headers showed:

```text
raw_socket_alive=1 listening=1 quiesced=1 failed=1
```

Donor and current vendor snapshots both report
1.38.2 but are not byte-identical. This review
therefore did not rely on the initial probe
against the current vendor: the probe was
rebuilt with the donor's extracted include
tree and reproduced the result.
[Donor build identity](../metadata/evidence/shutdown/DONOR_CLOSE_BUILD.json),
[result](../metadata/evidence/shutdown/ACTUAL_DONOR_CLOSE_RESULT.log),
[snapshot comparison](../metadata/evidence/shutdown/ASIO_CLOSE_COMPARISON.json),
[probe source](../metadata/evidence/shutdown/listener_close_probe.cpp.txt).

Brownfield's separate raw `SocketHandle`
adapter preserves its own numeric owner
when close fails. `test_socket_close_retry`
exercises that adapter with fake `SocketApi`,
not Asio close semantics. The results of
these different architectures cannot be
substituted for one another. Likewise,
`test_shipping_shutdown_contract` is an
enum static_assert test, not real worker/
cleanup fault injection.

Native-close error ownership in the product
was **open at this historical checkpoint**.
Blindly closing a raw numeric handle after
Asio has removed its record does not resolve
handle reuse or pending I/O/callback ownership.
Retry design must address native descriptor
state, cancellation completions, buffer
lifetime and DLL unload permissions together.
Vendor bytes were unchanged, and the donor
socket runtime was not imported wholesale.
This issue continued to block donor retirement
at the time; later source changes are covered
by the separate socket-retirement report.

## Scope, verification and remaining work

Nine additional donor files received selected-
branch comparisons, bringing manual records
to 76 files; 1,729 first-party files still
lacked manual review records. This does not
establish complete semantic closure for
all 76 files.
[Ledger](../metadata/evidence/shutdown/MANUAL_REVIEW_LEDGER.json),
[all open files](../metadata/evidence/shutdown/ALL_DONOR_FILES.csv).

Local Release 201/201 and affected Debug
13/13 passed. The three Production C exports
and their ordinals were unchanged; the
development DLL export names were 3,350
before and after, with no additions/removals.
Public headers, four vendors, PE/resource/
avatar, CFG/SDK/port and Golden linker
policy were unchanged. The partition
remained 64 Production and 74 retained
reference CPP files. Final source QA
and package validation are in
[VALIDATION.json](../metadata/evidence/shutdown/VALIDATION.json).

These are LLVM-MinGW/Wine-class results,
not real MSVC/Windows IOCP failures,
Golden or game acceptance. Native Windows/
Golden and game reruns remained pending.
Outside socket-close ownership, registry
provenance, aggregate memory budgets,
provider/handler shutdown liveness,
environment/console rollback, and review
of remaining source/test/tool families
were still open. Donors were not
declared obsolete.

Exact parent source ZIP SHA-256:
`ef4572d32076e0ace03039b17e0fb6103b084a395ccdd27fd59b76886f2d9d1c`.
Reviewed file differences are recorded in
[REVIEWED_CHANGES.json](../metadata/evidence/shutdown/REVIEWED_CHANGES.json);
the previously accepted binary/log package
cannot replace this parent.
