# Archive 12: QPC and publication crosswalk

**PARTIAL_A12_SOURCE_REVIEW / NO_NEW_PRODUCT_DEFECT_CONFIRMED / RESEARCH_REMAINS_OPEN**

This refreshes existing PR #136, preserving its original research history.
A12 source authority is retained `research/reconstruction-package@56eab8795fa65325e4fa6fe31a645eb55cc58469`;
product baseline is `main@1405c01750317454e8c289a7ebad77391089cdd9`. The eight-part A11 input is a historical
checkpoint, not the current source authority. A12's nine-part combined ZIP
SHA-256 is `539fc80596bfa058afdfe74b01cee95c1c1b807be3e983127510e4dc170acb6d`.

The [A12 registry](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/docs/RE_PACKAGE_IMPORT.json) and
[file inventory](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/docs/RE_PACKAGE_FILES.json) identify 6,514 reference
files. The selected rows below were re-read from that commit and checked for
exact byte lengths/SHA-256 against the inventory. Original split ZIPs were not
locally reassembled. No supplied script, DLL or executable was run. Source and
record claims below are bounded; unchanged hashes do not establish full semantic
review, original ABI parity or native test acceptance.

| Selected A12 input | SHA-256 |
| --- | --- |
| [anadius-execution-source/execution.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-execution-source/execution.cpp) | `c9a30ba892a526b9b00068d7e8d15927a60e2ecf7eda8d23a02bbfa2576f6884` |
| [anadius-execution-source/execution.hpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-execution-source/execution.hpp) | `eb6c3dd0fd279ee9aecafef33c60ba184ab424b3e003143b5271f91ce818ab55` |
| [anadius-startup-publication-source/publication.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-startup-publication-source/publication.cpp) | `7b7a50a8b28541b5b3461439c6918a7055acd8af054344c212e7cb5650bace82` |
| [anadius-startup-publication-source/publication_tests.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-startup-publication-source/publication_tests.cpp) | `7af4e858d48fc91b64989dde02db2fa4cc67de25995cf4c9681e435c78dccd4b` |

| Current product counterpart | SHA-256 at reviewed main |
| --- | --- |
| [src/runtime/execution.cpp](../src/runtime/execution.cpp) | `317dc0c460013ddf621cabc3f9b06d88c6db24fa3728f3be1463cb70d8d02b27` |
| [include/citrine_eclipse_mirage/runtime/execution.hpp](../include/citrine_eclipse_mirage/runtime/execution.hpp) | `0271621e3c5fa9325bd59a7eaf158df56ab53296ffe9d6a7db08551c1c013df8` |
| [src/runtime/application.cpp](../src/runtime/application.cpp) | `89e712de0949f2e6c200f504daa47740254bb49b59f1d91ce6ca5cbf9579f1a8` |
| [src/platform/execution_windows.cpp](../src/platform/execution_windows.cpp) | `0bae86f6b949aec256b34fe45172fdac0a98e21d66c506d562567096c0099a99` |
| [tests/unit/execution/concurrency_tests.cpp](../tests/unit/execution/concurrency_tests.cpp) | `3e03a9e0aa226c819b18eed7f5dafe510debcdde0e9eb1343c7d6019d656f8f9` |

## Rechecked A12 changes

A12 Execution uses short-lived lock-free `atomic_ref<uint32_t>` owner projections before/after the actual SRW owner operations while retaining the recovered raw layout. It also has a recursive SourceGuard around the clock/cache and flag operations. Optional Publication/ReadyStage observers and a bound-port payload distinguish source milestones from controller activation. The publication implementation rejects foreign issuer/reuse and only publishes a readable port snapshot with the real Server milestone. These are source-owner/observer contracts, not new required product exports.

The A12 supplied publication fixture still checks eight callers times one hundred samples and one frequency query under that guarded host owner. The current product instead deliberately permits two frequency getters racing at a cold caller-owned cache, publishing one value atomically. Its existing concurrency fixture asserts this two-query contract. Production Application initializes a positive QPC frequency before launching provider/extra workers, so that particular owner does not enter its startup workers with a cold cache.

## Findings and nonduplication

This remains a source-contract distinction, not proof of a shipping startup race. Preserve accepted #128 publication/payload tests, #243 native SRW handoff work and #347 process-wide fixture thread identities. Do not replace the product's deliberate cold-cache assertion with the donor's single-query assertion. The native Windows API adapter/error handling and externally constructed owners have separate assumptions; this limited review does not prove every error or foreign callback path.

Open #236 already owns the retained research ledger's A12 RPK-0001 recheck. This PR is the product-specific owner/call-site comparison, not a duplicate ledger rewrite. A future production change needs a concrete active-caller requirement or failing native reproducer for the current product, plus lifetime/error evidence. RPK-0001 and complete original-source equivalence remain open.

## Validation boundary and continuation

The selected A12 inventory checks and current product source/test crosswalk were performed for this update. Complete local Source QA and tracked clean-tree pre-push results belong to the updated PR head and are reported in its description. No fresh native Windows/MSVC, Production/Golden, original DLL or Consumer/game execution is claimed locally. Historical donor tests and merged PR receipts are not fresh execution here.

Continue unresolved source/owner questions through the retained reconstruction project and a scoped reproducer where applicable. This review does not close the entire archive/module or authorize source retirement, a runtime policy change, private CRT adoption or public ABI changes. Product/test/vendor bytes, retained reference snapshots and build/CI policy remain unchanged.
