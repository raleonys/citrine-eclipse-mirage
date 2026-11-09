> Current bindings and F01–F04 corrections:
> [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md).
> The body below is a historical record of the earlier task;
> its former export/resource descriptions are not the current contract.

> This is a historical record of the binding task. Later owner
> corrections for Windows I/O and vendor selection are in
> [WINDOWS_IO_FIX.md](WINDOWS_IO_FIX.md).

# Application integration — YF-RUNTIME-BIND-20261028

The owner pointed out that the bindings reported as missing in
the comparison should already have been implemented. This task
completes concrete application integration; reports or additional
abstract ports alone cannot satisfy it.

One mistake in the earlier comparison: `ScalarDependencies` was
a restricted alternative bridge. The
`config_application::read_config2/apply_config2` and `FilePass`
paths already supplied the complete hash-capacity/section-copy
implementation. Neither an eight-child limit nor a scalar-only
restriction applied to the entire configuration application.
The new owner uses the full implementation directly.

Application changes involve `src/net/asio_service.cpp`,
`src/platform/file_io_windows.cpp`,
`src/runtime/application*.cpp`, related headers, DLL
entry/management APIs, CMake, and actual integration tests.
The underlying functions are based on the existing RE modules
and supplied pseudo/assembly evidence for the
`FUN_180036d20` constructor. No custom CRT or comparison-only
application code is imported.

Product adaptations remain explicit and separate: visible
SURGE name, LocalAppData/SURGE, SURGE PNG, and a Debug-only
fixed-event log. The RE executable-relative main -> override
ordering is retained with `surge.cfg` ->
`surge_override.cfg`. The provider is a separate adapter
under a new name using the existing RE
version_check=2/create_online/vtable contract.
Native original ABI equivalence cannot be inferred from
these bindings.

Ownership: one Application owns configuration, effects,
events, provider, hooks, and service lifetimes. Asio
session callbacks and jobs keep their session alive.
Jobs are real standard-CRT threads; no fixed worker pool
replaces the source per-frame job model in this historical
snapshot. Cipher state remains per session. Stop closes
admission, cancels sockets, waits for I/O and handler jobs,
and then releases resources. Native hook rollback,
quiescence, and actual Windows acceptance are documented
separately.

DLL attach only creates Windows handles for the worker.
It does not set up the network, configuration, hooks,
provider, or C++ Application under loader lock.
`DllWaitReady`, `DllServerPort`, and `DllShutdown` were
management APIs of this new application at that checkpoint.
A later C-interface task added the SURGE import marker
and actual DllShutdown adapter; C++ source APIs moved
to a separate development DLL. The previous 548-byte
ABI claim was withdrawn for lack of source evidence.
Current contract: [PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md).

Validation targeted actual vendor-backed configuration/
Unicode file I/O, XML/LSX profile/content/achievement
requests over a real local socket, automatic startup on DLL
load, error-prefix rollback, and stopping pending socket/
handler jobs. Wine, native Windows/MSVC, and actual
Consumer/game evidence are separate classes.

After admission closes during abort, private source
wait gates are released so native callbacks waiting
for later incomplete phases cannot block draining.
This is not successful startup: `start()` returns an
error and the DLL wait API does not report success.
A partially activated hook set is not a ready application.

Companion-DLL phases are connected to actual
`load_phase_035850/load_list_035260/unload_035ab0`
functions. Every successful OS load is tracked by the
owner even if allocation to append it to the source
list fails. Cleanup releases leases once.

A later owner correction imported Brownfield's
`resources/version.rc` byte-identically.
Fixed VERSIONINFO numeric fields, four string fields,
and translation profile 0409/1200 were retained.
Following the last owner correction, the PNG is
no longer a PE resource; it is read-only C++ data
replacing the prior profile-avatar bytes. The separate
fallback PNG remains from RE.
[Current correction](GOLDEN_AVATAR_FIX.md).
The selected contract is in
[RESOURCE_METADATA_CONTRACT.json](../metadata/RESOURCE_METADATA_CONTRACT.json).
`inspect_dll.py` examines actual PE VERSIONINFO fields
and the then-current two-resource allowlist, not just
the image. That historical resource profile does not
automatically activate other PE header/ABI/Golden policies.

The production provider adapter treats an exception after foreign destructor entry
as an unknown destruction outcome. It retains the exact object and module records
and the original exception, rejects later calls before object/table access, and
keeps cleanup unsuccessful so the application owner remains retained. A missing
object/table/slot detected before entry remains retryable. Successful destruction
still clears the object and permits module-only retry if FreeLibrary fails. This
adapter policy does not change the recovered online source or establish native
provider ABI equivalence.

The stored foreign exception stays owned alongside the pinned provider module;
there is no reset or automatic retry after an unknown destruction outcome.
An ordinary successful destructor return creates no stored failure. The DLL
worker reaches owner.reset() and reports shutdown success only after stop()
returns, so this retained failure keeps both the application and worker image
lease alive until process termination.
