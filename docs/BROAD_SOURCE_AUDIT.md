> **Historical checkpoint.** Use [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

Later owner-provided Windows/Golden/native and game acceptance,
including DLL size measurements, is recorded in
[the current acceptance record](DLL_SIZE_AND_NATIVE_ACCEPTANCE.md).
All test and pending-work statements below are historical as of this audit.

# Whole-source and adapter audit

The XML findings below describe the earlier TinyXML2 audit checkpoint.
The current application and development DLL use pugixml behind first-party
compatibility code; the success/error and depth contracts are tested against
that reader. See [VENDORING.md](VENDORING.md) and
[XML_BACKEND_COMPARISON.md](XML_BACKEND_COMPARISON.md). The earlier measurements
and ruled-out suspicions below are historical evidence, not a description of
the current vendor implementation.

The earlier review focused on the delta of a new RE package;
five corrections were not evidence of an entire codebase audit.
This pass expanded coverage to **all 277 product sources/headers**,
the actual build partition, and call chains from the Application
owner to Windows adapters. Findings were not limited to the
previous five.

## Scope and meaning of the evidence

| Source group | Files | Work performed |
|---|---:|---|
| Production C++ | 64 | Actual CMake objects/link boundaries, OS/adapter, exception, ownership, and concurrency paths |
| Retained reference C++ | 74 | RE source comparison, test/host symbol linkage, exclusion from Production; not deleted |
| Headers | 139 | Source counterparts, interface/layout changes, direct CPP consumers |
| Files with earlier mapping | 248 | Token re-comparison with new RE input: 211 matches, 37 reviewed differences |
| Files outside earlier mapping | 29 | Application integration, explicit policies, and authorized donor/reconstruction adapters |

The last two rows classify the same files in the first
three rows; they are not additional files. Includes,
comments/whitespace, and recorded namespace transformations
were excluded from token comparison. This is not a compiler
AST audit or a measure of behavioral equivalence to the
original DLL.

Read [the file-level CSV](../metadata/evidence/BROAD_SOURCE_MATRIX.csv),
[the JSON inventory with hashes and symbol links](../metadata/evidence/BROAD_SOURCE_INVENTORY.json),
and [the validation record](../metadata/evidence/BROAD_AUDIT_VALIDATION.json)
together. The JSON also records external symbol links in
138 actual COFF objects and compiled test/host objects.
A test linking a symbol does not imply execution of
all branches of that function. Inline, virtual,
callback, and GetProcAddress paths were separately
considered; no line/branch coverage percentage was invented.

## Corrections in this audit

| ID | Source / defect | Correction and regression evidence |
|---|---|---|
| B01 | Execution mutex field `owner_48` was read before the native lock; a write by another thread caused a C++ data race. | Use `atomic_ref` while preserving layout. The real `execution.cpp` showed a TSAN race/exit 66 before the correction and a clean result after it. A four-thread test exercised 120,000 critical sections across three mutex types. |
| B02 | An exception during CommandLineToArgvW item conversion/vector insertion failed to call LocalFree. | Release the list on the exceptional path too. Injected conversion failure changed the expected free count from zero to one; normal/null paths were preserved. |
| B03 | Caller thread identity remained excluded from the unload scan after `DllShutdown` returned. | RAII clears the identity on every return. A real worker/cleanup-retry DLL fixture checks for zero after two failed and one successful shutdowns. This is not a complete proof of native unload safety. |
| B04 | A missing XML child's text could be null during `const char*` to `string_view` conversion. | Concrete serializer adapter rejects missing text with `invalid_argument`. Actual XML reader/router tests use QueryPresence/AreChunksInstalled with empty Id followed by a valid request. Other source-model serialization rules remain intact. |
| B05 | ShowIGO left the event mutex locked if queue creation/enqueue threw an exception. | RAII unlocks after successful lock acquisition. Exactly one unlock is verified for two separate failure points; the old body fails the same test. |
| B06 | A failed owned-event initializer could leave guard = -1 and the init mutex held. | Optional abort callback resets the owned runtime guard and releases the lock. Tests inject `bad_alloc` at first seed creation, retry with the same owner, check exactly one seed and peer-thread access. The pure model's default abort is a no-op; no private-CRT EH mechanism is invented. |
| B07 | A handle or temporary-path owner could be lost if stage/backup write/close/copy failed before returning a path. | Native backend reserves a record before acquisition, retaining handle/path until successful close/delete; `recover_atomic` retries. Injected WriteFile/CloseHandle/DeleteFileW/CopyFileW failures on real temporary files/handles verify target preservation and successful subsequent writing. The old backend fails the same test. |
| B08 | An unknown entitlement field produced `UNKNOWN MESSAGE` in the dialog instead of the actual diagnostic. | Convert the counted UTF-8 diagnostic to Unicode in the actual body; retain the SURGE title. Only MessageBoxW is intercepted in real Environment code. The prior body fails the field/value assertion. |
| B09 | IPv6 connect hook redirected to ::1, while the concrete server listened only on 127.0.0.1. | Add exclusive IPv4 and IPv6-only loopback listeners for the same chosen port. Before the change, ::1 failed with Connection refused; afterwards both families, IPv6 candidate collision/IPv4 release, and a shared session limit passed. If IPv6 is unavailable on the host, only the affected checks are explicitly skipped. |
| B10 | Extra worker published the final readiness flag before profile/factory became ready. | Keep worker completion in a private flag; open the callback's third gate only after publishing profile and accepting state. An actual Application with Windows mutexes and workers under controlled pause catches early return in the old body and releases waiters at the correct time in the new body. |

B01/B02/B05/B06/B04 relate to exception/concurrency
boundaries arising when source models run in C++ or
the concrete product; they do not imply that the
original machine code necessarily contained the same
defects. B03/B07/B08/B09/B10 were gaps in the owned
application/adapter integration. B09/B10 completed
previously approved IPv6 and startup-readiness
behavior; no new CFG, export, port, or SDK policy
was introduced.

## Reviewed areas

| Area | Reviewed boundary / test families |
|---|---|
| Application startup/shutdown | Worker join, readiness/profile publication, cancellation, ownership, timeout/retry; application_runtime, startup_publication, cleanup_retry, public lifecycle |
| Environment OS integration | Unicode/path, optional main/override CFG, avatar, directories, logging, actual dialog; configuration_platform, environment_dialog, application_runtime |
| Execution/argv/flags | Pre-lock owner access, mutex types/layout, argument cleanup, QPC publication before callbacks; execution, execution_concurrency, startup and TSAN |
| Config/VDF/hash/order | PEGTL actions/rewind/partial state, roots/order of two Config2 inputs, removed SDK/port overrides; parser, vdf_parser_regressions, configuration, config_application/hash/order |
| File/path/status/license/token | Stream false/throw/close, path root, standard filesystem and source rules; file_io, path, status, license_file_integration, licensing, token, windows_effects |
| Full copy helper | Mode/identity fallback, timestamp comparison, reverse close and fatal boundary; file_copy, file_copy_windows_integration |
| Atomic persistence | Acquisition/path transfer, stage/backup failure, reverse rollback and retry ownership; atomic_persistence, atomic_staging, achievement_runtime/writer |
| Hook installation/rollback/unload | Queued apply, SAFE-FREEZE build-copy pins, incomplete rollback, trampoline drain, handle retry; hook_transaction, hook_freeze, unload_guard, hook_engine_fixtures |
| Callback binding/lease/locks | Shared live state/original slots, common Guard/Pause, LastError, close/drain; production_callback, native_callback, host_init, hook_adapter_integration |
| Connect | Exact loopback, complete const address copy, narrow attempt-counter lock, readiness; production_network_hook, network_hook, actual listener tests |
| Hook 11 | 32,768 UTF-16 limit, protected read, suffix/null/fallback, SDK result/LastError; crypt_path_probe and callback tests |
| Registry/process/folder | A/W ABI boundaries, handle/map identity, Pause during registry wait; registry, host_callback, production_callback |
| Active transport/messages | Asynchronous greeting, two workers, per-session FIFO, limit/deadline, cancellation-buffer lifetime; server, handler_pool, network_policy, session_flow, listener_port |
| Event queue | Seed/guard retry, mutex exception, reserve/commit/retry, allocation/persistence ordering; event_queue/integration, misc_handlers, session_flow |
| Achievement | Map rollback, XML/INI batch, event publication after persistence; content_handlers, achievement_storage/runtime/writer, atomic_persistence |
| Handler/router | Command registration, XML null text, stable attribute copies; handlers, misc_handlers, dispatch_integration, messages |
| TinyXML bridge/printer | Document/wrapper lifetime, ErrorStr, attribute order/escaping, backend-node identity; xml_reader, xml_printer, dispatch_integration |
| Codec/hash/encoding | AES/padding/offset, existing error domains, SHA-1, UTF-8 versus filesystem-codepage distinction; crypto, hash, encoding and actual encrypted round-trip |
| Provider/extra library | Vtable slots and existing fixture, loading/ownership, worker dependencies; online, libraries, application_runtime. Full external-provider ABI not verified. |
| Time | Wrap/signed arithmetic and UTC/timestamp parsing; calendar, timestamp, utc |
| Fixed identities | SDK 10.6.2.31 / wire 10,6,2,31, product path, no CFG overrides; configuration, application_runtime, source-requirement checks |
| Reference transport/XML | All 68 CPP files and headers covered by source comparison and existing model/integration fixtures; separate linker-map verification of exclusion from Production |
| Reference composition | Remaining six CPP files and headers, test/source API linkage preserved; private CRT or new controller-admission policy not imported into Production |

## Ruled-out suspicions and open boundaries

The suspicion that TinyXML2 `ErrorStr` could be nullptr
after successful parsing was ruled out: the vendored
implementation returns an empty C string, and the
existing success test checks `empty()`. No unnecessary
reader change was introduced. Intentional throws at
abstract boundaries were not automatically classified
as missing implementations: LicenseFiles owns hashing/
token processing, an output-only achievement reader
does not read requests, and the native greeting path
does not require synchronous operation.

Execution's QPC cache is initialized in the Application
constructor before callback publication. Event seed/
guards are also created before session publication.
This actual product ordering is not proof of safe
concurrent cold initialization by every independent
C++ model consumer; off-startup uses of abstract
models need their own contracts and tests.

Outstanding acceptance limits: the updated source
had not been rerun under native MSVC/Golden or a real
game/Consumer in this audit. The external provider
ABI, original private CRT/global layout/exception
handling, every Windows/filesystem error code, and
all OS versions are not declared equivalent.
Atomic multi-file updates are not power-loss or
cross-process transactions. Arbitrary/unaligned
foreign pointer domains beyond the existing typed
SDK contract were not accepted.

## Preserved contracts and reruns

The 64 Production/74 reference split, three C
exports/ordinals, static CRYPT32, byte pins of four
vendors, version-metadata profile, profile/fallback
avatar bytes, and Release logging prohibition
were preserved. Production builds independently
with tests/source APIs disabled; inspecting a raw
compiler DLL is not equivalent to final Golden/MSVC
acceptance. Optional virtual methods were added to
C++ source interfaces, so development consumers
must rebuild from all the sources.

The normal native-acceptance launcher gathers all
tests. Separate Release/Debug CTest runs, source QA,
the actual linker-map boundary, and three
load/request/stop/unload cycles of the Production-only
runtime host are recorded in the validation results.
Four additional independent test targets cover
Execution concurrency, native atomic staging,
Environment dialog, and startup publication;
failure paths were added to existing tests too.

This report documents the reviewed scope and
identified fixes. The defect count does not certify
that no other defects remain. Later changes must
revisit the same files/call chains and native
Consumer scenarios as needed.

At this checkpoint, full Release 193/193,
affected Debug 17/17, and source QA 16/16 passed.
QA 16/16 passed after clean ZIP extraction alongside
a clean Production build with tests/source APIs
disabled; that DLL also completed three real
public-ABI runtime cycles. Detailed classifications,
regressions failing on the previous code, and
platform limits are retained in validation
JSON/XML/log files.
