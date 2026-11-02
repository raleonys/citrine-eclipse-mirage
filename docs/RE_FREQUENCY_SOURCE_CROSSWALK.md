# RPK-0001: original cold-frequency serialization versus product cache ownership

**Disposition: SOURCE_DIFFERENCE_CONFIRMED / SHIPPING_STARTUP_CASE_DISTINGUISHED / DONOR_PARITY_AND_GENERIC_CALLERS_OPEN**

This is the first bounded follow-up to the retained
[Reverse Engineering finding RPK-0001](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_PACKAGE_FINDINGS.json)
and its [source comparison](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_PACKAGE_COMPARISONS.md).
It maps the current accepted source graph, not a donor-native execution or an
authorization to adopt the donor's private source gate. Archive 11 combined
SHA-256 as pinned by the retained project:
`db394434e38c7edad33607a846ffdc36b04a1e5978d2fbe3d74da24bda5179da`.
The independent historical B49–B54 donor identity issue is tracked in
[PR #115](https://github.com/raleonys/citrine-eclipse-mirage/pull/115).

## Execution-model crosswalk

| Question | Retained donor reconstruction | Current Citrine Eclipse Mirage |
| --- | --- | --- |
| Cold frequency ownership | [`anadius-execution-source/execution.cpp`](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-execution-source/execution.cpp) holds a private recursive `SourceGuard` across cold frequency initialization and counter sampling. The donor header expressly limits direct-helper access, reentrance and multiple wrappers sharing state. | [`src/runtime/execution.cpp`](../src/runtime/execution.cpp) uses `std::atomic_ref<int64_t>` and compare-exchange for the caller-owned cache. Two concurrent callers may **both** invoke the platform frequency getter before either publishes; only one cached value is selected. No donor-style serialized counter-read gate is present. |
| Assertion actually exercised | The retained [publication test](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-startup-publication-source/publication_tests.cpp) asserts eight threads × 100 counter samples, one frequency query and 800 counter queries in its own guarded host model. This is a **supplied test assertion**, not independently rerun donor binary evidence. | [`tests/unit/execution/concurrency_tests.cpp`](../tests/unit/execution/concurrency_tests.cpp) deliberately synchronizes two frequency callbacks before publication and requires **two** queries, two correct samples, one selected 10 MHz cache value and no third query. It tests thread-safe cache publication, **not** the donor's one-query serialization. |
| Production application | The donor source model does not by itself establish the shipped product's startup schedule or other Consumer callers. | [`src/runtime/application.cpp`](../src/runtime/application.cpp) constructs `Execution` with a caller-owned zero cache, then explicitly queries `platform.query_performance_frequency()` and requires the result to be positive **inside `Application::Impl` construction**. Provider and extra workers are created later in `start()`. This specific startup owner therefore enters worker scheduling with an **already initialized frequency cache**, not the synthetic two-cold-caller schedule. |
| Platform boundary | The donor's `PreserveError` and source gate constrain its wrapper-specific error/callback order; they are not native source equivalence for the product. | [`src/platform/execution_windows.cpp`](../src/platform/execution_windows.cpp) calls Win32 `QueryPerformanceFrequency` and `QueryPerformanceCounter` and returns their numeric values; comments say the recovered adapter ignores the BOOL result. The production application explicitly rejects nonpositive initial frequency. This does not prove that every nonproduction or externally constructed `Execution` wrapper validates its supplied state. |
| Timers and waiters | The donor serializes its own source-owner access. Its 800-sample count is not a native Windows timing oracle. | [`tests/unit/execution/execution_tests.cpp`](../tests/unit/execution/execution_tests.cpp) checks conversion, signed arithmetic, polling, and cached/supplied nonzero frequency. [`tests/unit/execution/concurrency_tests.cpp`](../tests/unit/execution/concurrency_tests.cpp) separately tests cold publication; the proposed [A11-02 PR #128](https://github.com/raleonys/citrine-eclipse-mirage/pull/128) covers readiness payload visibility, not another clock lock. |

## Risk and evidence classification

1. **Product application cold-cache collision:** not demonstrated by the source startup sequence above, because the production Application initializes the cache before its internal worker threads. This is a **scope distinction**, not proof that no other test or caller can instantiate a cold `Execution`.
2. **Generic wrapper with two concurrent cold callers:** the current contract explicitly allows two frequency queries while preserving one atomic published value. Treat this as `INTENTIONAL_CURRENT_TEST_CONTRACT`, not a confirmed race or automatic donor parity failure.
3. **Original donor equivalence:** remains `EVIDENCE_BLOCKED`. The private `SourceGuard`, observer and callbacks represent a different ownership topology and have not been shown to be required by any active product Consumer.
4. **Error and exceptional getters:** the product Windows adapter mirrors the recovered behavior of ignoring raw BOOL results, but the production Application rejects nonpositive values. No complete source-to-Win32 failure/error preservation or foreign callback matrix was executed here.
5. **Future decision:** require an explicit product requirement for exactly-once clock query, or a reproducible native/sample failure, **before** adding a lock or changing the atomic-cache code. Reconcile all active consumers of `Execution` and validate external caller ownership; do not infer that a donor-local 800-call PASS should be transplanted directly into the product.

## Practical next acceptance

For a future bounded test, first audit the remaining `Execution` construction
sites and verify whether any product-linked instance can **actually** race at
frequency-cache zero with mutable platform results or concurrent counter reads.
Use a deterministic platform mock for a failing/invalid frequency case only
with a stated policy oracle. Run a native Windows/MSVC test for any product
lifetime/exception claim, preserve the deliberate two-query cold-cache
regression, and require an assertion-sensitive negative control before a
production fix. No new cache lock, frequency call count policy, ABI export,
private CRT, vendor code or Consumer behavior is authorized by this review.

**No tests were added or run in this PR.** It is source-linked research evidence
only; portable/native/Golden/game PASS is not inferred.
