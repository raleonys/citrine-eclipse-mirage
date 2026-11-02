# A11-01: donor DLL link blocker versus the current loader contract

**Status: SOURCE_COMPARISON / DONOR_LINK_BLOCKED / PRODUCT_DEFECT_NOT_ESTABLISHED**

This review covers A11-01 from the preserved
[Archive 11 research project](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_ARCHIVE11_REVIEW.md).
It compares the donor's reported, **failed** link attempt with the current
Citrine Eclipse Mirage loader/worker boundary. It neither imports the donor
runtime nor assumes that two different ownership graphs are binary-equivalent.

## Exact evidence identities

The retained [eight-part archive provenance](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_ARCHIVE11_PROVENANCE.json)
records combined ZIP SHA-256
`db394434e38c7edad33607a846ffdc36b04a1e5978d2fbe3d74da24bda5179da`.
The [donor full-link record](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/dll-link-gate-source/current-full-link.json)
belongs to that indexed research snapshot. It records:
`dll_link_attempted=true`, `dll_link_succeeded=false`,
`link_exit_code=1`, `dll_created=false`,
`dll_loaded_or_executed=false`, and
`production_dll_readiness=BLOCKED`. Its
`PASS_EXPECTED_BLOCKER` diagnostic means the failure was identified as
expected, **not** that a working donor DLL was produced.

The only two recorded unresolved external contracts are:

- `anadius::dll_link_gate::require_loader_attach_runtime(HINSTANCE__*, void*)`
- `anadius::dll_link_gate::require_loader_detach_quiescence(HINSTANCE__*, void*)`

The recorded link selected the donor's partial-core and hook-compatibility
archives, but its execution and original Windows loader behavior were **not**
verified by that record. The [B49-B54 comparison](SOURCE_DIFFERENTIAL_B49_B54.md)
independently cites another same-named input digest; the open discrepancy is
tracked by [PR #115](https://github.com/raleonys/citrine-eclipse-mirage/pull/115).
Do not silently attribute one archive's files to another.

## Current shipping entry and teardown crosswalk

| Boundary | Current product path | Proof and non-equivalence |
| --- | --- | --- |
| Loader attach | [`src/platform/dll_entry.cpp`](../src/platform/dll_entry.cpp), `DllMain(DLL_PROCESS_ATTACH)` creates only an event and worker thread; Application construction, networking, hook operations and provider loads occur in the worker outside loader lock. | There is no product export or direct ABI mapping to the donor's undefined `require_loader_attach_runtime` symbol. An unimplemented donor contract does not imply absent product startup. |
| Worker/image ownership | `application_worker` acquires an additional image reference with `GetModuleHandleExW` outside loader lock; it owns one `Application`, starts the runtime and processes numbered shutdown requests. | This is a source-level ownership observation, not evidence that the donor's Resources graph, private CRT or original image callback lifecycle matches it. |
| Failed startup | A failed `Application::start()` queues a cleanup request; stop exceptions leave its `unique_ptr` intact for another explicit request. | No fake successful attach or unconditional owner release is introduced; [paused-startup PR #102](https://github.com/raleonys/citrine-eclipse-mirage/pull/102) covers distinct ongoing acceptance. |
| Public shutdown | `DllShutdown(DWORD timeout)` is the third public export. It serializes concurrent callers, counts requests before signaling, and returns bounded success/timeout/failure statuses; an unsuccessful attempt preserves the caller's module reference obligation. | It is **not** a drop-in replacement for the donor's private missing `require_loader_detach_quiescence` hook. |
| Worker exit | After successful owner teardown the worker ends via `FreeLibraryAndExitThread`, releasing its own image reference. The caller releases its separate `LoadLibrary` lease only after successful shutdown. | This ordering is enforced by the current contract, not by the donor's unsuccessful link record. |
| Loader detach | `DllMain(DLL_PROCESS_DETACH)` signals the worker and closes stored Win32 handles when appropriate; it does **not** join threads or run application teardown under loader lock. | A loader-detach event by itself does not prove complete native quiescence or safe forced process lifetime. |

The current ABI is pinned in
[`metadata/PUBLIC_DLL_ABI.json`](../metadata/PUBLIC_DLL_ABI.json):
exactly `DllMain@1`, `SURGE@2`, `DllShutdown@3`. Historical
`anadius64!anadius` linkage is **not** claimed interchangeable with this
owner-approved interface. [`docs/PUBLIC_DLL_ABI.md`](PUBLIC_DLL_ABI.md)
describes the same distinction.

## Coverage, gaps and disposition

Existing [native shutdown failure fixtures](../tests/native/application_shutdown_failure_tests.cpp)
exercise a real entry-worker DLL and selected queued requests, cleanup failures
and retries. Existing source-linked application/startup tests and hosted
Windows/Golden receipts are tied to their **exact product commits**, not to the
donor's missing loader-hook implementations. The selected A11-06 post-start
cleanup ordering is reviewed separately in
[PR #119](https://github.com/raleonys/citrine-eclipse-mirage/pull/119)
and [native module stage-retry PR #125](https://github.com/raleonys/citrine-eclipse-mirage/pull/125).

**A11-01 disposition: DONOR_LINK_BLOCKED / CURRENT_ENTRY_SOURCE_MAPPED /
PARITY_NOT_PROVEN.** There is no demonstrated production defect from these
two unresolved donor symbols. Do **not** add dummy implementations or exports,
move expensive startup/teardown under loader lock, relink the donor's CRT, or
claim that a successfully linked current product DLL completes a different
donor target.

To advance the historical research, identify original loader attach/detach
entry targets and their real symbol/procedure dependencies, establish exact
original-linkage/Consumer expectations, and independently execute a fixed donor
candidate on native Windows if authorized and legally available. Any
**product** change separately requires a reproducible, current-source failure,
bounded correction, actual MSVC/Golden validation and owner approval.

This document contributes no production implementation, native executable test,
fresh donor binary, newly performed native acceptance or release publication.
