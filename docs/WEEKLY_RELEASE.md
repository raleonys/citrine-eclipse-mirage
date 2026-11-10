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

This policy applies to future candidates. Published `3.2.6-TALVOR` assets and earlier historical artifacts are retained unchanged; historical Brownfield `preloader_l_...FINAL_RELEASE.zip` artifacts use their separate former grammar.

Source QA, portable CTest, Windows Native MSVC, Production/Golden six gates and finalized DLL acceptance must complete on one source SHA before marking a draft complete. The plan supplies its semantic version to both Windows builds through `CITRINE_ECLIPSE_MIRAGE_VERSION`; matching build receipts are required before bundling. `citrine_eclipse_mirage.dll` carries Citrine Eclipse Mirage VERSIONINFO with identical numeric file/product versions (x,y,z,0) and string versions (x.y.z). Public C exports, CRT, SDK protocol and runtime behavior are unchanged. Only the owner publishes the draft after separate game acceptance.
