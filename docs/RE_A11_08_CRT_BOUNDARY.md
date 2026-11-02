# A11-08: retained private-CRT scenarios versus compiler-owned CRT

**SOURCE_REVIEW_ONLY / PARTIAL_SIX_MODULE_SAMPLE / NO_PRODUCT_DEFECT_CONFIRMED / CRT_REPLACEMENT_NOT_PROPOSED**

**Product baseline:** `main@4d2d083786283bca2f8d53e79e7a99aae364464d`.
**Donor archive:** retained eight-part Archive 11 SHA-256 `db394434e38c7edad33607a846ffdc36b04a1e5978d2fbe3d74da24bda5179da`. The [retained project](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_PACKAGE_PROJECT.md) owns complete file-by-file review; this document examines **six representative source-family READMEs**, not the full CRT or every implementation line. The historical differently hashed `(11)` donor described in [PR #115](https://github.com/raleonys/citrine-eclipse-mirage/pull/115) is not silently substituted.

## Reviewed retained source descriptions

- [PTD / thread storage](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-crt-ptd-source/README.md) — SHA-256 `ed6f77bb685b0ab30e00804b01e932e06fe3b6fea5bdc0b31c86732973da530b`
- [Native FLS binding](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-crt-fls-native-source/README.md) — SHA-256 `7ed4ca215f62f143651dbee5ebe683cebdf2e9b112ac1be21b551caf3131e18a`
- [Native heap API](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-crt-heap-native-source/README.md) — SHA-256 `ff01248a4de4daadaf1d9afee8961679b21a23dd21b2cd92ff0a869ab1f1b035`
- [On-exit callbacks](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-crt-onexit-source/README.md) — SHA-256 `58bb1b9e3dc40f175995d80a36daf2339ec83b0d2ca518b7dee858ac106c6207`
- [Termination](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-crt-termination-source/README.md) — SHA-256 `ba9858266f98ef79250406dc444b916e91ec7fae6374a4187053c685d54dde10`
- [Error mapping](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-crt-error-source/README.md) — SHA-256 `22b90e57d2a60b7e82360d0ed6b15df53d159db43e49e32862a27190479710bb`

These README statements include donor-side test counts and source claims, not freshly executed tests. Donor executable or generated binary evidence has not been run by this review.

## Actual product boundary

The active [CMakeLists.txt](../CMakeLists.txt) selects the compiler's standard MSVC CRT: **`/MT` / `/MTd`** for production and regular static/native test targets. The separate development C++ DLL and its C++ host use consistent **`/MD` / `/MDd`** instead, as documented in [BUILD_PARTITION.md](BUILD_PARTITION.md), to avoid unsafe cross-module C++ container ownership.

The real [`preloader_l_crt_host`](../tools/preloader_l_crt_host/main.cpp) probes a 4097-byte allocation, C++ exception unwind, vector/string use, a worker thread and one `thread_local` destructor. It is **not** a reimplementation or exhaustive test of private CRT internals. [VALIDATION.md](VALIDATION.md) distinguishes a compiler CRT smoke from reconstructed private-CRT ABI or actual Consumer/game acceptance. The [`crt_thread_contracts` fixture](../tests/unit/thread/thread_tests.cpp) exercises retained thread-launch model Effects with synthetic providers, not the production compiler's FLS/PTD implementation.

## Source-family applicability matrix

| Donor family and recorded scenarios | Current product relation | Decision / missing evidence |
| --- | --- | --- |
| Native heap API: exact `HeapAlloc`, `HeapReAlloc`, `HeapSize`, `HeapFree`, `GetLastError` argument and error forwarding | Product currently relies on the compiler CRT; its CRT smoke tests one owned allocation, not a donor heap adapter | **No heap API import.** A new test must identify a reachable product allocator boundary and its own failure/ownership contract first. |
| PTD and native FLS: sentinel reentry, per-thread state, `FlsFree` failure/retry and callback lifetime | Compiler CRT manages its internal FLS/PTD; product CRT smoke observes one `thread_local` destructor after worker join | **Not a product PTD surface.** Donor's native binding and its explicit single-domain restrictions cannot be assumed for the product. Real MSVC thread/module teardown proof requires an application-owned scenario, not donor CRT pointer offsets. |
| Onexit and termination: encoded cookie tables, recursive lock 2, callback registration during teardown, `cexit` and terminal exit pathways | Product uses compiler CRT initializers and an explicit [DLL worker shutdown](../src/platform/dll_entry.cpp); its public shutdown is **not** the donor `exit()`/onexit implementation | **No cookie/onexit/termination transplant.** Keep successful and failed explicit `DllShutdown` ownership separate from process exit; existing [shutdown native fixture](../tests/native/application_shutdown_failure_tests.cpp) covers selected owned retries, not arbitrary process-termination semantics. |
| CRT error mapping: private `errno` / `_doserrno` PTD offsets, fallback globals and table mapping | Existing [thread reference fixture](../tests/unit/thread/thread_tests.cpp) injects `last_error`, mapping and error-slot Effects in its recovered model | **Reference-only scenario, not proof of compiler CRT parity.** No observed issue justifies overriding compiler-owned errno cells or importing donor private offsets. |

## Strict triage gates for any subsequent test PR

1. Identify an actual **application or public-DLL entry boundary**, not just a same-named CRT operation. Specify the caller, owner and failure/teardown sequence in current product source.
2. Deduplicate against [PR #102](https://github.com/raleonys/citrine-eclipse-mirage/pull/102) (startup/shutdown overlap), [PR #125](https://github.com/raleonys/citrine-eclipse-mirage/pull/125) (module cleanup stages), and [PR #132](https://github.com/raleonys/citrine-eclipse-mirage/pull/132) (configuration default allocation limitations). Their exact-head native receipts do not transfer.
3. Where a missing case is real, create a **product-owned test** with a defensible fault boundary, negative control and actual MSVC Debug/Release results. Only a reproduced source/behavior defect can justify a production change.
4. Preserve vendor archive hashes, standard CRT selection, C++ development-DLL shared-CRT boundary, public three-export ABI, thread/module cleanup policy and retained research branches. No reconstructed private CRT, FLS index, onexit table, exception/SEH policy or allocator ABI may be shipped on the basis of this limited comparison.

**Disposition:** A11-08 remains **PARTIAL / REVIEW_OPEN**. These six donor-family descriptions offer hypotheses and exclusions; this PR adds no test executable, runtime behavior, Windows/Golden result or game acceptance. Unreviewed Archive 11 CRT files remain unreviewed, not approved or rejected.
