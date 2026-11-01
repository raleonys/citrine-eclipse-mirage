# preloader_l 3.2.5 SQUALERIS HOTFIX 19

**Immutable parent:** official HF18 ZIP SHA-256 `395084a6f6a19cbeed54b5faf249431357aa8fa5a2402a95dc66c47c921a30c0`.
**Governance:** HOTFIX G3 `NOT_APPLICABLE`; game, native/MSVC, CRT and Golden acceptance are not HOTFIX publication conditions. G0/G1/G2 and G4 source 8/8 plus one clean-extract 8/8 apply.
**Current publication authority:** Library `CURRENT_ARTIFACT.json` and its external publication receipt, not a sealed source document or metadata field.

## HF19 implementation

- Retain owned shell32 HMODULE after a failed `FreeLibrary`; report cleanup failure and allow retry, including partial hook-install unwind.
- Retain staged Config2 model-policy heap owner after failed physical release, preserving a durable serialized retry slot and preventing new model publication until cleanup completes.
- No changes to exports, protocol handlers, pinned vendors, public ABI, or mandatory Windows build entrypoint names.
- Local development builds use `scripts/build_orchestration/build_production.cmd` or optional compile-only `build_unstable.cmd` without a G0 release authorization requirement.

## Evidence scope

Source-only correctness tests cover failed release/retry and bounded policy freeze; no unrun Windows/runtime/game tests are claimed. Historical HF16 **4/4** and later **8/8** QA results are previous-version provenance, not evidence of this exact HF19 source. HF19 authoritative source **8/8** and fresh-extract **8/8** results are recorded in the external G4 release evidence, never inferred from this immutable source package.

Historical HF17 and HF18 preparation snapshots are preserved in their original immutable published source archives and chronological `CHANGELOG.md`. Those historical `NOT_RELEASE` labels never describe the current Library publication status of HF19.
