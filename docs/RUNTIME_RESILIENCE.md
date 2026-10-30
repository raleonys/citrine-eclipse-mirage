> Subsequent owner adoption extends the transport described here: full-send FIFO,
> asynchronous greeting, event delivery reservations, deadlines and byte/session
> budgets now follow [SESSION_TRANSPORT.md](SESSION_TRANSPORT.md). Its current
> contract supersedes the earlier unchanged-dequeue/output-budget statements below.

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
IO paths (profile, ticket, avatar and configuration) retain their existing behavior.

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
plus a real destination handle denying replacement. Real-socket worker tests cover
pipelined order, exception isolation, queue saturation, live IO under occupied
workers, and cancellation/drain on stop. LLVM-MinGW/Wine evidence is separate from
MSVC Golden and the owner's game acceptance. See the current evidence report.
