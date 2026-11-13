# Archive 11 A11-09 — Registry HKEY ownership and ABI crosswalk

**SELECTED_SOURCE_REVIEW / NO_SHIPPING_DEFECT_CONFIRMED / REGISTRY_ORIGIN_EVIDENCE_BLOCKED**

This is a **single C++ donor-source crosswalk**, not completion of Archive 11's 199 module groups / 4,266 reviewed-file candidates. Source comparison baseline: accepted `main@f0f82c81523beaa6b809f5e3b455452d40059d0a`. Retained donor [`anadius-registry-source/registry.cpp`](https://github.com/raleonys/citrine-eclipse-mirage/blob/e10561b256a5793f857ec4bed4b5a9fb9fcfbadd/reference-package/snapshot/anadius-registry-source/registry.cpp) SHA-256 `757e2ffb52077852d9ca509635a10bc5191f2176f6d05a1dd8c88131965680f5`, verified against the [retained file ledger](https://github.com/raleonys/citrine-eclipse-mirage/blob/e10561b256a5793f857ec4bed4b5a9fb9fcfbadd/docs/RE_ARCHIVE11_FILES.json). Retained Archive 11 ZIP SHA-256: `db394434e38c7edad33607a846ffdc36b04a1e5978d2fbe3d74da24bda5179da`; do not confuse with the historical different B49–B54 ZIP ([#115](https://github.com/raleonys/citrine-eclipse-mirage/pull/115)).

The donor and file-ledger links are pinned to retained Archive 11 commit `e10561b256a5793f857ec4bed4b5a9fb9fcfbadd`. The moving `research/reconstruction-package` head now contains an Archive 12 refresh; its Registry source differs from this Archive 11 hash and is not substituted into this comparison.

## Source identities at the reviewed baseline

| Product source | SHA-256 (reviewed baseline) | Role |
| --- | --- | --- |
| [`src/platform/registry.cpp`](../src/platform/registry.cpp) | `5b5fbdf646f1c8a2169078c97696e74f5e43fc9c80640201bc49581add4c1f48` | Value comparisons, original-first open/query, fabricate/close state |
| [`include/citrine_eclipse_mirage/platform/registry.hpp`](../include/citrine_eclipse_mirage/platform/registry.hpp) | `c66f67c0ef14edeb606b817ff074b76846d636478ac2b54964206ec4ad3e8c98` | `Handle` and per-callback scalar cells; current numeric handle sets |
| [`src/platform/hook_native_callbacks.cpp`](../src/platform/hook_native_callbacks.cpp) | `7b0df0c9e00497cc0fb5fe17574f0fd9576718684d19f8a2a1c4f1bee43dd1ee` | Real `HKEY` / `PHKEY` native entry adapters, without origin tickets |
| [`src/platform/hook_original_effects.cpp`](../src/platform/hook_original_effects.cpp) | `33b5c342470b1b1cc39a610f5770480ead8191f5911eeb32fbf22ab0c4b2a66d` | Exact typed original Win32 API calls; no invented return or handle namespace |

The table hashes belong to `f0f82c81523beaa6b809f5e3b455452d40059d0a`; local
links navigate current paths, not byte-identical current sources. These records
identify inspected text bytes, not binary identity, hook execution, real HKEY authentication or an accepted implementation patch.

## Observed contracts

| Domain | Retained donor `registry.cpp` | Accepted product |
| --- | --- | --- |
| Value lookup and A/W matching | Exact case-sensitive map key; caller path compared with delegated case-insensitive CRT behavior; original API called before override | Same source boundaries. [`registry_tests.cpp`](../tests/unit/registry/registry_tests.cpp) already tests A/W case matching and the original-before-override ordering |
| Client/language query | Snapshot incoming size before original call; client waits all steps; language uses tracked set even if flag subsequently changes; original type0 promoted to type1 | Same selected rule; `HandleCell` and `DwordCell` preserve SDK scalar storage sizes and original output identity. No new C++-pointer ABI is inferred |
| **Fabrication error cleanup** | A failed open inserts a fabricated handle and its route membership, without independently undoing a later set/log throw in this retained module | **Intentional product correction:** current `fabricate` saves original result/status and rolls back newly inserted set membership, releases the just-created `operator new(4)` allocation and restores outputs if a later operation throws. This is already fixed in merged [#98](https://github.com/raleonys/citrine-eclipse-mirage/pull/98). Do not revert to donor's shorter sequence |
| Close and numeric reuse | A fabricated handle is removed/freed without forwarding `RegCloseKey`; ordinary close forwards original first; successful close clears route sets | Same numeric membership model, plus product's exception-safe fabrication. Accepted [#99](https://github.com/raleonys/citrine-eclipse-mirage/pull/99) covers isolated native/fabricated A/W lifecycles and numeric reuse after completed close. This does **not** authenticate a same-number real/fabricated collision |

## Issue #109 provenance gate: what cannot be inferred

The current public callback wrappers receive `HKEY`, pointer outputs and no explicit issuer/owner/generation capability. Converting an `HKEY` to a numeric `Handle` preserves its bits, **not** whether it came from a native OS open or the fabricated allocation. `std::set<Handle>` membership is therefore insufficient to resolve a deliberately ambiguous same-number identity. The retained donor uses the same distinction by numeric membership; it does not supply the missing trusted provenance channel.

[Issue #109](https://github.com/raleonys/citrine-eclipse-mirage/issues/109) already records D01 origin/capability, D02 ambiguity, D03 API coverage, D04/D07 retirement and release gates. Merged [#113](https://github.com/raleonys/citrine-eclipse-mirage/pull/113) tests **alternative direct SDK handle issuance** and the current callbacks in an isolated HKCU tree; it explicitly does **not** install MinHook or prove Consumer import/dynamic resolution. Its head `99c3fbbbbfeb2351c69dda9d19bca233aba8c2d9` was integrated as `f0f82c81523beaa6b809f5e3b455452d40059d0a`. Do not duplicate #113's SDK calls, introduce speculative origin tags or widen eleven-hook coverage from this document. A real Consumer path or a securely provided provenance carrier remains unverified.

## Accepted fixture evidence and remaining gates

The exact #113 head completed [full validation run 42216368256](https://github.com/raleonys/citrine-eclipse-mirage/actions/runs/42216368256): all eleven required jobs passed, including Native Debug and Release with 217/217 CTests each, six Production/Golden stages and the final-name runtime check. Native JUnit artifacts 13502995881 (Debug) and 13500949676 (Release) each contain separate `PASS: alternate SDK Registry routes A` and `PASS: alternate SDK Registry routes W` lines and 362 lifecycle checks. Their download digests and receipt source-manifest identities were verified. These are historical receipts for that exact head, not validation inherited by this documentation PR.

Accepted [#224](https://github.com/raleonys/citrine-eclipse-mirage/pull/224) also extends the existing [native application fixture](../tests/native/application_runtime_tests.cpp): the real application installs its hooks, a W SDK open/query/close path reaches configured Registry state, and a loaded provider reenters that path through its auth getter. The disabled-provider probe is a negative control. This is already installed-hook evidence for that controlled W path; it does not inventory alternate A/W API chains or the real Consumer. Preserve and extend this fixture rather than repeat its existing smoke test.

| Issue #109 gate | Evidence established | Remaining work |
| --- | --- | --- |
| D01 origin/capability | Raw HKEY-to-Handle boundary and source identities reviewed | Trusted origin/owner/generation transport or a rigorously bounded alternative; root/parent observations do not establish authorization |
| D02 ambiguity | Accepted native/fabricated lifecycles and bounded completed-close numeric reuse | Distinguish synthetic same-number counterexamples from an actual OS alias; preserve original-call identity, LastError and ownership through relevant transitions |
| D03 API coverage | #113 direct SDK A/W issuance/core callbacks; #224 installed-hook W/provider reentry; accepted #255 also adds installed A/W and mixed-handle cases in that application fixture | Remaining alternate A/W API chains, observed callback ingress, dynamic resolution and applicable Consumer coverage |
| D04/D07 retirement | #98 failure-atomic fabrication and #99 lifecycle coverage retained | Proven origin/coverage followed by a reviewed ownership/retirement design, concurrent schedules and applicable native acceptance |

All four research gates remain open. Neither passing CI nor this source comparison confirms a shipping collision or closes Issue #109.

## Next acceptance and disposition

The next distinct evidence for #109 must be an observed native Consumer or equivalent actual-hook trace identifying the HKEY **issuer API, exact callback arrival, handle reuse epoch, all close/query consumers and LastError** through alternate API chains. Separate direct-SDK fixture results from actual hooked execution. If no trusted origin survives across those boundaries, keep the provenance-aware R13 ledger **integration-blocked**; do not guess from pointer ranges or numeric membership. Any later production-fix PR requires a scoped policy decision, a failing current-product test, exact-head MSVC Debug/Release and applicable Native/Golden acceptance.

The formerly proposed installed-hook A/W extension was integrated in
[PR #255](https://github.com/raleonys/citrine-eclipse-mirage/pull/255).
`tests/native/application_runtime_tests.cpp` retains the W/provider controls
and now exercises ANSI open/query/close, mixed A/W handle use, size-only and
undersized buffers, native-codepage client paths and unrelated-value refusal.
These are controlled fixture paths, not a complete alternate-API or Consumer
trace. Do not recreate them as a missing-test PR. The next distinct evidence is
an actually observed alternate issuer-to-callback chain with the identities and
retirement observations above. Keep physical hook installation and direct-core
fixture evidence separately identified.

If Consumer observation is then needed, implement opt-in diagnostics compiled only into Debug, with a machine-readable file receipt so the game need not print output. Record source/build identity, observed issuer/callback sequence, handle generations as observations, operation status and LastError, and make dropped/incomplete records explicit. Preserve original calls and LastError; diagnostics must not supply invented provenance or change routing. Verify compile-time exclusion from Production/Release. This instrumentation is proposed follow-up work, not included or executed here.

Use those observations to decide the remaining design: a reproducible shipping defect needs a focused failing regression before a fix; a surviving provenance gap keeps the R13 ledger blocked. Neither a successful controlled trace nor failure to observe a collision justifies universal safety claims. Close Issue #109 only when its evidence/design criteria are satisfied or the owner explicitly accepts a documented bounded disposition.

A11-09 stays open. This source-only PR touches no Registry implementation, native callback, production DLL/ABI, vendor, test, source API, CI, Golden, release or long-running reverse-engineering project branch. Source QA and hosted validation for this PR must be reported against its exact head in the PR description. Original-donor execution and real Consumer/game execution remain NOT RUN; previous-head results are not inherited.
