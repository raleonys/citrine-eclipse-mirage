# Child-authorized tracked work stop: concrete proposal

Independent source design assessment, 2026-11-04. Root has selected coherent tracked-family reconstruction as artifact work, including child last-release, beyond the earlier default-route assessment. No production edits, new REA activity, compilation, tests, linking, execution, or packaging.

## Stable Runtime capability

Append one optional `EnvironmentWorkStop* environment_work_stop=nullptr` to `server_runtime::Ports`. Its implementation is a stable Runtime::Impl member, never the run-local Worker at runtime.cpp:321. It outlives Graph/Application destruction under the existing Runtime ownership contract; no new unload prerequisite is needed.

Proposed interface:

```
class EnvironmentWorkStop {
public:
    virtual ~EnvironmentWorkStop() = default;
    virtual bool matches(const SessionMemoryDomain&, const IocpSchedulerEntry&) const noexcept = 0;
    virtual void stop_original(IocpSchedulerEntry&) = 0;
    virtual void stop_job(IocpSchedulerEntry&, const SessionMemoryDomain::JobPermit&,
                          const std::function<void()>& checkpoint) = 0;
};
```

It binds the exact Parent/scheduler and Runtime's actual retained raw `deps.dispatcher`/`deps.errors`. Environment retains its existing Parent lease and validates matches before producing a tracked pin. A missing capability rejects before retain/drop, rather than decrementing and discovering on oldcount1 that stop is unsupported. Host/custom configurations need not use it; actual OriginalDefault Runtime supplies it.

The child call grants only the immediate zero-packet02a190 operation. It does not grant TLS, waits, arbitrary completion posting, general Policy access, or lifecycle permission.

## Reuse02a190 through narrow existing subsets

Extract the existing stop methods into two small bases in worker_run.hpp:

```
class WorkerStopApi { // existing post_stop/get_last_error signatures
public:
    virtual ~WorkerStopApi() = default;
    virtual int32_t post_stop(uintptr_t,uint32_t,uintptr_t,OVERLAPPED*) = 0;
    virtual uint32_t get_last_error() = 0;
};
class WorkerRunApi : public WorkerStopApi { /* existing TLS methods */ };
class WorkerStopEffects { // existing signatures, no new source effects
public:
    virtual ~WorkerStopEffects() = default;
    virtual uint32_t exchange_stop_34(IocpSchedulerEntry&,uint32_t) = 0;
    virtual uint32_t exchange_wake_38(IocpSchedulerEntry&,uint32_t) = 0;
    [[noreturn]] virtual void raise_028240(Error,uintptr_t) = 0;
};
class WorkerRunEffects : public WorkerStopEffects { /* existing run methods */ };
void stop_worker_02a190(IocpSchedulerEntry&,WorkerStopApi&,Api&,WorkerStopEffects&);
```

Keep the function body and order unchanged. Current run/dispatcher providers inherit the same method signatures and convert to the narrow bases. This avoids new adapters containing fake dispatch/cache/CRT implementations. The child wrapper cannot call TLS because that interface is absent.

## Original actor

`stop_original` validates exact graph/Parent/scheduler and current original-worker admission, then invokes02a190 with Runtime's existing worker wrapper, actual errors, and a three-method adapter to `graph->bindings->worker`. Serving posts keep the admitted completion route. It must not fall back to raw SDK because original admission is missing.

Phase boundary to settle before coding: Runtime::policy currently allows Ready/Running, while Environment cleanup also permits Returned/Quiescing. Normal last-work release occurs while serving. If full tracked cleanup after Runtime return must be supported, define that exact operation explicitly; do not silently bypass original admission or introduce a new unload condition.

## Child Policy reservation

Add a narrowly default-rejecting `WorkerControl::stop_job` in runtime.hpp, overridden by Profile's actual Worker bridge:

```
virtual void stop_job(IocpSchedulerEntry&, const JobPermit&,
                      WorkerStopApi& permitted_raw_stop, Api& bound_errors,
                      const std::function<void()>& checkpoint);
```

Runtime creates the private raw-stop wrapper from its bound providers and calls this bridge. Profile dispatches directly to the concrete selected Policy's new stop_job, not through v.effect or Policy::enter, which intentionally enforce the original actor. Existing Policy admission remains unchanged globally.

Policy::stop_job reuses the established cleanup_job_exit_waits reservation pattern (policy.cpp:157-192):

1. Pin shared_from_this and its Parent/provider lease. Under Policy gate capture the exact Parent/scheduler, require Bound/not binding/not closing and a distinct child actor; reject a foreign scheduler or errors object. Bound errors must be the actual NativeApi shared by Policy/Runtime.
2. Check JobPermit against that Parent before active reservation. The permit verifies Parent identity, child thread/fiber, live Operation seal, current PTD and open phase (memory.cpp:222-234). A JobMemory pointer or operation_on_this_thread is insufficient.
3. Reserve the existing active flag under the same nonrecursive Policy gate. Reject/poison overlap just as current child cleanup does. Do not hold it across checkpoint, SDK, categories, allocation, callbacks or exception construction.
4. Before and after effects, check live permit, Policy state, exact scheduler/source graph and the borrowed Environment JobState checkpoint. Environment's selected shared JobState pins that synchronous callback; no allocating std::function copy or persistent callback storage is needed.
5. Call unchanged02a190 through the private raw-stop wrapper and local three-method WorkerStopEffects adapter.
6. On return check again and clear active. On exception retain the completed counter/stop/ownership prefix, poison through the existing child failure route and clear the reservation. Never re-add a released pin or defer the stop to a receipt.

Child stop exchange uses the exact atomic_ref operation already used by Policy::exchange_stop. Child wake exchange uses the SAME Policy gate as existing exchange_wake (policy.cpp:148-149), preserving high32. A separate Runtime mutex would not synchronize its low32 read/write with existing original-worker writes to word38.

Raise084530 validates the same category/context as Policy::raise, poisons and constructs the same compiler std::system_error(...,"post") mapping without calling its original-thread terminal() entry. This narrow child projection does not enable arbitrary Policy calls.

## Actual raw posting and immediate error

Ports.worker is Runtime::WorkerApi (runtime.cpp:33-39,159). During serving its post routes through Completion to an original-worker AdmittedDispatcherApi. That object requires its original thread/fiber and whole Parent Operation (server.cpp:22-35,59), so a child must not reuse it.

The private wrapper calls Runtime's actual retained raw deps.dispatcher only while the new exact child capability is active. It accepts only the live bound scheduler's port, bytes0/key0/null OVERLAPPED; it never exposes raw dispatcher in Ports. Capture a failed post's error immediately, before permission postchecks:

```
check_exact_child_scope();
auto result = raw_dispatcher.post(live_bound_port,0,0,nullptr);
if (!result) saved_error = raw_dispatcher.get_last_error();
check_exact_child_scope_preserving_last_error();
return result;
```

Its get_last_error revalidates then returns that latched failure value. On false post02a190 still calls the actual system-category provider unconditionally, even for status0. Stop/wake flags stay changed on failure; only nonzero status raises. SDK throw or post-effect permission failure retains/poisons the completed prefix, without guessing packet delivery.

Thus raw access is confined to one source operation under exact Parent/JobPermit/scheduler proof. It is not a blanket admission bypass, arbitrary post API, new receipt channel or delayed-stop policy.

## Integration with actual tracked rows

Integrate the earlier design-review.md family/flags proposal into existing OriginalDefault callbacks, preserving Host. Factory58 transitions initial→tracked; tracked copies/factories retain; moves transfer; destruction drops one pin and immediately calls this capability on oldcount1. Prepare40 and response20/88 implement saved flag/family transformations. Do not add disconnected leaf wrappers.

Keep native flags separate from HOST state_18/capability/Origin; real initial Slot roots remain real proof rows. Extend row/full-prefix/growth-source/protected-source validation for family and flags. Work pins are distinct from ledger refs, owner strong refs, IO-start pins and posted pins.

Environment::perform already captures the selected JobState and checks exact lineage; protected sources cannot be destructively moved/released (environment.cpp:97-123). Selected child operations pass that state's live permit/checkpoint; no selected JobState uses stop_original. Invoke outside Control::gate, keeping source metadata and JobState alive through final checks. Commit tracked row release once before any throwing stop effect so a retained facade cannot be retried to double-decrement.

## Production file set

- `anadius-socket-source/worker_run.hpp/.cpp`: narrow bases and stop parameter types; body unchanged.
- `anadius-server-runtime-source/runtime.hpp/.cpp`: capability, trailing Ports pointer, WorkerControl child method, stable member and private permitted raw-stop wrapper.
- `anadius-session-profile-source/profile.cpp`: concrete child bridge to selected Policy, bypassing only original-thread v.effect for this explicitly authorized operation.
- `anadius-session-policy-source/policy.hpp/.cpp`: bounded child stop reservation, shared flag locking, category/raise projection.
- `anadius-owned-environment-source/environment.hpp/.cpp`: actual family/flags callbacks and capability validation/use.

No DllMain/attach, unload conditions, dispatcher wait, PreparedMemory receipt, or ordinary Policy permission change is required. Trailing defaulted fields/default-rejecting virtuals preserve legacy aggregate/custom source compatibility until tracked support is selected.

## Validation design, not results

Use actual OriginalDefault chains initial→factory58→tracked copy/prepare/response20/88→move/destruction. Verify flags and one retain per produced row; actual child JobScope last-release performs exchanges/post synchronously before scope close. Cover wrong Parent/scheduler/actor/stale permit, invalid checkpoint/reentry, high32 preservation, old stop/wake nonzero, post success/false0/false nonzero/throw, error latching despite HOST checks, and no TLS/wait/arbitrary post access. Preserve original/admitted and Host scenarios. No case was built or run here.
