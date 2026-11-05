# Independent handler admission/gate review

**Confirmed source behavior:** the actual selected NativeJobs path serializes whole message bodies across all sessions of one server, and also prevents the original IOCP actor from entering Parent while a child body runs. This is not explained by a different Parent per session. The bounded native fork/receiver comparison belongs to the sibling audit; this note makes no new REA claim or production change.

## Actual selected ownership and held gates

DLL entry.cpp:90 explicitly selects NativeJobs through resources factory.cpp:115–122; this is not the default InlineJobs compatibility path. OwnedServer Impl creates one SessionMemoryDomain (server.cpp:152/157), and ServerView returns that same instance to the shared Application/Graph and every session slot. Each accepted session has separate owner/cipher/Logic, not a new Parent.

| Layer | Existing source scope and consequence |
|---|---|
|NativeJobs, jobs.cpp:154–156|One source_gate spans every child's acquire → process_message → return_memory, and is shared with outside-Parent physical reaping. A second detached physical child waits before entering its body, regardless of session.|
|Parent, memory.cpp:183–203|Operation's unique_lock retains the recursive gate until scope destruction. Impl stores a single active/thread/fiber/PTD/job_scope/job_seal set. Profile.cpp:702 retains that Operation throughout native processing, strings and opaque effects. It is not merely an allocation-metadata lock.|
|Profile, profile.cpp:703/711/717|Global active_calls must become exactly one for the child. Slot.processing adds a per-session retained-processor guard. Relaxing Parent alone would expose rejection/poison at the global Profile check.|
|Graph, graph.cpp:28–45/373–410|One weak JobCall, active/router_active/in_opaque/job_binding, shared writer inventory and per-call lock bookkeeping are installed for the entire child scope. A second distinct session also rejects this singleton scope.|
|Logic/providers|NativeProfile Invocation is per session; two distinct sessions avoid that particular reservation. Content, Information and Licensing still have one JobCall/busy/checking/binding reservation each. Those are additional HOST context assumptions on broader parallel routes, not evidence of a native server-wide strand.|

PreparedMemory itself already has separate JobState actor/PTD/scope records and only brief metadata locks for scope pinning; its per-job pins are distinct from Parent's singleton current-actor metadata. This does not prove the shared allocation/string/IO tables are ready for unlocked access.

AdmittedDispatcher::get releases Parent around the physical GQCS wait, then re-enters Parent before returning to dispatch. While a child holds Parent, OS networking/packet delivery may progress and physical child threads may exist, but original accept/receive/completion dispatch or a new frame fork cannot pass that re-entry. Detached siblings also remain behind NativeJobs.source_gate. Startup Online on its separate facade is a different actor/path and must not be conflated with child-vs-child progress.

## Strong bounded native comparison

Use two already-established, warmed sessions after startup, plaintext, show_lsx=false. A structurally valid LSX request containing an unsupported command takes the **implemented, nonfatal UnknownMessage** path, not a forged pointer/SDK call. Existing FH4 proof binds033d70's catch to07bd70; its prefix and counted request flushes finish before `07be45` invokes038bc0. The wrapper calls MessageBoxW with null owner and flags0x10. A remains in its detached handler until that ordinary modal dialog is dismissed. B sends registered GetConfig02e5f0, which builds literal XML without achievement/event/licensing/clock calls. No shared-log lease spans A's dialog, and B's chosen path needs no such lease.

Retained frame code025480 starts05e8e0, detaches04a458, then rearms0280d0 without joining the message body. Retained021770 invokes its bound function;028d30 calls033d70 at028d99 **before** response environment scheduling at028e7d. Thus response scheduling/its executor flags cannot retroactively serialize entry to the blocking handler. The sibling audit is closing exact fresh receiver/fork details; arbitrary same-session cipher concurrency and full native concurrency ABI are not inferred.

The relevant observable trace is whether B can be dispatched/respond before A's user dialog returns. Current source gates forbid that trace across distinct sessions. The weaker Grant/XML-write versus GetUTCTime timing example is unnecessary once this explicit dialog path is retained. No artificial foreign self-wait or throughput benchmark is needed.

## Boundaries and evidence

No unlock-only remedy is proposed: it would leave single-current-actor permits, Profile/Graph reservations and shared table lifetimes incoherent. Native event/achievement mutexes and per-session state are separate from the reconstructed server-wide guards and must remain distinguished. No new readiness or unload condition follows from this review.

Retained assembly EIDs:025480 `ev_17a121443001ec46ed018ca1fad07bc2cfd74c2e067c1f3ccf0bb7bd84b95c31`;021770 `ev_267ad2fba0da2ea44cbb89f4ef891512c4b721af9c7a6c38b4be9571c4a1bc19`;028d30 `ev_65681baf7e383e89ff3fa9f7f1032ce0865024b0d48e0ad7b76997e04088bc66`;GetConfig `ev_73b6f6da8bb1c590a9fdda96a768e029bbfa302ce7c4ad86a699f43c5e189a8f`. Unknown-message exact EH/funclet evidence is indexed in `outputs/anadius-unknown-message-source/native-diagnostic.json` and `outputs/anadius-no-response-source/native-catch.json`; the full retained records and wrapper are preserved there. Current source/proof hashes accompany this note. No build/test/link, original access, execution, status or package edit was performed.
