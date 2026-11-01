# Four-field research source handoff and retirement review

**Status: REVIEW_OPEN / SOURCE_IMPORT_PENDING / NOT_RELEASE**

The four source references are independent research snapshots. A source branch must not be merged wholesale into Citrine Eclipse Mirage `main` or treated as current game/native acceptance.  Source branch creation is not evidence of code import.

| Field | Snapshot | SHA-256 | Branch |
| --- | --- | --- | --- |
| Redfield | RF37 NOT_RELEASE | `e9e6eb8ffe0aa24d4e21a12273aeac32791119b07df82cccaa09394d363f1fc6` | `research/field-retirement/redfield` |
| Bluefield | BF02 FIX31 NOT_RELEASE | `20c707f6cc57901674c26345983f5975150ab67fd547f4f3e95221e78066f416` | `research/field-retirement/bluefield` |
| Greenfield | GF14 NOT_RELEASE; subsequent GF15-F20 are operator/research | `03edc7930459a67518b4c4833779b00f41e8fb4f55ba9ee179b0d3f76dbc2b23` | `research/field-retirement/greenfield` |
| Brownfield | SQUALERIS HOTFIX 19 — official canonical release | `c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab` | `research/field-retirement/brownfield` |

These were exact owner-retained archives whose local ZIP CRC and SHA-256 have been verified. No complete source tree has yet been uploaded to the four GitHub branches: the branches have README provenance scaffolds only. The owner has authorized uploading source for collaborative review. A separate upload handoff imports the source after local provenance/privacy checks.

## Scope and preservation

- `docs/FIELD_SOURCE_IMPORT_POLICY.json` pins the four allowed archive inventories and their exact provenance records. `scripts/project_files.py` verifies every present research snapshot byte before keeping `field-source/` outside the active application inventory and release source package. Missing, extra, altered or symlinked reference files fail source QA. This boundary does not remove the snapshots from Git or merge them into application targets.
- Existing Citrine Eclipse Mirage production code remains authoritative; do not bulk merge FIELD source snapshots or rewrite public C ABI/vendor/CRT/runtime policy without a narrow PR.
- **Brownfield** is the canonical 3.2.5 SQUALERIS HOTFIX 19 source, archive `preloader_l_3.2.5_SQUALERIS_HOTFIX_19.zip` (SHA-256 `c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab`, 3,971,740 bytes, 1,898 ZIP entries). This identity is backed by the official HF19 publication receipt and Library CURRENT_ARTIFACT pointer.
- **Yellowfield**, not Brownfield: 3.2.6 TALVOR R18 (`preloader_l_3.2.6_TALVOR_R18_source.zip`, SHA-256 `1f2a6581a08dbafa940031353c8723e1947e00f0a4681f3bceb1d9f65e3d509e`) is a distinct NOT_RELEASE development/research line. Its `SOURCE_REVISION.md` explicitly says Yellowfield. Do not import TALVOR R18 into the Brownfield branch.
- Greenfield GF15 FIX02 operator embeds byte-identical GF14 source, GF16-GF20 are bounded static/donor evidence (not a later production source snapshot).
- Bluefield FIX31 source work retains D02, D06, D07, D13, D14 and D21 open decisions. Redfield RF37 is NOT_RELEASE.
- The Brownfield HF19 archive includes a GPL-3.0 license text. Preserve third-party notices and independently review file-level provenance and distribution permissions; the presence of a license file alone cannot prove rights to every bundled source.
- Do not publish original game or donor binaries, passwords, tokens or provenance-free third-party material.

## Reconciliation work

Each reviewed item must have source field, exact input hash, original file/function, target Citrine source or open issue, observed effect, tests and disposition. Supported dispositions: `ALREADY_IMPLEMENTED`, `TRANSFERRED_AND_TESTED`, `INTENTIONAL_DIFFERENCE`, `NOT_APPLICABLE`, `OPEN_REMEDIATION`, `EVIDENCE_BLOCKED`.

Audit `metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json`; `metadata/evidence/recovery/CANDIDATES.json` (DC01-DC21, with DC01-DC03 already corrected separately); `metadata/evidence/recovery/MANUAL_REVIEW_LEDGER.json` (67 selected-branch records out of 1,805 inventory files, not global semantic closure); Bluefield B15-B18 and FIX31 lifecycle; Redfield RF37; Greenfield GF14/GF15-GF20. The distinct Yellowfield/TALVOR D326 2,000-stage audit was reviewed but still has 766 blocked-evidence stages; it is not Brownfield HF19 acceptance or a Brownfield-specific audit. Review relevant GitHub issues #16 and #20.

Do not close research/acceptance gaps on the basis of a source import. Independent actual Windows/MSVC/IOCP/Golden/Consumer evidence is needed where applicable. Keep the retirement PR as **draft** until the per-field source import receipts and disposition matrix are examined and the owner explicitly approves. No automatic release publication or branch deletion.
