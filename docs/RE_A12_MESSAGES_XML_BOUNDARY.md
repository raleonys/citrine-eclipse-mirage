# Archive 12 message processing and XML reader review

## Current consolidated review (2026-11-13 UTC)

This bounded report is carried by [PR #358](https://github.com/raleonys/citrine-eclipse-mirage/pull/358), preserving [original PR #178](https://github.com/raleonys/citrine-eclipse-mirage/pull/178) at `081a0371c47f6e39fece988c1629ef389ada9bf7`. Product source was rechecked at `35b074519594a3b2d58cac21e07c2a84fb3ad1f1`; reference authority remains A12 `56eab8795fa65325e4fa6fe31a645eb55cc58469`. Documentation acceptance does not complete the research project.

**Current disposition.** The earlier depth-500 distinction is superseded by accepted #359. Current first-party reader policy rejects an entered child list at stack size 98: paired nesting 98 succeeds and 99 fails, with separately tested self-closed-leaf and EOF/attribute precedence. Pugixml 1.16 remains unchanged. #350 UnknownMessage continuation, #234 NoResponse and prepared event-delivery ownership remain accepted. The source delta and current reader assertions were read; old 5,120-check/corpus receipts below remain tied to their earlier head.

**Next evidence gate.** Retain current depth-100 policy and vendor pins. Further malformed-input/exception/UI/foreign-host equivalence needs an uncovered case and fresh matching evidence, not restoration of the historical depth-500 statement.

| Product counterpart at current reviewed main | SHA-256 |
| --- | --- |
| [src/protocol/messages.cpp](../src/protocol/messages.cpp) | `dda0871d186751b2588a79720292dfecc80d31ecae11a668b1b58c14f39fb983` |
| [src/xml/reader.cpp](../src/xml/reader.cpp) | `65a948782f75b57ca4f66e7e1267ac90e013a8951f7080e894251ea92993c7e4` |
| [src/xml/reader_policy.cpp](../src/xml/reader_policy.cpp) | `49c6eb01d8ccf21290ae6ed4d17a7a5a7a7575ef83c9d115cc23331c3f643e4b` |
| [src/xml/reader_values.cpp](../src/xml/reader_values.cpp) | `ce49cebfeb830e95cb2b7492399a950ce3beda6e16750f032f81cb5846b63007` |
| [tests/unit/messages/messages_tests.cpp](../tests/unit/messages/messages_tests.cpp) | `151eae79a45c3d6f24e880a43d8de668ea9eb96360458c9540a81e139bc28228` |
| [tests/unit/xml_reader/reader_tests.cpp](../tests/unit/xml_reader/reader_tests.cpp) | `47cab3635f0f70a065dcf42a9687899f0242aabbf264147997a6b34fc8e9b01c` |

The selected reference rows were checked against the retained A12 inventory again. Current product deltas were read where hashes changed. Fresh product test results belong to the consolidated PR description and its exact head; historical PASS/NOT_RUN records below are not transferred. No archived program, script or DLL was executed.

## Earlier review checkpoint

The following text preserves the earlier source/product baseline and evidence record from PR #178. Its old open-PR labels, product hashes and depth/profile statements describe that checkpoint; the current disposition above supersedes them.

**PARTIAL_SOURCE_COMPARISON / ARCHIVE12_RECHECKED / NO_NEW_PRODUCT_DEFECT_CONFIRMED**

This updates the existing PR #178 research workstream against the current
Archive 12 delivery and accepted `main@eea758600fe0801ec91c31f85de563afff1d75a7`.
It replaces that proposal's Archive 11-only crosswalk. It does not close the
Archive 12 reconstruction project or claim complete A11-09/module equivalence.
The original PR commit remains a historical checkpoint; its observations are
not silently promoted to current conclusions.

## Exact inputs and scope

- Retained research commit: [`56eab8795fa65325e4fa6fe31a645eb55cc58469`](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/docs/RE_PACKAGE_PROJECT.md).
- Delivery: `archive-12`, combined ZIP SHA-256
  `539fc80596bfa058afdfe74b01cee95c1c1b807be3e983127510e4dc170acb6d`,
  232,446,047 bytes, nine parts; 6,514 expanded files / 860,673,543 bytes.
- [Import registry](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/docs/RE_PACKAGE_IMPORT.json) and
  [provenance](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/docs/RE_ARCHIVE12_PROVENANCE.json) retain the original
  delivery claims. The [file inventory](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/docs/RE_PACKAGE_FILES.json)
  SHA-256 is `ef3ca82c912f778455ab3f9a02a5905ddd1177bbe932cd0cdb4f5dd59f5f58a8`.
- This recheck verifies byte lengths and SHA-256 of **45 files** in the two
  `anadius-messages-source/` and `anadius-xml-reader-source/` families against
  that A12 inventory. It is not a new verification of the nine original parts,
  whole ZIP CRCs or all 6,514 payloads: the original parts/ZIP were not available
  locally. No supplied script, executable, object or DLL was run or loaded.
- Relative to retained Archive 11 commit
  `e10561b256a5793f857ec4bed4b5a9fb9fcfbadd`, these families contain **20
  byte-identical files and 25 added files, with no changed existing files**.
  All additions are in `parse-status-native-audit-20261104/`. A12 is the authority
  for this review even where its source hashes happen to equal the old hashes.

The compared source/test identities below were read again from A12. The product
hashes describe the accepted main snapshot, not the proposed documentation bytes.

| A12 source or test | SHA-256 |
| --- | --- |
| [anadius-messages-source/messages.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-messages-source/messages.cpp) | `2eda8d8cdd82a783b694ef1c8f7ad97b643d3b50589694a76239c14e1a970f9c` |
| [anadius-messages-source/messages.hpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-messages-source/messages.hpp) | `42a33b619026a7a3201adbff54d853252d46a18d8d2252b7804c39929d4f54d4` |
| [anadius-messages-source/control.hpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-messages-source/control.hpp) | `be829115003ad3870c2fe3a9e62e0245a260cd8b88761821ce56a309fe44c7d7` |
| [anadius-messages-source/messages_tests.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-messages-source/messages_tests.cpp) | `cf6dbb6f06e57e11dbba91a6a1e97cc34e0634e8e19cb5cbcd3dc51da8f5249a` |
| [anadius-xml-reader-source/reader.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-xml-reader-source/reader.cpp) | `0c80c24764ad925ada9ca2c485a984ba490f07d7a596db1eb12c2f18af817d66` |
| [anadius-xml-reader-source/reader.hpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-xml-reader-source/reader.hpp) | `f73003ab563784f7dbc51db9bf853b28f08c80f2cc398a67c49145a3a7bd118e` |
| [anadius-xml-reader-source/reader_tests.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-xml-reader-source/reader_tests.cpp) | `750b046372b025ad5e6f3abe130e20a3760be732a35b0c86d29fd641aabcbb30` |

| Product counterpart | SHA-256 at reviewed main |
| --- | --- |
| [src/protocol/messages.cpp](../src/protocol/messages.cpp) | `dda0871d186751b2588a79720292dfecc80d31ecae11a668b1b58c14f39fb983` |
| [src/xml/reader.cpp](../src/xml/reader.cpp) | `65a948782f75b57ca4f66e7e1267ac90e013a8951f7080e894251ea92993c7e4` |
| [src/xml/reader_policy.cpp](../src/xml/reader_policy.cpp) | `b8a3df07021304850332977b1343b6eca6577f81d87941f722dc2c6d939ab753` |
| [src/xml/reader_values.cpp](../src/xml/reader_values.cpp) | `ce49cebfeb830e95cb2b7492399a950ce3beda6e16750f032f81cb5846b63007` |
| [tests/unit/messages/messages_tests.cpp](../tests/unit/messages/messages_tests.cpp) | `151eae79a45c3d6f24e880a43d8de668ea9eb96360458c9540a81e139bc28228` |
| [tests/unit/xml_reader/reader_tests.cpp](../tests/unit/xml_reader/reader_tests.cpp) | `16e42b9a1236f5e16a9fbc69f9e02a9f5245982bb4221662078a609b3dfb70dd` |

## Current behavior and dispositions

These are bounded source comparisons. Matching function names, addresses, test
names and selected paths are not whole-file semantic or original-DLL ABI proof.

| Boundary | A12 source and current product | Disposition / evidence |
| --- | --- | --- |
| Registration and request selection | The ordered 39 command/RVA pairs match exactly. Both dispatchers select the first command child, require LSX/Request/id/recipient, reject a Request sibling and retain the special ChallengeResponse branch. | `ALREADY_IMPLEMENTED`; actual source registry comparison, existing messages fixture and merged #117. Additional command children are not newly forbidden. |
| Reset and initial greeting | Both reconstruct the same Cipher in place before clearing the two state bytes. Initial greeting uses the supplied version, random challenge bytes, lower-case hex and printer terminator. | `ALREADY_IMPLEMENTED` for inspected call/order branches; original RNG, object layout and exception ABI remain outside this conclusion. |
| Hex and challenge response | Both treat invalid nibbles as zero, discard an odd final nibble, ignore the fixed-cipher encryption bool in the challenge path, and use the same version-dependent key selection. | `ALREADY_IMPLEMENTED`; existing codec/reset/challenge fixture paths. This does not validate arbitrary external crypto/provider behavior. |
| Parse/retry/framing | Both pass input through its first NUL, distinguish error state from return code, preserve error8 encrypted detection and one-time error7 version erasure/retry, and frame successful plain/encrypted output with NUL. | `ALREADY_IMPLEMENTED` for selected paths; A12's added static binary analysis supports the narrow numeric-status mapping below. |
| NoResponse terminal | A12 catches its selected `NoResponseRequiredError` before response logging/encryption/framing/event selection. Current product also catches its shared typed terminal and returns an empty result at that boundary. | `ALREADY_IMPLEMENTED`; the old PR's statement that the product dispatches without this catch is superseded by merged #234. Existing messages continuation tests and #266's actual Application/TCP cases own acceptance. They are not absent tasks to recreate. |
| Unknown/rejected request | A12 catches its dedicated UnknownMessage marker only with an optional diagnostic effect. Accepted #350 now gives the product a dedicated marker and optional counted console/converted native-dialog diagnostic; normal diagnostic completion returns an empty response and permits the next request on that session. Missing reporters, ordinary handler failures and diagnostic/conversion/allocation errors still propagate. | `IMPLEMENTED_SELECTED_COMPATIBILITY`; this was owner-selected policy and merged as `eea75860`, not a new defect discovered by this audit. Actual Application/TCP, encrypted/plain and isolated/coalesced successor tests belong to accepted #350. Original private exception ABI, real UI and arbitrary foreign-host equivalence remain unproved. |
| Event ownership | A12 eagerly removes one event even when event encryption fails. Product retains that behavior only in its pure source path; its prepared-response path reserves the event and settles delivery after completion, with retry retention and exception-safe unlock. | `INTENTIONAL_DIFFERENCE`; preserve accepted receipts. Existing event queue/MessageHandler cases and merged #143/#173/#248/#340 cover distinct boundaries. A12 eager dequeue is not a fix for the product's asynchronous path. |
| Reader ownership and values | A12 uses owning TinyXML2 documents with stable wrappers, ordered attributes, nullable GetText, C-string truncation and detailed ErrorStr. Product uses document-owned pugixml wrappers plus first-party policy/value code. | `IMPLEMENTED_WITH_DIFFERENCES`; inspect ownership, nullable/empty text, comments/CDATA and normalization separately. Existing reader/value/corpus tests are bounded evidence; no complete malformed-input equivalence is claimed. |
| Vendor and depth | A12 carries TinyXML2 11 compatibility source with a modified depth100 header (98 nested elements accepted; 99 rejected). Product deliberately uses immutable pugixml 1.16 with its depth500 compatibility policy (498 accepted; 499 rejected). | `INTENTIONAL_DIFFERENCE`; preserve owner-approved #149. Neither A12's vendor bytes nor its lower depth limit are adopted. Its embedded original-parser release identity is not proved by the selected upstream compatibility version. |
| Allocation and retained views | A12's reader fixture checks owned input, wrapper identity and document independence; it does not supply the current product's one-shot pugixml and lazy-value allocation-fault matrix. | `EXISTING_PRODUCT_COVERAGE`; preserve accepted #308/#321 vendor/lazy-value failure, prior-view retention and retry assertions. Their existence is not full original allocator/ABI equivalence. |

The A12 messages README still says dispatcher exceptions are not translated to
empty output. Its actual `messages.cpp` includes the two selected catches and
`messages_tests.cpp` contains `no_response_tests` and `unknown_tests`. This review
uses those pinned implementations/tests for the bounded conclusion; it does not
copy the stale README statement or modify the preserved archive.

## What A12 adds: numeric XML status evidence

The 25 added files retain a static original-binary analysis of the same input
DLL hash `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
They contain four failed absent-target requests, an isolated session's successful
replies, an export/bundle, selector-discovery notes and a report. Failed requests
remain failures and are not included as successful evidence.

This recheck verifies all **12 successful reply-file SHA-256 values** declared
by the retained `evidence.json` against the A12 files. It inspects the saved
producer/consumer evidence rather than starting a new REA/Ghidra session:

- Attribute parse failure sets numeric **7** at the retained 004370 producer.
- Ordinary text parse failure sets numeric **8** at 004ea0; CDATA uses a different
  branch/code and is not conflated with ordinary text.
- 033d70 tests document error state and uses the corresponding 8/7 detection and
  one-time retry branches. 034730 is the selected string erase operation, not a
  second parser. These correspond to the current first-party reader policy and
  message-core conditions.

The retained audit closes only its proposed **numeric-status mismatch candidate**
without a source change. This is stronger evidence than the old report's source
crosswalk alone, but it is static binary-analysis evidence, not newly executed
native parser, socket, original-DLL or game acceptance. Unresolved virtual-tail
and terminal/no-return analysis warnings remain in the retained records.

Pinned [audit report](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-xml-reader-source/parse-status-native-audit-20261104/report.md) and
[evidence index](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/anadius-xml-reader-source/parse-status-native-audit-20261104/evidence.json) record the original scope and limits. The
original audit's `compile=false` and `tests_run=false` are preserved as facts;
no supplied historical PASS is promoted to fresh execution here.

## Validation provenance and open work

- A12 identity checks: import/inventory linkage, all 45 selected file size/hash
  records, all 12 declared successful analysis-reply hashes and ordered 39-entry
  product/source registration comparison pass.
- At prior research head `f705c79663af3479f06e3ffcc17c89d50e8d061b`, real CMake Release `portable_xml_reader` and
  `portable_xml_reader_corpus`: **2/2 PASS**, 5,120 reader checks and 12,087 frozen
  corpus rows, including 20 reviewed malformed-value corrections. These run
  the product code at that prior head. The selected XML source and reader/corpus
  test bytes are unchanged at this synchronization; this is not a fresh rerun. The corpus is the product's separately pinned
  diagnostic baseline, not fresh execution against the A12 donor DLL or its
  modified depth100 library.
- Complete source QA and exact-head hosted checks are reported with this PR's
  synchronized commit. Old-head successes do not validate the updated proposal.
- No fresh donor build, donor/original DLL load, native Windows/MSVC message
  session, production Golden or real Consumer/game execution is claimed locally.

Continue this same research workstream with A12 as its source authority:

1. Retain a per-boundary A12-to-current-product mapping. Mark a previously fixed
   test/behavior as implemented rather than dropping the record or recreating it.
2. Keep unknown-message native host behavior, optional diagnostic ownership and
   original exception ABI distinct from the selected continuation now accepted in #350;
   preserve ordinary-error propagation and explicit asynchronous receipt ownership.
3. Extend selected malformed-input, allocation, lifetime and full session evidence
   where a concrete uncovered scenario is found. Any confirmed production defect
   receives its own focused code/test proposal; this report does not authorize a
   blanket exception fallback, vendor swap or changed event-delivery policy.
4. Link further A12 findings to the retained reconstruction project. This PR can
   finish its bounded report after owner review without declaring the archive,
   module family or original-source equivalence completed or retiring inputs.

No production/test/vendor bytes, public ABI, CI policy, historical evidence or
retained reference-package files are changed by this documentation update.
