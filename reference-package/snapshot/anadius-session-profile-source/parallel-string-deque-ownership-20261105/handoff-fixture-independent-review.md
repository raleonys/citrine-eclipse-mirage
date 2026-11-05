# Independent handoff fixture source review

2026-11-05. Read-only manual review of the separate test author's frozen `outputs/anadius-worker-handoff-source/handoff_tests.cpp`, SHA256 `714af17f024cd4143874e969e2f27bf75e6d00142adbc3584758ae26cf8ee94d`, against this stage's actual before copy and the selected typed production APIs. This reviewer did not author that test file. No compiler, test process, native DLL, linker or package action ran.

**Result: no blocking source/API/fixture-flow finding in the two new cases.** This is not a claim that they compile or pass. Existing main scenarios/three retained children remain registered; main now also calls both `parallel_owned_deques()` and `parallel_string_transfer()`.

## Parallel owned queues

The fixture uses the existing actual Domain/NativeThreads/PreparedMemory machinery and two real Parent-owned session controls. Root seeds the two maps through recovered026670 and ends that seeding Operation before offering either child the queue. There is no lingering Root map-view pin incorrectly used as an idle/wait condition.

The exact A-thread128-byte heap pause occurs inside its new map allocation, after the physical allocation is ledger-tracked and outside the Ledger mutex. It holds A's own recovered growth prefix; B's heap calls are not paused. B first attempts A's queue and checks the intended nonpoisoning same-queue reservation rejection, then grows and disposes its distinct actual queue/control while A remains held. Its expected map/reserved/cumulative counters agree with A still having its old map plus its private unpublished allocation, and B having finished its own map free.

B's actual native join and Parent zero-timeout reap happen before A is released. Original close while A remains active checks false/Ready without poisoning. After A resumes, its exact permit still checks, both final growth counters are four, A's final map is physically disposed and all actual jobs/backing/FLS ownership drain through the established fixture. Different subobject accesses and event/join synchronization avoid treating the no-op QueueGate as proof of arbitrary concurrent service-list mutation.

## Large string/header ownership transfer

Root constructs actual aligned request and transfer strings with shared stateless Concurrent Strings, validates the exact header resolver, and keeps its whole Operation alive while B later mutates/frees the transferred allocation. All three legacy observation APIs are required to throw; the actual Parent buffer count remains two, so no fake empty-header drain is asserted.

Root relinquishes only the transfer allocation's own view before child eligibility. An independent real SDK range lease remains, and B's first free must reject while that lease is live. Root actually resets that lease and signals B; only then may B assign/release the transferred string and physically free its old Root-produced allocation, while Root's Operation and A's unrelated request view remain active. This directly exercises last-use handoff instead of merely waiting for Root Operation exit.

A prunes without scanning/pinning unrelated headers, reads only its actual immutable request and keeps that view until its own source use ends. B performs recovered009eb0 copy into an independent aligned response. B relinquishing its own request view cannot consume Root/A views; the protected free still rejects. B's response Operation ends before its actual JobMemory return, while Root remains active; original then ends its Operation, joins/reaps B and verifies A alone still protects the request.

Original's fresh scope uses a resolver-backed FrameReadContext with no header union. Actual020cb0 moves the child-produced response into an OwnedIo response task; invoke_task(flag0) runs recovered022280 discard. Its TransferApi checks the exact raw base/extent, verifies physical free and permits neither a stale header span nor a duplicate free. The IO envelope is also actually freed through the no-TLS-state source path. The remaining Parent buffer is A's different request, which A rereads after that cleanup before exit. Final request release and Parent/Prepared close follow real actor and native/FLS exit order.

The test intentionally uses null task control and flag0. It covers aligned response ownership/header cleanup, not the full session strong-owner response-write pipeline or Graph/NativeJobs end-to-end progress. It does not claim all three changed socket destroy consumers execute here; root's separate socket source cases cover those independent helper paths.

## Manual API/dependency checks

TransferEffects implements the selected five IoCompletionEffects methods, and TransferApi implements FrameReadApi plus FrameReadTerminal. FrameReadContext and PostedEnvironment aggregate prefixes match the appended resolver fields; OwnedResponseContext receives the real IO response memory and actual header owner. The nullable response-dispatch context is valid because flag0 never selects invocation. Existing common source inputs include `anadius-session-profile-source/strings.cpp`, and the handoff target is already linked to the shared core source archive in both CMakeLists.txt and build.ps1; no new source dependency was silently assumed validated by an old archive.

Event owners outlive launched tasks on failures, waits are finite, ledger locks are not held across the controlled pauses, and joins precede destroying shared fixture state. Payload prefixes shared across threads are immutable or transferred before eligibility; event/return/join order precedes reuse. Actual thread/PTD provenance is supplied by NativeThreads and explicit Parent JobMemory entry, not manually forged TLS/work flags.

The cases remain source-only prerequisites. They do not prove compiled behavior, native ABI/EH equivalence, arbitrary shared payload writes, foreign provider thread safety, or the final same-session/distinct-session modal/GetConfig application trace. No production policy was enabled by reviewing them.
