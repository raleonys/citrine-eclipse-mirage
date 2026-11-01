# Anadius (11) B49-B54 source differential: native lifetime, shutdown, and factories

**Classification:** source audit / proposed follow-up tests / no production patch / no release.
**Comparison base:** Citrine Eclipse Mirage `main` commit `d9c860d17769738fd6f03a58a7f3c4b9f875b067`.
**Donor:** user-supplied `dll-reverse-engineering(11).zip`, ZIP SHA-256 `4b98177c7f7d827994e209fe75a918d9c54f25194c4adf19d9d419bdf061f0fe`.
**Date:** 2026-11-01. **Evidence quality:** static source comparison. The donor's 356 groups and 19 terminal scenarios are its own recorded results; they were **not rerun** in this GitHub worktree. Windows/MSVC, native game, and original donor runtime parity were **not run** for this differential.

## Scope and separation

Do not equate a function name or an existing Asio component with validated behavioral equivalence. The repository contains **two different ownership graphs**:

1. Shipping `src/net/asio_service.cpp` uses Asio sockets, session callbacks, pending read/write/timer counters and deferred native-close retention.
2. Reference `src/net/transport_*.cpp` reconstructs specific original transport algorithms against Effects interfaces. Reference compilation does not prove these functions execute on the shipping DLL path.

The donor B49-B54 packages integrate their **own** RuntimeOwner/SourceBackend/Domain thread and Resources graph. B49/B50's native evidence uses overlapped **named-pipe** operations. They do not prove real `AcceptEx`, `WSARecv`, `WSASend`, foreign thread or original DLL/Consumer compatibility. Their full production DLL link remains blocked on `require_loader_attach_runtime` and `require_loader_detach_quiescence`. No recovered CRT or alternative hook engine is proposed for import.

## Per-phase findings, evidence and disposition

| ID | Donor mechanism and primary source | GitHub source inspected | Outcome | Required proof / bounded action |
| --- | --- | --- | --- | --- |
| D49-A | B49 `anadius-accept-stop-source/stop.cpp`: `CancelIoEx` is issued by the native worker for the exact listener/owned OVERLAPPED; TRUE/FALSE/ERROR_NOT_FOUND are not finality | `src/net/asio_service.cpp` `queue_accept`, `retire_io`; `src/net/transport_cancellation_native.cpp` | **TEST_GAP**, not proven regression: Asio cancellation and pending-accept callback tracking exist, but the exact B49 kernel packet/submission-generation proof is not the same contract | Real Windows delayed accept, cancel-vs-success/NOT_FOUND, missing/duplicate/late completion; assert handler/listener/buffer lifetime and no new session after stop |
| D49-B | B49 separate source callback route lease / admission seal | `src/net/asio_service.cpp` `accept_pending`, `stopping`, `active_sessions` | **DIFFERENT_OWNER_GRAPH**: current callbacks are serialized by the IO executor, and two families share admission, but donor's source epoch proof is not directly represented | Drive both IPv4/IPv6 accept completions through simultaneous shutdown; verify no post-stop rearm |
| D50-A | B50 `anadius-session-stop-source/README.md` and `anadius-io-memory-source/io_memory.cpp`: exact socket+operation+buffer+control binding before native submit and physical completion/after-reap before release | `src/net/asio_service.cpp` `Client::async_receive`, `async_send`, `close`, `stop` | **PARTIAL_CORRESPONDENCE**: pending read/write/timer counts and callback-held owners exist. Neither a named-pipe B50 PASS nor this static review proves delayed Windows socket finality | Native WSA/Asio incomplete read/write, cancellation, late callbacks, pending timer, buffer lifetime, repeated stop |
| D50-B | B50 does not treat close or cancellation API return as I/O completion | `src/net/transport_closing.cpp` `cancel_and_close_026fb0`; `src/net/transport_close.cpp` `close_026e50`; `src/net/asio_service.cpp` `NativeClose` | **SOURCE_CONTRACT_DIFFERENCE / HIGH**: the reference close path clears `object.state.socket`, socket state and cancellation control after an ignored `close_026e50` result. Shipping Asio retains uncertain or would-block close state. This does **not** establish a shipping bug or donor parity defect. | First create a reference-only failed-close fixture proving actual expected source behavior and operation ownership. Do not silently alter reconstructed native semantics; separately validate shipping no-early-free with real Windows I/O |
| D50-C | B50 finality requires native completion/after-reap | `src/net/asio_service.cpp` `Impl::stop` | **LIVENESS_TEST_GAP**: after worker join, stop runs a single 100 ms `io.run_for` pass, then reports incomplete cleanup for retry. This is a conservative failure rather than automatic UAF; delayed completion and retry progress remain to be measured | Inject delayed completions exceeding 100 ms and prove exact owner retention across two or more `stop()` attempts; never claim the timeout makes the API final |
| D51-A | B51 `anadius-bootstrap-native-source/native_entries.cpp` and `anadius-bootstrap-owner-source/owner.cpp` pauses new Application entries before backend stop; active accepted caller is drained | `src/platform/dll_entry.cpp`, `src/runtime/application.cpp` | **TOPOLOGY_DIFFERENCE**, not confirmed bug: the DLL worker invokes `Application::start` and later `stop` serially, while `accepting` gates handler creation. A donor-style foreign Application entry is not the same ABI on this target | Probe concurrent `DllShutdown`, repeated stop and startup exception, with live worker/handler callbacks; no new global gate absent an actual reachable concurrent entry |
| D52-A | B52 owned Configuration factory: 21 methods, 19 source routes plus two explicit effects; facade retirement and failure atomicity | `src/config/application.cpp`, `src/config/config_startup.cpp`, `src/config/parser.cpp` | **PARITY_NOT_PROVEN**: matching parser/function names do not validate signed tolower, codepage, exact ordering, partial initialization, or destructor-time callbacks | Differential fixture for missing/empty/duplicate Config2 keys, BOM, malformed VDF, path failure, write failure, callback during retirement; capture native Consumer XML before policy changes |
| D53-A | B53 owned remaining Services policies, cached shutdown stages, required `close_after_cleanup` | `src/net/transport_service_registry.cpp`, `transport_scheduler_registry.cpp`, `src/runtime/application.cpp` | **DIFFERENT_FACTORY_OWNERSHIP**: individual services exist but not the donor's narrower explicit retirement facade. No source evidence yet that replacing the working graph improves reliability | Native service-construction fault matrix and phase-by-phase stop/close retry; prove no duplicate cleanup/free or leaked owner |
| D54-A | B54 online factory requires a live code lease and `foreign_quiescent` predicate before final physical retirement | `src/runtime/online.cpp` `cleanup_0366f0`, `src/runtime/application.cpp` `Provider::close` and `Impl::stop` | **HIGH-PRIORITY PROOF GAP**: stop joins local workers and drains hooks/service before unloading the online module, but static source does not provide the donor's independent proof of all foreign jobs/callbacks having exited. This is **not** a confirmed UAF. | Synthetic foreign module with active callbacks during teardown, failed FreeLibrary retry, concurrent getter/destructor, and code/image pin verification on real Windows |
| D54-B | B54 borrowed provider strings vs owned-copy getter with code-binding lease | `src/runtime/online.cpp` `copy_c_string`, `src/runtime/application.cpp` vtable getter wrappers | **PARITY/LIFETIME_GAP**: copied values are obtained from borrowed foreign C strings; no tested foreign module lifetime for the entire getter and copy window | Test owned getter with deliberately delayed foreign string invalidation and unload attempt; preserve observed Consumer values and error behavior |

## Risk classification

- **Confirmed source-code difference:** D50-B, D50-C, D51-A, D53-A and D54-A. A difference is not automatically a defect.
- **Confirmed production bug:** **none established by this read-only comparison**.
- **Highest-risk unverified boundaries:** D50-B reference close ownership, D50-C late Asio completions, D54-A foreign module retirement.
- **Behavioral authority:** exact original Anadius x86/x64 evidence and actual CollegeFB27/Madden27 requests/callbacks outrank a donor C++ reconstruction or a matching function name.
- **Release status:** no native, Golden, shipping, game, ABI or Consumer PASS is inherited from this audit.

## Corrective work split into reviewable follow-up PRs

1. **Native I/O lifetime regression PR (D49-A/B, D50-A/C):** Windows x64 source-linked Asio accept/receive/send cancellation under late completion and repeat stop, including two-family accepts. First demonstrate a RED fixture on the pinned code before a bounded production fix. Preserve `NativeClose`, completion-owned buffers, generation tags and session receipt semantics.
2. **Reference closing contract PR (D50-B):** distinguish reconstructed return semantics from production close ownership. Establish reference-only failing-close fixture and source donor/Consumer evidence. Only change reference implementation if it violates its declared recovered contract; do not transfer a synthetic safety policy as original parity.
3. **Foreign provider lifetime PR (D54-A/B):** synthetic Win64 module with blocked getter, destructor and FreeLibrary failures; trace exact module/active-call ownership. Add a dedicated code lease or quiescence predicate only if a reachable gap is proven. Never free a module after a failed external finality test.
4. **Startup and factory parity PR (D51-A, D52-A, D53-A):** source-frozen fixture matrix for startup race, Config2 parser/application ordering, services construction and staged cleanup. Avoid wholesale import of donor RuntimeOwner, custom CRT, PTD/FLS or alternative code-patching engines.

## Proposed executable acceptance matrix

| Suite | Key negative | Pass condition |
| --- | --- | --- |
| Windows Asio accept | Cancel races completed accept, two families, stop before callback | No accepted post-stop session; no early listener/OVERLAPPED release; all pending callbacks observed or ownership retained |
| Windows socket receive/send | Cancel/close races delayed callback, buffer consumed after stop | No UAF; no duplicate completion/receipt; error/byte count and session retirement correct |
| Repeat shutdown | Completion arrives after first 100 ms drain attempt | First incomplete result does not free resources; later retry succeeds without duplicate close or rearm |
| Reference `cancel_and_close_026fb0` | `close_026e50` failure with live control and pending operation | Source parity determined explicitly; no unsupported assertion that clearing model state proves kernel finality |
| Application bootstrap | Concurrent shutdown signal while start is in progress | Existing serialized worker contract verified, never stops still-creating worker without correct join |
| Config/Services | Constructor exception, partial publish, late callback, retry | Exact ownership and single cleanup, unchanged route/XML output |
| Online module | Getter blocked while stop begins; FreeLibrary failure | No unload while foreign code runs, failure retains owner and reports incomplete shutdown |

## Validation and publication boundary

Performed: archive-level evidence review and pinned GitHub source reads; no edits to production, vendor, exports, ABI, hooks or runtime policy. **Not performed:** local full source QA, portable test suite, Windows/MSVC/CRT/Golden, Consumer/game differential or original donor runtime execution. This document is a review backlog, **not acceptance of any runtime fix**. The future PRs above must supply pre-fix RED evidence, post-fix GREEN evidence, proper source pins and genuine Windows receipts. Owner review is required; never merge this documentation solely on the basis of existing CI history.
