# Bounded asynchronous session transport

The owner explicitly adopts the additional Brownfield transport improvements:
full-delivery request ordering, event delivery reservations, asynchronous greetings,
socket deadlines, and connection/frame/output limits. These are scoped adaptations
to the existing source architecture, not wholesale donor application imports.

## Active defaults

| Policy | Application default |
| --- | --- |
| Simultaneously admitted sessions | 128 |
| Incoming wire frame, including terminating NUL | 1 MiB |
| Combined outgoing response/event, or greeting | 8 MiB |
| Receive idle | 30 seconds |
| Started-frame deadline | 10 seconds |
| Send idle | 5 seconds |
| Owned handler workers / pending jobs | 2 / 128, retained from worker-pool adoption |

The supplied reference uses a 4096-byte receive buffer. Its literal size is not
copied: the existing NUL-delimited/encrypted protocol and larger consumer responses
are preserved. These bounds are explicit product policy, not measured maximum
consumer payloads. Windows/MSVC and game compatibility must still be validated.

`SessionPolicy` supports explicit C++ source construction for isolated hosts and
tests. Application uses its declared defaults. No CFG, environment or provider
input changes these values. Listener range 3226-3236 and fixed SDK identity remain
separate policies. There are no added production exports or vendored source edits.

## One request through full delivery

The native session runs at most one handler at a time and does not admit the next
frame until the current combined response/event has completed all socket writes.
Buffered pipelined input remains in order. The native output queue therefore has
at most one active response owner. Empty responses/frames resume asynchronously;
zero-progress or impossible write counts fail the session rather than acknowledging
an incomplete output. The pure source Scheduling defaults preserve their original
launch/greeting behavior for isolated RE fixtures.

A shared prepared-response owner carries wire bytes and a delivery receipt from
the worker to the IO executor and through every partial-write callback. Cancellation
resolves the receipt but retains the buffer until the outstanding operation's
completion object is destroyed. A closed session never resumes buffered input.

## Event reservation and retry

The real native MessageHandler reserves the front plaintext event under the event
mutex while preparing encrypted output. That front cannot be duplicated by another
session. Preparing or initiating a write does not pop it. Full combined delivery
atomically marks the reservation committed. Partial writes, socket errors,
encryption failure, oversize output, cancellation and dropped prepared responses
release it for a later request instead of consuming it.

Commit/retry completion is a lock-free atomic state transition. Physical unlink of
a committed front happens during the next locked queue access. This prevents an IO
completion from waiting behind achievement disk persistence holding the event mutex.
Queue producers continue to append normally; completion never touches deque storage
without its existing mutex. Reservation tokens reject stale/double completion.

A full socket write is local TCP acceptance, not an application acknowledgement.
A peer can receive bytes before a later error becomes visible; retry is therefore
not an exactly-once delivery guarantee. This change does not roll back an already
persisted GrantAchievement when its response cannot be delivered. Persistence and
network delivery are separate transactions. The initial DOWN seed is preserved.

## Asynchronous greeting and deadlines

The greeting uses the same owned asynchronous write pipeline as responses. A peer
that does not read cannot occupy the IO thread in a synchronous retry loop.

Deadlines use steady_clock. Delivered bytes refresh receive idle; the first fragment
starts a fixed frame deadline that later fragments cannot extend. Only positive
socket-write progress refreshes send idle. Handler/queue processing pauses receive
timing. Generation-tagged timer callbacks cannot close a session using a cancelled
older deadline. Completion callbacks also check expiry before accepting late progress.
Stopping clears timers and cancels sockets; all cancellation callbacks and workers
are drained before the service and queue owners can be released.

## Admission and size handling

At the session cap the listener stops arming accepts; a retiring session resumes
admission. The OS backlog is bounded separately through the listen request. The cap
counts admitted live sockets, not connections waiting in the OS backlog. Both
loopback listeners share it: already pending accepts are checked again before
constructing a handler, so simultaneous family completions cannot exceed it. Closed
sessions with still-running handlers may remain owned until those handlers finish;
the worker/queue bounds remain in force.

Reads allocate no more than the remaining frame budget, capped at the existing
64 KiB read quantum. A terminating NUL at the exact limit is valid; a full buffer
without it fails the requesting session. Pipelined frames survive bounded reads.
Oversize output is rejected before socket submission, and its receipt is cancelled.
A response is generated before its size is checked; the wire limit does not claim
to preclude every transient allocation made by a handler or XML printer. Internal
application maps and the producer event queue are not given new capacity policies.

## Evidence

`session_flow_contracts` controls partial writes, response/event completion,
cancellation buffer lifetime, zero progress, exact/oversize frames and monotonic
policy boundaries. Event integration tests use the actual message core and owned
queue to prove reservation exclusion, retry and encryption-failure retention.
`network_policy_contracts` uses real sockets for admission/re-admission, limits,
receive/frame/send expiry, stale timer cancellation, slow-greeting isolation,
backpressure and stop with armed timers. The earlier pool, atomic persistence,
hook, SDK/config, export and DLL lifecycle tests remain part of validation.

The current report is metadata/evidence/SESSION_TRANSPORT.json. Cross-build/Wine PASS
is distinct from native MSVC Golden and a new game run. No dist promotion or Library
synchronization is part of this work.
