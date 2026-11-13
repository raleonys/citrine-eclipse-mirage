# Consolidated Archive 12 reviews and remaining research

## Scope and current decision

Owner-authorized consolidation, 2026-11-13 UTC. Accepted product: `35b074519594a3b2d58cac21e07c2a84fb3ad1f1`. Retained A12 authority: `research/reconstruction-package@56eab8795fa65325e4fa6fe31a645eb55cc58469`. The delivery inventory contains 6,514 reference files; this proposal performs selected source/record comparisons, not a global review of every file or fresh donor/native/Consumer acceptance.

PR #358 now carries eleven bounded documentation proposals: its original history report plus the ten reports below. Every source head is retained as a parent in the consolidation history and was preserved in a verified external Git bundle before integration. Closing an absorbed proposal records its transfer here; owner acceptance of this PR remains separate. No application, test, vendor, ABI, build, CI or README bytes are adopted from these reports.

| Original PR | Bounded subject | Preserved proposed head | Current report |
| --- | --- | --- | --- |
| [#119](https://github.com/raleonys/citrine-eclipse-mirage/pull/119) | Shutdown and retained owners | `2d5f34f5a9f29d5bd087ab08da24ea69e9cce756` | [RE_A12_TEARDOWN_OWNER_MATRIX.md](RE_A12_TEARDOWN_OWNER_MATRIX.md) |
| [#132](https://github.com/raleonys/citrine-eclipse-mirage/pull/132) | Defaults and allocation boundaries | `73cc2605c26fc7b7e44ac9c9d7b31fae2126c658` | [RE_A12_DEFAULTS_CONTRACT.md](RE_A12_DEFAULTS_CONTRACT.md) |
| [#136](https://github.com/raleonys/citrine-eclipse-mirage/pull/136) | QPC and startup publication | `9797ad91a31807b1dc95ed112bea4340950faca7` | [RE_FREQUENCY_SOURCE_CROSSWALK.md](RE_FREQUENCY_SOURCE_CROSSWALK.md) |
| [#139](https://github.com/raleonys/citrine-eclipse-mirage/pull/139) | Compiler CRT and reference applicability | `e2edb60208422a2e49de2cf96c8ad2375c27ff51` | [RE_A12_CRT_BOUNDARY.md](RE_A12_CRT_BOUNDARY.md) |
| [#140](https://github.com/raleonys/citrine-eclipse-mirage/pull/140) | Achievement persistence | `0234579e2255204076d912eeec54bd4e6829909c` | [RE_A12_ACHIEVEMENT_CROSSWALK.md](RE_A12_ACHIEVEMENT_CROSSWALK.md) |
| [#141](https://github.com/raleonys/citrine-eclipse-mirage/pull/141) | Admission, hooks and image lifetime | `2a52a47126d5a74bbeb54cabae74c4468d42ed0c` | [RE_A12_ADMISSION_OWNER_CROSSWALK.md](RE_A12_ADMISSION_OWNER_CROSSWALK.md) |
| [#178](https://github.com/raleonys/citrine-eclipse-mirage/pull/178) | Messages and XML | `081a0371c47f6e39fece988c1629ef389ada9bf7` | [RE_A12_MESSAGES_XML_BOUNDARY.md](RE_A12_MESSAGES_XML_BOUNDARY.md) |
| [#191](https://github.com/raleonys/citrine-eclipse-mirage/pull/191) | License, token and key files | `043c67354c9113a9e7c6d08dd88fe7fdaf5b7321` | [RE_A12_LICENSE_TOKEN_CROSSWALK.md](RE_A12_LICENSE_TOKEN_CROSSWALK.md) |
| [#356](https://github.com/raleonys/citrine-eclipse-mirage/pull/356) | Reference linked image and loader | `183e4ebb6cd5b0ec6cf59b4de572dd434845b557` | [RE_A12_LOADER_LINK_BOUNDARY.md](RE_A12_LOADER_LINK_BOUNDARY.md) |
| [#357](https://github.com/raleonys/citrine-eclipse-mirage/pull/357) | Registry ownership and provenance | `9b36db2e13fb449178aea47a0448111ebe610c2f` | [RE_A12_REGISTRY_HKEY_BOUNDARY.md](RE_A12_REGISTRY_HKEY_BOUNDARY.md) |

The previous PR-history report is preserved at [its exact original commit](https://github.com/raleonys/citrine-eclipse-mirage/blob/1294cef438bb9a30028d4b0c4169dde69c86e42a/docs/PR_REVIEW_FOLLOWUPS.md). Its 346-PR snapshot (286 merged, 54 open, six closed-unmerged), later 287-merge-tree replay and four local portable results remain historical observations. They do not describe today's open count, prove every merge semantically correct or execute the current proposal. Original per-report source hashes and validation limits are retained in each earlier-checkpoint section.

## What changed in the current comparison

- Five earlier report groups have changed product counterparts. Application/Environment gained accepted #350 UnknownMessage handling; Asio gained accepted #344/#336 test seams while production keeps its original concrete containers/threads. Those deltas do not replace the clock, stop-order or license ownership contracts.
- The XML review is materially updated for accepted #359: first-party depth policy now follows the A12 depth-100 profile, including paired 98/99 nesting, self-closed-leaf handling and failure precedence. Its older depth-500 comparison remains historical. Pugixml and all vendor pins remain unchanged.
- #280/#312/#324/#334/#341/#343 and the other existing fixture proposals are accepted work, not missing tests. #147 is accepted Debug diagnosis and does not prove the intermittent status-4 refusal fixed.
- The owner closed #362/#363 to pursue a different hook approach. Their completion/retirement/relocation proposals are preserved work, not accepted production behavior and not prerequisites of this report.
- A12's supplied linked image and Registry build-record identities remain bounded source/static evidence. Three export aliases to a RET leaf do not establish useful exported behavior, native loader safety or Consumer equivalence.

No newly confirmed application defect follows from these comparisons. A source difference is classified against the product's actual contract and tests; any confirmed correction needs its own scoped reproducer and code/test proposal.

## Three independent work packages

| Owner / location | Deliverable | Completion gate |
| --- | --- | --- |
| [#358](https://github.com/raleonys/citrine-eclipse-mirage/pull/358), target main | Current bounded reports, accepted follow-ups, source identities and this navigation | Scoped source review, inventories, exact-head validation and owner review. This can finish while broader research remains open. |
| [#236](https://github.com/raleonys/citrine-eclipse-mirage/pull/236), target research/reconstruction-package | A12 clock/publication file rows and RPK-0001 requirement/caller evidence | Hash-bound row review and honest in-review status; native/error/foreign-caller gaps stay explicit. No speculative exactly-once product change. |
| [#81](https://github.com/raleonys/citrine-eclipse-mirage/pull/81), Draft four-field research | Source provenance/dispositions plus a separately assessed QA/package proposal | The file-to-product matrix and owner retirement decision; QA boundary changes need their own current safety review before adoption. Reference branches remain retained. |

## Concrete remaining evidence

| Topic | What is already established | Next discriminating work |
| --- | --- | --- |
| Clock/publication | Two cold product queries and one serialized donor query are intentional selected contracts; product Application preinitializes a positive frequency. | #236 records external cache/owner sharing and any exactly-once requirement, QPC failure and native LastError evidence. |
| Registry | Accepted language-temporary scope, failure-atomic fabrication and direct/installed A/W fixtures exist. | [Issue #109](https://github.com/raleonys/citrine-eclipse-mirage/issues/109): observed issuer, actual callback ingress, reuse epoch, query/close chain and error state. Numeric membership alone supplies no provenance. |
| Loader/shutdown/hooks | Worker startup, explicit cleanup retries, native fixture schedules and Debug refusal diagnosis exist in the product. | The retained reconstruction needs a loader-safe host plan and exact attach/close/foreign-owner observations; a linked donor image is insufficient. A new hook approach is separate work. |
| XML/messages | Current depth-100 profile, accepted continuation/NoResponse, event receipts and reader/corpus contracts exist. | A concrete uncovered malformed-input, exception/UI or foreign-host case; preserve accepted event and vendor policy. |
| Four-field source retirement | Four source families and their original provenance are a continuing research scope. | #81 needs per-file/function dispositions, applicable evidence and explicit owner retirement. Importing bytes does not complete semantic review. |

## Separate QA/package assessment for #81

The preserved #81 head contains actual Python QA changes, not only a report. It verifies and excludes `field-source/` snapshots from application inventory/source packages, using a new import policy and regressions. Those changes are intentionally excluded from #358.

Before adopting that code, reconcile it with current canonical-path and duplicate-key rules, Python 3.9 Windows reparse/junction detection, parent confinement, exact registered fields and file kinds, malformed record behavior, and output/source-package tamper controls. The old implementation uses plain json.loads and is_symlink-only checks; it has not been refreshed to the newer QA boundaries. A current acceptance test set must cover the supported controls. This is a proposal review, not a newly demonstrated active-product defect.

The #81 Draft metadata is restored under the owner's instruction. Its long-term research, QA proposal and eventual retirement decision are independently visible; none is merged or completed by this documentation consolidation.

## Evidence and preservation boundary

All original proposal heads remain Git ancestors; no shared history is rebased or force-updated. Current report product pins are computed from the exact accepted main above. Selected A12 input rows are rechecked against the retained inventory. Archived agent notes are reference data, and no archived script, test executable or DLL is run by this work.

Complete local QA, focused product checks, tracked pre-push results and hosted checks belong to their exact proposed heads and are reported in the PR description. Old source/Wine/native/Golden/game receipts are never relabelled as current execution. This PR makes no source-retirement, CRT/vendor, release or public-ABI decision.
