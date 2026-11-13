# Archive 12: admission and shutdown boundaries

**PARTIAL_A12_SOURCE_REVIEW / NO_NEW_PRODUCT_DEFECT_CONFIRMED / RESEARCH_REMAINS_OPEN**

This refreshes existing PR #141, preserving its original research history.
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
| [anadius-application-admission-source/admission_tests.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-application-admission-source/admission_tests.cpp) | `75c3fd2ff53f65966b18da309ce8d1be8727dc3e1aee490fe986b235c530ac78` |
| [anadius-application-admission-source/admission_integration_tests.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-application-admission-source/admission_integration_tests.cpp) | `a51340c165d8d0449a98741fea9f07bf8becacb3d68bb16dc06e5f226cb8ecd1` |
| [anadius-native-source/native_callbacks.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-native-source/native_callbacks.cpp) | `2d37a2dba5a1f7cbcb2c47f0791edcb79153e4fd6abc6d5040168733e9b3d85f` |
| [anadius-bootstrap-runtime-source/factory.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-bootstrap-runtime-source/factory.cpp) | `65340d53acefe5bc2fa10024b78abcaff852200369d63677fb013c10646f44a5` |

| Current product counterpart | SHA-256 at reviewed main |
| --- | --- |
| [src/platform/dll_entry.cpp](../src/platform/dll_entry.cpp) | `d638b43a6b627d0182a9eb63590cd7c4eaad3c81d67f4265e1318566d238c84d` |
| [src/runtime/application.cpp](../src/runtime/application.cpp) | `89e712de0949f2e6c200f504daa47740254bb49b59f1d91ce6ca5cbf9579f1a8` |
| [src/platform/hook_native_callbacks.cpp](../src/platform/hook_native_callbacks.cpp) | `9c1b913d2dbfdc2ac25e23b53357764579eed81e9da57f8be751dfc727251b4c` |
| [include/citrine_eclipse_mirage/platform/hook_native_callbacks.hpp](../include/citrine_eclipse_mirage/platform/hook_native_callbacks.hpp) | `514e02789ee26ee8f56bda3c825055f1289a2e539098893942d9a0559587fef5` |
| [tests/unit/native/native_tests.cpp](../tests/unit/native/native_tests.cpp) | `e9534dcbc9f2318995b76c21c6dc3b3c94dd4445ca2ef8dfe5261889ee4b4bab` |

## Rechecked source-owner topology

The A12 selected admission core/integration fixtures describe a dedicated Application native entry seal, a count of already admitted calls, cached-call refusal and retained Runtime/Domain ownership. The selected native callback implementation also has optional admission/direct-forwarding paths; its Runtime factory coordinates source stop and registry admission. These are source-model integration boundaries, not native tests newly run here.

Product Application start/stop is owned by one DLL worker rather than the donor's Entry::Application ABI. Its accepting flag gates new session handlers, while native Binding leases eleven Win32/Winsock hook callbacks and retains Error/Context ownership. The worker counts explicit shutdown requests and retains Application after failure. Product callback guards, source factory and session admission are distinct owners; the donor's application seal cannot be copied wholesale onto unrelated OS callbacks.

## Corrected coverage and unresolved boundary

The earlier report called #128 and #130 unmerged. Both are now accepted, together with #102 public startup/shutdown schedules and the existing Binding lifetime tests. Merged #148 strengthens the engine boundary; #239 supplies genuine suspended-peer relocation, while #352 adds retained-cleanup barriers, real WINAPI target/trampoline calls and bounded thread churn. These must not be proposed again as absent native hook work.

Those bounded cases do not establish arbitrary entry before lease acquisition, foreign code continuing through unload or image safety for every host. Application external-thread scans, trampoline leases, native Context/Error lifetime, DLL image references and Asio admission remain separate proof obligations. No newly reproduced production defect is claimed.

Retain the existing worker, hook set, standard CRT and public export contract. A further live-hook test must identify a discriminating schedule not covered by accepted peer/live-call/churn scenarios and preserve the application's caller-quiescence contract. Complete original-source/Consumer parity remains open; this review changes no callback admission policy.

## Validation boundary and continuation

The selected A12 inventory checks and current product source/test crosswalk were performed for this update. Complete local Source QA and tracked clean-tree pre-push results belong to the updated PR head and are reported in its description. No fresh native Windows/MSVC, Production/Golden, original DLL or Consumer/game execution is claimed locally. Historical donor tests and merged PR receipts are not fresh execution here.

Continue unresolved source/owner questions through the retained reconstruction project and a scoped reproducer where applicable. This review does not close the entire archive/module or authorize source retirement, a runtime policy change, private CRT adoption or public ABI changes. Product/test/vendor bytes, retained reference snapshots and build/CI policy remain unchanged.
