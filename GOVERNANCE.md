# Citrine Eclipse Mirage governance

## Authority and scope

This document defines the intended GitHub-centered contribution and release model for Citrine Eclipse Mirage. The repository remains private until the owner separately approves a visibility change.

The owner's latest explicit instructions take precedence over repository guidance. Technical safety requirements and compatibility contracts remain effective unless an approved change explicitly replaces them. A successful check does not prove a different validation class.

This is the active governance authority. Historical task orders remain accessible through Git history and specialist documents, not as a second live policy file. Build scripts, release-state metadata and CI triggers still require separate verified migrations.

## Source control and contributions

- `main` is the accepted integration branch. Only the owner merges reviewed pull requests. Do not write, reset, force-push, or delete `main` directly.
- Use the retained `bugfix` branch for routine changes; synchronize it with `main` without rewriting shared history. Preserve active dedicated audit branches.
- Pull requests are opened ready for review, never as draft PRs. They describe scope, validation results, limitations, and any pending checks.
- A pull request is a proposed upstream change, not a separate release. A merged pull request updates the source tree; it does not create a release or tag.
- Do not change repository visibility, licenses, public ABI, internal source/target identities, or upstream dependency pins without explicit authorization.

## Quality assurance

The intended public-repository configuration runs source integrity, portable tests, Windows native acceptance, and Production/Golden validation for each pull request. Required checks must finish successfully before a normal merge. Failed product checks are resolved on the pull request, not merged in anticipation of a revert.

A skipped check is not a PASS. CI infrastructure failures may be rerun or repaired without being classified as product defects. Source changes are checked against exact source and dependency identities; no test evidence is automatically inherited from a different commit.

The repository's current private workflow only runs source and portable checks automatically for ordinary pull requests. Windows native and Golden jobs remain manually dispatched until a separate CI migration enables and verifies their PR triggers and the applicable merge protections. Do not claim that this intended policy is already enforced.

Local build, debug, and focused testing remain available developer tools; separate publication-governance credentials or repeated release gates are not prerequisites for ordinary development. During the migration, existing tracked hooks and QA scripts are updated only through reviewed changes, not silently bypassed.

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
- Do not reintroduce Brownfield G0-G4 gates, mandatory eight-shard release cycles, Library current-pointer transactions, or obsolete non-release classifications as new requirements.

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

Remaining work includes metadata rationalization, removal of obsolete state files, PR-triggered Windows/Golden QA, and verified merge protection. Each migration must report its actual result and must not advertise pending controls as active.

## Retained technical boundaries

The former scope ledger mixed superseded task decisions with live application constraints. Current details are owned by source and specialized machine contracts; historical instructions are preserved in Git history, not repeated as mandatory active work.

- Preserve the exact three-entry production export/ordinal contract and the separate development C++ ABI. An ABI change requires explicit owner review.
- The shipping hook architecture retains scoped native callback, rollback, safe quiescence, destination filtering, resource lifetime and original-call semantics; no process-wide byte-order hook may be introduced.
- Preserve accepted vendor pins, the standard compiler CRT, source/reference link partition, current configuration and network protocols, and the existing startup/shutdown contracts.
- Golden checks still validate actual finalized production PE copies and not merely synthetic fixtures. Do not change PE identity, imports or resources without independent review.
- Tie published source, binary, and archive receipts to the precise accepted commit and checksums. A ZIP package is an output, not a second release authority.
- `metadata/` and specialist `docs/` files provide detailed machine and implementation constraints. Retiring a stale task log does not authorize source rewrites or waive existing tests.
