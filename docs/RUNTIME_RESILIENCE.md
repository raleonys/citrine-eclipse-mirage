> Subsequent owner adoption extends the transport described here: full-send FIFO,
> asynchronous greeting, event delivery reservations, deadlines and byte/session
> budgets now follow [SESSION_TRANSPORT.md](SESSION_TRANSPORT.md). Its current
> contract supersedes the earlier unchanged-dequeue/output-budget statements below.
> Current IPv4/IPv6 listeners are in [AUTOMATIC_LISTENER.md](AUTOMATIC_LISTENER.md);
> the earlier connect-only change below did not itself add that listener.

# Runtime resilience

The owner explicitly adopts three scoped durability changes after reviewing
Brownfield/Bluefield comparisons. Application protocol logic remains based on the
supplied RE. These are deliberate exceptions to source identity, not claims that
the historical implementations already behaved this way.

## Connect destination and caller storage

Only IPv4 `127.0.0.1:3216` and IPv6 `[::1]:3216` wait for readiness and redirect
the port to the selected listener. An already selected port, any other port,
other 127/8 addresses, mapped IPv6 addresses and remote IPs pass through without
startup waits, attempt increments or redirect logging. A zero selected port does
not produce an invalid rewrite. A complete stack copy preserves IP, flow and
scope bytes; the caller's const sockaddr is never modified. The native callback
and original-call boundary now use read-only spans. The attempt counter's narrow
synchronization, terminal attempt behavior, binding leases and Winsock LastError
contract are preserved. No IPv6 listener is added by this change.

## Achievement persistence

`PersistenceEffects::persist_achievement` serializes XML and INI before entering a
single checked native batch. Startup creation of achievements.ini and the explicit
achievement XML/INI writers also use checked replacement. Other recovered binary
IO paths (profile, avatar and configuration) retain their existing behavior.
License-ticket adapters now use checked write/close status as documented in
[WINDOWS_IO_FIX.md](WINDOWS_IO_FIX.md); they are not atomic replacement writes.

Each destination is staged under an exclusively created same-directory name.
Write counts, FlushFileBuffers and close are checked before replacement. Existing
files are copied to exclusive recovery names without removing the live originals.
MoveFileExW performs each replacement. If a later replacement fails, already
replaced destinations are restored in reverse order, including original absence.
If rollback fails, exact backup ownership remains in the native factory; later
updates must recover before staging anything new. Application stop also retries
recovery and can retain the Application through the existing retryable DLL stop.
Successful commit tolerates temporary-file deletion failure and retries cleanup
later rather than reporting a false failure after state has committed.

The achievement mutex protects the tentative record. Failure restores its prior
value, or erases a newly inserted record. A completion event is allocated under
the event mutex before file commit, remains unpublished during persistence, and
is published without allocation only after success. Failure discards the event.
The existing initial DOWN seed remains an independent initialization event. This
change does not add socket-delivery acknowledgement or change later event dequeue
behavior; those are separate policies.

The `event_queue_contracts` fixture faults actual deque growth in initially empty
and seed-retained `OwnedStorage` queues. A thread-local allocation probe requires
failure before a counter-only persistence callback, unchanged input/publication,
exact prior FIFO bytes and one successful unarmed retry. Successful probes remain
queued so storage must grow instead of recycling an empty spare block. This checks
producer allocation ordering; the controlled callback performs no disk IO or
GrantAchievement, and host allocation results do not establish the MSVC checked-STL
route or general operating-system memory exhaustion.

The `event_queue_integration_contracts` seed regression measures eligible
container/string allocations during a successful getter, then faults each observed
site in a fresh `OwnedStorage` owner. A controlled Initialization provider opts into
abort/reset; failure must leave no engaged partial storage, cleanup registration
or completed guard. The same getter then retries the exact DOWN seed once, and a
hot empty queue is not reseeded. The probe allows smaller checked-STL bookkeeping
allocations and does not model native CRT guard/atexit locking, every allocator
failure or restart of single-use Application startup.

Individual replacements are atomic. XML/INI together are a process-local rollback
transaction, not a power-loss atomic filesystem transaction. Concurrent external
writers and automatic recovery after process termination are not supported. Files
left after an interrupted process are preserved, not scanned or deleted on startup.
New native write/flush/replace imports are intentional implementation dependencies;
PE import-module order and the three-entry export contract remain unchanged.

## Handler scheduling

Two owned worker threads replace per-message thread creation. The shared admission
queue holds at most 128 pending jobs; excess work closes the requesting session
through the existing adapter error path without blocking the IO thread. These
internal constants are not configuration overrides or copied message-size limits.

The native Scheduling adapter opts into ordered processing. A session pauses input
processing after one frame until its handler has posted the response back to the IO
executor. This preserves cipher/request order across pipelined messages and keeps
at most one handler job per session. Other sessions can run on the second worker.
Empty frames resume reading; a handler exception closes only its session. Output
writes still use the original socket queue and partial-write continuation; this
change does not impose a total frame size or output-byte budget.

Stop first prevents new work and cancels sockets, discards queued jobs, then joins
running handlers before releasing the IO work guard and draining callbacks. No job
or callback may outlive its retained session or service. A running handler is not
forcibly terminated; stop waits for it to return. The pure source Scheduling default
retains the original launch boundary for RE fixtures and explicit source users.

## Validation boundary

Native callback tests exercise read-only Windows memory and both address families.
Atomic persistence tests inject staging, replacement, rollback and cleanup failures,
plus a real destination handle denying replacement. The same controlled
`AtomicFiles` recovery matrix runs as `portable_atomic_recovery` and Windows
`atomic_recovery_matrix_contracts`, each with a 30-second budget. Its 64 combinations
vary original presence and independent rollback failures for three changed
destinations before a fourth replacement fails. Assertions cover exact binary/empty
bytes and backups, reverse-order recovery after errors, first-error preservation,
remaining-owner retries, refusal to stage until recovery, idempotent cleanup and
a subsequent successful group. Its in-memory filesystem does not establish native
filesystem semantics or power-loss atomicity.

Real-socket worker tests cover
pipelined order, exception isolation, queue saturation, live IO under occupied
workers, and cancellation/drain on stop. LLVM-MinGW/Wine evidence is separate from
MSVC Golden and the owner's game acceptance. See the current evidence report.
