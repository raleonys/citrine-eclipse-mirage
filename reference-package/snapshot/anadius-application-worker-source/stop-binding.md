# Controller-to-worker cancellation binding — source only

Status: **implemented, not compiled or executed**. The user now requires an
explicit rebuild request before any compilation, test or link run. Previous
full-build results and ZIP describe their frozen source snapshot, not these
pending source edits.

The existing server Runtime already consumes an optional accept_stop::Stop.
Its selected SessionIo mode tracks accept/receive/send/Posted work and waits
for real kernel packets and source acknowledgements. Previously Worker exposed
only a startup-event wake; a provider created inside worker configure was not
automatically the controller's stop target.

Settings now accepts an optional controller-created Stop. For example:

```cpp
accept_stop::Options stop_options;
stop_options.lifetime = actual_cancellation_backing;
stop_options.scope = accept_stop::Scope::SessionIo;
stop_options.other_work_drained = real_admission_and_raw_job_proof;
inputs.worker.accept_stop = accept_stop::Stop::create(std::move(stop_options));
// Create/choose the Stop on the original Worker controller/fiber.
```

No proof implementation or cancellation scope is invented by the binding.
The provider must be Fresh, exclusively selected for this Runtime and have
backing that really owns its supplied SDK/policy state. Resources rejects a
nonfresh supplied Stop before consuming Inputs; Worker creation checks again.
After configure returns, the worker rejects a different Runtime stop pointer
and assigns the exact Settings-owned pointer before Graph/server construction.

request_stop marks the existing startup state, then copies the selected shared
Stop and reserves stop_requesting under the Worker mutex. The actual request,
independent proof and IOCP post happen outside that mutex. The reservation
outlives local shared-owner retirement. Nested lifecycle retirement/request or
controller readiness reentry cannot turn a poisoned prefix into success.
Concurrent ordinary worker routing remains allowed so Stop's existing epoch
postcheck can invalidate an overlapping proof for retry.

FALSE does not always mean retryable. Fresh/Bound/not-running, overlapping
routes or unmet prerequisites can return a known false. A failed IOCP post
already poisons Stop even if it returns false; Worker observes that state and
poisons/retains rather than retrying an unknown prefix. TRUE means only request
admission, not cancellation completion or physical packet drain. No loan/OV/
socket or Graph ownership is released by this controller path. CancelIoEx and
actual source completion routing remain on the admitted original worker.

SourceBackend's existing order remains external stop_sources, Worker request,
callback pause and joins. For SessionIo, stop_sources must independently stop
new application/foreign/raw-job admission and finish raw/deferred work, while
allowing existing tracked IO/session work to remain for the selected Stop.
Waiting there for cancellation that only the later Worker request can trigger
would create a cycle; this binding does not manufacture that prerequisite.
Existing finite join/stop retries can request again after a known false.

Started pre-admission/unknown prefixes retain their previous poison policy and
do not infer a cancellable SDK binding. With no selected Settings stop, the
previous startup-event-only path remains. Existing configure-only stop users
still carry their own external controller/request obligations.

Settings ownership retires configure captures before Stop, then effects/state/
lifetime. finish and Worker destruction refuse an active request reservation.
The additional shared field changes Settings and embedding Inputs HOST layout;
all passing/allocating consumers must be rebuilt together when authorized.
Worker's bootstrap base interface and opaque public facade layout are unchanged.

Required future validation, not run:

1. Exact pointer installation, donor clearing and conflict rejection before SDK
   startup, including a configure-supplied different Stop.
2. Fresh/Bound/route-overlap/prerequisite false then later successful request;
   no source cleanup/join completion assumed from the request result.
3. Post FALSE/throw, proof exception and swallowed lifecycle/readiness reentry;
   no known false/Poisoned conflation, duplicate cancellation or lost owners.
4. Accept/receive/send/Posted cancellation with real own overlapped IOCP packets,
   success/995/ERROR_NOT_FOUND races, exact acknowledgements and worker exit/FLS.
5. Same-Domain Resources/Runtime stop and pre-unload retry, plus legacy no-Stop
   behavior, capture destruction order and all affected Settings consumers.
