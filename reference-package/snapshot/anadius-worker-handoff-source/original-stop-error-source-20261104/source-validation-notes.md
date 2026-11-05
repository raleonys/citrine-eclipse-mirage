# Original serving Runtime stop failure: source validation

Only `outputs/anadius-worker-handoff-source/handoff_integration_tests.cpp` was edited in this stage. **No compilation or test execution occurred.** No compiler, linker, reconstructed/original DLL, ZIP update or REA session was invoked. This is an authored integration regression against current source; earlier retained native stop/formatter evidence is supplementary, not a fresh native claim or a passing runtime result.

## Actual seam verified before editing

- `anadius-socket-source/completion_dispatcher.cpp:81` calls `api.set_last_error(0)` before its `api.get(...)`.
- Runtime's local Admitted wrapper forwards set_last_error through Runtime metadata checkpoints. This wrapper does not acquire the session Policy's ordinary Call reservation.
- `anadius-owned-server-source/server.cpp:46` requires the existing admitted Parent Operation, clears its saved physical-wait status, then calls the raw lower set_last_error. The later get implementation releases the Operation before its physical wait. Thus the raw setter is inside whole Parent admission and before release.
- Router completes the actual callback and calls application.after_completion before dispatch returns. The dispatcher then performs its ordinary packet work decrement before the next set_last_error. The selected trigger therefore runs after the final ordinary Policy callback has returned and its active reservation is clear.
- Runtime EnvironmentWork selects the current admitted target for a serving original actor; the retained raw child/returned-cleanup path is not selected. The real cap itself validates Runtime source/phase, original actor and exact scheduler. The fixture additionally observes backend Running; no new Runtime phase getter is introduced.

## Selected mode and natural last release

`--original-stop-failure` creates a fresh actual NativeDomain, controller-prepared ApplicationMemory and original NativeThreads worker using the existing fixture. It creates no secondary child. During the first actual prepare_peer callback it captures the real Ports/capability and adds exactly one explicitly fixture-owned work pin. No normal packet counter or stop/wake flag is overwritten. The ordinary Graph/Profile/Runtime pipeline processes its existing two peers and all59 packets: three accept, twenty receive (eleven zero-sized), eighteen posted and eighteen send completions. Every normal packet still decrements through the actual dispatcher. The retained pin prevents original native automatic stop, leaving work1 after the final accept callback and packet decrement.

Only raw Dispatcher.set_last_error(0), after shutdown_done, triggers the test. It requires the original actor, actual Parent Operation, backend Running, real Policy Bound/activefalse, session/job0, IO Open/pending0, Accept pending0/not poisoned, work1, native stop/wake0, exact59 counts and empty source fallback queue. It does not demand IO allocations0: legitimate native TLS caches are retained until worker exit, which this deliberate failure mode does not claim to reach.

The same current admitted Operation is used; the fixture does not enter a substitute Parent scope or invoke a lifecycle callback from inside another Policy call. The captured actual EnvironmentWorkStop.finish_original consumes the one pin. Its actual atomic decrement reaches old1 and invokes native02a190 through Policy and Runtime.

## Saved-error and owned diagnostic proof in the authored regression

1. The raw lower Dispatcher post intercepts exactly one original-actor zero-byte/key/null-operation stop packet for its own IOCP. It sets SDK LastError77 and returns false without queuing a wake. All other modes retain the original real PostQueuedCompletionStatus behavior.
2. The raw get_last_error hook captures the SDK value before performing its own test metadata. It requires77 and no prior post-return clobber, and records exactly one physical read.
3. Runtime's existing StopApi must latch that result before its borrowed post-return checkpoint. The supplied checkpoint checks the actual actor and existing Parent and then deliberately sets LastError0x1234 on every invocation after the failed post. The fixture records that clobber rather than inferring it from ambient LastError after the throw. Policy's whole-call PreserveError may restore an earlier ambient value during unwind.
4. The actual failure must throw a plain std::system_error with code77, the module native category selected by the immutable injected FormatMessageA/LocalFree pair, and exact `pqcs: native original stop ` text. Static callback functions use storage inside the still-live retained probe. They require exact native020950 flags1300/language400, requested code77, initially-null pointer, counted CRLF mutations and one Format/Free pair. Free overwrites the entire SDK buffer; the caught what() must already own its text.
5. After the catch inside the raw SDK setter, the test requires consumed=true, work0, stop/wake1, one failed post on the original actor, one raw77 read, at least one recorded1234 clobber, once-formatted owned text and Policy Poisoned/activefalse. Parent remains admitted. It calls ExitProcess137 without returning from the raw setter, so the outer Runtime checkpoint cannot mask the intended error-prefix observation with a later poisoned-phase rejection.

This is deliberately a process-terminal failure prefix. It does not pretend that failed posting caused normal worker return, native FLS exit, TLS cache drain, application teardown or provider/domain close. Existing ordinary and successful prepared-child modes retain their complete prior teardown assertions. The pin provenance is a fixture-owned work pin consumed through the actual capability, not an end-to-end OwnedEnvironment-row destruction claim.

## Future default harness

The no-argument main retains the ordinary scenario followed by the actual prepared-child last-release scenario, then launches its own executable with `--original-stop-failure` using CREATE_NO_WINDOW. The parent waits at most40 seconds, requires process code137 and closes its own joined process/thread handles. The isolated child has the existing thirty-second original-worker barrier timeout if the trigger is never reached. This registration is authored source only; no child process was launched during this stage.

## Before snapshots and frozen candidate

Normal copies were taken before editing. The pending metadata contained55 changed sources and remains historical evidence; it was not rewritten by this author.

| File | SHA256 |
| --- | --- |
| `before/handoff_integration_tests.cpp.before.txt` | `677b7f63d1e8031455492237cb9e33864bc1085842003ca4f60812fc7eac5756` |
| `before/pending-source-status.before.json` | `3ed70255838c173e94b840e04d5b25fff7a58bac7b2ccaf06ab3b4e4ef6c3fcb` |
| current `handoff_integration_tests.cpp` | `d2eb5c962e85876bc0551974a684797546c1a901e30ec689c1cb8c9f02ab4f5f` |

Static diff/whitespace review found no whitespace defect. Independent fixture review is pending at this exact candidate hash. An explicit user rebuild instruction remains required before compilation or execution validation.
