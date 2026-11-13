# Archive 12: defaults and allocation boundaries

## Current consolidated review (2026-11-13 UTC)

This bounded report is carried by [PR #358](https://github.com/raleonys/citrine-eclipse-mirage/pull/358), preserving [original PR #132](https://github.com/raleonys/citrine-eclipse-mirage/pull/132) at `73cc2605c26fc7b7e44ac9c9d7b31fae2126c658`. Product source was rechecked at `35b074519594a3b2d58cac21e07c2a84fb3ad1f1`; reference authority remains A12 `56eab8795fa65325e4fa6fe31a645eb55cc58469`. Documentation acceptance does not complete the research project.

**Current disposition.** All five product counterpart hashes in the earlier review still match accepted main. Product branding/defaults, independent factory state and the supported allocation-failure boundaries remain intentional. The MSVC Debug constructor boundary from #76 must not be replaced by an unsafe throwing probe.

**Next evidence gate.** A new scenario requires a reachable failure point and a discriminating assertion; terminal noexcept construction needs an isolated process expectation rather than a fabricated recovery path.

| Product counterpart at current reviewed main | SHA-256 |
| --- | --- |
| [src/runtime/application_defaults.cpp](../src/runtime/application_defaults.cpp) | `75093d8c8f0de28b0b2a69f367ec37fc040e0c542335ddb4343a3e637da78979` |
| [include/citrine_eclipse_mirage/config/configuration.hpp](../include/citrine_eclipse_mirage/config/configuration.hpp) | `996ca52c402a6419fa0a1fe10d05a00cf40c828da97ee383642c4fedbfc984eb` |
| [include/citrine_eclipse_mirage/core/protocol_identity.hpp](../include/citrine_eclipse_mirage/core/protocol_identity.hpp) | `c2bc3e7cb25ab40a614c3b79667994fe2e53eee6abbb825b41c2af8ce3ac4d8a` |
| [tests/unit/config_defaults/defaults_tests.cpp](../tests/unit/config_defaults/defaults_tests.cpp) | `3ec76e8dce9a3f140420f8c2bb1e6ecae10b0792b7a67455dc3f3a010dbee0b1` |
| [tests/unit/config_defaults/defaults_integration_tests.cpp](../tests/unit/config_defaults/defaults_integration_tests.cpp) | `09f715fcdfd28ad90090d77a8d6ab7798159a2fb4a5ff41f9037f2c50c109d7b` |

The selected reference rows were checked against the retained A12 inventory again. Current product deltas were read where hashes changed. Fresh product test results belong to the consolidated PR description and its exact head; historical PASS/NOT_RUN records below are not transferred. No archived program, script or DLL was executed.

## Earlier review checkpoint

The following text preserves the earlier source/product baseline and evidence record from PR #132. Its old open-PR labels, product hashes and depth/profile statements describe that checkpoint; the current disposition above supersedes them.

**PARTIAL_A12_SOURCE_REVIEW / NO_NEW_PRODUCT_DEFECT_CONFIRMED / RESEARCH_REMAINS_OPEN**

This refreshes existing PR #132, preserving its original research history.
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
| [anadius-config-defaults-source/defaults.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-config-defaults-source/defaults.cpp) | `4ae57170b870bcde9f3a6c0c1ee0f5f2e5ee48e631fc1876a6256fbb6fb7d6c8` |
| [anadius-config-defaults-source/defaults_tests.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-config-defaults-source/defaults_tests.cpp) | `3f33c443d3eae2de0dc02612e4368b3a16481e93433a2de5c30d387306e15d00` |

| Current product counterpart | SHA-256 at reviewed main |
| --- | --- |
| [src/runtime/application_defaults.cpp](../src/runtime/application_defaults.cpp) | `75093d8c8f0de28b0b2a69f367ec37fc040e0c542335ddb4343a3e637da78979` |
| [include/citrine_eclipse_mirage/config/configuration.hpp](../include/citrine_eclipse_mirage/config/configuration.hpp) | `996ca52c402a6419fa0a1fe10d05a00cf40c828da97ee383642c4fedbfc984eb` |
| [include/citrine_eclipse_mirage/core/protocol_identity.hpp](../include/citrine_eclipse_mirage/core/protocol_identity.hpp) | `c2bc3e7cb25ab40a614c3b79667994fe2e53eee6abbb825b41c2af8ce3ac4d8a` |
| [tests/unit/config_defaults/defaults_tests.cpp](../tests/unit/config_defaults/defaults_tests.cpp) | `3ec76e8dce9a3f140420f8c2bb1e6ecae10b0792b7a67455dc3f3a010dbee0b1` |
| [tests/unit/config_defaults/defaults_integration_tests.cpp](../tests/unit/config_defaults/defaults_integration_tests.cpp) | `09f715fcdfd28ad90090d77a8d6ab7798159a2fb4a5ff41f9037f2c50c109d7b` |

## Rechecked defaults and response boundaries

The A12 factory still initializes named empty state, content ID zero, numeric persona/user identities, enabled fake-auth/pretend-connected, six emulator attributes and fourteen game attributes. It supplies username `anadius` and a separate configuration SDK value `10,5,96,47512`. Product defaults deliberately select `SURGE`; its LSX Challenge SDK identity is separately specified by protocol_identity. A field in one graph is not proof that a similarly named assignment is missing in the other.

Current defaults and integration fixtures already check empty paths/containers, zero not-yet-bound port, account/language fields, exact attribute sets, independently mutable factory results and default/override serialized responses. The A12 source and supplied assertions were read; supplied test counts were not rerun or inherited.

## Allocation-failure disposition

The product test explicitly treats MSVC Debug iterator-proxy/default-string allocation inside `noexcept` construction separately. Its Debug path constructs State before arming throwing map-insertion failures; non-MSVC-Debug follows the broader factory allocation loop. Merged #76 documents and repairs the original invalid fault-injection boundary. This is not evidence that the shipping factory leaks or that intentionally terminating standard-library behavior must become recoverable.

Retain standard CRT, branding and SDK policy. An additional allocation test requires a reachable supported failure boundary, isolated terminal-process expectations where applicable, and fresh native Debug/Release evidence. Recreating the old unsafe constructor probe, replacing CRT, or copying the donor's call-count oracle would not resolve this proof boundary. A12 equivalence remains partial rather than a newly confirmed production defect.

## Validation boundary and continuation

The selected A12 inventory checks and current product source/test crosswalk were performed for this update. Complete local Source QA and tracked clean-tree pre-push results belong to the updated PR head and are reported in its description. No fresh native Windows/MSVC, Production/Golden, original DLL or Consumer/game execution is claimed locally. Historical donor tests and merged PR receipts are not fresh execution here.

Continue unresolved source/owner questions through the retained reconstruction project and a scoped reproducer where applicable. This review does not close the entire archive/module or authorize source retirement, a runtime policy change, private CRT adoption or public ABI changes. Product/test/vendor bytes, retained reference snapshots and build/CI policy remain unchanged.
