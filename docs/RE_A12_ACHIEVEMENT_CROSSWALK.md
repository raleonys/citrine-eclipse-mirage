# Archive 12: achievement persistence contracts

**PARTIAL_A12_SOURCE_REVIEW / NO_NEW_PRODUCT_DEFECT_CONFIRMED / RESEARCH_REMAINS_OPEN**

This refreshes existing PR #140, preserving its original research history.
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
| [anadius-achievement-storage-source/storage.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-achievement-storage-source/storage.cpp) | `2a2f23e278f35e8920047c2a9444e1fd150220d01bb2d727d1f3378a43684d13` |
| [anadius-achievement-writer-source/writer.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-achievement-writer-source/writer.cpp) | `20bd58a3fb48d3db046725fa6c154da475072c9dbd29f21e7c6a34c617319b87` |
| [anadius-achievement-runtime-source/runtime.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-achievement-runtime-source/runtime.cpp) | `5ec1e7c2fc2dced97d781450ad15058bc9cc2c9f5486d7ff3c35fe81bfd9e2b7` |

| Current product counterpart | SHA-256 at reviewed main |
| --- | --- |
| [src/runtime/achievement_storage.cpp](../src/runtime/achievement_storage.cpp) | `82409fa58fc88a9e82d678f11179088c3108e56c7d7211699355887d96871235` |
| [src/runtime/achievement_writer.cpp](../src/runtime/achievement_writer.cpp) | `8dc98966b7fafc1fb1b8a83b683fd9173994242e542cfec1298e54850b5ac6bf` |
| [src/runtime/achievement_runtime.cpp](../src/runtime/achievement_runtime.cpp) | `ac0cbc6e59043badacf438b0c1dd590bc795505a6b14916a69323d2ab83ccbda` |
| [tests/unit/achievement_runtime/runtime_tests.cpp](../tests/unit/achievement_runtime/runtime_tests.cpp) | `bd7429d9ff8eb4543d8c0223a40c41ac10597bb979b3aef362d527ead3c2fb16` |
| [tests/unit/achievement_storage/storage_tests.cpp](../tests/unit/achievement_storage/storage_tests.cpp) | `42aef77f78c2bfdd8118b9f0bd26c28b939b34d145d1eec49f9f7ed27518a85e` |

## Rechecked storage, writer and runtime

A12 retains a storage/parser model, counted INI serialization and separate XML/INI persistence calls. The writer emits ordered map records with CRLF and signed/unsigned field rules; it creates directories before serializing and ignores the plain binary writer's boolean result. Its runtime writes XML separately and verifies that the writer uses the same achievement map identity. These are inspected source contracts, not fresh donor filesystem execution.

The current product deliberately uses checked native atomic XML/INI batches. Since the earlier report, accepted #188 guards unsafe INI sections, #205 checks achievement XML number ranges, #261 validates Windows path leaves/conversion order and #296 prepares the full INI before creating its directory. An invalid section therefore does not justify recreating the old directory-first ordering. Existing storage/writer/runtime tests own format, rejected-input, rollback and retry assertions. Merged #244 covers timestamp suffix/XML retention; source-backed failure/lifetime coverage must be credited rather than restated as absent.

## Deliberate differences and remaining evidence

Preserve checked writes, atomic ownership, safe path/section/numeric policies and grant-state/event rollback. The old donor unchecked write result or unvalidated section bytes are not corrective policies. Source similarity does not prove native CRT locale/overflow, reparse races, cross-process/power-loss atomicity or complete Consumer behavior.

Open #280 owns the atomic recovery matrix, #346 the event-allocation-before-persistence assertion and #348 queue seed allocation abort/retry. These are distinct existing proposals, not missing PRs to duplicate. A new code/test request needs a demonstrated current-product failure or a reachable assertion absent from those scopes. This A12 crosswalk remains partial and introduces no runtime or test change.

## Validation boundary and continuation

The selected A12 inventory checks and current product source/test crosswalk were performed for this update. Complete local Source QA and tracked clean-tree pre-push results belong to the updated PR head and are reported in its description. No fresh native Windows/MSVC, Production/Golden, original DLL or Consumer/game execution is claimed locally. Historical donor tests and merged PR receipts are not fresh execution here.

Continue unresolved source/owner questions through the retained reconstruction project and a scoped reproducer where applicable. This review does not close the entire archive/module or authorize source retirement, a runtime policy change, private CRT adoption or public ABI changes. Product/test/vendor bytes, retained reference snapshots and build/CI policy remain unchanged.
