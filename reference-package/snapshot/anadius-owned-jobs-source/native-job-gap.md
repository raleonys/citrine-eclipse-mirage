# Native message-job path and physical join correction

Current checkpoint: **source-only; no compilation, tests, link or binary
execution**. The user requires an explicit rebuild instruction first. Prior
build receipts and ZIP refer to their frozen source snapshot.

## Retained native evidence

The historical REA/Ghidra listing identifies anadius64.dll SHA-256
`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
No original file was opened or executed in this step. REA binary_session was
read only: no active target; REA 6.5.0, Ghidra 12.1.4 available. Availability
does not mean a new binary analysis or fresh native observation occurred.

The retained full assembly/pseudocode listing hashes still match the historical
frame-read and thread evidence. Those hashes name the complete listings, not
the extracted frame-read files. Current extracted files are separately hashed
in join-exception.evidence.json.

| Native address | Observed operation | Retained evidence |
| --- | --- | --- |
| 0x180025526 / 0x18002552d | Put message function token 0x180028d30 into closure +0x28 | ev_17a121443001ec46ed018ca1fad07bc2cfd74c2e067c1f3ccf0bb7bd84b95c31 |
| 0x18002554b | Select message thread entry 0x180021770 | same assembly evidence |
| 0x180025556 | Call 0x18005e8e0, then check returned handle and ID | same assembly evidence |
| 0x18002557f | Call detach 0x18004a458 | same assembly evidence |
| 0x18004a458 | Close original thread handle; does not join | ev_218bf3eda319b5a7858921c077bd0fd00cadc3c2d550d8952be3eef945886923 |
| 0x18005e8e0 | Existing recovered begin-thread source, lower CreateThread boundary | ev_b2508bf2d6430c3f029f1f6090cadaa12d2315a853312d93ad48f42602adfb0e |

Sources: ../anadius-socket-source/frame-read-evidence.asm.txt and
frame-read-evidence.json, ../anadius-thread-source/evidence.json,
../anadius-source/bootstrap.cpp and ../anadius-thread-source/threads.cpp.
These are static observations; no original scheduling/interleaving was observed.

## Actual source gap

The existing Profile defaults to InlineJobs, which executes the runner before
start returns on its bound worker/fiber. Explicit providers may defer on that
same worker. Neither is a separate native message thread.

Replacing only Jobs::start with CreateThread would fail existing admission:

1. session_profile::Application::Impl::metadata checks the bound worker/fiber;
   run_job enters that path through effect().
2. application_graph::Impl::affinity and session_policy::Impl::affinity require
   the original graph/policy worker. Their mutable state was designed under
   that serialization policy.
3. PreparedMemory::Borrow::memory checks its admitted worker. A new native
   thread having a valid Domain PTD alone does not give it this capability.
4. The whole Parent Operation must cover source access; source submission must
   not join another worker while holding that Operation. Worker-return and
   external-quiescence paths also require current jobs/closures to retire.

No thread/fiber check was removed or replaced with an always-true proof.
InlineJobs remains the selected default. Native message-worker integration is
still OPEN; current graph tests are not native detached-thread equivalence.

The next implementation needs an explicit admitted message-worker capability
and exact closure/session ownership, then matching Graph/Policy serialization,
actual message entry/thread-exit cleanup, and observed OS-thread/FLS completion
before provider/image retirement. Each must preserve source ordering and avoid
waiting on a worker that needs the caller's held Parent Operation. Merely adding
another stop receipt or launching a thread does not establish those contracts.

Subsequent source checkpoint: `../anadius-worker-handoff-source/job-memory.md`
now implements the narrow move-only backing-memory offer, exact sibling-worker
admission and actual child-thread exit retention. It is unbuilt/unexecuted and
does not yet authorize a SessionMemoryDomain Parent Operation, Profile, Graph
or Policy call by itself. The subsequent unbuilt checkpoint
`../anadius-session-memory-source/job-parent.md` adds the pinned explicit Parent
Operation; Profile/Graph/Policy admission remains the next source dependency.

## Implemented correction in physical thread tracking

TrackingThreads already preserves a separate duplicated join handle when the
source closes its original detached handle. Its stop_and_join formerly unwound
after a throwing wait/close boundary without marking the result unknown. A
subsequent call could retry the same numeric handle. If an injected close had
already taken effect before throwing, that number could be reused.

The wait/close pair now catches exceptions, sets sticky poison under its metadata
mutex, and rethrows. The existing Drain guard clears only the active reservation.
The unknown current record and suffix remain retained; already observed and
successfully closed prefix records stay committed. Later calls return false
before any SDK wait or close. Poisoned destruction retains the existing terminal
contract; no speculative physical close or raw-job drain success is fabricated.

Known WAIT_TIMEOUT/WAIT_FAILED and close FALSE outcomes retain their prior retry
behavior. The control caller's incoming LastError remains preserved, including
exceptions. Real SDK functions follow their nonthrowing outcome contract; this
change handles the injectable boundary's unknown outcome. No class layout,
virtual slot or export changed.

Four isolated test modes were added **as unexecuted source**: wait throw, close
throw, and each throw after a successful first record. They describe retained
suffix/committed prefix, LastError, no SDK replay and terminal destruction.
Existing normal failure/timeout retry scenarios remain. These sources still
need compilation and behavior validation when the user requests a rebuild.
