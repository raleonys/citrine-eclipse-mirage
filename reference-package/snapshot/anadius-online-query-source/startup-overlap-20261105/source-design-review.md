# Source design: startup/query overlap — 2026-11-05

Read-only architecture assessment. Fresh native eligibility/wait evidence is assigned to the SDK agent in this stage and is still pending; this note does not assume QueryImage's exact native wait behavior or declare the candidate confirmed before that evidence. No production/test/status edit, build/tests, linking, execution, packaging or REA query was performed by this source reviewer.

## The candidate reaches the actual source provider

The OriginalApplication worker path (`application-worker-source/worker.cpp:74`) admits serving without adding the HOST online/host-ready milestone wait. The recovered Online startup source sets online state before its owned getter copies (`online-source/online.cpp`), while Content's actual AvatarRequest route reaches `Online::avatar_virtual_28` through `content-effects-source/effects.cpp:73`. Licensing's selected owned auth copy reaches the same Online provider at `licensing-effects-source/effects.cpp:96`.

The Online facade already permits overlapping Open calls: `online-factory-source/factory.cpp:65` increments a count without rejecting an existing call. Ordinary methods have no controller-thread/fiber affinity; only close/physical retirement requires the controller. The facade keeps its owned Binding and all backing stable until active calls finish.

The narrower **Binding** currently rejects all query overlap. `online-object-source/object.cpp:65–71` treats every begin_object as the single active_ reservation. `online-query-source/query.cpp:31–44` uses that reservation for status/getter/avatar/auth. Thus a slow startup copy_username and a simultaneous valid same-object avatar/auth call conflict there; Online.invoke converts a thrown overlap into facade poison. This is not a path through the obsolete OnlinePlatform compatibility provider. Whether native wait rules make the particular startup/avatar schedule eligible remains the fresh audit's question.

## Minimal conditional implementation proposal

If fresh native evidence confirms an eligible same-object overlap, separate lifecycle-exclusive activity from admitted object-query count. Keep default HOST Binding semantics serialized with an immutable constructor query policy (for example Serialized versus OriginalConcurrent). The actual Online factory selects concurrent query admission only when its existing Options lifetime profile is OriginalCrt; legacy/Cooperative/OnlinePlatform callers retain the default unless explicitly selected. Do not infer it from arbitrary CFG contents or readiness values.

1. `online-version-source/version.hpp`: constructor policy and private query count; keep active_ for lifecycle/export transactions. Snapshot.active can remain the aggregate `active_ || queries!=0` so existing emptiness checks do not need a new public count or ABI field. A trailing query count is unnecessary unless a meaningful consumer needs it.
2. `version.cpp`: load/resolve/version-call/free/destructor must reject any nonzero query count in addition to their existing checks, before lower SDK/lease retirement. The destructor's existing unresolved-owner policy must include live queries. No mutex spans foreign code, SDK, string allocation/copy, code-lease factory or owner retirement.
3. `online-object-source/object.cpp`: resolve_factory/create/destroy likewise reject live queries. begin_object validates exact adopted object/module/factory lease, lifecycle inactivity, known object and nonpoisoned state; in serialized policy also require count0. Then increment under metadata gate. end_object decrements exactly one reservation under that gate; it must not clear lifecycle activity or another query's reservation. Same-object queries need no new thread/fiber affinity or same-actor prohibition.
4. `online-query-source/query.cpp`: QueryUse validates and reserves before reading the vptr/slot. Its exceptional constructor currently sets active_=false globally: adapt that to poison/mark unknown and release **only its own** counted query. Other active queries must remain counted until they finish. Do not decrement twice from a throwing constructor and destructor. Existing lower-function or string-copy exceptions release their own query through the normal guard without silently clearing global poison/unknown state.
5. `online-factory-source/factory.cpp`: pass the immutable query policy at its existing Binding construction point. Online's facade count/phase and controller-only close/retirement already provide the correct outer ownership; do not add a global OnlineReady barrier, wait, service serialization or foreign-safety attestation.

## Module/code and result lifetime

No new per-query module reference, allocation pool or shared owner registry is needed. Binding already owns the actual factory_code_ shared reference acquired through `module-reference-source/reference.cpp`'s typed GetModuleHandleExW/FreeLibrary lease. Once lifecycle/rebind/free/destruction reject count>0, that exact module/code owner remains retained throughout every vptr read and foreign call. The same object cannot be destroyed/replaced through this Binding until all queries return. Direct caller destruction of the Binding while its member call runs remains outside its lifetime contract, as before; the actual Online facade keeps it owned.

`copy_getter` and `copy_auth` keep QueryUse alive through the foreign return, first-NUL scan, allocation and counted owned copy. Their per-thread LastError guards preserve the actual outgoing foreign error around metadata/copy work. Parallel queries do not need a shared error latch because LastError is thread-local. Existing status/avatar results are values. Raw borrowed getter/auth compatibility APIs end their reservation on return and still require caller-controlled result consumption; do not pretend the count makes foreign buffers immutable or surviving arbitrary provider mutation. The actual startup/auth consumers use the owned-copy overrides.

Unknown vptr/slot layout still poisons and retains the object/code prefix. If another admitted query already exists, failure in one cannot erase the other's count; lifecycle cleanup must remain blocked until that actual call ends. No retry/automatic destroy/free is authorized by a query exception. No new resource-unload condition should be attached to healthy parallel queries.

## Important scope boundaries

The foreign provider's own same-object concurrency/buffer semantics are a native contract question, not something to repair by serializing calls that the original permits. Fresh evidence must show the caller schedule and absence of a readiness wait on the chosen candidate. A test should use an actual retained typed object/module and a deterministic slow first startup getter plus concurrent real query route; an arbitrary public method called without native eligibility is insufficient.

Shared Configuration cells are separate from Binding ownership. The startup source later mutates username/persona/user_id/avatar strings, and content handlers may read some of them. A controlled overlap **before the first username getter returns**, with stable object/online publication and content state, can demonstrate this concrete schedule without concurrently mutating those strings. It must not be reported as solving every shared-State race or arbitrary concurrent profile-update behavior. Current online flag/object cells are plain source storage; cross-thread publication beyond controlled synchronization remains a separate source/native memory-model boundary. This note proposes no global-ready wait or broad State/atomic framework.

## Reviewed source hashes

| Path relative to outputs | SHA256 |
|---|---|
|anadius-online-version-source/version.hpp|512c3f20766a3fd5691d06933e69c0ca5228aec3a16e5c156eb5cd9d5fa4537b|
|anadius-online-version-source/version.cpp|328443d0ac8d3bae0b5f12878c955087cf99a402beecf7e6879b345e16e9c7d0|
|anadius-online-object-source/object.cpp|917a7503768e47250f94df059b2ad98fec03a022c264aa2a0566e9ae36dfce78|
|anadius-online-query-source/query.cpp|55bf8cf5ffc9ac78dfd6d96956b62bc03cd06e6e0265877a86315d1dc36c36d3|
|anadius-online-factory-source/factory.cpp|f6a613a5c9cc007a722298efdc0c181c1270c496a179eb952a0f12fbbb8b95c9|
|anadius-online-factory-source/factory.hpp|505b2d0fad73a89a19539d4c35acea1b967c1e1cd3eea2e98650793604b0b45a|
|anadius-online-source/online.cpp|00020732bf6e36d53b9bfafe80d6cb85f303b8b329624562d45991f916e50cb8|
|anadius-module-reference-source/reference.cpp|8c96c32509f51367c161325c92f0a9910de8528c7522cf97e1eadf0e6d1456ca|
|anadius-content-effects-source/effects.cpp|8224ff1c1b10c5a3bb3d91aa8762f574a657a17dc4a83dee59dc19b745e5a510|
|anadius-licensing-effects-source/effects.cpp|c66cf34ed9b2e23772c48e834810e5afca0200083dd14b30e0eb508547e7e5d9|
|anadius-application-worker-source/worker.cpp|a384138a6dbc7c2c4348e1c77b6d6cb33f3cff3fe2f45eb91822ad71cf09ff8a|

Decision remains with root after the fresh native report and actual before snapshot. No source change or completion claim follows from this design alone.
