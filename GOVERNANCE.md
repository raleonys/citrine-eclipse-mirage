# Citrine Eclipse Mirage governance

## Authority and scope

This document defines the GitHub-centered contribution and release model for Citrine Eclipse Mirage. The repository became public by owner decision on 2026-11-02; release publication, ABI changes and other protected decisions remain separately governed.

The owner's latest explicit instructions take precedence over repository guidance. Technical safety requirements and compatibility contracts remain effective unless an approved change explicitly replaces them. A successful check does not prove a different validation class.

This is the active governance authority. Historical task orders remain accessible through Git history and specialist documents, not as a second live policy file. Build scripts, release-state metadata and CI triggers still require separate verified migrations.

## Source control and contributions

- `main` is the accepted integration branch. Only the owner merges reviewed pull requests. Do not write, reset, force-push, or delete `main` directly.
- Ordinary one-PR branches, including bugfixes, individual audit findings and standalone changes, are temporary. Create a fresh descriptive branch from current accepted `main`, never reuse the retired shared `bugfix` branch, and delete each temporary head after its PR closes (merged or unmerged). First record its exact tip, resolve dependencies and preserve needed unmerged work. Never rewrite shared history.
- Dedicated research, reverse-engineering and reconstruction workstreams may retain their branches across successive PRs and after an individual PR closes. This exemption is for identifiable continuing projects, not every branch prefixed `research/`; keep a recorded scope, owner and closure condition. Retire a sustained-work branch only on an explicit owner decision after verifying source/evidence preservation and live dependencies.
- **Audit branches are temporary by default.** An ordinary `audit/*` PR does not gain retention merely from its name or its findings. Only a specifically designated long-running audit campaign, approved by the owner with a documented continuing scope and review points, may use the sustained-work exception. Evaluate existing audit branches individually before cleanup; do not bulk-delete unreviewed branches or silently classify them all as retained.
- GitHub's **Automatically delete head branches** setting is repository-wide and can remove eligible retained research or designated long-running audit heads following merge. Leave it disabled unless an independently verified exception protects every retained branch; manually clean up closed temporary PR branches. Never delete `main` or discard unmerged work.
- Ordinary contribution PRs are opened ready for review, not as drafts. Existing [research PR #81](https://github.com/raleonys/citrine-eclipse-mirage/pull/81) is an explicitly owner-authorized exception: its continuing four-field research/source-provenance workstream remains a GitHub Draft until the owner decides it is ready. This does not authorize draft mode for routine audit/bugfix PRs or for newly created research PRs without a separate explicit owner decision. Every PR documents scope, validation results, limitations and pending checks.
- A pull request is a proposed upstream change, not a separate release. A merged pull request updates the source tree; it does not create a release or tag.
- Do not change repository visibility, licenses, public ABI, internal source/target identities, or upstream dependency pins without explicit authorization.

## Quality assurance

Pull request creation runs lightweight source/hash integrity without scheduling builds or the full regression suite. Full validation is deferred until a PR head update newly integrates current `main`, or is requested by a label after a successful later synchronization verification. An initial label alone does not authorize full validation. Required checks must finish successfully before a normal merge. Failed product checks are resolved on the pull request, not merged in anticipation of a revert.

A manual synchronization preparation verifies the live open PR, selected branch head and exact current main without scheduling full tests. Its successful workflow job records the PR number, head SHA and main SHA. The validation label is accepted only after that job and its preparation run have completed, after PR creation and before the label event. A premature label cannot adopt a later verification: remove it before preparation and re-add it after success. This supports a later explicit synchronization request even when main has not changed and no merge commit is needed. Agents must not prepare or label a new PR merely to obtain initial green checks.

A skipped check is not a PASS. CI infrastructure failures may be rerun or repaired without being classified as product defects. Source changes are checked against exact source and dependency identities; no test evidence is automatically inherited from a different commit.

The opening workflow contains one basic Source integrity job. Actions, C/C++
and Python CodeQL analyses use the same later-synchronization gate as full
validation for PRs and manual diagnostics. Accepted-main pushes run only the
three CodeQL analyses in that workflow, bound to the exact pushed main commit;
readiness, Source QA and Windows/Golden/reference jobs remain skipped. GitHub's
separate Default Setup must be migrated to the reviewed
advanced workflow by an account with the required setting permission; until
then, autogenerated scans can still run on PR creation. Existing required
Windows/source status contexts and strict main integration are preserved.

README badges are repository-owned SVGs under `resources/badges/`. A bounded
publisher updates only eleven SVGs on the long-lived generated `badges` branch
after completed main integrity runs or nondraft release changes. This data
branch is retained independently of temporary source PR heads; its generated
image updates do not rewrite main, source inventories or release artifacts.

The full validation workflow accepts PR `synchronize` events, explicit `run-full-validation` PR label requests and standalone manual dispatch. Use the PR label request for merge validation: a standalone successful dispatch does not establish that GitHub has associated all required checks with the PR. It verifies the live head and current default-branch ancestry before scheduling Source QA, Linux portable, Windows MSVC Native Release/Debug, Production/Golden, CRT, Debug, Compile Only, Windows portable Debug/Release and the reference graph. Ordinary PR creation and accepted-main pushes do not start these builds. Specialized diagnostics are manual; weekly stable-release validation is unchanged.

GitHub calls any PR head push `synchronize`; this event alone does not prove integration. Automatic full validation requires that current `main` was absent from the previous head and is present in the new head. Ordinary edits on an already synchronized branch wait for an explicit PR label request. Retaining the label does not automatically validate later ordinary head edits. Ineligible full jobs are skipped at job level, before matrix expansion or runner allocation; waiting is not reported as a product-test failure. Skipped placeholders are not native acceptance: the required expanded Windows status contexts remain absent until the actual matrices run, so merge protection still waits for real results. Readiness API errors remain errors of the readiness job. The active ruleset still requires all eleven source/portable/Windows/reference contexts and strict current-main integration. No branch-protection relaxation is part of this scheduling change.

Build, debug, and focused testing remain available developer tools; separate publication-governance credentials or repeated release gates are not prerequisites for ordinary development. During the migration, existing tracked hooks and QA scripts are updated only through reviewed changes, not silently bypassed.

## Changelog maintenance

Maintain a source-linked `CHANGELOG.md` alongside relevant PRs. A PR-only CI coverage check requires a changed changelog file; an exceptional omission requires the owner-reviewed `changelog-exempt` label and a written PR rationale. Report merged PR development separately from source-revision imports and actual published release tags. Do not invent historical R-series labels, dates or release claims: unresolved archive provenance must remain explicitly incomplete until artifact and checksum verification.

Reserve `### This PR` for the current unmerged proposal. When creating or synchronizing a PR, reconcile notes already present on fetched `main` into the corresponding `### Integrated YYYY-MM-DD` sections, with links to the merged PRs. Verify each integration against the merge commit reachable from `main` and use the UTC date of GitHub `merged_at`; preserve the change details and existing historical entries. Do not carry accepted notes forward as if they belonged to the current PR, invent integration dates, or describe integration as a published release.

## Weekly stable releases

- There is one regular GitHub Release flow: a weekly stable-release draft built from the accepted `main` commit.
- GitHub Actions prepares the source-bound candidate and its Windows/Golden evidence. The owner reviews the draft and manually publishes it.
- The owner separately runs real Consumer/game acceptance before stable publication. This acceptance is not repeated for every pull request.
- A failing unmerged pull request does not stop a weekly draft based on an already accepted `main` commit. A failed weekly candidate remains unpublished until resolved.
- Prospective weekly semantic versions advance 3.2.6 TALVOR → 3.2.7 ERINTHAR → 3.2.8 FORMADRIL → 3.2.9 CETRION → 3.3.0 VESPERKAI and follow the owner-approved LUTRAVEX registry. The retired -RNN weekly model must not be used.
- Upload exactly `Citrine_Eclipse_Mirage_x.y.z_CODENAME.zip` and its `.zip.sha256` checksum as release assets. The binary ZIP contains exactly `citrine_eclipse_mirage.dll`, `citrine_eclipse_mirage.dll.sha256` and the project `LICENSE` at its root, with no other files or directories. GitHub supplies the tag's automatic source archives; source companion ZIPs, receipts and test evidence remain workflow artifacts, outside the release assets and binary ZIP. Minor and patch are one digit (0..9). Do not change historical releases or compatibility-sensitive binary/Golden fields.
- Associate released assets and retained results with the exact commit, binary and package checksums. Verify the actual distributed archive once; ordinary PR work does not require repeated ZIP fresh-extract campaigns.
- Do not reintroduce Brownfield G0-G4 gates, mandatory eight-shard release cycles, Library current-pointer transactions, or publication classifications on build and test results.

## Documentation and metadata ownership

The repository root owns `README.md` (public overview), `GOVERNANCE.md` (normative policy), `AGENTS.md` (agent workflow), `LICENSE`, `CHANGELOG.md`, and required build entrypoints. Technical explanations belong under `docs/`; executable contracts, inventories, and exact pins belong under `metadata/`.

The former `governance/CURRENT_SCOPE.md` is retired after operational rules were consolidated into this file and `AGENTS.md`. The obsolete root `RELEASE_STATE.json` and `SOURCE_REVISION.md` are retired. Active runtime verification now reads `metadata/RUNTIME_CURRENT_STATUS.json` directly; the Git commit and weekly draft tag identify source and publication lineage.

Keep meaningful negative tests, source/vendor identity, ABI, lifetime, and actual native/Golden coverage. Remove redundant workflow machinery only after proving that its failure-detection purpose remains covered. Preserve historical evidence without treating historical states as current authority.

## Project license

Project-owned first-party source code and project-authored accompanying material are licensed under **GNU General Public License version 3 only** (`GPL-3.0-only`), not `GPL-3.0-or-later`. The root [LICENSE](LICENSE) contains the full, unmodified GNU GPL version 3 text (29 June 2007).

Third-party material and vendor sources retain their separate applicable licenses and notices. This first-party license does not replace, reassign or silently sublicense third-party notices. Existing third-party source, license files, attribution and vendor SHA pins remain intact in the repository and source archives. The binary release ZIP includes the complete project root license under the three-file packaging rule above; source availability and other GPL obligations must be reviewed before publication.

## Public readiness and disclosure

Public documents should not disclose unnecessary implementation or integration details. Review the full source tree, metadata, history, license status, and distributed assets before any public visibility change. Repository visibility is changed only with explicit owner approval.

Public project dates and schedules use UTC without owner location or regional time-zone names. Changelog dates come from the UTC date of GitHub's `merged_at`; converting a displayed schedule must preserve its actual instant. New Git author and committer timestamps use UTC. Retained historical evidence, existing Git history and previously distributed artifacts require a separate preservation review before redaction; editing the current source does not erase older public copies.

An owner-approved privacy redaction records both original and replacement SHA-256 identities and updates the corresponding current-copy references and byte counts. Preserve all historical execution outcomes, binary digests and evidence limits; a redacted copy is not a new test result. Retained-file hash verification remains mandatory for the approved replacement bytes.

## Migration record

The initial PR-triggered Windows/Golden migration has executed; it is no longer
a universal outstanding task. For example, the exact
[PR #352 head](https://github.com/raleonys/citrine-eclipse-mirage/actions/runs/42889357574)
passed all eleven required contexts, Native Debug/Release and Production/Golden.
That receipt validates its identified head, not later changes or game execution.
Metadata rationalization and donor-retirement research remain separately scoped;
[the documentation index](docs/INDEX.md) distinguishes current guides from
historical checkpoints. A configured job is not a PASS before actual execution.

## Retained technical boundaries

The former scope ledger mixed superseded task decisions with live application constraints. Current details are owned by source and specialized machine contracts; historical instructions are preserved in Git history, not repeated as mandatory active work.

- Preserve the exact three-entry production export/ordinal contract and the separate development C++ ABI. An ABI change requires explicit owner review.
- The shipping hook architecture retains scoped native callback, rollback, safe quiescence, destination filtering, resource lifetime and original-call semantics; no process-wide byte-order hook may be introduced.
- Preserve accepted vendor pins, the standard compiler CRT, source/reference link partition, current configuration and network protocols, and the existing startup/shutdown contracts.
- Golden checks still validate actual finalized production PE copies and not merely synthetic fixtures. The owner-approved project identity uses Citrine Eclipse Mirage resources and release-derived matching file/product versions. Other PE identity, import and resource changes require independent review.
- Tie published source, binary, and archive receipts to the precise accepted commit and checksums. A ZIP package is an output, not a second release authority.
- `metadata/` and specialist `docs/` files provide detailed machine and implementation constraints. Retiring a stale task log does not authorize source rewrites or waive existing tests.
