# Archive 12: shutdown ownership review

## Current consolidated review (2026-11-13 UTC)

This bounded report is carried by [PR #358](https://github.com/raleonys/citrine-eclipse-mirage/pull/358), preserving [original PR #119](https://github.com/raleonys/citrine-eclipse-mirage/pull/119) at `2d5f34f5a9f29d5bd087ab08da24ea69e9cce756`. Product source was rechecked at `35b074519594a3b2d58cac21e07c2a84fb3ad1f1`; reference authority remains A12 `56eab8795fa65325e4fa6fe31a645eb55cc58469`. Documentation acceptance does not complete the research project.

**Current disposition.** The worker-owned Application, ordered stop stages, retained handles and explicit retry remain the product contract. The Application change since the earlier review is accepted #350 UnknownMessage dispatch; the Asio changes from #344/#336 add controlled client-list/thread seams while production retains std::vector/std::thread. They do not replace stop ordering. The recovery matrix #280 is accepted, not a missing proposal. Accepted #147 adds a fixed Debug scan record and host reporting after shutdown returns; it did not establish a cause for the historical intermittent refusal.

**Next evidence gate.** A further teardown claim needs a distinct supported failing stage, exact retained owner and repaired retry. Foreign caller quiescence and actual Consumer unload remain separate observations.

| Product counterpart at current reviewed main | SHA-256 |
| --- | --- |
| [src/platform/dll_entry.cpp](../src/platform/dll_entry.cpp) | `d638b43a6b627d0182a9eb63590cd7c4eaad3c81d67f4265e1318566d238c84d` |
| [src/runtime/application.cpp](../src/runtime/application.cpp) | `7a2be80fd20f60a6e5b279ff8836c4be875040b5f3f32692219cce9265e4348b` |
| [src/net/asio_service.cpp](../src/net/asio_service.cpp) | `5b4f8de8607a3978c53caf0926e11a0385644fb8944846b85fc664d9db12c070` |
| [tests/native/application_shutdown_failure_tests.cpp](../tests/native/application_shutdown_failure_tests.cpp) | `cf6859408767beed320bbf679b866f1377aa0597ee97d04c5701215aecf6eb3b` |

The selected reference rows were checked against the retained A12 inventory again. Current product deltas were read where hashes changed. Fresh product test results belong to the consolidated PR description and its exact head; historical PASS/NOT_RUN records below are not transferred. No archived program, script or DLL was executed.

## Earlier review checkpoint

The following text preserves the earlier source/product baseline and evidence record from PR #119. Its old open-PR labels, product hashes and depth/profile statements describe that checkpoint; the current disposition above supersedes them.

**PARTIAL_A12_SOURCE_REVIEW / NO_NEW_PRODUCT_DEFECT_CONFIRMED / RESEARCH_REMAINS_OPEN**

This refreshes existing PR #119, preserving its original research history.
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
| [anadius-pre-unload-source/shutdown.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-pre-unload-source/shutdown.cpp) | `2653388e3e007376b9e038648b3a3f34d29daa45e604cc7dd11f4455c7664d25` |
| [anadius-pre-unload-source/shutdown.hpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-pre-unload-source/shutdown.hpp) | `d4feb92ad786738bfa22f818ddff9e814576f4b8bcda21cf3934210f112e699e` |
| [anadius-bootstrap-runtime-source/factory.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-bootstrap-runtime-source/factory.cpp) | `65340d53acefe5bc2fa10024b78abcaff852200369d63677fb013c10646f44a5` |

| Current product counterpart | SHA-256 at reviewed main |
| --- | --- |
| [src/platform/dll_entry.cpp](../src/platform/dll_entry.cpp) | `d638b43a6b627d0182a9eb63590cd7c4eaad3c81d67f4265e1318566d238c84d` |
| [src/runtime/application.cpp](../src/runtime/application.cpp) | `89e712de0949f2e6c200f504daa47740254bb49b59f1d91ce6ca5cbf9579f1a8` |
| [src/net/asio_service.cpp](../src/net/asio_service.cpp) | `6f21224c3e0fd5c4ce757ce3d028c6e7e1982b7198ec09454bd635ff7833b8b8` |
| [tests/native/application_shutdown_failure_tests.cpp](../tests/native/application_shutdown_failure_tests.cpp) | `cf6859408767beed320bbf679b866f1377aa0597ee97d04c5701215aecf6eb3b` |

## Rechecked owner boundaries

A12's cooperative `pre_unload::Controller` retains Runtime and Domain owners, requires its original controller thread/fiber, and distinguishes pending close from a sticky poisoned state after unknown failures. Its header excludes DllMain/loader-lock/process-termination/CRT-exit use. The selected Runtime factory closes source workers before source exit cleanup; this does not establish whole-image foreign quiescence.

The product has a different owner graph: one DLL worker owns Application; its stop seals session admission, joins owned workers, retires Asio, runs file recovery, quiesces hooks, releases extra DLL leases and closes the provider. A failed earlier stage prevents later stages during that request. The worker retains the same Application for another explicit request. A12's Controller/fiber ABI is not imported.

## Corrected completion and overlap

The old report's A11-06-X1 is no longer an unimplemented test proposal. Merged #168 now has `staged_file_hook_public_shutdown`: the real file recovery returns, the fixture injects a boundary exception, the next explicit request encounters hook-disable failure, and the final request retires the same owner. It is **not** a failed-native-rollback test. Merged #125 covers extra/provider module-release staging; #160 quarantines an unknown provider-destruction result; #172 tests post-load allocation failures. Merged #101/#102/#104/#105/#106/#107 own separate provider/startup/socket schedules.

Actual rollback-matrix work is already open in #280. Do not duplicate those scenarios or call A12's cooperative close a replacement for the public worker. Further tests need a distinct reachable boundary and discriminating assertions. Arbitrary foreign/provider activity, complete cross-stage native failure combinations and real Consumer unload equivalence remain open; no production bug is established by this review.

## Validation boundary and continuation

The selected A12 inventory checks and current product source/test crosswalk were performed for this update. Complete local Source QA and tracked clean-tree pre-push results belong to the updated PR head and are reported in its description. No fresh native Windows/MSVC, Production/Golden, original DLL or Consumer/game execution is claimed locally. Historical donor tests and merged PR receipts are not fresh execution here.

Continue unresolved source/owner questions through the retained reconstruction project and a scoped reproducer where applicable. This review does not close the entire archive/module or authorize source retirement, a runtime policy change, private CRT adoption or public ABI changes. Product/test/vendor bytes, retained reference snapshots and build/CI policy remain unchanged.
