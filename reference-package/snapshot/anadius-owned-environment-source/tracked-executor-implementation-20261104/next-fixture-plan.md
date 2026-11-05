# Smallest next prepared-child / serving-error fixture

Read-only plan. No existing frozen source, report, hash, test or native evidence was edited; no compiler or test was run.

## Recommended fixture

Use **`outputs/anadius-worker-handoff-source/handoff_integration_tests.cpp`**, a separate bounded mode beside its existing scenario. Reuse:

- `NativeDomain` (line48), synchronized real process-heap/FLS observation and actual same-Domain `NativeThreads`.
- `real_worker_handoff()` (325), `Plan` (284), `worker_entry` (314), and `worker_graph` (287). These already create controller `PreparedMemory`, acquire its Borrow on a distinct real worker, and install actual Graph/Runtime/Policy through `Graph::install_server_prepared` (302).
- `prepare_peer` (182) as the original-worker callback with a real live session and Parent Operation; `Hooks::completion_owner` (230 onward) already captures `Application` and `Ports` before session preparation.
- `Dispatcher::get` (132–146) as the existing **outside-Parent** wait point. It already proves Parent is released and is therefore the correct place to join/reap an additional bounded proof child.

Estimated isolated addition: approximately100–140 source lines for one probe record/entry and a selected mode, reusing the existing native begin-thread/handle/FLS lifecycle. No new production provider, getter, debug setter or native-thread framework is needed. Keep the existing59-packet/two-peer scenario and all assertions unchanged in its ordinary mode.

## Actual child bridge probe

1. In the selected mode's first `prepare_peer`, capture the actual copied `app.ports()` and its real `environment_work_stop` pointer; assert `matches(parent,scheduler)`. Call `parent.prepare_job()` under the **already-held** Parent Operation; do not open a nested original Operation. Keep the JobMemory object stable in the probe record.
2. Acquire one explicitly fixture-owned scheduler work unit with ordinary atomic `fetch_add(1)`, documenting its ownership. This is a direct capability-contract probe, not a claim that a native Environment row was constructed. Family retain/copy/move integration already has separate tests.
3. Launch one real same-Domain child through the existing `begin_thread_05e8e0`/NativeThreads pattern. Add that shared provider to Plan's retained lifetime rather than invent a new scheduler.
4. Child: `offer.acquire(); { auto op=parent.enter(offer); auto permit=op.job_permit(); ... }`, using the exact pattern already present in `handoff_tests.cpp::parent_job_borrow` (141 onward). Capture work immediately before release, require oldcount>1, and call the **actual Runtime** `cap.finish_job(scheduler,permit,checkpoint,consumed)`. Check consumed=true and exactly one decrement while Parent serializes source access. The checkpoint checks the real permit/Parent; it grants no Graph or original-worker API access. After Operation destruction, call `offer.retire()`.
5. Join the child and observe its real FLS exit only from the original worker's existing outside-Parent Dispatcher wait point. Close the caller-owned native handle and call original `parent.reap_jobs()` outside Operation. Do not infer physical exit from retire=true. Remove the probe record only after all handles/offers are resolved, before normal server teardown.

This executes actual Runtime validation/private child route plus actual Policy JobPermit validation and its separate work reservation, while its ordinary worker call may still be active. It does not call the original-only Runtime completion API from the child. The pending accept/read work makes this a **nonlast** decrement, so it adds no IOCP packet and should leave the original packet/FIFO assertions intact.

**Coverage left open:** a genuine child oldcount1 zero-stop/post branch, and end-to-end child destruction of a tracked Environment row, are not proved by this minimal probe. Adding an auxiliary Environment lease is not a free substitution: its close requires whole source drain, and changing Graph's private Slot source/family or querying a fake permit would require extra production/test design. Do not silently claim those cases covered.

## Why the other fixtures are larger

- `graph_integration_tests.cpp::prepared_native_profile` (580) has real prepared NativeJobs children, but their fixed `NativeJobs::Impl::entry` (jobs.cpp149) acquires/processes/returns internally; public JobHandoff does not expose its live Operation/JobPermit. `native_post` runs **after return_memory**, too late to borrow child authority. External manual offers also must not race NativeJobs' outside-Parent maintenance. Replacing that backend or adding a test hook solely for this probe is larger.
- `environment_tests.cpp::EnvFixture` and `policy_tests.cpp::PolicyFixture` use sole-owned Parents. They cannot mint a valid job permit; their current controlled cap/StopOnly tests are not actual Runtime child coverage.
- `handoff_tests.cpp` has the exact job-permit/real-thread pattern but no Runtime/Policy graph. Reuse its pattern in the already assembled integration fixture rather than assemble another server framework there.

## Original serving failed-post error latch

Use a second **isolated retained-prefix mode** in the same integration fixture. The first injected `receive_sdk` (approximately205) executes on the real original worker while Runtime is Running and its actual AdmittedDispatcher holds Parent. It can obtain `app.ports().environment_work_stop` without a new getter. Unlike `Hooks::after_completion`, this seam is not inside the ordinary Policy lifecycle callback reservation.

For this terminal mode only, set up the scheduler's explicit last-work input (count1, stop/wake0), then call actual `cap.finish_original(...)`. Configure `Dispatcher::post` to fail exactly the zero-stop packet and set error77. Count lower `get_last_error` calls; use a checkpoint that overwrites LastError after admission to prove Runtime's private latch preserves77. Expect consumed=true, work0, flags1, one failed post and one immediate lower error capture, followed by system `post` exception77 and retained poisoned Policy/Runtime. Exit the isolated process at that exact prefix; do not resume the interrupted SDK initializer, fake drain or attempt normal teardown. A failure-error0 variant can check the no-raise return, but must likewise stop at a controlled prefix because these inputs intentionally replaced an in-flight work count.

The currently authored `policy_tests.cpp::tracked_work_policy` (269) uses `StopOnly` directly and covers Policy stop branches; it does **not** test Runtime's serving selection of the live admitted API or private saved-error latch. The child route above similarly bypasses that admitted API by design, so the two tests answer different questions.

No implementation is authorized by this plan alone. Root should choose these modes and preserve the normal fixture before any source edit or future explicit build/test run.
