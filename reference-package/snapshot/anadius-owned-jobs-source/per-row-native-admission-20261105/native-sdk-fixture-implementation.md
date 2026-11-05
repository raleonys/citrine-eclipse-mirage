# Actual NativeJobs SDK source fixture

Status: source implementation and static review only. No compilation, project
test execution, link, reconstructed DLL execution, or ZIP update was performed.

Owned changed source: `outputs/anadius-application-graph-source/graph_integration_tests.cpp`.
Final SHA-256: `0d65bfb6d588b571eb4bdb62c8bda3fd0ec31c2eabbd588bbcdb19c96c4c9f23`.
Recorded-stage before SHA-256: `b80e0d82a7ee1c2a09f2ecd588148f7891d04936b9b7dd0f702ed757e1088354`.
Paused partial draft SHA-256: `566def697caa323a7556c193816d6fd594011d003e0ea27ac41d81a153f98985`.

## Authored source scenarios

- `--prepared-native-parallel-same` selects one real NativeJobs owner, one real
  Backend Parent explicitly configured with OriginalConcurrentActors, and two
  actual native prepared jobs on the SAME Session/Logic.
- `--prepared-native-parallel-distinct` uses the same composition, grows the
  initial-one session/Logic and native job registries, and selects distinct
  original-owned Session/Logic rows for A and B. The synthetic B SDK receive
  remains pending until A has entered its supported Unknown diagnostic; this
  controls request arrival, never source child eligibility.
- The existing controlled `--prepared-profile-parallel` constructor body is
  unchanged from the recorded before source. Its fixture-only returned event
  and ParallelJobs implementation remain separate from the new actual owner.
  The two new modes are also added to the existing isolated subprocess driver.

`NativeJobsObservation` only captures the actual Application/Ports during bind
and forwards every Jobs/PreparedJobs action to ONE NativeJobs instance. It has
no row registry, work ownership, body policy, fake memory offers, or alternate
business roots. NativeJobs owns preparation, native launch, detach, scheduler
pin, receipt, physical polls and handle retirement. Concrete Graph/Profile,
Policy, Domain/PreparedMemory/PTD/FLS, native wrapper and real kernel IOCP are
used. Sockets and peer input are synthetic SDK boundaries already used by this
source harness; scripted cancellation995 is not a real network cancellation.

The source schedule checks these distinct boundaries:

1. The DuplicateHandle observer holds A's lower start return until A enters the
   supported UnknownParallel Dialog callback. A is therefore executing before
   start returns and before its detach. No NativeJobs event is used.
2. While A remains held, B executes actual GetConfig with its separate long
   request id. The real native post enqueues its actual IOCP receipt, signals
   the test observer, and holds only its SDK return. B's DuplicateHandle return
   waits for that publication, proving B's body and memory return precede its
   own start return/detach.
3. Runtime consumes B's real receipt while B's post callback is still held.
   Outside Parent, the driver calls actual NativeJobs quiescent maintenance and
   requires Bound/pending2/completed1, one live source A, no private handle
   close, and no detach-event API. This covers receipt before notified/post
   return without treating receipt as physical/FLS exit. Only then does the
   driver release the held post return.
4. B's ordinary original-worker response and unchanged exact live A Session
   identity are observed while A is held. The driver's independent B proof
   handle must actually signal with exit code0; actual NativeJobs maintenance
   then leaves exactly A's physical row/PreparedMemory offer. The B private
   handle closes once while A's remains live. Root metadata and distinct-session
   registry growth are checked on the same actual composition.
5. Only after B response plus actual native/FLS/Parent reaping does the driver
   release A. It observes A's real native exit outside Parent for deterministic
   final test teardown. Both real receipts and scheduler pins drain exactly
   once; Graph/Profile/Policy/NativeJobs close through normal source ownership.

The test owns handshake events as observations. NativeJobApi create_event and
set_event are trapping callbacks: any production NativeJobs call is terminally
rejected. The old normal70, delayed-exit, growth, late-close, SessionIo, concrete
Worker and retained SDK/error source assertions now require ZERO event calls
and ZERO event-set calls. They still require the actual source original-handle
detach, one private-handle close per physically retired row, real thread/FLS
proof, and exact source job/work counts. No fake barrier close is counted.
Those old modes retain the actual default Serialized Parent policy: the whole
original Parent Operation covers start and detach, so their post-denial
started1/detached1 ordering remains bounded by that selected existing policy,
not by an invented NativeJobs barrier. The two new modes explicitly opt in to
Concurrent policy to exercise the earlier execution schedule.

The separate Policy probe remains real: two prepared child offers, one held
current-thread cleanup A, independently completed cleanup/native exit B, and
one ordinary original cleanup produce3. The two actual frame child cleanups
and two original response-tail cleanups add4, so final expected cleanup count
is7. There is no counter reset or synthetic ownership drain. Returned Policy
offers are reset only after actual NativeJobs/Parent maintenance outside Parent.

The new modes use a separate ParallelExecutionPlatform with actual Win32 SRW
operations on Execution's own word and atomic observation counters. Existing
controlled/sequential ExecutionPlatform behavior is preserved.

## Native evidence and limits

This fixture follows this stage's saved REA/Ghidra evidence: `0x18004a458`
detaches through `0x18007f0a8` CloseHandle with no event/wait; `0x18005e8e0`
forwards flags to `0x18007f468` CreateThread and returns its native handle/id.
The saved caller evidence separately places flags0/launch before detach.
The complete inline replies and exported four-record bundle are in this stage.

No authored assertion has been executed. This report does not claim a compiler
pass, runtime pass, full native equivalence, production Concurrent opt-in, native
exception equivalence, or native full-child028d30 response/021770-tail ordering.
The actual current reconstructed continuation still runs on the original
worker. Generic tracked Environment/finish_environment_work overlap and other
mutable providers remain separate open fidelity tasks.
