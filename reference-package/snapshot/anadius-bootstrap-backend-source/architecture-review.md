# B3 bağımsız mimari ve ömür incelemesi

27 Ekim 2026. **PASS** — dört üretim dosyası exact hash ile incelendi; önceki696kaynak dosyası aynı. Yeni native gövde/kapanış0. Bu bir test veya tam DLL eşdeğerlik sonucu değildir.

| Dosya | SHA-256 |
|---|---|
|`backend.hpp`|`7073712738416083f0a41da7fa88ae050762791cdec5f51201b8dcebda7677db`|
|`backend.cpp`|`c144112d064589d28a4b6e761f43861ab05e3f4cc48b6f9b8310789bb7720a2e`|
|`tracked_threads.hpp`|`414a94314cecedd2908a073a6206a0adfdb7847e80df75911d0ba29e792277a7`|
|`tracked_threads.cpp`|`31fc08d0311ed281465d9ffe7e255581c6848a5f83c67f2b435e17084efc8573`|

## Bulgular

- **single_concrete_backend** (`backend.hpp:50`): SourceBackend concretely implements the existing ProviderOwner and Services interfaces; no new virtual provider interface is introduced. Its resource pack still requires actual lower implementations.
- **owned_resources** (`backend.hpp:25`): All provider objects are unique-owned; optional backing state is declared first and therefore outlives dependent providers during reverse destruction.
- **caller_seed** (`backend.hpp:12`): Execution mutex/cache/three readiness bytes, online handles/flag, library state, host values/targets/original slots are caller-supplied state; construction invents no original global defaults.
- **resource_validation** (`backend.cpp:8`): Every required lower object and stop_sources callable is validated before the implementation is published. JoinApi is validated by TrackingThreads construction.
- **shared_execution** (`backend.cpp:75`): One Execution instance aliases the exact server/online/library readiness bytes and one frequency cache; server completion publishes the same first flag online waits for.
- **source_server** (`backend.cpp:99`): The first worker calls the existing server source using live config.server_port, the shared server-ready byte and shared Execution; actual ServerBackend remains mandatory.
- **source_online** (`backend.cpp:100`): The second worker calls the existing online source with config path/profile aliases and owned module/object/online state. The wrapper binds wait and finish to the shared Execution and second byte.
- **online_aliases** (`backend.cpp:94`): Online StateRefs bind the same live executable/cookie/profile/avatar/user fields as the configuration owner, not detached copies.
- **library_main_paths** (`backend.cpp:102`): Synchronous load paths are refreshed from post-configuration state at the actual main load route; the existing library phase source selects and appends loaded handles.
- **library_worker_paths** (`backend.cpp:34`): Worker library paths are refreshed after the original wait2 completes, so changes made while prerequisites complete are not copied too early. UTF16-to-wstring conversion is an explicit source compatibility snapshot, not native alias/allocator equivalence.
- **source_library_phase** (`backend.cpp:103`): The existing main/worker library phase body is reused; only worker=false waits and publishes its third readiness byte. Existing loaded state is retained across phases.
- **source_host** (`backend.cpp:140`): The between-workers hook uses actual host-init source, the same configuration object and owned values/targets/original slots; actual hook effects remain supplied.
- **source_cleanup** (`backend.cpp:143`): Cleanup routes reuse unload035ab0, onlinecleanup0366f0 and hookcleanup0102b0. Bootstrap.on_exit retains their existing order; unload intentionally does not clear loaded handles.
- **remaining_services** (`backend.cpp:129`): Six Services methods are source-routed; eleven policies still forward to the required lower Services object. This adds zero native bodies or native provider closures.
- **publication_timing** (`backend.cpp:128`): Worker routes are published only during register_exit after B2 native application binding is established. bind_state merely caches references and cannot publish routes or create jobs.
- **shared_state_once** (`backend.cpp:110`): bind_state is accepted only once and is rejected after stop. Configuration and console pointers remain owned by the enclosing RuntimeOwner lifetime.
- **route_lease** (`backend.cpp:82`): Raw worker function pointers resolve through one per-image gated backend and counted leases; no gate is held while application algorithms execute. An unbound late call terminates without reading the old backend.
- **route_owner** (`backend.cpp:88`): A second backend cannot take over an existing backend route. routes_published distinguishes a never-published backend from the actual published owner.
- **stop_order** (`backend.cpp:118`): stop_sources is mandatory and runs before tracking admission is closed. True must exclude new application/worker/hook/exit/terminate admission while allowing already-created workers to finish, without unloading their providers.
- **stop_reentry** (`backend.cpp:115`): SourceBackend guards its own stop callback against lifecycle reentry; RAII restores the control flag on false, exception or successful stop.
- **route_unpublish** (`backend.cpp:123`): Only successful thread drain plus atomically checked route ownership and zero route leases permits unpublish. False/throw leaves backend and B2 native binding live for retry.
- **join_api** (`tracked_threads.hpp:13`): Six exact Win32 SDK function pointer types support duplicate/wait/close and LastError preservation. native_join_api returns rebuilt import addresses without executing a target binary.
- **precreation_ticket** (`tracked_threads.cpp:44`): A stable list ticket is allocated before possibly starting a worker, preventing post-create tracking allocation loss. Each create owns a separate ticket even if Windows reuses original HANDLE values.
- **create_outcome** (`tracked_threads.cpp:53`): A throwing create has unknown outcome and permanently poisons quiescence instead of erasing its ticket. A returned zero handle safely removes its reservation.
- **no_context_read** (`tracked_threads.cpp:61`): After successful lower creation only the returned handle is accessed; no Context field is read or written because the worker may already have freed it.
- **duplicate_policy** (`tracked_threads.cpp:67`): DuplicateHandle retains a separate same-process join handle without closing or changing the original detached handle. Failure/exception keeps an untracked poisoned ticket and never fabricates safe quiescence.
- **last_error** (`tracked_threads.cpp:14`): Tracking records the lower create result LastError and restores it after duplicate/bookkeeping; wait/close/control probes similarly preserve the caller error value.
- **wait_and_close** (`tracked_threads.cpp:99`): Waits occur outside the registry mutex; only WAIT_OBJECT_0 followed by successful CloseHandle removes a ticket. Timeout/failure/unexpected result or failed close retain the ticket for retry.
- **pending_and_drain** (`tracked_threads.cpp:84`): A not-yet-returned create prevents success, as does poison. New create requests are rejected after stop, while existing worker cleanup methods continue forwarding to their live lower provider.
- **destruction** (`tracked_threads.cpp:34`): Tracking destruction with retained/unknown jobs or active drain is an explicit terminal lifetime violation. SourceBackend likewise refuses destruction while its route is still published.
- **crt_scope** (`backend.hpp:86`): Compiler CRT governs new rebuilt C++ bookkeeping once; this source package installs no PE entry and invokes no recovered outer CRT entry. A future compiler-CRT entry must not call the recovered outer CRT initializer too. Full ABI/EH/global equivalence remains unproven.
- **registrar_limit** (`backend.hpp:44`): std::atexit alone cannot implement cancellation. Actual registrar/stop_sources and application resource-cleanup strategy remain required; joining threads alone does not close late exit/terminate or hook callback lifetime.

## Açık sınırlar

- Actual config/file/console/thread/runtime/server/online/hook/registrar effects are required resources, not reconstructed by this composition.
- stop_sources must truthfully exclude every future admission/registration; actual cancellation/unregistration cannot be inferred from a boolean callback.
- Application exit-resource cleanup is separate from join quiescence; no provider is automatically unloaded before workers finish.
- UTF16 to host wstring conversion introduces compatibility storage/allocation and does not establish original MSVC alias/allocator failure equality.
- Control methods are serialized and non-reentrant; provider synchronization remains mandatory for concurrent shared application fields.
- state_lifetime outlives owned resource components, but arbitrary peer-provider destructor dependencies remain a caller/factory contract; generic provider internals are not proved by this review.
- No PE entry, exact native CRT/ABI/EH/global graph or complete runnable DLL is claimed. Generic CRT leaf work remains deferred.
