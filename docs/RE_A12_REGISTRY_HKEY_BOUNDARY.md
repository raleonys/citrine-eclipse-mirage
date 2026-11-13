# Archive 12 Registry ownership and source/build boundary

## Current consolidated review (2026-11-13 UTC)

This bounded report is carried by [PR #358](https://github.com/raleonys/citrine-eclipse-mirage/pull/358), preserving [original PR #357](https://github.com/raleonys/citrine-eclipse-mirage/pull/357) at `9b36db2e13fb449178aea47a0448111ebe610c2f`. Product source was rechecked at `35b074519594a3b2d58cac21e07c2a84fb3ad1f1`; reference authority remains A12 `56eab8795fa65325e4fa6fe31a645eb55cc58469`. Documentation acceptance does not complete the research project.

**Current disposition.** All six earlier product counterpart hashes still match accepted main. #337 already scopes the language-log temporary; #240 owns query alias ordering; #98 owns fabrication rollback. #341 has merged and concerns the fixture fault-probe boundary. Issue #109 remains open: scalar HKEY membership and supplied link records do not provide trusted issuer/generation provenance.

**Next evidence gate.** The next evidence is an observed issuer-to-callback/query/close chain with reuse epochs and LastError, through a distinct alternate API or applicable Consumer trace. Instrumentation needs its own bounded design and Release exclusion.

| Product counterpart at current reviewed main | SHA-256 |
| --- | --- |
| [src/platform/registry.cpp](../src/platform/registry.cpp) | `b9a6ff60fa2a1d2c5f472cf1bb425db171d5ce9c7df1db735649c8ecaa8c0d6e` |
| [include/citrine_eclipse_mirage/platform/registry.hpp](../include/citrine_eclipse_mirage/platform/registry.hpp) | `8662cd03498131f45f6f9193b72b35f2479b09e02aaff7eb8d9c774850b6081e` |
| [src/platform/hook_native_callbacks.cpp](../src/platform/hook_native_callbacks.cpp) | `9c1b913d2dbfdc2ac25e23b53357764579eed81e9da57f8be751dfc727251b4c` |
| [src/platform/hook_original_effects.cpp](../src/platform/hook_original_effects.cpp) | `1d950f2f1f7549391c4a4bf18232f64b94048c63dd78e54f278407e434b42dc8` |
| [tests/unit/registry/registry_tests.cpp](../tests/unit/registry/registry_tests.cpp) | `6878178e0658bd2f8f752f1f62ae08aec0f127973b3ae65b2f2cabbaac992f80` |
| [tests/native/application_runtime_tests.cpp](../tests/native/application_runtime_tests.cpp) | `cd4c4de89db63be19b003c1fdf08c40dd78c26ea7a5121e7b1976b3d2827aaa0` |

The selected reference rows were checked against the retained A12 inventory again. Current product deltas were read where hashes changed. Fresh product test results belong to the consolidated PR description and its exact head; historical PASS/NOT_RUN records below are not transferred. No archived program, script or DLL was executed.

## Earlier review checkpoint

The following text preserves the earlier source/product baseline and evidence record from PR #357. Its old open-PR labels, product hashes and depth/profile statements describe that checkpoint; the current disposition above supersedes them.

**PARTIAL_A12_SOURCE_REVIEW / NO_NEW_PRODUCT_DEFECT_CONFIRMED / ORIGIN_EVIDENCE_BLOCKED**

This continues the bounded Registry research accepted in PR #198. Its
[Archive 11 report](RE_A11_09_REGISTRY_HKEY_BOUNDARY.md) remains a historical
checkpoint at its exact original source/product hashes. New work uses A12
`research/reconstruction-package@56eab8795fa65325e4fa6fe31a645eb55cc58469` and product `main@eea758600fe0801ec91c31f85de563afff1d75a7`.
The nine-part A12 ZIP SHA-256 is
`539fc80596bfa058afdfe74b01cee95c1c1b807be3e983127510e4dc170acb6d`.

Ten selected A12 inputs below were re-read and their lengths/SHA-256 verified
against the [A12 file inventory](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/docs/RE_PACKAGE_FILES.json).
This is source and retained-record inspection, not independent verification of
every analysis reply or execution of the donor's scripts/tests/native DLL.
Original ZIP parts were not reassembled. The original binary was not loaded.

| Selected A12 input | SHA-256 |
| --- | --- |
| [anadius-registry-source/README.md](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-registry-source/README.md) | `ddc65f2c14720251899bb9d33c62f1af00b64853c205ca890876d0e0c2b40e93` |
| [anadius-registry-source/registry.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-registry-source/registry.cpp) | `be3a6151a754397231909e4fea9f2ade1e5e3512c3a48fabcb67f12da3701176` |
| [anadius-registry-source/registry.hpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-registry-source/registry.hpp) | `f3393a359bbafe7feef5725aae73967f18e65814cafd0726792b51fbe5716d34` |
| [anadius-registry-source/registry_tests.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-registry-source/registry_tests.cpp) | `8d8bc4717034b6cac5cc3d5893270fde4fe695fe6a02b0f88ca4f745d32450c1` |
| [anadius-registry-source/abi_cells.hpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-registry-source/abi_cells.hpp) | `4fd3d0a6169f8feb22d60f982b94194c3d4b47f2eee1478444e3767070a92351` |
| [anadius-registry-source/language-log-lifetime-20261104/report.md](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-registry-source/language-log-lifetime-20261104/report.md) | `e4c764ff2ac7f39b53f1219cb35f6a8553f41888e44a54fd4dfd467ba5ff10cc` |
| [anadius-registry-source/language-log-lifetime-20261104/evidence.json](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-registry-source/language-log-lifetime-20261104/evidence.json) | `a1b5e937f576706ac55fc82fc7928863fb3a813b7fe29d985dbb9ed9b21bf24f` |
| [source-workspace/history/rebuild-20261105T182000Z/source-snapshot.json](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/source-workspace/history/rebuild-20261105T182000Z/source-snapshot.json) | `b7362cf518727f6fc997bfc1377bc4d83e2751903ead78cf376251cb0bf87e6d` |
| [source-workspace/source-snapshot.json](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/source-workspace/source-snapshot.json) | `9199f873c0285d6614d033675d844e1e691c73ca686d3bef365b38923f077a90` |
| [source-workspace/build-record.json](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/source-workspace/build-record.json) | `359927eea05c1d1f6dddc99be05a4a4f0df393ff82ac4bc9e5e2355a15ad7252` |

## New A12 evidence and accepted product counterpart

A12's shared A/W query places language resolve/log/finish in a nested block,
ending the narrow temporary before invoking the replacement value writer.
Its retained static report identifies the selected finish-log, conditional heap
free and replacement-writer addresses for both A and W. Inline strings skip the
heap free; this does not mean the temporary survives into the writer. Provider
terminal/no-return assumptions remain assumptions rather than original exception
or private CRT proof.

The accepted product already has this scope and lifetime order in
`src/platform/registry.cpp`, through merged #337. Do not create a duplicate fix
or infer that the current product retains the earlier temporary. Preserve the
original-capacity snapshot, original-call-before-override, client-before-language
selection, writer-before-type-zero update and fabricated-key fallback. Accepted
#240 separately covers query output alias ordering. No new product divergence
is confirmed in these selected paths.

The earlier static Registry report (4 November) records no rebuild at that
stage. It must not be treated as the status of the later 5 November linked
image. The current source-snapshot hash is
`9199f873c0285d6614d033675d844e1e691c73ca686d3bef365b38923f077a90`,
exactly the source snapshot named in the current full-link receipt. Its Registry
row matches the current A12 `registry.cpp` hash in the table above; the prior
retained snapshot has the old A11 `757e2ffb...` hash. The common build record's
actual bytes also match the hash named by both the current snapshot and link
receipt. These byte/record links were independently checked for this review.

Thus the earlier no-build stage is superseded by later supplied build records
for this selected source. It is not proof that the current DLL omits the edit.
Conversely these identities do not establish independent binary behavior,
reexecute the linker, verify every object/toolchain input or prove runtime
and original ABI equivalence. Keep source/static, supplied build records and
fresh native/Consumer execution as separate evidence classes.

## Ownership and deliberate correction

A12 still uses three numeric handle sets and scalar cells with caller-owned
storage. It does not add a trusted issuer/owner/generation carrier at the public
HKEY callback ingress. Scalar-cell copying addresses SDK C++ aliasing/storage
identity; it does not authenticate where a same-number handle originated.

The product's accepted #98 exception-safe fabrication remains a deliberate
correction beyond A12's shorter allocation/insert/log sequence: preserve newly
inserted membership rollback, output/status restoration and release of the
just-created allocation on the supported throw boundary. It is not an arbitrary
fallible-free retry protocol. Preserve #99's lifecycle coverage rather than
import the donor sequence or invent origin tags.

Issue #109 remains open. Accepted #113 supplies direct SDK A/W issuance, while
#224 and #255 supply controlled installed-hook W/provider and A/W/mixed-handle
paths. Those are distinct evidence classes, already present; do not propose
the same smoke tests as missing. Open #341 concerns disarming a fixture fault
probe before assertion and does not prove or repair HKEY provenance.

## Remaining evidence and disposition

A distinct next scenario needs observed issuer API, actual callback ingress,
handle reuse epoch, all query/close consumers and LastError through an alternate
API chain or applicable real Consumer trace. A controlled numeric collision is
not proof of a real OS alias, and numeric membership alone cannot resolve a
missing provenance capability. Generic callback concurrency, foreign callers,
private allocator/CRT/exception behavior and original/game equivalence remain
unproved. Keep #109's evidence/design gates and the retained research open.

This PR adds no instrumentation or runtime policy. A diagnostic would need its
own bounded Debug-only design, original-call/LastError preservation, explicit
dropped-record handling and verified Production/Release exclusion. Do not
weaken eleven-hook/ABI policy or infer universal safety from a quiet trace.

## Product identity and validation boundary

The product paths below were read at the pinned main; tests and accepted PR
receipts are crosswalks rather than freshly executed native acceptance here.
Complete local Source QA and clean-tree pre-push for this documentation head
are reported in its PR description. No donor/native Windows/MSVC, production
Golden, original DLL, loader, real hook trace or Consumer/game run is claimed.

| Product counterpart | SHA-256 at reviewed main |
| --- | --- |
| [src/platform/registry.cpp](../src/platform/registry.cpp) | `b9a6ff60fa2a1d2c5f472cf1bb425db171d5ce9c7df1db735649c8ecaa8c0d6e` |
| [include/citrine_eclipse_mirage/platform/registry.hpp](../include/citrine_eclipse_mirage/platform/registry.hpp) | `8662cd03498131f45f6f9193b72b35f2479b09e02aaff7eb8d9c774850b6081e` |
| [src/platform/hook_native_callbacks.cpp](../src/platform/hook_native_callbacks.cpp) | `9c1b913d2dbfdc2ac25e23b53357764579eed81e9da57f8be751dfc727251b4c` |
| [src/platform/hook_original_effects.cpp](../src/platform/hook_original_effects.cpp) | `1d950f2f1f7549391c4a4bf18232f64b94048c63dd78e54f278407e434b42dc8` |
| [tests/unit/registry/registry_tests.cpp](../tests/unit/registry/registry_tests.cpp) | `6878178e0658bd2f8f752f1f62ae08aec0f127973b3ae65b2f2cabbaac992f80` |
| [tests/native/application_runtime_tests.cpp](../tests/native/application_runtime_tests.cpp) | `cd4c4de89db63be19b003c1fdf08c40dd78c26ea7a5121e7b1976b3d2827aaa0` |
