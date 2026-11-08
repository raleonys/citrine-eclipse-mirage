# Weekly semantic releases

On Fridays at **13:30 UTC**, the GitHub workflow prepares a **draft**, not a published release. The owner separately reviews native/Golden and real Consumer/game acceptance before manually publishing.

## Approved numbering and LUTRAVEX

Retire old `<series>-RNN` tags. Start with `3.2.6-TALVOR`, then `3.2.7-ERINTHAR`, `3.2.8-FORMADRIL`, `3.2.9-CETRION`, `3.3.0-VESPERKAI`. The 34 owner-approved (2026-10-31) assignments through `3.5.9-COLOZHEM` are pinned in `metadata/WEEKLY_NAMING_POLICY.json` from the project Library master. A reserved codename is not a published release.

Minor and patch may be 0..9 only. On patch 9, increment minor and reset patch: `3.2.9 → 3.3.0`; on minor 9 and patch 9, increment major: `3.9.9 → 4.0.0`. Reject `3.2.10`, `3.10.0`, duplicate roots and unapproved future assignments. First candidate bootstraps automatically; optional manual `start_tag` must match the exact next approved version. An unchanged completed source creates no new release; incomplete drafts retry on the same source SHA. Tags and published/unrelated releases are never overwritten.

## New external files

GitHub tag: `3.2.6-TALVOR`. Validated Windows ZIP: `Citrine_Eclipse_Mirage_3.2.6_TALVOR.zip`. Companion source ZIP: `Citrine_Eclipse_Mirage_3.2.6_TALVOR_source.zip`. Each has a .sha256 sidecar; `RELEASE_RECEIPT.json` binds the exact source, DLL and evidence. Historical Brownfield `preloader_l_...FINAL_RELEASE.zip` artifacts are immutable and use a separate former grammar.

Source QA, portable CTest, Windows Native MSVC, Production/Golden six gates and finalized DLL acceptance must complete on one source SHA before marking a draft complete. No new ZIP naming alters compatibility-sensitive PE VERSIONINFO, exports, compiled DLL filename, CRT, SDK protocol or runtime ABI. Only the owner publishes the draft after separate game acceptance.
