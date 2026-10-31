# yellowfield governance

## Authority and scope

This document defines the intended GitHub-centered contribution and release model for yellowfield. The repository remains private until the owner separately approves a visibility change.

The owner's latest explicit instructions take precedence over repository guidance. Technical safety requirements and compatibility contracts remain effective unless an approved change explicitly replaces them. A successful check does not prove a different validation class.

This is the foundation of a staged governance migration. It does not claim that all older scripts, metadata, CI triggers, and documents have already been migrated.

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

## Weekly stable releases

- There is one regular GitHub Release flow: a weekly stable-release draft built from the accepted `main` commit.
- GitHub Actions prepares the source-bound candidate and its Windows/Golden evidence. The owner reviews the draft and manually publishes it.
- The owner separately runs real Consumer/game acceptance before stable publication. This acceptance is not repeated for every pull request.
- A failing unmerged pull request does not stop a weekly draft based on an already accepted `main` commit. A failed weekly candidate remains unpublished until resolved.
- Stable or significant cumulative versions may receive a codename, without inventing another release class.
- Associate released assets and retained results with the exact commit, binary and package checksums. Verify the actual distributed archive once; ordinary PR work does not require repeated ZIP fresh-extract campaigns.
- Do not reintroduce Brownfield G0-G4 gates, mandatory eight-shard release cycles, Library current-pointer transactions, or obsolete non-release classifications as new requirements.

## Documentation and metadata ownership

The repository root owns `README.md` (public overview), `GOVERNANCE.md` (normative policy), `AGENTS.md` (agent workflow), `LICENSE`, `CHANGELOG.md`, and required build entrypoints. Technical explanations belong under `docs/`; executable contracts, inventories, and exact pins belong under `metadata/`.

The legacy `governance/` directory, `RELEASE_STATE.json`, and `SOURCE_REVISION.md` are migration targets. Remove them together with their active references and test dependencies in focused follow-up changes; they must not be deleted while required checks still depend on them.

Keep meaningful negative tests, source/vendor identity, ABI, lifetime, and actual native/Golden coverage. Remove redundant workflow machinery only after proving that its failure-detection purpose remains covered. Preserve historical evidence without treating historical states as current authority.

## Public readiness and disclosure

Public-facing project naming uses lowercase `yellowfield`. Public documents should not disclose unnecessary implementation or integration details. Review the full source tree, metadata, history, license status, and distributed assets before any public visibility change. Repository visibility is changed only with explicit owner approval.

## Migration record

This document establishes the agreed target model. Remaining work includes consolidation of legacy scope rules into this document and `AGENTS.md`, canonical documentation ownership, metadata rationalization, removal of obsolete state files, PR-triggered Windows/Golden QA, and verified merge protection. Each migration change must show its actual result and must not advertise pending controls as active.
