# Prepared real child last-release regression source

Edited only `outputs/anadius-worker-handoff-source/handoff_integration_tests.cpp`. This stage is **source validation authored and independently reviewed, not compiled or executed**. No compiler, project test, linker, reconstructed/original DLL, native target, ZIP update or REA session was run. Prior tracked-executor native evidence is supplementary; this is not a new native-evidence stage.

## Entry points and preserved baseline

The no-argument test main now runs the ordinary scenario and then the new child-last-release scenario, each in a fresh NativeDomain. Focused `--child-last-release` runs only the new scenario. The ordinary two-peer/59-packet Graph/Runtime/Policy/NativeLogic/XML/Router/AES/FIFO assertions remain unchanged. Existing teardown clears `running` and native heap observation, releases Graph/state/provider lifetimes, closes the Domain and checks heap/FLS balance before a subsequent scenario begins.

One controller-created NativeThreads provider is retained as a shared owner for original and probe child, not constructed on the source worker. There is no NativeJobs backend or extra concurrent maintenance owner in this fixture. The test reuses its real prepared installation through `Graph::install_server_prepared` and its existing native procedure/FLS/handle join pattern.

## Source timing of the actual last release

1. The first `prepare_peer` callback, already under the real original Parent Operation, captures actual `Application::ports()` and its Runtime EnvironmentWorkStop capability, prepares one real JobMemory offer and adds **one explicitly fixture-owned work pin**. The pin is consumed only by the child capability call. No production metadata/debug setter is added.
2. A real same-Domain NativeThreads child starts, proves distinct native actor/PTD/FLS/context and acquires its exact offer. It signals ready without querying or entering Parent, then waits on its release event. The original waits for ready before continuing ordinary source processing, serializing child startup allocator effects against that source continuation.
3. Normal Graph processing runs all59 real operation packets: three accepts,20 receives (including11 zero-sized),18 posted responses and18 sends. The final-owner0 path handles terminal203/995 only after session/job drain. `after_completion` sets shutdown_done; the dispatcher's subsequent decrement leaves exactly the one probe pin.
4. Only the next lower `Dispatcher::get`, after its existing short independent Parent Operation has ended, considers releasing the child. It asserts live sessions/jobs0, open base IO/pending0, AcceptEx pending0/not poisoned, exact packet counters, work1, native stop/wake0 and empty scheduler fallback queue. A zero-timeout **owned** IOCP probe must see WAIT_TIMEOUT before signaling, so it cannot consume the child's future wake. The fixture restores its own probe's LastError effects.
5. Total allocation count is deliberately not required to be0 at this point: legitimate IO/posted/handler TLS caches remain until the native worker loop exits. They are neither cleared nor reclassified by the fixture. The existing final worker-return allocations0 assertion still checks their normal cleanup.
6. After release, the child enters `Parent(offer)`, obtains the real live `operation.job_permit()`, checks the same actual Runtime cap and calls `finish_job`. It asserts **old work count1**, stop/wake still0 and no IO/AcceptEx loans before this call. It then asserts consumed=true, work0, stop/wake1 and exactly one real zero-packet post from its own native actor through Runtime's retained raw Dispatcher import. There is no original pre-stop or manual flag reset.
7. The child exits its Operation/permit scope before `offer.retire()`, then returns71 through the actual native wrapper. The original remains outside Parent while joining the child and its FLS exit, calls `parent.reap_jobs()` there and expects exactly one, then closes its own thread/event handles and releases captured offer/Ports/provider references.
8. The ordinary GQCS then consumes the actual child zero wake. It is not counted as another operation packet; native dispatch returns0 and leaves the usual one relay for the existing external drain hook. Thus the original handled count remains59. The unchanged source worker cache cleanup, Graph/provider retirement, original-thread join/FLS checks and controller memory/domain closure follow normally.

The source therefore exercises the actual Runtime/Policy **child oldcount1→0 stop/post branch**, not a nonlast substitute, controlled cap or original-worker release. It does not claim end-to-end child destruction of a particular OwnedEnvironment row; public family row/copy/move tests remain a separate stage. Here the one capability-owned work unit has an explicit fixture origin so its accounting and actor boundary are isolated.

## Static review findings corrected

- Controller `make_threads` initially returned unique_ptr while the probe retained shared_ptr; the source now creates one shared provider from that returned owner up front.
- Probe native id now uses `std::uint32_t`, matching the recovered begin-thread output argument rather than relying on DWORD having the same C++ typedef.
- A pre-ready Parent accessor would block on the original's held Parent mutex. It was moved after the release event; pre-ready work is offer acquire plus lock-free identity/FLS checks only.
- No-argument main invokes the new case, so an eventual ordinary test harness run does not silently omit it.

Independent audit in `independent-fixture-audit.md` found no remaining blocker at the final source hash. This is static review, not a runtime pass. CRLF-aware whitespace diff checks were also clean. Existing snapshot implementations were inspected: Application/IO/Accept snapshots are gated metadata reads, and job reaping requires the original worker outside Operation.

## Scope left open

The optional original-serving failed-post saved-error-latch terminal scenario was not added in this stage. Native error/order evidence and prior Policy StopOnly tests are separate; this successful child post does not cover error0/nonzero or throwing lower post outcomes. Unknown thread/SDK/timeout failures retain the fixture through its existing process-terminal conventions; no native EH, unload, game compatibility, arbitrary concurrent allocator safety or ABI equivalence is asserted.

## Preserved snapshots and hashes

Before editing, the current source plus both central metadata files were copied normally. The pending-source snapshot contains54 changed sources; those frozen files were not rewritten by this author.

- Before source SHA256: `4bf2ea25fb3d2468852a797a711643a3ee677931e4070133c6d4cbbeb1065d75`.
- Final source SHA256: `677b7f63d1e8031455492237cb9e33864bc1085842003ca4f60812fc7eac5756`.
- `pending-source-status.before.json`: `9fb062e73c2333785918b3267a354e5d156d6695da08b3ddd980b8f50d188e9c`.
- `active-work-plan.before.json`: `013beaefd91a6e2e785dd3ac8a3c2cb12b7bedd7a242140ea0dbd54957c13e26`.

The next explicit user build/test request is required before any execution validation.
