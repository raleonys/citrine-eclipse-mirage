# Active release contract architecture

The current release source-contract gate is `scripts/check_active_release_contract.py`.
It derives identity, ordinal, expected official filename, and official parent from
`RELEASE_STATE.json`, then cross-checks metadata, normative mirror hashes, pending
acceptance declarations, and uncommitted publication state. **No active contract
may hard-code the HF18 or HF17 ordinal, identity, source ZIP SHA, or governance
revision.** Each release supplies those as metadata; external G0 independently
verifies the exact authority and content.

The historical HF16 and HF17 contract scripts remain byte-for-byte intact for
provenance. They are **not** in the active 64-step release QA catalogue. A
catalogue regression gate rejects a version-pinned release-contract script as
an active step. Synthetic ordinal-18 and ordinal-19 regression tests prove
that the same active checker works without source edits, and mutation tests
reject incorrect parent, pointer, acceptance, and governance bindings.

These are separate boundaries:

- Local compile, Debug, native tests, and local Golden build: never require G0.
- Active source contract: structural checking, no external credentials or G0 call.
- Official release QA: independently pinned G0, approved catalog hash and
  eight-shard source/fresh-extract requirements remain separate mandatory gates.
- Publication: G4/accepted evidence and atomic pointer transaction; no
  publication side effects are performed by these source checks.

This source-stage migration does not claim official G0, G1, G2, G4, game
acceptance, a Windows/MSVC run, or release publication. The existing catalog
scope remains `IDS_QA_ONLY_NOT_AUTHORIZED` until independently accepted for
release; a clean source-only contract PASS does not change that scope.

## External authority and immutable QA catalog (generic G0/G4 design)

The active `scripts/check_governance_first.py` is an alias to the **same**
`scripts/check_external_g0.py` implementation for every new hotfix. No
HOTFIX-number-specific G0 logic is invoked. It verifies six operator-controlled
files and trusted SHA pins outside the candidate tree, the exact official
parent, version progression, user-authorized release intent, current Library
pointer, the candidate source ZIP, and the source manifest. It writes one
`universal-independent-g0-receipt.v1` prepublication report **outside** the
candidate tree when `DLL_RD_G0_REPORT_PATH` is given. That report never grants
G4, does not update the Library pointer, and does not replace independent
approval of the trusted inputs.

`metadata/RELEASE_QA_CATALOG.json` is **always source-only** with scope
`IDS_QA_ONLY_NOT_AUTHORIZED`. Its 64 steps are not an authority credential.
The release QA runner accepts that catalog only after `check_release_qa_authority.py`
verifies both the *externally pinned* G0 receipt and the *externally pinned*
source ZIP/catalog SHA, plus the candidate tree manifest. Requiring the catalog
to mark itself `APPROVED_RELEASE` was contradictory and is removed. This step
checks authorization once as an external G0 computation and performs inexpensive
receipt/source checks on each shard; it does NOT rerun the full G0 process.

Operator variables for G0 are the six path/pin pairs in
`scripts/check_external_g0.py` and `DLL_RD_G0_REPORT_PATH` for the output.
Before starting eight shards, place this output outside the repository and
independently pin its SHA as `DLL_RD_TRUSTED_G0_RECEIPT_SHA256`; set
`DLL_RD_G0_RECEIPT_PATH`, `DLL_RD_G0_CANDIDATE_ZIP`,
`DLL_RD_TRUSTED_G0_CANDIDATE_ZIP_SHA256`, and
`DLL_RD_TRUSTED_RELEASE_QA_CATALOG_SHA256` for each authoritative QA shard.
The `--authority-preflight` option validates that boundary **without running a
shard or claiming G4**. The 8/8 source and one clean-extract 8/8 run remains
mandatory at HOTFIX G4, then separately verified publication/Library readback.

Tests that generate their own fake operator/pin documents are only synthetic
positive-path diagnostics, never independently authenticated publication
receipts. Local production/debug/native/CRT/unstable builds remain G0-independent.

The retired HF18-specific external G0 implementation is preserved in the independent remediation evidence; it is deliberately absent from the active shipping source.
