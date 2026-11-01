# preloader_l 3.2.5 SQUALERIS HOTFIX 19 — Release notes

**Immutable parent:** official HF18 ZIP SHA-256 `395084a6f6a19cbeed54b5faf249431357aa8fa5a2402a95dc66c47c921a30c0`.
**Governance:** HOTFIX G3 `NOT_APPLICABLE`; game, native/MSVC, CRT and Golden acceptance are not HOTFIX publication conditions. G0/G1/G2 and G4 source 8/8 plus one clean-extract 8/8 apply.
**Current publication authority:** Library `CURRENT_ARTIFACT.json` and its external publication receipt, not a sealed source document or metadata field.

## Corrective production work

- Corrects module ownership loss on `FreeLibrary(FALSE)` during normal addon teardown and failed-install rollback; preserves retry opportunity and returns truthful failure.
- Corrects failed physical model-policy release losing its stack owner: retains pending ownership in a durable slot, serialized with Config2 writers; subsequent writes/clear retry or report failure without dropping the owner.
- Keeps HF18 Config2 overlay and selected 7/9/9/11 hook architecture; route/export/vendor/ABI unchanged.

## Validation semantics

Focused C++ tests and fail-once injections validate the bounded fixes. Official HOTFIX G3 is `NOT_APPLICABLE`; no game or native acceptance required. Exact-source **8/8** plus one clean-extract **8/8** must be recorded externally at G4; this source document is not itself a release receipt. The Library `CURRENT_ARTIFACT` pointer is the publication authority.

Historical HF17/HF18 preparation notes remain in their immutable official release archives and chronological changelog; their old pending labels do not describe HF19.
