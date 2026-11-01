# TinyXML2 production migration remediation — experimental source checkpoint

Parent: exact HF11 Candidate10 ZIP SHA-256 `94cd31c1ae476b6b2ad0eec7a60c8e5c071ee1cad824cd49d0c667eeab35a5bc`.
Published canonical baseline remains 3.2.5 SQUALERIS HOTFIX10; these changes are not official HF11 publication.

## Closed within source scope

* `DirectChildIndex` now records the immutable TinyXML2 DOM sibling validation result once, while preserving owned index and direct child O(1) access. Projected nested child views receive a one-time index. `next_direct_child` and `find_direct_child_text` consult it rather than rescanning every sibling per iteration. The existing fail-closed non-whitespace content contract remains.
* Portable GCC/Clang executable regression covers 4,096 direct children, nested child sequences, DOM lifetime after source buffer overwrite, and rejection of non-whitespace parent text.
* Migrated source/behavior QA from deleted custom scanner and Store units to current TinyXML2 production paths: XML grammar and depth, hard cap, semantic edges, release assurance, repository structure, provenance, inherited ownership, remediation regressions, and historical behavioral shards. QA retains executable assertions and known negative cases.
* Current vendor is unmodified upstream `master` commit `8224e427b655b83dae5e2298f1e6919523a78737`, source macro still reports 11.0.0; exact `.cpp/.h` and LICENSE are byte-identical to user-supplied upstream. Historical 11.0.0 tag remains provenance, not active vendor revision. The commit is the source identity; file hashes bind release integrity. A02-A (#1082/#1086) and A02-B remain OPEN, not accepted as safe by this import.
* Registered previously uncataloged internal QA scripts and normalized active filename governance. The authoritative eight-shard runner includes semantic-once DOM and new cached-child regression.

## Open; publication blocked

* **Input entity and character reference parity (HF11-C10-A02):** the pinned upstream engine can accept unsupported/invalid numeric references and undefined entities in unused fields. A single DOM does not expose raw entity spelling after decoding. This checkpoint **does not** claim a whole-document XML 1.0 acceptance guarantee. Avoid an unapproved custom lexical parser, second parse, or untracked vendor patch. Exact Consumer/Donor rejection policy and a single-engine fail-closed remediation must be established first.
* **PI/LSX root mixed-text authority** is not directly closed by the bounded local fixtures. Current TinyXML2 fail-closed processing-instruction behavior remains; donor/Consumer exact acceptance requires independent confirmation.
* **Compile-time raw/decoded type split** remains review-only; runtime TextRepresentation checks and semantic-once fixture remain. No public ABI change authorized.
* **Exact Windows acceptance** for this modified source is NOT_RUN: Production/Golden/FALCION, Debug, Native/CRT and CollegeFB27/Madden27 must rerun before any promotion.
* Eight authoritative QA shards must pass clean extraction; focused tests are not a substitute.

No new JSON engine, bridge parser, source-range scanner or PI normalization hack added.
