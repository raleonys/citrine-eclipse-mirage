# Archive 11 A11-09 — Registry HKEY ownership and ABI crosswalk

**SELECTED_SOURCE_REVIEW / NO_SHIPPING_DEFECT_CONFIRMED / REGISTRY_ORIGIN_EVIDENCE_BLOCKED**

This is a **single C++ donor-source crosswalk**, not completion of Archive 11's 199 module groups / 4,266 reviewed-file candidates. Source comparison baseline: accepted `main@532cba2926260206b281c736956f7a16bb32209c`. Retained donor [`anadius-registry-source/registry.cpp`](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/reference-package/snapshot/anadius-registry-source/registry.cpp) SHA-256 `757e2ffb52077852d9ca509635a10bc5191f2176f6d05a1dd8c88131965680f5`, verified against the [retained file ledger](https://github.com/raleonys/citrine-eclipse-mirage/blob/research/reconstruction-package/docs/RE_ARCHIVE11_FILES.json). Retained Archive 11 ZIP SHA-256: `db394434e38c7edad33607a846ffdc36b04a1e5978d2fbe3d74da24bda5179da`; do not confuse with the historical different B49–B54 ZIP ([#115](https://github.com/raleonys/citrine-eclipse-mirage/pull/115)).

## Current source identities

| Product source | SHA-256 (accepted main) | Role |
| --- | --- | --- |
| [`src/platform/registry.cpp`](../src/platform/registry.cpp) | `5b5fbdf646f1c8a2169078c97696e74f5e43fc9c80640201bc49581add4c1f48` | Value comparisons, original-first open/query, fabricate/close state |
| [`include/preloader_l/platform/registry.hpp`](../include/preloader_l/platform/registry.hpp) | `c66f67c0ef14edeb606b817ff074b76846d636478ac2b54964206ec4ad3e8c98` | `Handle` and per-callback scalar cells; current numeric handle sets |
| [`src/platform/hook_native_callbacks.cpp`](../src/platform/hook_native_callbacks.cpp) | `7b0df0c9e00497cc0fb5fe17574f0fd9576718684d19f8a2a1c4f1bee43dd1ee` | Real `HKEY` / `PHKEY` native entry adapters, without origin tickets |
| [`src/platform/hook_original_effects.cpp`](../src/platform/hook_original_effects.cpp) | `33b5c342470b1b1cc39a610f5770480ead8191f5911eeb32fbf22ab0c4b2a66d` | Exact typed original Win32 API calls; no invented return or handle namespace |

These SHA-256 records identify inspected text bytes, not binary identity, hook execution, real HKEY authentication or an accepted implementation patch.

## Observed contracts

| Domain | Retained donor `registry.cpp` | Accepted product |
| --- | --- | --- |
| Value lookup and A/W matching | Exact case-sensitive map key; caller path compared with delegated case-insensitive CRT behavior; original API called before override | Same source boundaries. [`registry_tests.cpp`](../tests/unit/registry/registry_tests.cpp) already tests A/W case matching and the original-before-override ordering |
| Client/language query | Snapshot incoming size before original call; client waits all steps; language uses tracked set even if flag subsequently changes; original type0 promoted to type1 | Same selected rule; `HandleCell` and `DwordCell` preserve SDK scalar storage sizes and original output identity. No new C++-pointer ABI is inferred |
| **Fabrication error cleanup** | A failed open inserts a fabricated handle and its route membership, without independently undoing a later set/log throw in this retained module | **Intentional product correction:** current `fabricate` saves original result/status and rolls back newly inserted set membership, releases the just-created `operator new(4)` allocation and restores outputs if a later operation throws. This is already fixed in merged [#98](https://github.com/raleonys/citrine-eclipse-mirage/pull/98). Do not revert to donor's shorter sequence |
| Close and numeric reuse | A fabricated handle is removed/freed without forwarding `RegCloseKey`; ordinary close forwards original first; successful close clears route sets | Same numeric membership model, plus product's exception-safe fabrication. Accepted [#99](https://github.com/raleonys/citrine-eclipse-mirage/pull/99) covers isolated native/fabricated A/W lifecycles and numeric reuse after completed close. This does **not** authenticate a same-number real/fabricated collision |

## Issue #109 provenance gate: what cannot be inferred

The current public callback wrappers receive `HKEY`, pointer outputs and no explicit issuer/owner/generation capability. Converting an `HKEY` to a numeric `Handle` preserves its bits, **not** whether it came from a native OS open or the fabricated allocation. `std::set<Handle>` membership is therefore insufficient to resolve a deliberately ambiguous same-number identity. The retained donor uses the same distinction by numeric membership; it does not supply the missing trusted provenance channel.

[Issue #109](https://github.com/raleonys/citrine-eclipse-mirage/issues/109) already records D01 origin/capability, D02 ambiguity, D03 API coverage, D04/D07 retirement and release gates. Open [#113](https://github.com/raleonys/citrine-eclipse-mirage/pull/113) tests **alternative direct SDK handle issuance** and the current callbacks in an isolated HKCU tree; it explicitly does **not** install MinHook or prove Consumer import/dynamic resolution. Do not duplicate #113's SDK calls, introduce speculative origin tags or widen eleven-hook coverage from this document. A real Consumer path or a securely provided provenance carrier remains unverified.

## Next acceptance and disposition

The next distinct evidence for #109 must be an observed native Consumer or equivalent actual-hook trace identifying the HKEY **issuer API, exact callback arrival, handle reuse epoch, all close/query consumers and LastError** through alternate API chains. Separate direct-SDK fixture results from actual hooked execution. If no trusted origin survives across those boundaries, keep the provenance-aware R13 ledger **integration-blocked**; do not guess from pointer ranges or numeric membership. Any later code PR requires a scoped policy decision, a failing current-product test, exact-head MSVC Debug/Release and applicable Native/Golden acceptance.

A11-09 stays open. This source-only PR touches no Registry implementation, native callback, production DLL/ABI, vendor, test, source API, CI, Golden, release or long-running reverse-engineering project branch. **No native Windows/MSVC, Source QA, original donor, Golden or Consumer/game execution is claimed by this document.** Existing previous-head test results are not inherited.
