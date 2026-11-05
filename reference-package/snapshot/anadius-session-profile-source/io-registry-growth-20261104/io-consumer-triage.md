# IO registry consumer triage — 2026-11-04

Read-only source mapping before dynamic IO implementation. No compiler, project tests, REA session, target execution or packaging. Search boundary: production .cpp/.hpp in IO memory, accept, Runtime, Profile, coherent flow, Stop, owned Environment/Jobs/Policy and socket completion/cache helpers; tests, fixtures, saved before files and historical reports were excluded.

## Owner data and public snapshots

OwnedIoMemory::Impl owns records, generations, storage and packets vectors (io_memory.cpp:38–41). Records already point to individually owned Record objects, so OperationStorage/projection/OVERLAPPED object identity can remain stable while these lookup arrays grow. generations is private: no public generation span or persistent external uint64 reference was found. IoMemoryHandle/AcceptHandle and copied IoLoan values carry slot+generation scalars; old per-index counters must be copied, not reset.

storage_bindings()/dispatcher_bindings() return borrowed span snapshots (io_memory.cpp:172–173). OwnedAcceptMemory forwards both (owned.cpp:166–167). Immediate index/scan users in accept validation and Runtime Router reacquire them around source callbacks; no existing promise makes copied spans self-refreshing.

## Persistent span-bearing consumers

| Consumer | Retained field / location | Growth requirement |
| --- | --- | --- |
| Runtime bind | runtime.cpp:158 local IoCompletionEnvironment is copied into Ports and AsyncAcceptContext at162 | Supply the live storage view once before publication. |
| Runtime worker | CoherentIoDispatcher.environment_ (coherent_io.hpp:55), constructed runtime.cpp:282 | Its copied environment must resolve current storage. |
| Runtime worker loop | DispatcherState.operations at runtime.cpp:320; stored by reference in RecoveredDispatcherWorkerEffects | Whole-run stack lifetime spans every callback/allocation/wait. Use current packet view at binding_for, not initial span. |
| Actual Profile | Application::Impl const Ports at profile.cpp:95; receive_context constructed around231 | Copies must retain a live view capability; const Ports cannot be fixed by assigning each old snapshot. |
| Profile Effects hierarchy | RecoveredHandlerDispatchEffects.context_.environment (handler_dispatch_bridge.hpp:18), RecoveredComposedWriteEffects.context_.environment (composed_write.hpp:67) | Both handler-task and async-send contexts own copied environments. Rebinding only receive_context is insufficient. |
| Owned Environment | environment.cpp:61 optional Ports; response scheduling uses p.environment.storage at177 | Actual posted/cache allocation consumes this retained copy later. |
| Session Policy | policy.cpp:23 optional Ports; application matching at49 compares copied storage identities every call | Compare current views; otherwise partial rebinding can falsely reject the same graph. |
| Inline/NativeJobs | jobs.cpp:18/109 optional Ports, stored at48/181 | Their storage spans are checked at bind (45/178); no later storage dereference was found in Jobs itself. Live-view copying covers them without extra registration. |
| Alternate dispatcher adapters | OwnedIoDispatcherEffects.environment_, IoDispatcherEffects.environment_, PostedDispatcherEffects.environment_ | Not the selected Coherent Runtime route, but copying a supplied live-capable environment should retain compatibility automatically. |

CoherentIoMemory itself retains references to base/accept, not an IO table span. Its sources_ span is the independent Profile environment-source registry; the preceding session-growth stage already provides bind_environment_sources. Do not confuse it with IO storage.

## Raw packet aliases beyond spans

AsyncReceiveOperation.packet and AsyncSendOperation.packet are assigned inside base packets at io_memory.cpp:156/160. AsyncAcceptOperation.packet is assigned into that same table at owned.cpp:136. The three receive/send/accept packet helpers dereference those public pointers; source metadata changes/tampering must not be silently bypassed.

A per-Record optional DispatcherOperationBinding** external_packet is sufficient for the selected accept alias. Base growth can repair internal receive/send pointers and this one registered external field before retiring the old packet array. Validate prior pointer values before repair. Accept must register its exact stable op.packet field and unregister before its variant is destroyed; base clear/projection/free must clear a registered field while it is still alive. Existing OwnedAccept::project calls base.project_posted before retiring the old sidecar, so later unbind must accept an already-cleared null alias. Stable Accept Row nodes are required because Row variants/Task references and raw accept OVERLAPPEDs survive callbacks and kernel loans. Its fixed row coverage check at owned.cpp:57 must also grow to cover new base indices.

## Snapshot/call-stack boundaries

PostedEnvironment contains a storage span by value (posted_operation.hpp:31); IoCompletionEnvironment nests it (io_completion.hpp:87). complete_receive/send/posted helpers and cache/handler helpers also receive copies by value. A pointer to an owner-held live span in PostedEnvironment plus current_storage() with legacy fixed-span fallback lets all these copies, including const Ports, follow the owner without a callback-registration framework. DispatcherState can use the corresponding live_operations/current_operations() field. The owner updates these stable span fields before freeing old arrays; existing Parent serialization remains required.

Immediate storage lookup loops that must change are posted_operation.cpp:7 storage_at and handler_dispatch.cpp:6 lookup_storage. Selected identity checks that must use current_storage() are runtime.cpp:127–128; profile.cpp:797; environment.cpp:197; jobs.cpp:45/178; policy.cpp:49/139. This includes both sides of Policy's p-versus-own comparison, not only literal ports variable names. completion_dispatcher.cpp:15 binding_for must resolve current operations after opaque wait/epoch calls. Runtime Router reacquires the packet after completion_owner returns; its slot/generation/operation checks must remain.

SessionIo Stop retains copied Loan/Entry identities and raw operation/OVERLAPPED pointers (stop.cpp:15–26), not storage/packet spans. active_loans()/pending_loans() are owned metadata copies and resolve current generations on later use. Stop therefore needs stable source operations, not a span rebind. Its explicit options.capacity/Entry limit (stop.cpp:93 and bind145) remains a separate optional stop-profile resource bound; it must not be introduced as a new mandatory OriginalCrt unload condition.

No new native ABI, concurrency or packet-finality claim follows from this mapping. Public snapshots remain snapshots unless they explicitly carry the selected live-view capability.
