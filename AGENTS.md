# Citrine Eclipse Mirage agent workflow

This document is the operational entrypoint for agents and contributors. [GOVERNANCE.md](GOVERNANCE.md) is the canonical project policy; `docs/` describes the current design, and `metadata/` holds machine-checkable contracts. The owner's latest explicit decisions take precedence. Historical task notes are not new instructions.

## Branches and pull requests

- Never edit, commit, push, reset, delete, or force-update `main` directly. Only the owner merges reviewed PRs. Never self-approve, enable auto-merge, or bypass failed checks.
- Use permanent `bugfix` for normal fixes, documentation, and tests. Before writing, inspect current branch, working tree, main/bugfix refs, and open PRs. Fetch `main` and synchronize by fast-forward or normal merge only; do not rewrite shared history. Preserve others' uncommitted work.
- Use dedicated branches only for explicitly scoped sustained audits/projects. Never delete main, bugfix, or active audit branches. Retire finished branches only after owner confirmation, preservation and open-PR review.
- Read current `AGENTS.md` and `GOVERNANCE.md` from fetched `main` and the working branch before modifying source. Consult `docs/GITHUB_WORKFLOW.md` for mechanics.
- Every PR with user-visible, QA, build, governance or compatibility impact must update `CHANGELOG.md` under unreleased development history, or obtain the owner-reviewed `changelog-exempt` label with an explicit rationale in its PR description. A PR merge is not a published release, and missing R revisions must not be invented.
- Open PRs ready for review, never as draft PRs. Report exact scope, validation performed, failures, and pending checks. Ready status is not approval or proof of native acceptance.
- Use English for new repository text, PR descriptions and commits. Use "foreign-language", "non-English" and "foreign-character scan" in language-audit work. Public project identity is `Citrine Eclipse Mirage`. Preserve ABI-sensitive internal identifiers.
- Obtain explicit owner approval for public README edits, visibility changes, publication, license changes, new public disclosures and internal source/target renames.

## Source identity and QA

- `metadata/SOURCE_FILES.json` pins application/test/vendor inputs; `metadata/ACTIVE_SOURCE_MANIFEST.json` separately pins the whole deliverable. Review changed first-party bytes, paths and digests explicitly. Do not silently regenerate vendor pins or accept unrelated drift.
- Before pushing or opening a PR, commit reviewed changes and both applicable inventories, require a clean tree and run complete local `python -B scripts/run_source_qa.py` plus appropriate focused tests. Use the tracked `.github/hooks/pre-push`; never bypass it. CI independently checks pushed commits.
- Report only tests actually run. Source/static, portable, Wine, native Windows/MSVC, Golden, and real Consumer/game execution are distinct evidence classes. Neither a skipped check, old receipt nor synthetic fixture is a fresh native PASS.
- Keep generated build/test outputs under ignored `build/`, `dist/` or hosted workflow artifacts. Do not add new files under `docs/evidence/` or `metadata/evidence/`. Do not delete or rewrite retained historical evidence without a separately approved preservation review.
- GitHub-centered PR QA and a weekly owner-approved stable-release draft are the intended system. The current private CI still requires manual Windows Native/Golden dispatch; never claim the future public PR checks and required-merge protections are currently active.

## Technical invariants

- First-party sources follow `src/<module>/` and `include/preloader_l/<module>/`; tests belong to `tests/`, pinned third-party source to `third_party/`. Preserve explicit build target ownership and separate production/reference/test-only link graphs. See `docs/BUILD_PARTITION.md`.
- The production DLL preserves exactly `DllMain@1`, `SURGE@2`, and `DllShutdown@3`. Do not add management exports, change ordinal/layout or mix the separate C++ development API into the public ABI. See `exports/preloader_l.def` and `metadata/PUBLIC_DLL_ABI.json`.
- Keep `DllMain` loader-safe. Perform runtime/network/hook initialization and teardown outside loader lock. Preserve retries, held leases, callback drain, failure-atomic rollback, cleanup ownership and native quiescence. See `docs/ARCHITECTURE.md`, `docs/UNLOAD_RECOVERY.md` and `metadata/UNLOAD_RECOVERY_CONTRACT.json`.
- Preserve the scoped eleven-hook architecture and destination-aware Winsock redirect. A process-wide `ws2_32!htons` detour is prohibited. Protect original-call effects, LastError, rollback and trampoline ownership; use `docs/HOOK_ENGINE_HARDENING.md` and relevant `metadata/` contracts.
- Keep standard compiler CRT and the accepted exact four vendor trees: Asio, MinHook, PEGTL and TinyXML2. Preserve their source pins and required portable alias descriptors. No custom CRT, unapproved vendor modification, new VDF policy or default changes. Consult `metadata/VENDOR_LOCK.json`, `docs/VENDORING.md`, and parser contracts.
- Keep active SDK/listener/transport, persistence, configuration, avatar fallback, Windows filesystem and Debug-only diagnostic behavior unless a narrowly approved change proves equivalence. Current machine contracts, not superseded historical work orders, are authoritative.
- Preserve Golden PE layout, resource/import/export, checksum and source-pinning requirements; validate a finalized copy without corrupting raw build output. A Golden fixture PASS is not production DLL acceptance. See `docs/GOLDEN_PROTOCOLS.md` and `docs/VALIDATION.md`.
- Do not import unrelated Brownfield application/CRT implementations or carry forward historical acceptance as current execution. Internal compatibility identifiers are not public project branding.

## Navigation and change discipline

Read `docs/DEVELOPMENT.md` for build commands, `docs/GITHUB_WORKFLOW.md` for PR rules, `docs/VALIDATION.md` for evidence semantics, and `docs/WEEKLY_RELEASE.md` for the current weekly draft flow. Read relevant subsystem docs and exact metadata before touching affected behavior.

When current guidance conflicts with source contracts, report the discrepancy and reconcile only the smallest justified change. Never weaken or skip a test simply to obtain a green PR.
