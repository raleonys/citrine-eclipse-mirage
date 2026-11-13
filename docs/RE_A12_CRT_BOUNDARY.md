# Archive 12: private CRT applicability

## Current consolidated review (2026-11-13 UTC)

This bounded report is carried by [PR #358](https://github.com/raleonys/citrine-eclipse-mirage/pull/358), preserving [original PR #139](https://github.com/raleonys/citrine-eclipse-mirage/pull/139) at `e2edb60208422a2e49de2cf96c8ad2375c27ff51`. Product source was rechecked at `35b074519594a3b2d58cac21e07c2a84fb3ad1f1`; reference authority remains A12 `56eab8795fa65325e4fa6fe31a645eb55cc58469`. Documentation acceptance does not complete the research project.

**Current disposition.** The two product source hashes recorded earlier remain unchanged. Standard compiler CRT remains accepted. #343 has merged: portable bootstrap/thread models are existing controlled coverage. It does not establish private PTD/FLS/allocator ABI equivalence. The six selected A12 README files are an applicability sample rather than a full private-CRT implementation review.

**Next evidence gate.** Retain standard CRT. Any useful additional test needs an actual product entry, valid fault contract and lifetime observation; complete private-CRT equivalence is not a requirement established by this report.

| Product counterpart at current reviewed main | SHA-256 |
| --- | --- |
| [tests/unit/thread/thread_tests.cpp](../tests/unit/thread/thread_tests.cpp) | `6ee291034beb8675d7277204ada62ae836cdb25da1a79a8962fcf44fb1087ed0` |
| [metadata/CRT_POLICY.json](../metadata/CRT_POLICY.json) | `3c959c822cd35ff50a03724ca16337380aa283b1eeb43b3f85f4c4b16f7377df` |

The selected reference rows were checked against the retained A12 inventory again. Current product deltas were read where hashes changed. Fresh product test results belong to the consolidated PR description and its exact head; historical PASS/NOT_RUN records below are not transferred. No archived program, script or DLL was executed.

## Earlier review checkpoint

The following text preserves the earlier source/product baseline and evidence record from PR #139. Its old open-PR labels, product hashes and depth/profile statements describe that checkpoint; the current disposition above supersedes them.

**PARTIAL_A12_SOURCE_REVIEW / NO_NEW_PRODUCT_DEFECT_CONFIRMED / RESEARCH_REMAINS_OPEN**

This refreshes existing PR #139, preserving its original research history.
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
| [anadius-crt-ptd-source/README.md](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-crt-ptd-source/README.md) | `ed6f77bb685b0ab30e00804b01e932e06fe3b6fea5bdc0b31c86732973da530b` |
| [anadius-crt-fls-native-source/README.md](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-crt-fls-native-source/README.md) | `7ed4ca215f62f143651dbee5ebe683cebdf2e9b112ac1be21b551caf3131e18a` |
| [anadius-crt-heap-native-source/README.md](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-crt-heap-native-source/README.md) | `ff01248a4de4daadaf1d9afee8961679b21a23dd21b2cd92ff0a869ab1f1b035` |
| [anadius-crt-onexit-source/README.md](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-crt-onexit-source/README.md) | `58bb1b9e3dc40f175995d80a36daf2339ec83b0d2ca518b7dee858ac106c6207` |
| [anadius-crt-termination-source/README.md](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-crt-termination-source/README.md) | `ba9858266f98ef79250406dc444b916e91ec7fae6374a4187053c685d54dde10` |
| [anadius-crt-error-source/README.md](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-crt-error-source/README.md) | `22b90e57d2a60b7e82360d0ed6b15df53d159db43e49e32862a27190479710bb` |

| Current product counterpart | SHA-256 at reviewed main |
| --- | --- |
| [CMakeLists.txt](../CMakeLists.txt) | `5f2c3f94b3a1247b312f527df639bf319b55390cdee1ab32de9120e8ec580694` |
| [tools/citrine_eclipse_mirage_crt_host/main.cpp](../tools/citrine_eclipse_mirage_crt_host/main.cpp) | `deabcd42a1fea2df29b9f080017160be0f222fef11837b5b03c76846d9395cf6` |
| [tests/unit/thread/thread_tests.cpp](../tests/unit/thread/thread_tests.cpp) | `6ee291034beb8675d7277204ada62ae836cdb25da1a79a8962fcf44fb1087ed0` |
| [metadata/CRT_POLICY.json](../metadata/CRT_POLICY.json) | `3c959c822cd35ff50a03724ca16337380aa283b1eeb43b3f85f4c4b16f7377df` |

## Scope of the A12 recheck

The selected A12 inputs are six module descriptions: PTD, native FLS, native heap, onexit, termination and CRT error cells. This is explicitly a description-level applicability sample, not an implementation-line review of the private CRT archive. Their supplied native/test PASS counts are historical reference claims, not newly executed product results.

The FLS description distinguishes the recovered uninitializer ignoring FlsFree from an added native binding that retains its exact slot and dependencies for retry. It assumes one active domain and stopped users. Heap documentation describes typed Windows API forwarding without invented allocator ownership; PTD/error descriptions bind private layout/cells. Onexit/termination descriptions concern reconstructed callback/exit ownership. These remain useful questions but do not establish the same compiler-CRT ABI or application lifetime graph.

## Current product policy and coverage

Current CMake retains compiler CRT `/MT` and `/MTd` for Production/regular tests and matching `/MD` and `/MDd` across the separate C++ development DLL/host. The renamed `citrine_eclipse_mirage_crt_host` checks owned allocation, unwind, containers/thread and a thread_local destructor; the reference thread fixture uses selected Effects. Neither is proof of private PTD/FLS offsets, allocator ABI, onexit-cookie behavior or arbitrary process-exit equivalence.

Merged #76 addresses the defaults Debug harness boundary, #102 public shutdown during startup and #125 owned module-stage retries. Existing #132 covers defaults comparison. Open #343 adds portable execution of reference thread/bootstrap models; it is not a replacement CRT and not native compiler FLS acceptance. Do not open duplicate tests or import private CRT objects from a same-named operation.

A new executable scenario must identify a current application-owned entry, owner, valid fault contract, negative control and native teardown observation. Complete donor CRT semantic review, native boundary equivalence and Consumer/game acceptance remain open. No production defect or permission to replace the standard CRT is inferred.

## Validation boundary and continuation

The selected A12 inventory checks and current product source/test crosswalk were performed for this update. Complete local Source QA and tracked clean-tree pre-push results belong to the updated PR head and are reported in its description. No fresh native Windows/MSVC, Production/Golden, original DLL or Consumer/game execution is claimed locally. Historical donor tests and merged PR receipts are not fresh execution here.

Continue unresolved source/owner questions through the retained reconstruction project and a scoped reproducer where applicable. This review does not close the entire archive/module or authorize source retirement, a runtime policy change, private CRT adoption or public ABI changes. Product/test/vendor bytes, retained reference snapshots and build/CI policy remain unchanged.
