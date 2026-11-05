# Tracked work release bridge implementation

Source-only implementation, 2026-11-04. This author changed only the five production files below. Root owns worker_run.hpp/.cpp's narrow stop API/effect bases; the other source author owns Environment family/flags integration. No compilation, project tests, linking, DLL execution, REA session, or package update. This is implementation documentation and self-review, not an independent review or test pass.

Before copies: `before/runtime.hpp.before.txt`, `runtime.cpp.before.txt`, `profile.cpp.before.txt`, `policy.hpp.before.txt`, `policy.cpp.before.txt` in this stage.

| File | Final SHA256 |
|---|---|
| anadius-server-runtime-source/runtime.hpp | b558cbfb48936b021c86d038a501fd83d9229c4a53fd2165de6121d270a3f656 |
| anadius-server-runtime-source/runtime.cpp | c2cd34da990d9c2b2ba5463181ece96f985f72d9ef22dfd06c9ca9693a1857c2 |
| anadius-session-profile-source/profile.cpp | f0744429c82f91a86509e17e9b530dc2bcce8626c55eae943dc06518e737f298 |
| anadius-session-policy-source/policy.hpp | 77b073c58846de51e57a035462add6db66c6b94e8849dbf60703fd59285318cd |
| anadius-session-policy-source/policy.cpp | b00747912def3dd0c14dd5be59aa8b42fa1459d8981330f96e56f51378e7fefe |

## Actual interface and release ownership

Trailing defaulted Ports field `EnvironmentWorkStop* environment_work_stop=nullptr` exposes a stable Runtime::Impl member. It supplies `matches(Parent,scheduler) noexcept`, `finish_original(scheduler, const function<void()>& checkpoint, bool& consumed)` and `finish_job(scheduler, const JobPermit&, checkpoint, consumed)`.

WorkerControl has a default-rejecting `finish_environment_work(scheduler, const JobPermit* job, WorkerStopApi&, Api& errors, checkpoint, consumed)`. Actual Profile::Worker forwards to the concrete selected Policy without calling its original-thread-only v.effect. Null job selects exact original actor; nonnull job selects exact live child scope. Host/custom WorkerControl implementations retain their prior source compatibility.

The capability delegates the complete validated release to Policy, rather than receiving a count that was already decremented elsewhere. Policy validates the exact errors object and scheduler/Parent, reserves, checks borrowed source permission, performs seq_cst fetch_sub, immediately sets consumed=true, then invokes unchanged02a190 only for oldcount1. All post-drop exceptions preserve consumed=true. Neither bridge adds a rollback pin, deferred receipt, or second decrement. Environment owns its separate row/pin-consumption commit based on that flag.

## Child and ordinary Policy reservations

Policy now has a separate environment_work_active flag. A valid exact child JobPermit is not rejected solely because an ordinary Policy active flag is set. It pins the selected Policy/Parent lease and touches only the scheduler stop/work fields and immutable category mapping. It does not modify or clear that other active flag.

The new flag guards bind/install/destruction, ordinary enter/valid/leave, child exit-cleanup admission and consequently close. Work overlap/reentry poisons; the work catch clears only its own flag. The child effect uses the SAME Policy gate for word38 low32 exchange as existing original exchange_wake, preserving high32. No nonrecursive gate spans checkpoint, SDK, category or exception construction.

Actual original waiting flow was traced: Runtime retains its lifecycle Call across run, while AdmittedDispatcherApi releases Parent only around the physical get and wait maintenance (`server.cpp:48-57`). That physical wait is wrapped by Runtime::policy, not a retained Policy::call active body. The existing child cleanup remains separately nonreentrant; tracked release does not inherit its unrelated active refusal.

The borrowed Environment checkpoint is used throughout this synchronous finish, not Policy's stored serving checkpoint. Parent JobPermit verifies exact Parent, thread/fiber, live current Operation seal and PTD before effects and after callbacks. Original work checks bound Policy actor and whole Parent buffer-context permission. SDK wrapper and Policy each perform their relevant exact-scope checks.

## Returned/Quiescing original cleanup

Runtime's capability explicitly permits its Ready/Running/Returned/Quiescing source phases, matching the bound backend and open IO/accept memory. Returned/Quiescing originals still require the exact bound thread/fiber and live whole Parent Operation plus the passed cleanup checkpoint. This does not call Runtime's serving-only policy checkpoint.

Ready/Running original releases select Runtime Completion's current target once, preserving its actual admitted dispatcher when present. Child releases, and specifically authorized Returned/Quiescing original cleanup, select Runtime's retained actual raw dispatcher. Both pass through one private narrow WorkerStopApi with the borrowed Environment checkpoint. Before and after posting/status access it validates Runtime identity, the live TLS index, Parent/actor permission and that exact source proof. The wrapper permits only the bound scheduler's current port with bytes0/key0/null OVERLAPPED. There is no TLS/wait or arbitrary completion API. This is explicit cleanup authority, not a missing-admission raw fallback.

On false post the wrapper captures actual lower get_last_error immediately, before metadata/PTD/source postchecks, and returns that latched value to02a190. This latch also applies to the serving original route. A post callback that corrupts the Environment source fails the immediate source postcheck before system-category lookup. The source still unconditionally resolves category on false/status0 for a valid source, raises only for nonzero status with084530, and retains changed flags on failure. The local work effect maps the same compiler system_error("post") without entering original-thread-only Policy::terminal.

## Original active/retirement reachability

Original finish refuses an already active ordinary Policy body. This preserves the existing opaque original callback reentry boundary; it was checked against actual release consumers:

- Policy.external_quiescent holds active through GraphRetirement, but its preceding local_idle requires zero live sessions/jobs/active application calls and zero scheduler work.
- GraphRetirement calls app.reap. Profile's only owned root Environment::release site is reap. Slot roots are created as Initial, remain real protected proof rows, and are not replaced by derived factory outputs. Their destruction carries no tracked native work pin.
- Tracked factory/copy temporaries are consumed/destroyed by the generic IO/response wrappers. Those execute after Policy.completion_owner has returned, outside its ordinary active call. Environment.close does not force-release stray rows.

Thus no current normal tracked release under original Policy.active was found. A future explicit lifecycle observer that calls back into tracked destruction while its Policy body is active remains an opaque reentry case, not silently granted general access. Exact child release is the separate permitted exception implemented here.

## Binding and lifetime checks

Policy.bind validates a nonnull capability's exact Parent/scheduler match. Policy.application's complete Ports identity comparison now includes the capability pointer. Runtime supplies its member when creating Application, after graph projection and before binding providers. It is not the run-local dispatcher Worker. Existing graph retirement and Parent lease contracts keep the member and retained SDK providers alive; no extra unload finality condition was added.

## Validation status

Only read-only diff/source inspection and file hashing were used. Root is authoring actual Policy source regressions; the other test author owns Environment source cases. They were not executed here. Independent review of these five files and the Environment integration is still required before claiming the source stage reviewed, and an explicit rebuild remains required for compiler/runtime evidence.
