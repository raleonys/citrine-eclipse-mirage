# Weekly semantic releases

On Fridays at **13:30 UTC**, the GitHub workflow prepares a **draft**, not a published release. The owner separately reviews native/Golden and real Consumer/game acceptance before manually publishing.

## Approved numbering and LUTRAVEX

Retire old `<series>-RNN` tags. Start with `3.2.6-TALVOR`, then `3.2.7-ERINTHAR`, `3.2.8-FORMADRIL`, `3.2.9-CETRION`, `3.3.0-VESPERKAI`. The 34 owner-approved (2026-10-31) assignments through `3.5.9-COLOZHEM` are pinned in `metadata/WEEKLY_NAMING_POLICY.json` from the project Library master. A reserved codename is not a published release.

Minor and patch may be 0..9 only. On patch 9, increment minor and reset patch: `3.2.9 → 3.3.0`; on minor 9 and patch 9, increment major: `3.9.9 → 4.0.0`. Reject `3.2.10`, `3.10.0`, duplicate roots and unapproved future assignments. First candidate bootstraps automatically; optional manual `start_tag` must match the exact next approved version. An unchanged completed source creates no new release; incomplete drafts retry on the same source SHA. Tags and published/unrelated releases are never overwritten.

## New external files

Each new release uploads exactly two files. For example, tag `3.2.7-ERINTHAR` has:

- `Citrine_Eclipse_Mirage_3.2.7_ERINTHAR.zip`: exactly three files at the archive root: validated `citrine_eclipse_mirage.dll`, `citrine_eclipse_mirage.dll.sha256` (the DLL's checksum), and the project's `LICENSE`.
- `Citrine_Eclipse_Mirage_3.2.7_ERINTHAR.zip.sha256`: the SHA-256 checksum of that ZIP, with its exact filename.

GitHub supplies the automatic **Source code (zip)** and **Source code (tar.gz)** links for the release tag. Do not upload an additional source ZIP, source checksum, receipt or test log. The binary ZIP has no other files or directories: receipts, publication status, test evidence and separate vendor license files stay outside it. Third-party license files remain intact in the repository's source archives.

The workflow still verifies the exact source package, Source QA, portable CTest, native MSVC and all six Production/Golden gates before preparing a draft. Source validation and Windows evidence stay in their Actions artifacts. The separate `weekly-release-receipt` artifact retains `RELEASE_RECEIPT.json` and its checksum for 14 days, alongside the existing evidence retention period. This internal receipt binds the source, DLL and final binary ZIP hashes; the uploader requires a matching receipt outside the public asset directory and rejects extra local or remote assets.

`DRAFT` in an internal preparation receipt describes the workflow's draft step. Publishing a release does not rewrite uploaded ZIP bytes, so publication-state receipts must not be embedded in downloadable packages. The owner reviews and manually publishes the draft after separate game acceptance.

Completed release notes contain the source identity, validated build summary, download guidance and generated changelog. They contain no draft heading or instruction to publish the draft, so the same text remains suitable after owner publication. The incomplete-asset warning exists only during preparation and is removed once both uploads verify. Internal game-acceptance status remains in workflow evidence; the notes do not claim game acceptance.

This policy applies to future candidates. Published `3.2.6-TALVOR` assets and earlier historical artifacts are retained unchanged; historical Brownfield `citrine_eclipse_mirage_...FINAL_RELEASE.zip` artifacts use their separate former grammar.

Source QA, portable CTest, Windows Native MSVC, Production/Golden six gates and finalized DLL acceptance must complete on one source SHA before marking a draft complete. The plan supplies its semantic version to both Windows builds through `CITRINE_ECLIPSE_MIRAGE_VERSION`; matching build receipts are required before bundling. `citrine_eclipse_mirage.dll` carries Citrine Eclipse Mirage VERSIONINFO with identical numeric file/product versions (x,y,z,0) and string versions (x.y.z). Public C exports, CRT, SDK protocol and runtime behavior are unchanged. Only the owner publishes the draft after separate game acceptance.

## Local build version upkeep

The committed `default_project_version` and its release-tag provenance in
`metadata/BRANDING_AND_IDENTITY.json` identify the default for local and offline
source-archive builds. Explicit `CITRINE_ECLIPSE_MIRAGE_VERSION` values still
override it for release candidates and the corresponding Golden checks/receipts.

After a stable release is published, **Refresh local release version** reads
GitHub's latest published stable release and prepares a small ordinary PR.
Only the version fields in the branding metadata, an unreleased changelog entry
and the active-source manifest change. Drafts, prereleases, superseded release
events, invalid codename/version bindings and older versions cannot advance or
roll back the default. Repeated events reuse an open PR; a conflicting existing
branch is left for review rather than force-updated. The workflow can also be
run manually from the default branch to retry upkeep.

The workflow uses the tracked pre-push Source QA gate. It never writes main,
approves or merges a PR, publishes a release, or changes uploaded assets.
The owner merges the update after the required later-synchronization checks;
local builds use the new default after pulling that merge. Until then they
continue to use the committed previous default, and a draft release alone does
not alter it. No network access or Git metadata is required during local builds.

Repository Actions settings must allow GitHub Actions to create pull requests
for this workflow's `GITHUB_TOKEN` to open the proposal. The workflow requests
`contents: write` and `pull-requests: write`; it does not use approval APIs.
GitHub may suppress initial workflow events for a PR created with that token.
The later owner-requested synchronization must therefore establish the exact-head
required checks before merge. This workflow does not enable or relax repository
settings, bypass protection, or request full validation on initial creation.
