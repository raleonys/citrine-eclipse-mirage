# Native socket ownership and I/O retirement

A shutdown owner was added to address the Application-owned socket
gap confirmed in the preceding review. A lost record after an Asio
close failure no longer silently becomes success. Safe retry is
distinguished from uncertain results; pending I/O and buffer lifetime
are part of the shutdown acceptance criteria. Donor retirement
remains **BLOCKED**.

## Native-result policy

Microsoft explicitly states that a handle remains valid on
`WSAEWOULDBLOCK`. The same documentation also notes that handle
values can be reused and that a close return does not necessarily
mean all pending I/O completions have finished.
[closesocket contract](https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-closesocket).

| Result | Implemented ownership behavior |
|---|---|
| Successful close | Release the native record; retain the session until associated callbacks finish. |
| Would-block | Retain the exact raw handle. Before retry, apply the Application's default linger-off policy; accept successful native close only once. |
| Other close failure | Record the numeric value as uncertain. Do not perform blind native close, socket queries or reuse. Block successful cleanup. |
| Linger-option failure during retry | Do not call close. Preserve retained ownership; move to uncertain on descriptor/not-initialized errors. |

Uncertain state is not cleared automatically. If it cannot be
proven that a record still identifies the same kernel socket,
closing it again is unsafe. The DLL worker therefore retains
the same Application/I/O owner and image lease and returns
shutdown cleanup failure. A later request reports that state
again without touching the numeric handle. No force-clear CFG
setting or Production export was added. This is not a claim
that every native failure is automatically recoverable.
Retained retry relies on the Application having sole ownership
of the socket and on balanced Winsock-scope lifetime.

## Ownership scope

First-party retained wrappers cover Client sockets, IPv4/IPv6
acceptors, bind candidates and temporary sockets transferred
to accepted callbacks. If a wrapper would be destroyed after
a failed native close, its record is moved to a preallocated
deferred list. Client-constructor and handler failures also
preserve ownership of native resources. The first deferred
failure stops admission; a failed bind cleanup cannot be left
behind while the next port is opened. The normal occupied-port
scan remains 3226–3236.

Deferred capacity is reserved before socket acquisition as the
policy session cap + 8. Published clients/listeners can have
at most two additional accept callbacks and two bind candidates.
Acquisition stops after the first failed retirement. Associated
I/O and deferred records are destroyed only after their callback
targets. The retirement path does not depend on fresh heap
allocation for this list.

The Client list now holds strong ownership; a Client only
borrows its Impl, avoiding an ownership cycle. A session is
not removed from capacity before native close, timer and
read/write callbacks finish. Counts are rolled back on
initiation exceptions and decremented once on every
completion exit path. Completed clients are pruned during
subsequent admission.

Shutdown retains the ordering: the I/O runner closes sockets,
then runner join, active-handler drain and I/O-context drain.
The final I/O drain uses a 100 ms attempt; on timeout a
retryable owner is retained. Success jointly requires the
context to be naturally stopped, all accept callbacks completed,
native/deferred records empty, and clients retired.
The interval does not provide a global shutdown-time bound
that preempts user/provider code or any individual callback.

The existing response-completion closure already owns its
PreparedResponse buffer; that behavior is preserved.
Cancellation produces a negative receipt, and a buffer
still in use by the kernel remains alive through completion.
No positive delivery acknowledgement was added for
incomplete or cancelled sends.

This change does not modify Asio vendor bytes, its private
service implementation or header macros in Production.
It uses the public Asio API and does not introduce a mandatory
`release()` operation that would raise the Windows 7 target.
The scope is sockets acquired or received by first-party
code. It does not claim to close every conceivable OS failure
path within the vendor's own temporary objects.

## Test evidence

`socket_retirement_contracts` contains nine scenarios with
real IOCP sockets/timers and the Production implementation:
retained retry, option failure, uncertain open/closed results,
write-buffer/receipt-completion lifetime, timer retirement,
Client-constructor rollback, bind rollback, and listener/
accept retirement. Native fault injection is restricted to
specified sockets in the test executable. Numeric reuse is
not reported as observed unless it actually occurs.
The central oracle is that uncertain values never receive
another native operation and unrelated sockets remain alive.

`socket_dll_retirement_contracts` runs the real DLL worker,
Application and Asio service in three isolated child processes.
Hooks are disabled in the fixture Options; this is not a hook-
patching test. Would-block retry shuts down the same Application,
does not repeat native close after success, and releases the
image lease. The two remaining children produce controlled
results in which close was not performed, or close completed
but reported an error. In both cases the worker lease prevents
premature unload even if the caller invokes FreeLibrary.
Only test-process exit/OS reclamation releases the quarantined
owner; no test-only unquarantine API was added to Production.
The parent removes dedicated fixture directories after the
children have terminated.

Using the same final harness sources and the same old core
archive, the exact preceding and corrected sources were built.
All 12 contract scenarios FAILED before and PASSED after.
[Before/after records](../metadata/evidence/socket-retirement/PINNED_RESULTS.json)
and build receipts bind source, harness and core-archive hashes.
These are controlled Win32 boundary failures during tests,
not evidence of real-world Windows failure frequency or
native MSVC acceptance.

Complete local, Debug and source-QA results are in
[VALIDATION.json](../metadata/evidence/socket-retirement/VALIDATION.json).
Public header declarations and Production C ABI are unchanged:
DllMain@1, SURGE@2, DllShutdown@3.
Compiler-generated/helper symbol differences in the
development DLL are recorded separately in
[SOURCE_API_DELTA.json](../metadata/evidence/socket-retirement/SOURCE_API_DELTA.json);
private template emission is not the fixed C ABI.

Native Windows/Golden and game acceptance must be rerun
against the modified source. Registry provenance,
aggregate memory budgets, provider/handler liveness
and review of remaining donor source/test/tool families
are still open. The manual donor branch ledger covers
76 files; this is not closure of all donor semantics.

Exact parent source ZIP SHA-256:
`e0b6964082c4343359d824684b5c60e667867db54f089796862080a81f73d8ac`.
Changed-file pins for this source are in
[REVIEWED_CHANGES.json](../metadata/evidence/socket-retirement/REVIEWED_CHANGES.json).
