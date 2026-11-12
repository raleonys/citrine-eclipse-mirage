# Archive 12: license and token boundaries

**PARTIAL_A12_SOURCE_REVIEW / NO_NEW_PRODUCT_DEFECT_CONFIRMED / RESEARCH_REMAINS_OPEN**

This refreshes existing PR #191, preserving its original research history.
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
| [anadius-license-source/license.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-license-source/license.cpp) | `87ea1eebab23fa2bbaff52689e4a076428e5cd8f292e028fbba985d4ce4376c0` |
| [anadius-license-source/key_processor.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-license-source/key_processor.cpp) | `47593a5b42db7bb069254fda51b97fd2557a9392cafa3263baf0f6b469e6690b` |
| [anadius-token-source/token.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-token-source/token.cpp) | `fba652fd5f6027ac2d0988237bac10f4cefc9a74fceecc2554667b8484fee020` |

| Current product counterpart | SHA-256 at reviewed main |
| --- | --- |
| [src/core/license.cpp](../src/core/license.cpp) | `a982ee939382b5a1b774d842cf36392ee5eabb72d80bab6478a845b5d3a909e9` |
| [src/core/token.cpp](../src/core/token.cpp) | `95d8241b62fdf2302e0b9877f6262a1bfd2167877196c0731076e10e50903726` |
| [src/runtime/application_environment.cpp](../src/runtime/application_environment.cpp) | `cb7c63e67d3798cb8b1a582f70e7cfe6bb238e45e1681c2e731bccc08209682c` |
| [src/core/license_files.cpp](../src/core/license_files.cpp) | `bedeaac9a100548d913b6a0ffc302d1cbdf8dccc020b9afcafc0b977f5b82713` |
| [tests/unit/license/license_tests.cpp](../tests/unit/license/license_tests.cpp) | `f061792280841d902ad96b3bbfd5d31111996936739181b9fb11ec9e3557f45b` |
| [tests/unit/token/token_tests.cpp](../tests/unit/token/token_tests.cpp) | `653221d3ac5344e4e4df51e6e7e87eecf1c2c035797b70f0b1b33567edf548eb` |

## Rechecked token, license and key paths

The selected A12 token source checks status kind2, a 65-byte encrypted prefix, CBC processing and literal counted GameToken markers. Congruent short buffers outside the safe original memory domain are explicitly refused; padding/decryption bool handling and marker search are not replaced with DOM/entity parsing. Current token tests own these selected boundaries; neither set proves arbitrary original out-of-bounds behavior.

The A12 license source has an optional immutable AuthenticationCode binding and OwnedAuthEffects copy path. Current product retains its caller-owned fallback reference and immediately copies a provider const-char result before writing XML. Merged #186 covers auth scratch reuse; #111 covers distinct profile getters. They are accepted work, not pending missing tests. Current persistent attempt/block behavior, RequestLicense and InvalidateLicense paths already have source-linked unit/native coverage, including merged #166.

The selected A12 KeyForLicense source preserves an existing regular file, constructs counted XML after the 65-byte prefix, creates the parent and ignores the plain writer boolean. The current Environment writer uses its own accepted source contract and native #163 coverage. This is not the checked atomic achievement writer; do not propagate unchecked write assumptions across the two owners. Accepted #74 guards the separate ContentId path boundary, and #206 corrects ticket filename UTC formatting.

## Remaining proof and overlap

Maintain the accepted auth/provider object and code lifetime, stored fallback identity, token error domain and key-file format. Importing the donor's extra auth-owner ABI requires a demonstrated active-product need; this source-design difference is not a defect by itself.

Open #312 owns cache failure/explicit provider cleanup, #324 binary-read exception unwinding and #334 checked-ticket write-status retention; accepted #290 covers other Environment/config failure integration. None is full native provider concurrency or original Consumer acceptance; do not duplicate their bounded claims or borrow unexecuted PASS results. A distinct native/provider/locale/cancellation issue must be supported by a current-product reproducer before a code change. This A12 review does not complete the remaining source families or retire donor inputs.

## Validation boundary and continuation

The selected A12 inventory checks and current product source/test crosswalk were performed for this update. Complete local Source QA and tracked clean-tree pre-push results belong to the updated PR head and are reported in its description. No fresh native Windows/MSVC, Production/Golden, original DLL or Consumer/game execution is claimed locally. Historical donor tests and merged PR receipts are not fresh execution here.

Continue unresolved source/owner questions through the retained reconstruction project and a scoped reproducer where applicable. This review does not close the entire archive/module or authorize source retirement, a runtime policy change, private CRT adoption or public ABI changes. Product/test/vendor bytes, retained reference snapshots and build/CI policy remain unchanged.
