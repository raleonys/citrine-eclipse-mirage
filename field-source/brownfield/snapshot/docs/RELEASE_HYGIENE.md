# Release Hygiene — 3.2.5 SQUALERIS

- Root contains canonical entry/state files only.
- Root `LICENSE` is complete GPL-3.0-only; third-party license boundaries remain intact.
- Build orchestration lives under `scripts/build_orchestration/`; root BUILD launchers and active `scripts/build/` are forbidden.
- `docs/` is live-only and contains no `history`, `historical`, `research` or `governance` archive wrappers.
- Governance mirror is direct `docs/PROJECT_GOVERNANCE_CONTRACT.md`.
- Active metadata lives under `metadata/`; historical exact-byte evidence is retained in Library provenance.
- FALCION is the final repository/package hygiene gate.

- HOTFIX19 is the active authorized source revision; immediate published parent HOTFIX18 SHA-256 `395084a6f6a19cbeed54b5faf249431357aa8fa5a2402a95dc66c47c921a30c0`. HOTFIX19 source snapshot remains prepublication until external G4 commit. Master `2026-10-18.2` governs; G3 is NOT_APPLICABLE for HOTFIX. Historical snapshots never define current status.
