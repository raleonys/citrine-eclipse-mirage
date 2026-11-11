# Citrine Eclipse Mirage agent workflow

This document is the operational entrypoint for agents and contributors. [GOVERNANCE.md](GOVERNANCE.md) is the canonical project policy; `docs/` describes the current design, and `metadata/` holds machine-checkable contracts. The owner's latest explicit decisions take precedence. Historical task notes are not new instructions.

## Branches and pull requests

- Never edit, commit, push, reset, delete, or force-update `main` directly. Only the owner merges reviewed PRs. Never self-approve, enable auto-merge, or bypass failed checks.
- Bugfix, routine audit and other single-PR tasks use fresh descriptive branches from fetched `main`; do not reuse a closed task branch or the retired shared `bugfix` branch. Remove a temporary head after its PR closes (merged or unmerged), after recording its tip and preserving dependencies and unmerged work. Inspect the working tree, refs and open PRs before writing; synchronize by fast-forward or normal merge, never by shared-history rewrite.
- An identifiable continuing research, reverse-engineering or reconstruction project may keep its dedicated branch across PRs. Ordinary `audit/*` branches are **not** retained by default: clean them up like other task branches. A long-running audit campaign may retain a dedicated branch **only with explicit owner designation** and documented continuing scope and closure criteria. Evaluate existing audit branches individually, never bulk-delete unresolved work or assume their names grant exemption.
- GitHub's repository-wide **Automatically delete head branches** setting may delete retained research and designated audit heads; do not enable it without verified protection exemptions. Retire sustained-work branches only after owner decision, SHA/provenance preservation and dependency review. See `docs/GITHUB_WORKFLOW.md`.
- Read current `AGENTS.md` and `GOVERNANCE.md` from fetched `main` and the working branch before modifying source. Consult `docs/GITHUB_WORKFLOW.md` for mechanics.
- Every PR with user-visible, QA, build, governance or compatibility impact must update `CHANGELOG.md` under unreleased development history, or obtain the owner-reviewed `changelog-exempt` label with an explicit rationale in its PR description. A PR merge is not a published release, and missing R revisions must not be invented.
- Open ordinary PRs ready for review; do not create or convert drafts without explicit owner authorization. Preserve the already owner-authorized **Draft research PR #81** (four-field source provenance and retirement review) in Draft state until the owner directs readiness; it is not a routine audit PR. Report exact scope, validation performed, failures and pending checks. Ready status is not approval or proof of native acceptance.
- Use English for new repository text, PR descriptions and commits. Use "foreign-language", "non-English" and "foreign-character scan" in language-audit work. Preserve ABI-sensitive internal identifiers.
- Use UTC for public project dates, schedules, release notes and PR descriptions. Do not publish owner location or regional time-zone names. Derive changelog dates from GitHub `merged_at` in UTC; preserve actual scheduled instants. Create new Git author/committer timestamps in UTC (`TZ=UTC`); do not rewrite accepted history or retained evidence as part of routine cleanup.
- Obtain explicit owner approval for public README edits, visibility changes, publication, license changes, new public disclosures and internal source/target renames.

## Source identity and QA

- `metadata/SOURCE_FILES.json` pins application/test/vendor inputs; `metadata/ACTIVE_SOURCE_MANIFEST.json` separately pins the whole deliverable. Review changed first-party bytes, paths and digests explicitly. Do not silently regenerate vendor pins or accept unrelated drift.
- Before pushing or opening a PR, commit reviewed changes and both applicable inventories, require a clean tree and run complete local `python -B scripts/run_source_qa.py` plus appropriate focused tests. Use the tracked `.github/hooks/pre-push`; never bypass it. CI independently checks pushed commits.
- Report only tests actually run. Source/static, portable, Wine, native Windows/MSVC, Golden, and real Consumer/game execution are distinct evidence classes. Neither a skipped check, old receipt nor synthetic fixture is a fresh native PASS.
- Keep generated build/test outputs under ignored `build/`, `dist/` or hosted workflow artifacts. Do not add new files under `docs/evidence/` or `metadata/evidence/`. Do not delete or rewrite retained historical evidence without a separately approved preservation review.
- PR creation runs lightweight source/hash integrity only. Full Source QA, Linux portable, Windows/MSVC, Golden and reference checks run after a `synchronize` update newly incorporates current `main`, or by a `run-full-validation` label added after a successful later synchronization verification for that exact PR/head/main. Standalone manual dispatch remains diagnostic. Ordinary commits on an already synchronized branch do not automatically start full validation. Never add `run-full-validation` or dispatch synchronization preparation merely because a PR was opened. The initial label alone must not start full validation. Focused diagnostics are manual; weekly release validation remains separate.
- Opening CI has one basic source-integrity job. CodeQL's three language analyses use the full-validation readiness gate for PRs and manual diagnostics; accepted-main pushes run only those CodeQL analyses in that workflow, without Source QA or Windows/Golden builds; the separate GitHub Default Setup must be migrated by an account with the required setting permission. Repository YAML does not disable an inaccessible server setting. See `docs/REPOSITORY_BADGES_AND_OPENING_CI.md`.
- When the owner requests synchronization, merge current `main` normally, push and inspect the exact-head full validation run. If no push is needed and no successful or active eligible PR full-validation run exists, explicitly prepare the later synchronization request using `operation=synchronize-pr`, the PR number and exact current main SHA; wait for that preparation run to succeed, then add `run-full-validation`. Remove an old/premature label before preparing, and never select standalone diagnostics to bypass this sequence; a readiness-only or skipped run is not an active test run. Do not create empty commits to trigger CI, duplicate automatic runs, relax required checks or count a skipped/unrequested test as PASS. See `docs/GITHUB_WORKFLOW.md` for the label request and retry commands.

## Technical invariants

- First-party sources follow `src/<module>/` and `include/citrine_eclipse_mirage/<module>/`; tests belong to `tests/`, pinned third-party source to `third_party/`. Preserve explicit build target ownership and separate production/reference/test-only link graphs. See `docs/BUILD_PARTITION.md`.
- The production DLL preserves exactly `DllMain@1`, `SURGE@2`, and `DllShutdown@3`. Do not add management exports, change ordinal/layout or mix the separate C++ development API into the public ABI. See `exports/citrine_eclipse_mirage.def` and `metadata/PUBLIC_DLL_ABI.json`.
- Keep `DllMain` loader-safe. Perform runtime/network/hook initialization and teardown outside loader lock. Preserve retries, held leases, callback drain, failure-atomic rollback, cleanup ownership and native quiescence. See `docs/ARCHITECTURE.md`, `docs/UNLOAD_RECOVERY.md` and `metadata/UNLOAD_RECOVERY_CONTRACT.json`.
- Preserve the scoped eleven-hook architecture and destination-aware Winsock redirect. A process-wide `ws2_32!htons` detour is prohibited. Protect original-call effects, LastError, rollback and trampoline ownership; use `docs/HOOK_ENGINE_HARDENING.md` and relevant `metadata/` contracts.
- Keep standard compiler CRT and the accepted exact four vendor trees: Asio, MinHook, PEGTL and pugixml. Preserve their source pins and required portable alias descriptors. No custom CRT, unapproved vendor modification, new VDF policy or default changes. Consult `metadata/VENDOR_LOCK.json`, `docs/VENDORING.md`, and parser contracts.
- Keep active SDK/listener/transport, persistence, configuration, avatar fallback, Windows filesystem and Debug-only diagnostic behavior unless a narrowly approved change proves equivalence. Current machine contracts, not superseded historical work orders, are authoritative.
- Preserve Golden PE layout, resource/import/export, checksum and source-pinning requirements; validate a finalized copy without corrupting raw build output. A Golden fixture PASS is not production DLL acceptance. See `docs/GOLDEN_PROTOCOLS.md` and `docs/VALIDATION.md`.
- Operator tools report the selected build profile and its validation result. Use the current build/distribution receipt names and Golden vocabulary consistently in scripts, policies and CI consumers; publication state belongs to the weekly release receipt. Source QA rejects retired build labels. Historical evidence and original archive names remain byte-exact.
- Do not import unrelated Brownfield application/CRT implementations or carry forward historical acceptance as current execution. Internal compatibility identifiers are not public project branding.

## Navigation and change discipline

README SVGs belong under `resources/badges/`. The long-lived `badges` branch
contains only the eleven generated SVGs in that same folder and is not a source
PR branch to retire after merge. Its bounded Git-data publisher may update
those images without application manifests; source changes still require the
normal inventories, local QA and tracked pre-push hook. Never use this data
branch or its automation to write main, application code or release assets.

Use `docs/INDEX.md` to distinguish current guides from historical checkpoints.
When code, workflow or contract changes, update its current documentation in
the same PR; keep dated results tied to their original source and do not copy
an old pending/PASS status into current guidance.

Read `docs/DEVELOPMENT.md` for build commands, `docs/GITHUB_WORKFLOW.md` for PR rules, `docs/VALIDATION.md` for evidence semantics, and `docs/WEEKLY_RELEASE.md` for the current weekly draft flow. Read relevant subsystem docs and exact metadata before touching affected behavior.

When current guidance conflicts with source contracts, report the discrepancy and reconcile only the smallest justified change. Never weaken or skip a test simply to obtain a green PR.
