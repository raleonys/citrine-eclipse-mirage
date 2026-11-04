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

Pull request creation runs lightweight source/hash integrity without scheduling builds or the full regression suite. Full validation is deferred until a PR head update newly integrates current `main`, or is explicitly requested on a synchronized branch. Required checks must finish successfully before a normal merge. Failed product checks are resolved on the pull request, not merged in anticipation of a revert.

A skipped check is not a PASS. CI infrastructure failures may be rerun or repaired without being classified as product defects. Source changes are checked against exact source and dependency identities; no test evidence is automatically inherited from a different commit.

The full validation workflow accepts PR `synchronize` events, explicit `run-full-validation` PR label requests and standalone manual dispatch. Use the PR label request for merge validation: a standalone successful dispatch does not establish that GitHub has associated all required checks with the PR. It verifies the live head and current default-branch ancestry before scheduling Source QA, Linux portable, Windows MSVC Native Release/Debug, Production/Golden, CRT, Debug, Compile Only, Windows portable Debug/Release and the reference graph. Ordinary PR creation and accepted-main pushes do not start these builds. Specialized diagnostics are manual; weekly stable-release validation is unchanged.

GitHub calls any PR head push `synchronize`; this event alone does not prove integration. Automatic full validation requires that current `main` was absent from the previous head and is present in the new head. Ordinary edits on an already synchronized branch wait for an explicit PR label request. Retaining the label does not automatically validate later ordinary head edits. Ineligible full jobs are skipped at job level, before matrix expansion or runner allocation; waiting is not reported as a product-test failure. Skipped placeholders are not native acceptance: the required expanded Windows status contexts remain absent until the actual matrices run, so merge protection still waits for real results. Readiness API errors remain errors of the readiness job. The active ruleset still requires all eleven source/portable/Windows/reference contexts and strict current-main integration. No branch-protection relaxation is part of this scheduling change.

Build, debug, and focused testing remain available developer tools; separate publication-governance credentials or repeated release gates are not prerequisites for ordinary development. During the migration, existing tracked hooks and QA scripts are updated only through reviewed changes, not silently bypassed.

## Changelog maintenance

Maintain a source-linked `CHANGELOG.md` alongside relevant PRs. A PR-only CI coverage check requires a changed changelog file; an exceptional omission requires the owner-reviewed `changelog-exempt` label and a written PR rationale. Report merged PR development separately from source-revision imports and actual published release tags. Do not invent historical R-series labels, dates or release claims: unresolved archive provenance must remain explicitly incomplete until artifact and checksum verification.

## Weekly stable releases

- There is one regular GitHub Release flow: a weekly stable-release draft built from the accepted `main` commit.
- GitHub Actions prepares the source-bound candidate and its Windows/Golden evidence. The owner reviews the draft and manually publishes it.
- The owner separately runs real Consumer/game acceptance before stable publication. This acceptance is not repeated for every pull request.
- A failing unmerged pull request does not stop a weekly draft based on an already accepted `main` commit. A failed weekly candidate remains unpublished until resolved.
- Prospective weekly semantic versions advance 3.2.6 TALVOR → 3.2.7 ERINTHAR → 3.2.8 FORMADRIL → 3.2.9 CETRION → 3.3.0 VESPERKAI and follow the owner-approved LUTRAVEX registry. The retired -RNN weekly model must not be used.
- New archive names are `Citrine_Eclipse_Mirage_x.y.z_CODENAME.zip` (source companion `_source.zip`); minor and patch are one digit (0..9). Do not rename historical artifacts or compatibility-sensitive binary/Golden fields.
- Associate released assets and retained results with the exact commit, binary and package checksums. Verify the actual distributed archive once; ordinary PR work does not require repeated ZIP fresh-extract campaigns.
- Do not reintroduce Brownfield G0-G4 gates, mandatory eight-shard release cycles, Library current-pointer transactions, or publication classifications on build and test results.

## Documentation and metadata ownership

The repository root owns `README.md` (public overview), `GOVERNANCE.md` (normative policy), `AGENTS.md` (agent workflow), `LICENSE`, `CHANGELOG.md`, and required build entrypoints. Technical explanations belong under `docs/`; executable contracts, inventories, and exact pins belong under `metadata/`.

The former `governance/CURRENT_SCOPE.md` is retired after operational rules were consolidated into this file and `AGENTS.md`. The obsolete root `RELEASE_STATE.json` and `SOURCE_REVISION.md` are retired. Active runtime verification now reads `metadata/RUNTIME_CURRENT_STATUS.json` directly; the Git commit and weekly draft tag identify source and publication lineage.

Keep meaningful negative tests, source/vendor identity, ABI, lifetime, and actual native/Golden coverage. Remove redundant workflow machinery only after proving that its failure-detection purpose remains covered. Preserve historical evidence without treating historical states as current authority.

## Project license

Project-owned first-party source code and project-authored accompanying material are licensed under **GNU General Public License version 3 only** (`GPL-3.0-only`), not `GPL-3.0-or-later`. The root [LICENSE](LICENSE) contains the full, unmodified GNU GPL version 3 text (29 June 2007).

Third-party material and vendor sources retain their separate applicable licenses and notices. This first-party license does not replace, reassign or silently sublicense third-party notices. Existing third-party source, license files, attribution and vendor SHA pins must remain intact. All future release artifacts must include the complete root license and the applicable third-party notices; source availability and other GPL obligations must be reviewed before publication.

## Public readiness and disclosure

Public documents should not disclose unnecessary implementation or integration details. Review the full source tree, metadata, history, license status, and distributed assets before any public visibility change. Repository visibility is changed only with explicit owner approval.

## Migration record

Remaining work includes metadata rationalization, verifying first PR-triggered Windows/Golden acceptance on the exact commit, and evaluating additional required check contexts. A configured CI job is not a PASS before actual execution.

## Retained technical boundaries

The former scope ledger mixed superseded task decisions with live application constraints. Current details are owned by source and specialized machine contracts; historical instructions are preserved in Git history, not repeated as mandatory active work.

- Preserve the exact three-entry production export/ordinal contract and the separate development C++ ABI. An ABI change requires explicit owner review.
- The shipping hook architecture retains scoped native callback, rollback, safe quiescence, destination filtering, resource lifetime and original-call semantics; no process-wide byte-order hook may be introduced.
- Preserve accepted vendor pins, the standard compiler CRT, source/reference link partition, current configuration and network protocols, and the existing startup/shutdown contracts.
- Golden checks still validate actual finalized production PE copies and not merely synthetic fixtures. Do not change PE identity, imports or resources without independent review.
- Tie published source, binary, and archive receipts to the precise accepted commit and checksums. A ZIP package is an output, not a second release authority.
- `metadata/` and specialist `docs/` files provide detailed machine and implementation constraints. Retiring a stale task log does not authorize source rewrites or waive existing tests.
