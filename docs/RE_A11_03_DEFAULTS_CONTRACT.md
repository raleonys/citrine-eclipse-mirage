# A11-03: Archive 11 configuration defaults source comparison

**Disposition: SOURCE_REVIEW_ONLY / NO_PRODUCT_DEFECT_CONFIRMED / DONOR_PARITY_NOT_PROVEN.**
**Product comparison base:** `main@4d2d083786283bca2f8d53e79e7a99aae364464d`.
This is a bounded assertion-by-assertion comparison, not a new startup implementation, configuration migration, or native test receipt.

## Exact input identities and scope

- Retained eight-part Archive 11, combined ZIP SHA-256 `db394434e38c7edad33607a846ffdc36b04a1e5978d2fbe3d74da24bda5179da`; see [intake ledger](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_ARCHIVE11_REVIEW.md) and [long-lived project](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_PACKAGE_PROJECT.md).
- Donor [defaults.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-config-defaults-source/defaults.cpp), SHA-256 `4ae57170b870bcde9f3a6c0c1ee0f5f2e5ee48e631fc1876a6256fbb6fb7d6c8`; donor [defaults_tests.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-config-defaults-source/defaults_tests.cpp), SHA-256 `3f33c443d3eae2de0dc02612e4368b3a16481e93433a2de5c30d387306e15d00`. These are retained reference inputs, not compiled product dependencies.
- Product [initial_state](../src/runtime/application_defaults.cpp) (SHA-256 `cc36e01fb302d11d91e0a9245e53bfabf667e35d3fa30c7dedcc130ac46d8fd1`), [configuration State](../include/preloader_l/config/configuration.hpp), [protocol identity](../include/preloader_l/core/protocol_identity.hpp), [defaults unit tests](../tests/unit/config_defaults/defaults_tests.cpp), and [defaults integration tests](../tests/unit/config_defaults/defaults_integration_tests.cpp) are compared at the pinned base.
- The other historical same-named `(11)` ZIP in [PR #115](https://github.com/raleonys/citrine-eclipse-mirage/pull/115) has a **different** recorded digest. No claims from that distinct historical input are silently transferred to this retained archive.

## Donor assertion -> current product evidence

| Donor assertion family | Current code or test | Assessment |
| --- | --- | --- |
| Unbound paths, empty content/entitlements/achievement containers and unset registry/language overrides | `State{}` and `constructor_contract()` check named empty paths, containers and flags | **Covered as current-product defaults**, not original binary-layout equivalence. |
| Content ID `"0"`, zero listening port, enabled fake-auth/pretend-connected and disabled rerun/last-profile | `initial_state()` plus explicit unit assertions | **Covered**; port zero denotes *not yet bound*, not a chosen fixed port. |
| Donor username `"anadius"` and two numeric profile identifiers | Product deliberately uses `"SURGE"`; the numeric persona/user identifiers match; unit and integration responses check this | **Intentional branding difference**; donor name must not replace owner policy. |
| Donor `State.sdk="10,5,96,47512"` | Current `configuration::State` has no `sdk` field; the separate `protocol_identity::sdk_version="10,6,2,31"` is the LSX Challenge identity | **Different field and purpose**; not evidence of a missing constructor assignment or permission to change protocol identity. |
| Six emulator attributes, fourteen game attributes, exact textual dates, language list and account strings | `constructor_contract()` asserts exact key/value set and language ordering; `default_responses()` checks representative serialization | **Covered at model/response level**, not complete Consumer-byte equivalence. |
| Independently owned repeated factory results | `independent_values()` mutates one result and verifies the next; current test also changes extra-DLL and port state | **Covered for selected mutable fields**; no donor container ABI adoption. |
| Missing configuration, defaults carried through external response, and configured override | `defaults_integration_tests.cpp`: `missing_configuration()`, `default_responses()`, `configured_responses_and_isolation()` | **Current-product integration coverage**; not a direct run of the donor factory or original Consumer. |
| Repeated allocation failure at each donor factory allocation point, followed by a clean subsequent factory call | `allocation_unwind()` on non-MSVC-Debug platforms arms allocation failure around `initial_state()`; MSVC Debug arms **after** constructing the State and probes map insertions only | **Partial**, with an explicitly documented MSVC Debug constructor-failure gap; no evidence of a shipping leak. |

## Important MSVC Debug limitation

The current unit harness replaces `operator new` inside its **own test executable** and counts live tracked allocations. On MSVC Debug, constructing a `std::basic_string` or related default state can allocate a debug iterator proxy inside a `noexcept` constructor. Forcing that allocation to fail may terminate the process rather than throw into `allocation_unwind()`. The existing test therefore constructs `initial_state()` unarmed and then injects failures into throwing map insertion sites. Its `--probe` modes are diagnostic, not proof that the complete factory survives arbitrary injected failures in MSVC Debug.

The donor test exhausts allocation failure positions in **its own test harness**, and neither its success nor its ABI assumptions transfer to the product. Do not change the product's CRT or replace `noexcept` semantics to force donor-test parity. A newly proposed constructor-failure regression needs an isolated process, a clearly specified supported failure boundary, and actual MSVC Debug/Release receipts; it must not assert that an intentionally terminating library operation is recoverable.

## Follow-up gates, not established fixes

1. Retain the intentional `SURGE` identity, the separate protocol SDK version and the current zero-port/14+6 defaults. A different donor literal alone is **not** a product bug.
2. Before a new test PR, identify a *reachable, supported* allocation-failure boundary missing from the existing tests. Preserve the distinction between normal exception propagation and MSVC Debug `noexcept` termination; require a discriminating negative control.
3. Verify any new behavior against actual exact-head Source QA and Windows/MSVC native Debug and Release; only applicable product/Golden and Consumer runs can support their separate acceptance claims.
4. Do not close A11-03 as complete donor parity, import custom CRT/SDK/container ABI, overwrite pinned vendor/production code, or interpret this document as a runtime PASS.

No product source, tests, CMake/CI, release artifacts, exports, hooks or vendor pins are changed by this review. The retained research project branches and master ledger continue independently; owner review controls any future implementation or merge.
