# DLL R&D Football — Project Rules Master

Status: **ACTIVE / RELEASE-INDEPENDENT / FAIL-CLOSED**  
Master revision: **2026-10-19.1**  
Scope: current and future `preloader_l` research, audit, remediation, internal source-state, release-candidate, hotfix, experimental and final-release work unless an explicit later user decision supersedes a rule.

## 0. Purpose, precedence and recovery

This Library-level file is the release-independent governance recovery master for DLL R&D Football. It exists so the project can be reconstructed after chat/context loss without relying on transient conversation state.

Precedence is:

1. newest explicit user decision for this project;
2. this Library `PROJECT_RULES_MASTER` after it has been updated to reflect that decision;
3. the packaged governance contract and machine-readable invariants of the current applicable official release;
4. release-specific state/provenance files;
5. historical governance/evidence, which remains preserved but does not override a newer explicit rule.

Every new standing project rule must be synchronized into this Library master. Before **any** official release is published (Final Release, Release Candidate, Hotfix Release or Experimental Release), that release's packaged governance mirror must be synchronized from the current Library master **normative rules only**. Library-only governance debt/backlog is explicitly excluded from the release mirror and from release-packaged machine-readable governance invariants. The release must record the Library master revision and SHA-256 used for synchronization. A conflicting or stale packaged governance mirror blocks publication until reconciled. Historical release snapshots are preserved rather than rewritten.

Recovery after context loss:

1. read `/DLL R&D Football/governance/PROJECT_RULES_MASTER.md` and `.json`;
2. locate the newest relevant release/source artifact in `/DLL R&D Football/releases/`;
3. read its `docs/PROJECT_GOVERNANCE_CONTRACT.md` and `metadata/PROJECT_GOVERNANCE_INVARIANTS.json`;
4. read current `RELEASE_STATE.json`, `SOURCE_REVISION.md`, `CHANGELOG.md`, acceptance metadata and applicable specialized contracts;
5. read `/DLL R&D Football/Remediations/` and `/DLL R&D Football/research/` only for the workstream being resumed.

## 0.0.1. DEVELOPMENT BUILD / RELEASE PUBLICATION SEPARATION — USER SUPERSESSION (2026-10-18)

**Newest explicit user decision; overrides conflicting pre-build clauses in Section 0.1 prospectively.** The release governance system was established to ensure ChatGPT-generated **release packages are correct**, not to prevent the user from compiling, debugging, testing, or building Windows binaries. Its former unconditional G0-before-any-build rule was an erroneous coupling and is retired for local build activity. No future assistant or tool may reinstate G0 as a mandatory prerequisite to invoking a compiler merely because release metadata or external Library pins are incomplete.

**Build without publication authority:** Local MSVC/CMake compile and link, debug, production/Golden pipeline, native/CRT test build, game experimentation and focused developer QA may run without external Library access, G0 release intent, current-release master SHA inputs, approval receipts or 8/8 authoritative release shards. The local toolchain, environment sanitization, build-directory confinement, source integrity, process exit codes, and the actual binary/test failure checks still apply. Developer artifacts must be clearly marked `LOCAL_BUILD`/`NOT_RELEASE` and cannot mutate any canonical Library release pointer. A local `dist/preloader_l.dll` is a build output, never official publication.

**Publication remains distinct and strictly verified:** Only a separately invoked, explicitly identity-authorized canonical release promotion may update `CURRENT_ARTIFACT` or report official release PASS. This action continues to require independent governance master/intent/source/parent pins, G0/G1/G2 receipt integrity, truthful G3 applicability, exact source 8/8 plus one fresh-extract 8/8 at Hotfix G4, atomic Library transaction/readback and postcommit verification. Failed checks block **promotion and claims of official acceptance**, not ordinary compilation. No `--skip-release-checks` promotion bypass is permitted.

**Research/support packages:** Scoped static QA, source and ZIP SHA/CRC, fresh extract, negative/tamper tests and honest unresolved findings remain expected for packages that claim those checks; an incomplete unrelated G0 release ceremony must not block research or tooling. Do not fabricate PASS or change immutable prior release receipts. HF17 is an immutable historical published release; HOTFIX18 became current after its separately verified official commit.

## 0.0.2. FINAL-ONLY GAME ACCEPTANCE — USER SUPERSESSION (2026-10-18)

**Latest explicit class-wide decision; effective immediately for current/future publication.** Only the release class `FINAL_RELEASE` may have **mandatory CollegeFB27/Madden27 game launch, runtime, network, gameplay, game-owned callback or game-acceptance evidence** as a publication prerequisite. For **every other class** (`HOTFIX_RELEASE`, `RELEASE_CANDIDATE`, `EXPERIMENTAL_RELEASE`, and non-release development/research/source states), the required-game-evidence set is **empty**. Neither G0/G1/G2, a separately targeted changed-surface test, G3=`NOT_APPLICABLE`, nor G4 may demand a game test, screenshot, game log, network receipt, current-game version, signoff, evidence freshness or a game-acceptance PASS before publication. No hook/transport/MinHook/CFG/Store/registry/XML or other changed surface creates an exception. The user need not supply game results for these non-Final classes; a pending/not-run optional game result cannot block their publication.

**Honest evidence without a gate:** Existing game evidence may remain in historical ledgers and separately user-requested game testing may still be performed, with actual provenance and `NOT_RUN`/`PENDING`/`PASS`/`FAIL` recorded truthfully. Such evidence is observational, never a publication dependency or a reason to repeat a run for non-Final classes. No earlier game's PASS is inherited or relabeled fresh. Mandatory source/ABI/security/build/integrity checks within each class remain intact, as do the mandatory Final-only G3/native/game requirements. Known source or safety failures cannot be hidden by making game acceptance inapplicable.

**Unconditional precedence and implementation obligation:** This rule explicitly supersedes every earlier or later-located generic and specialized clause requiring fresh game acceptance for HOTFIX or a changed hook/transport surface, including historical TinyXML2/MinHook HOTFIX11 wording. Historical exact-release assertions remain unmodified records, not current gate requirements. Any release checker, release QA catalog, scripted dependency, policy mirror or receipt issuer requiring game evidence for non-Final must be corrected; it must never treat those clauses as an active blocker. This is a release-class policy, not a discretionary waiver or an exception requiring new approval.

## 0.1. GOVERNANCE_FIRST_G0_TO_G4_RELEASE_GATE_CONTRACT — MANDATORY (2026-10-17)

**Authority and order.** Every newly prepared research/supporting package, internal source-state, intended official release, build handoff and canonical publication begins with a fresh Library-level master read. The latest explicit user decision prevails; this master is the normative synchronized second authority. G0 precedes production source mutation, source packaging, build, QA and handoff. The gate must not use an untrusted package-provided `PASS` as its own evidence. A missing/old master binding, wrong release type, stale parent or wrong authorization **stops work fail-closed**, not just marks QA incomplete. Every builder and publication entrypoint must actually invoke the gate, and independent fresh-extract verifier must replay it. Shipping must not rely on a bypassable standalone document or manual checklist.

**G0 — Governance First / preflight:** verify current master MD/JSON revision and independent SHA-256; newest explicit user instruction, intended official identity, classification (only FINAL_RELEASE, RELEASE_CANDIDATE, HOTFIX_RELEASE, EXPERIMENTAL_RELEASE or explicitly NON_RELEASE_WORK), exact accepted parent hash, declared scope, release vs publication authorization, and original source/ZIP integrity. A non-release source/research/support package is not a fifth release type. **Stop before source mutation/build/QA/packaging on failure.** A hotfix being developed has release type `HOTFIX_RELEASE` with PENDING acceptance, *not* `HOTFIX_PREPUBLICATION_CANDIDATE`; no claim of publication follows from that type.

**G1 — Repository/metadata:** cross-check RELEASE_STATE, RELEASE_AUTHORIZATION, GOVERNANCE_BINDING, PUBLICATION_TRANSACTION, acceptance freshness/ownership, source manifest, CHANGELOG, license and source/repository layout. Mirror current Library *normative-only* governance into docs/PROJECT_GOVERNANCE_CONTRACT.md and metadata/PROJECT_GOVERNANCE_INVARIANTS.json; bind independent current master MD+JSON SHA and revision. Do not silently import Library-only debt or rewrite historical published snapshots. Preserve inherited historical QA behavior; never relax an acceptance condition merely to force PASS.

**G2 — Class-aware source/package QA:** G0/G1 identity, manifest, exact ZIP/CRC/SHA, ownership, focused changed-surface source/behavior QA and tamper negatives remain required for **every official release type**. Run the authoritative source **8/8** and one clean-extract **8/8** here for `FINAL_RELEASE` only. For `HOTFIX_RELEASE`, `RELEASE_CANDIDATE` and `EXPERIMENTAL_RELEASE`, **do not run the eight-shard universe in G2**; record it as deferred until **G4**, when it is mandatory. No release-class exception permits false PASS, relaxed historical tests or skipped focused G2 checks.

**G3 — Final Release Windows/native/Consumer (FINAL_RELEASE ONLY):** this formal gate applies **only** to `FINAL_RELEASE` and requires actual Windows/MSVC compile+link, exact binary SHA, applicable native/CRT/Golden and CollegeFB27/Madden27 game/network acceptance, with traceable per-class evidence and freshness. Static/test-only mock/portable tests cannot substitute for a required Final native or game receipt. For `RELEASE_CANDIDATE`, `HOTFIX_RELEASE` and `EXPERIMENTAL_RELEASE`, record G3=`NOT_APPLICABLE` (never `PASS`, never `SKIPPED_PASS`); pending or failed optional/native investigations remain truthful in the acceptance ledger, but G3 does not gate those classes. This release-class rule supersedes older generic G3-for-all and changed-surface G3-publication wording **prospectively**; historical exact-release gates/evidence are not rewritten. Applicable G0/G1/G2 and separately authorized focused source/security checks remain mandatory and cannot be bypassed using G3=`NOT_APPLICABLE`.

**G4 — Canonical transaction:** enforces the class-specific eight-shard schedule above before publication (Final: rerun source 8/8 + fresh-extract 8/8; Hotfix/RC/Experimental: first and only authoritative source 8/8 + fresh-extract 8/8 at this stage). Requires **new explicit identity-specific user publication authorization** distinct from source development and independent **G0/G1/G2 PASS for every official release type**. Require **G3 PASS only when release type is `FINAL_RELEASE`**; for `RELEASE_CANDIDATE`, `HOTFIX_RELEASE` and `EXPERIMENTAL_RELEASE`, require a truthful G3=`NOT_APPLICABLE` receipt and **do not require G3 execution or PASS**. G4 still requires exact official artifact+sidecar+evidence upload and byte-identical Library readback, atomic/idempotent CURRENT_ARTIFACT commit, post-commit verification and only then supersession of the previous official release, subject to the type-specific shipping/current-pointer policy. Production preparation, canonical identity and development approval are **not** publication. On failure the previous official current pointer remains unchanged.

### 0.1.1 Authoritative 8-shard QA scheduler — release-class + stage aware (2026-10-17.4)

**Explicit user decision:** Full deterministic 8-shard QA must not run at every gate for every release. Its invocation depends on both the **official release class** and the **G gate**. This controls QA fan-out, **not** whether G0/G1 metadata and G2 focused QA/integrity are mandatory.

| Classification | G0 | G1 | G2 | G3 | G4 |
|---|---|---|---|---|---|
| `FINAL_RELEASE` | source 8/8 | source 8/8 | source 8/8 + clean extract 8/8 | source 8/8 **and** native/Consumer acceptance | source 8/8 + clean extract 8/8 |
| `HOTFIX_RELEASE` | focused G0 only | focused G1 only | focused G2 QA/integrity, **no eight shards** | `NOT_APPLICABLE` | **source 8/8 + clean extract 8/8**, then publication |
| `RELEASE_CANDIDATE` | focused G0 only | focused G1 only | focused G2 QA/integrity, **no eight shards** | `NOT_APPLICABLE` | **source 8/8 + clean extract 8/8**, then publication |
| `EXPERIMENTAL_RELEASE` | focused G0 only | focused G1 only | focused G2 QA/integrity, **no eight shards** | `NOT_APPLICABLE` | **source 8/8 + clean extract 8/8**, then class-authorized publication |
| `IDS`, source-state, checkpoints and other `NON_RELEASE_WORK` | independent G0 preflight and scoped QA; **no eight shards** | not a release gate | focused provenance/integrity only; **no eight shards** | not a release gate | **prohibited**; **no eight shards** |

**Execution contract:** G0's basic master/identity preflight executes first, before invoking any scheduled full shard fan-out. For Final, run the shard universe **once upon completion of each gate** against that gate's exact source state; a changed source SHA invalidates the previous gate's shard evidence. For all official release classes, G4's required shards **must PASS on the exact artifact to be committed** before the Library publication transaction; source QA means shards 0..7, and `clean-extract 8/8` means exactly one clean extraction plus shards 0..7 on that tree, not eight extractions. Final G2/G4 each requires its own stage receipt; G4 cannot silently inherit a prior G2 shard PASS. A gate's helper invocations must not rerun the universe redundantly.

**Internal states are never official releases:** IDS/source-state and evidence/checkpoint packages have **zero mandatory eight-shard invocations**, but always retain their strict SHA/manifest/ZIP/focused changed-surface tests, rerunnable regressions, and honest unresolved-acceptance ledger. If later used as an official release source, the official release begins a newly authorized G0→G4 flow with its type-specific full shard scheduling.

**Fail-closed:** No non-Final G4 may commit without its newly completed, exact-source **8/8 worktree + 8/8 clean-extract** results. No Final gate may be recorded PASS without its mandatory eight shards. No self-asserted PASS, different-hash reuse, false NOT_APPLICABLE, altered historical acceptance or shortened QA universe is permitted. Existing published HF17 G2 8+8 PASS and governance master 2026-10-17.2 evidence are **immutable history**, not amended retroactively.

**Gate-class precedence:** class-specific G3 applicability and §0.0.2 govern all current/future publication. Old hotfix/changed-surface game-acceptance wording is historical evidence only and cannot be reauthorized through a targeted gate as a mandatory non-Final game acceptance. Applicable independently required source/security checks retain their ordinary fail-closed behavior. A recorded failure of an actually executed safety test may still require independent disposition; `NOT_APPLICABLE` is not permission to hide a known failure or misstate test results.

**Required machine/entrypoint enforcement:** the release source embeds an owned governance gate checker plus SHA-bound G0–G4 receipts and mandatory calls from production/build QA/sealing/pub entrypoints. The *first* action of every entrypoint is G0, including Windows handoff and post-extraction verification. No `--skip-governance`, unverified pass-carry, or identity fallback. Every failure produces a machine-readable BLOCKED receipt with cause. Governance gate tools/checkers must have explicit metadata/source ownership and be covered by regression cases (wrong type/parent/master, stale mirror, missing authorization, false current, tamper, forged PASS). Gate implementation can be introduced incrementally, but while any entrypoint remains unbound, **project-wide enforced** must NOT be claimed and publication remains blocked.

**HF17 official post-commit reconciliation (2026-10-17):** the user explicitly authorized canonical publication of `preloader_l 3.2.5 SQUALERIS HOTFIX 17` as `HOTFIX_RELEASE`. The exact immutable source SHA-256 is `1e629c07eeb21edaefd6f7f6fd9481ddb550116f8e77078cd02b1d11baa090b8`; it was verified G0/G1 PASS, G2 8/8 working-tree + 8/8 fresh-extract PASS, G3=`NOT_APPLICABLE` (not PASS), and G4 Library publication and `CURRENT_ARTIFACT.json` commit PASS, with pointer SHA-256 `7e14e2b0df82effb7826381295ba90a7fd3dfefadb5774b44e27f1a55201b836`. Exact previous canonical HF16 source SHA-256 `9b380e4d953dffd03ace8b762e1199f11ca8ca4f373d86a0c9b3c074d647082c` remains immutable and retained as the immediate historical predecessor. Windows/MSVC, Golden, CRT, native and both games were not executed for HF17; do not inherit HF16 runtime acceptance. The sealed HF17 source and release receipts were correctly pinned to master `2026-10-17.2`; this `2026-10-17.3` master reflects **post-commit Library state only**. Do not rewrite historical source/mirror/receipts or imply that G3 ran. New future source development must use **the now-current HF17** as cumulative parent unless the user explicitly directs another lineage.

## 1. Official release taxonomy, identity and publication

The project has exactly **four official release types**. No fifth release class, wave release, checkpoint release, audit release or remediation release may be introduced unless the user explicitly changes this rule.

### 1.1 FINAL RELEASE

Purpose: the authoritative stable semantic release intended as the normal production/shipping baseline.

- Identity is the semantic version + explicitly approved codename, with release type recorded as `FINAL`.
- A Final Release is feature/fix complete for its semantic scope and is promoted only by explicit user decision.
- It must satisfy the complete applicable release-acceptance stack: authoritative source QA partitioned across 8 deterministic shards; Final Release packaging; one exact-package integrity/manifest/hash verification pass; one clean extraction of that exact package into a new empty directory followed by the same 8 source-QA shards against that single extracted tree; final SHA-256/provenance binding; and all required Windows/MSVC/native/game/network acceptance for the changed surface.
- Once promoted, it becomes the canonical stable baseline for subsequent work unless a later accepted Hotfix Release becomes the current shipping baseline or a newer Final Release supersedes it.
- Final promotion is a publication decision, not an automatic consequence of passing tests.

### 1.2 RELEASE CANDIDATE

Purpose: an **official pre-final release** for a semantic version/codename whose intended scope is substantially complete but which may still require final polish, release-blocking correction, and/or completion of one or more Final-level acceptance classes before Final promotion.

- Identity is the target semantic version + approved codename with release type `RELEASE_CANDIDATE`; the RC ordinal is part of that official release identity/provenance.
- An RC is already an official published release artifact. It is **not** a candidate for another subordinate release artifact, source-candidate layer, staging release, or pre-RC identity.
- The intended Final scope is substantially complete before RC designation. After RC designation, only release-blocking corrections/final polish are added unless the user explicitly reopens scope.
- Every RC binds an exact source parent, complete delta/provenance and a packaged **official RC artifact**.
- Final-level acceptance classes may be explicitly `PENDING` on an RC. Pending acceptance is represented in the acceptance freshness/ownership ledger and does not create a second-level candidate/source-candidate release identity.
- RC validation must be sufficient for the claims actually made by that RC. If the eventual Final Release is byte-identical in all release-relevant source/package inputs to an already accepted RC, exact hash/provenance identity may be used to carry forward still-applicable acceptance evidence instead of rerunning unchanged validation. Any byte, packaging, build, QA-infrastructure or acceptance-relevant change invalidates that reuse for the affected class.
- Promotion from RC to Final always requires explicit user approval.

### 1.3 HOTFIX RELEASE

Purpose: a bounded corrective production release derived from an already accepted Final/Hotfix lineage to resolve validated defects or regressions without opening unrelated scope.

- Identity retains the owning semantic version + codename and records release type `HOTFIX`; any hotfix ordinal belongs only to release identity/provenance, never active production filenames.
- `CLEAN_MINIMAL_FIX_STANDARD` is mandatory: smallest complete defect-closing delta; no unrelated refactor, feature, naming cleanup or speculative hardening.
- The first Hotfix Release derives from the exact accepted production parent. If several hotfixes are required, each next Hotfix Release uses the exact immediately preceding accepted Hotfix source as parent. There is no separate merge stage.
- A Hotfix Release records the defect(s), root cause, exact parent, source delta, validation performed, remaining carry-forward and acceptance freshness.
- As a published production release it must pass the authoritative source/package QA and all **applicable G0/G1/G2** and focused changed-behavior checks. **G3 is `NOT_APPLICABLE` for HOTFIX_RELEASE. CollegeFB27/Madden27 game evidence is never a Hotfix publication prerequisite, regardless of any changed hook/transport surface.** Optional results, if collected, are honestly recorded but cannot block Hotfix G4. Historical release-specific native evidence and immutable accepted receipts are not reclassified.
- The newest accepted Hotfix Release may become the current shipping baseline for that semantic release while the historical Final baseline remains preserved.

### 1.4 EXPERIMENTAL RELEASE

Purpose: an explicitly non-stable publication for unresolved research, diagnostics, instrumentation, architecture experiments, risky mechanism validation or proof-of-concept behavior.

- Identity records release type `EXPERIMENTAL` and the exact semantic/codename lineage it derives from.
- It must be unmistakably marked non-final/non-production and must never silently become canonical or shipping.
- Intentional divergence from shipping behavior is allowed only when bounded, documented and necessary for the experimental purpose.
- Every Experimental Release records exact parent/provenance, purpose, non-goals, intentional divergences, safety boundary and focused validation.
- Experimental-only code, behavior or evidence cannot flow into RC, Hotfix or Final merely because it worked experimentally; it requires explicit disposition, clean/minimal implementation where applicable, and revalidation in the target release class.

### 1.5 Publication boundary

- **Only these four types may be published as release artifacts:** Final Release, Release Candidate, Hotfix Release, Experimental Release.
- `REMEDIATION_READY`, audits, research packages, diagnostics, reports, patches, QA logs, evidence bundles, checkpoints and internal source states are supporting/internal artifacts, **not release types**. They may be retained in Library for provenance/recovery but must not be presented or published as releases.
- The prior named Wave model (`W1`, `W2`, etc.) is **SUPERSEDED and abolished prospectively**. Historical Wave records remain immutable historical provenance only.
- Internal authorized remediation work still follows exact-parent discipline: each successive accepted internal source state derives from the exact immediately preceding accepted source state unless the user explicitly declares a new lineage. No final bulk merge model is permitted.
- Active **source/module/test/tool filenames** never encode RC/HOTFIX/EXPERIMENTAL or old Wave bookkeeping. This prohibition does not apply to official release artifact/package filenames, which must serialize the complete canonical official release identity under Section 1.9. Release-type markers otherwise belong only in release identity, publication metadata and provenance where required.
- There is **no** current `FULL SOURCE`, `FULL HISTORY`, `SOURCE CANDIDATE`, Wave, checkpoint, audit or remediation release class. Only the four official release types in this section may be published as releases. Historical artifact filenames containing obsolete terms remain immutable provenance and are not renamed retroactively.
- Published official release packages and release-bound exact-byte provenance remain immutable. Evidence contents and their original source hashes/derivation must remain traceable. Superseded research/audit wrapper containers may be removed only through the user-authorized, fully verified content-compaction transaction in Section 18.2; content immutability is not a requirement to retain every duplicate wrapper forever.
- Release history is consolidated by semantic identity while detailed technical history remains traceable: **identity is consolidated, history is preserved**.

#### 1.5.1 NO_CANDIDATE_OF_A_RELEASE_CONTRACT

Preparation state and release state are distinct. No official release may itself acquire a subordinate candidate/staging release identity.

- Final Release, Release Candidate, Hotfix Release and Experimental Release are the only official release identities.
- `SOURCE_CANDIDATE`, `CANDIDATE_SOURCE`, `RC_CANDIDATE`, `HOTFIX_CANDIDATE`, `PRE_RC`, `PRE_HOTFIX`, `PRE_RELEASE`, `STAGING_RELEASE` and equivalent candidate-of-a-release identities are forbidden in active release identity, package identity, publication status and current-release naming.
- Work performed before explicit Final/RC/Hotfix production authorization remains release-neutral internal state: remediation, internal source state, planning/readiness work, focused QA/evidence or other supporting artifact.
- A Release Candidate is already an official RC release; a Hotfix Release is already an official corrective production release. Neither receives a second candidate layer.
- Historical filenames using obsolete candidate/source-candidate wording remain immutable provenance only and must not be reused as active naming templates.

### 1.5.2 Internal Development State (IDS) — authorized accelerated internal development workflow

**Status: ACTIVE / NORMATIVE / FAIL-CLOSED.** Explicitly approved by the user on 2026-10-15 for prospective DLL R&D Football work. **IDS is a workflow/state classification, NOT a fifth official release type**, and it never modifies the four-type Final/RC/Hotfix/Experimental taxonomy. `NOT_RELEASE` is a safety/status label, not a release identity. IDS is not a return to the abolished Wave model or a `SOURCE_CANDIDATE` release class. Historical artifacts keep their exact names and bytes.

**Purpose and boundaries:**

- Use IDS for incremental, user-authorized implementation/remediation, technical debt retirement and source-linked research validation before any official publication. IDS may contain real source changes but must never be represented as a shipping/accepted release or silently activate a release ordinal, semantic-version bump, publication, canonical pointer, or official release artifact.
- Every IDS state is a **complete cumulative source snapshot** with an unambiguous internal state ID and `NOT_RELEASE` status. Each state derives from the SHA-256-pinned **immediate preceding accepted internal source state**, or from an exact official release when explicitly starting a new branch. No skipped parent, unbound cherry-pick, retroactive rewrite or end-stage bulk merge is allowed. Any deliberate fork/reset requires an explicit lineage decision recorded before applying changes.
- The unmodified authoritative official release remains immutable throughout IDS development. Internal source-state artifacts and research/evidence go to appropriate internal Library areas; they never update `/releases/` as a new published release and never change `CURRENT_ARTIFACT.json`.
- IDS package naming must be **release-neutral**, identify the technical workstream/state and display `IDS`/`NOT_RELEASE` without encoding `FINAL`, `RC`, `HOTFIX` or `EXPERIMENTAL` as a new release identity. Identifiers inside source/module/test/tool filenames remain responsibility-based and independent of development-state bookkeeping.

**Minimum required QA/evidence for every materialized IDS state:**

1. Record exact parent source/archive hash, current source archive hash, full member/active-source manifest, protected files, deterministic parent-to-child file delta, reason/scope/non-goals, and source/QA ownership. Validate archive CRC/path safety/duplicate names and exact-byte manifest integrity; exercise a clean extraction with at least focused gates relevant to the changed surface.
2. Run targeted **executable** regressions for changed behavioral paths whenever runnable, source/compile or ABI/link boundary checks where applicable, repository/production source graph/build-policy/CRT/exports checks affected by the delta, and targeted negative/fail-closed cases for ownership, lifecycle, cancellation or transaction changes. A syntax-only pass may be claimed only as syntax, never runtime/native acceptance.
3. Maintain a machine-readable **deferred acceptance and freshness ledger** for all relevant validation classes, including source 8-shard QA, clean-extract 8-shard QA, MSVC build/link, native IOCP, Golden/CRT, game/network and any workstream-specific gates. Record `PASS_FRESH`, `PASS_CARRIED_FORWARD`, `PENDING`, `FAIL`, `BLOCKED` or `NOT_APPLICABLE` with source hash, evidence, owner, invalidation triggers and exact carry-forward boundary. Not-run or timed-out work must remain pending; failed mandatory minimal gates block advancement.
4. Preserve verification logs and an independently rerunnable bounded verifier as evidence where feasible. A SHA/hash/manifest PASS never substitutes for compile, executable, native, game or network PASS. Any manual exception must state the unsupported class and must not promote its status.

**Progression and risk control:**

- Small, independently testable, cumulatively applied slices are preferred. Do not delay basic targeted validation or continue across a failing minimal gate merely to gain speed. Disputed ownership/ABI/consumer behavior, native-only correctness, security-critical change, or growing deferred-native-test exposure triggers an explicit integration/Windows gate or documented user risk acceptance before further dependent changes.
- IDS may defer authoritative 8/8 working-tree + 8/8 clean-extract QA and any **release-class-applicable** Windows/Golden/native acceptance as explicitly recorded debt, never an implicit PASS. **Game acceptance debt only exists for a prospective `FINAL_RELEASE`; it is not a requirement or publication blocker for Hotfix/RC/Experimental/non-release work, even when the hook/transport surface changes.**
- The last accepted IDS snapshot is an eligible **source parent for release preparation**, not an automatically authorized Final/RC/Hotfix. Any official artifact still requires exact identity, explicit user production/publication instruction, synchronized governance mirror, release-class-applicable QA/native evidence, **game evidence only for FINAL_RELEASE**, and transactional publication. An IDS cannot itself be promoted by renaming a ZIP or updating the canonical pointer.

**Scope and grandfathering:** This rule applies prospectively. All previously published releases, evidence archives and prior checkpoints remain byte-identical; migration or retrospective relabeling is prohibited. The governance Library master MD and machine JSON must express the same IDS contract, with sidecar SHA-256 updated together; future official release mirrors import normative IDS rules while excluding Library-only debt.

### 1.6 Release identity immutability and artifact correction

- Release identity is immutable once a release is designated/published. Its semantic version, approved codename and one of the four official release types remain unchanged for that release lineage.
- Packaging/repackaging, rebuild, respin, re-upload, metadata correction, documentation correction, regenerated ZIP/container bytes, renewed sidecar, or other artifact-maintenance action is **not** a new release type and must never be encoded as a release-name modifier.
- Forbidden active release-name/type modifiers include examples such as `REPACK`, `REPACKED`, `REBUILD`, `RESPIN`, `RERELEASE`, `FIXED`, `CORRECTED`, `REFRESH`, `REV`, `REVISION`, `V2`, `FINAL2`, `COMPLETE`, `SOURCE_CANDIDATE`, `FULL_SOURCE`, checkpoint/audit labels, or any equivalent fifth-class/variant wording.
- A Final Release remains a **Final Release**; a Release Candidate remains a **Release Candidate**; a Hotfix Release remains a **Hotfix Release**; an Experimental Release remains an **Experimental Release**. RC/hotfix ordinals that are part of the applicable standard class identity (for example `RC02` or `HOTFIX01`) are not separate release classes.
- If artifact bytes must be corrected while the release identity remains applicable, the correction is represented only through artifact provenance: exact hashes, timestamps/version records where applicable, and explicit `supersedes_artifact` / `superseded_by_artifact` binding. The release name/type itself must not change merely because artifact bytes were regenerated.
- A genuinely new Hotfix Release or newer semantic Final/RC/Experimental release may be created only when its normal release semantics are actually met; it must not be invented as a naming workaround for repackaging.
- Publication/repository QA must reject any release artifact or metadata that introduces a non-standard release class or a repack/rebuild/correction suffix as part of release identity.

### 1.7 User-controlled semantic-version frontier and release authorization

The semantic-version frontier is controlled exclusively by explicit user instruction.

- The assistant must never advance to a newer semantic version autonomously, including for Experimental Releases.
- If the currently authorized semantic version is `3.2.5 SQUALERIS`, no `3.2.6` release of any class may be created until the user explicitly authorizes the `3.2.6` version bump.
- QA PASS, remediation completion, packaging need, architecture progress, release readiness or assistant judgment does not authorize a semantic-version bump.

**Release Planning Authorization**

- The user may authorize planning for a prospective Final Release, Release Candidate or Hotfix Release, including scope, exact parent, proposed ordinal/identity, fix set, validation plan, acceptance criteria, dependency/blocker analysis and release-readiness preparation.
- Planning authorization is preparatory only. It does **not** authorize creation, packaging, materialization or publication of the planned Final/RC/Hotfix release artifact or identity.

**Release Production Authorization**

- Actual creation/publication of a Final Release, Release Candidate or Hotfix Release requires a separate explicit user instruction naming or unambiguously authorizing the exact release identity to produce.
- Production authorization must not be inferred from planning authorization, QA success, readiness, previous discussion or generic continuation instructions such as `devam`.
- A planned release may remain planned indefinitely and is not produced until explicit production authorization is given.
- New RC ordinals and new Hotfix ordinals are production release identities and therefore require explicit production authorization.

**Experimental Release**

- The assistant may initiate an Experimental Release autonomously only within the currently user-authorized semantic-version frontier and only as explicitly non-production.
- Experimental work cannot advance the semantic-version frontier and never authorizes or implies a Final/RC/Hotfix release.
- Creating an Experimental Release does not reserve, activate, consume or approve the next semantic version.

This authorization model is cumulative with Section 1.6: artifact correction/repackaging does not create a new release identity, and only the four official release classes may exist.


### 1.8 Publication authorization channel

- This DLL R&D Football project conversation/page is the project’s authoritative command channel for **Final Release, Release Candidate and Hotfix Release production/publication/finalization**, and for any canonical/current artifact-pointer change.
- No Final Release, Release Candidate or Hotfix Release may be produced, packaged as that release identity, published, finalized, promoted, superseded as current/canonical, or have its canonical artifact pointer changed without an **explicit user command in this project conversation/page** authorizing the exact production/publication action.
- Experimental Release is the sole exception to production authorization: under Section 1.7 it may be initiated, packaged and published autonomously only within the currently user-authorized semantic version and only as explicitly non-production/non-canonical. It may not advance the semantic-version frontier, become current/canonical shipping state, or transition to Final/RC/Hotfix without the applicable explicit user authorization.
- Passing QA, completing acceptance, preparing an artifact, producing a hash, or concluding that no technical blockers remain does **not** itself authorize Final/RC/Hotfix production or publication. Readiness and production/publication authorization are separate states.
- A generic continuation command authorizes continuation of the already-scoped work, but it does not silently create a new Final/RC/Hotfix release identity, new RC/Hotfix ordinal, semantic-version bump or broader release scope.
- Production/publication authorization provenance for Final/RC/Hotfix must record the explicit user decision and the exact artifact identity/hash being made current. Experimental artifacts record their non-production status and exact parent/provenance.


### 1.9 CANONICAL_OFFICIAL_RELEASE_IDENTITY_GRAMMAR

For the user-authorized production-track release classes **Final Release, Release Candidate and Hotfix Release**, the project has one canonical complete human-readable identity grammar. Semantic version + codename alone is never a complete official release/build identity for these classes.

Canonical identities are exactly:

- **Final Release:** `preloader_l X.Y.Z CODENAME FINAL RELEASE`
- **Release Candidate:** `preloader_l X.Y.Z CODENAME RELEASE CANDIDATE NN`
- **Hotfix Release:** `preloader_l X.Y.Z CODENAME HOTFIX NN`

Grammar rules:

- `preloader_l` is the literal project/product prefix.
- `X.Y.Z` is the semantic version. `X` is a non-negative decimal integer. `Y` and `Z` are each exactly one decimal digit in the range `0` through `9`; therefore minor or patch components such as `10`, `11`, etc. are forbidden. Version progression rolls the appropriate higher component instead of creating `X.10.Z` or `X.Y.10`.
- `CODENAME` is the exact approved LUTRAVEX codename bound to that semantic version and is rendered in its canonical uppercase form.
- `NN` is an exactly two-digit, zero-padded ordinal in the range `01` through `99`. `00`, one-digit ordinals and three-or-more-digit ordinals are forbidden.
- Final Release has no ordinal. Release Candidate and Hotfix Release always have the required `NN` ordinal.
- Canonical human-readable identities use the literal words `FINAL RELEASE`, `RELEASE CANDIDATE NN`, or `HOTFIX NN`. Shorthand such as `RC01`, `HOTFIX06`, bare `FINAL`, or semantic version + codename alone may exist only in explicitly historical/provenance/storage contexts; they are not the canonical complete build/release display identity.
- This exact production-track grammar does not redefine `EXPERIMENTAL_RELEASE` naming; Experimental remains governed by its existing explicit non-production identity rules until a separate exact grammar is explicitly approved.

The complete canonical identity must be used consistently on release-facing/current surfaces, including:

- human-facing build/release name and `BUILD_ID` heading/display;
- `SOURCE_REVISION` release heading/display;
- `RELEASE_STATE` / current-artifact canonical display field;
- official release report/title and publication metadata;
- official release package/archive filename serialization.

Official package/archive filenames deterministically serialize the canonical identity by replacing spaces with underscores while preserving token order:

- `preloader_l_X.Y.Z_CODENAME_FINAL_RELEASE.zip`
- `preloader_l_X.Y.Z_CODENAME_RELEASE_CANDIDATE_NN.zip`
- `preloader_l_X.Y.Z_CODENAME_HOTFIX_NN.zip`

Storage-directory shorthand may exist for repository organization only when it is unambiguously bound to the canonical identity in metadata; it must never replace the canonical display identity.

The canonical project release identity is separate from donor-facing/compatibility-sensitive PE metadata. This rule does **not** authorize changing Golden/donor-compatible VERSIONINFO, binary ABI metadata or other externally constrained binary identity fields merely to embed project release terminology.

Release/repository QA fails closed when:

- a Final/RC/Hotfix build or publication surface omits the release class or required ordinal;
- `Y` or `Z` exceeds one decimal digit;
- an RC/Hotfix ordinal is missing, non-zero-padded, `00`, or above `99`;
- codename/version binding disagrees with the approved LUTRAVEX registry;
- two current release-facing surfaces serialize different official identities;
- an official release package filename does not serialize the same complete canonical identity recorded in release metadata.

Historical published artifacts remain immutable and are not retroactively renamed. This grammar applies prospectively to newly produced official release identities/artifacts and to current source/build policy after adoption.

## 2. Codename governance — LUTRAVEX Golden Naming Scheme

- One semantic version has one approved codename prospectively; one codename identifies one semantic version prospectively.
- Roots are globally reserved across project history. Previously used/reserved roots are not reused for unrelated identities; provenance-preserving derived lineage is allowed only when explicitly intentional.
- New codenames follow an original Latin-animal-root + synthetic/Bakugan-like suffix style.
- Avoid overused suffix/root families; vary root family, suffix family, syllable structure, length and phonetic shape.
- **Codename generation is performed in 10-semantic-version batches by default.** A batch remains proposed until explicitly approved by the user.
- Before proposing/assigning names, check release names, package names, changelog, protocols/gates and the LUTRAVEX registry for collisions.

Current approved 3.2.x assignments:

- 3.2.0 SCIURONYX
- 3.2.1 CERVRAX
- 3.2.2 MUSTORIN
- 3.2.3 MELOTRIX
- 3.2.4 RANQUEL
- 3.2.5 SQUALERIS
- 3.2.6 TALVOR
- 3.2.7 ERINTHAR
- 3.2.8 FORMADRIL
- 3.2.9 CETRION

## 3. Behavioral evidence/source hierarchy

The current effective hierarchy is:

1. **Main behavioral authority: Consumer + Donor1 (Anadius).**
2. **Supporting protocol/error evidence: Rust SDK + Maxima.**
3. **Supplementary/reference only: historical Primary (`version.dll` + `winmm.dll`).**
4. **Secondary confirmation/reference: Donor2 / 1911.**

Rules:

- Historical Primary is not the project’s primary authority, not a mandatory parity target and has no veto over aligned Consumer + Donor1 evidence.
- `PRIMARY_EXACT` is deprecated and must not be used as a current approval target/quality tier.
- Rust SDK/Maxima are especially useful for route schemas, typed errors, response families and opaque numeric error semantics, but do not independently replace Consumer + Donor1.
- Blaster/Blaster12 is not an independent evidence source; it collapses into its underlying `version.dll` + `winmm.dll` evidence.
- Evidence does not authorize source mutation automatically. Consumer compatibility, scope, failure semantics and explicit disposition are still required.

## 4. Explicit route/error contract rule

A missing handler, silence, dropped request, timeout or generic unknown-route result is not considered semantically equivalent to a known evidence-backed route contract merely because the eventual UI outcome looks similar.

When Consumer + Donor1, with applicable Rust support, identifies the intended contract, implement it explicitly and route-locally. Relevant dimensions include route recognition, dedicated handler semantics, response envelope, error type/code, intentional no-response classification, session continuity and failure ordering.

Do not fabricate behavior where evidence is absent.

## 5. Audit, finding lifecycle and change authorization

- Audit/research and remediation are separate lineages. Audit source is immutable.
- Do not patch an unresolved root merely to make a test pass; dedupe, root cause, authority and disposition come first.
- Current finding lifecycle statuses are exactly:
  - `OPEN`
  - `EVIDENCE_BLOCKED`
  - `REMEDIATION_READY`
  - `DEFERRED`
  - `CLOSED_FIXED`
  - `CLOSED_NO_CHANGE`
  - `SUPERSEDED`
  - `DUPLICATE`
- Severity, confidence and release impact are separate axes.
- Completion/acceptance states remain distinct: `IMPLEMENTED`, `VERIFIED_SOURCE`, `VERIFIED_NATIVE`, `RELEASE_ACCEPTED`.
- Production/runtime/protocol/config/ABI changes are classified before mutation using the Change Authorization Matrix (e.g. `RESEARCH_ONLY`, `DOC_ONLY`, `QA_ONLY`, `ARCHITECTURE_ONLY`, `BEHAVIOR_FIX`).
- `ARCHITECTURE_ONLY` work requires an explicit semantic-delta check.
- Supersession records use explicit `supersedes`, `effective_from` and `reason` fields where applicable.

## 6. Automatic pre-release REMEDIATION_READY retention

For validated findings, bugs, bug reports and other evidence-backed defect classes, once the root cause and a concrete fix path are sufficiently established, the remediation must be packaged immediately in the same workstream as a self-contained `REMEDIATION_READY` artifact so it cannot be lost before the next semantic release.

This automatic retention rule does **not** apply merely to ordinary governance decisions, naming rules or documentation-policy choices.

A remediation package should retain, as applicable:

- finding/root-cause identity and lifecycle status;
- affected route/subsystem/files/functions;
- evidence/source hierarchy used;
- rationale for remediation;
- exact intended fix shape/algorithm and observable contract;
- explicit non-goals and preserved behavior;
- ABI/protocol/config/session/error implications;
- expected bounded source delta / patch plan;
- focused regressions and release-class-applicable source/native acceptance (game acceptance required only for FINAL_RELEASE);
- dependencies/blockers;
- rejected/superseded alternatives and rationale;
- provenance, parent hash, manifest, hashes and deterministic verifier(s);
- prepared patch/source delta and validation state if already produced.

`REMEDIATION_READY` packaging does not itself authorize canonical-source mutation.

## 7. CLEAN_MINIMAL_FIX_STANDARD

All fixes/remediations are designed and implemented as the **smallest complete clean change** that closes the validated root cause while preserving the intended contract.

- Do not mix unrelated refactors, naming cleanup, formatting churn, architectural redesign, speculative hardening or opportunistic feature work into a defect fix unless strictly required for correctness/safety.
- Minimize touched files, symbols, control-flow surface, ABI/protocol/config impact and behavioral delta.
- Preserve valid behavior outside the exact finding scope.
- Prefer direct, comprehensible, maintainable fixes over workaround chains, duplicate authorities, compatibility hacks, hidden side effects or temporary shims.
- Minimal does not mean incomplete. Use the smallest self-consistent fix set that closes the whole defect, including transaction boundaries, rollback, QA false-pass causes and required invariants.
- Necessary tests/QA/provenance are part of the minimal fix set; unrelated test churn is excluded.
- Every `REMEDIATION_READY` package and implemented fix/hotfix states its minimal-fix boundary, explicit non-goals and intentionally excluded cleanup/hardening.
- Broader desirable changes become separate findings/remediations/hardening items.

## 8. Exact-parent internal remediation integration

The named Wave model is abolished. Internal remediation is not given W1/W2/W3 release or branch identities.

- Once a `REMEDIATION_READY` item is explicitly authorized for implementation, apply it to the exact current accepted source state for that lineage.
- Each successive accepted internal source state records the exact parent SHA-256, bounded source delta and validation state against that predecessor.
- Later fixes are applied on top of the already accepted prior fixes; there is no deferred final bulk merge.
- Shared-file collisions or regressions are resolved at the point they appear before further remediation is accepted.
- Isolated work is allowed for unresolved research/experimentation, but it remains Experimental/internal and cannot be treated as shipping integration until explicitly dispositioned.
- Internal source states are not releases and are not published as release artifacts. Publication happens only as one of the four official release types in Section 1.

## 9. RELEASE_VALIDATION_TIERING

Validation effort is tiered by whether work is an unpublished internal source iteration or one of the four published release types.

### 9.1 Unpublished internal remediation iterations

Normal internal fix iterations use focused/risk-based validation and at minimum preserve:

- exact parent SHA-256/provenance;
- exact source delta;
- focused regression coverage for changed behavior;
- affected subsystem/static/source gates;
- relevant syntax/build-policy checks;
- impacted ABI/protocol/lifecycle/config/serializer/state invariants;
- explicit validation/carry-forward state.

They do **not** automatically require published-release packaging, authoritative eight-shard release QA, package-integrity replay, clean-extract replay, or release sidecar generation, because they are not release publications.

Escalate immediately when an internal change modifies QA runner/sharding, repository-contract gates, build orchestration, packaging, manifest/hash generation, lifecycle/teardown frameworks or similarly broad/high-blast-radius shared infrastructure. Modified validation infrastructure must be proven trustworthy before later work relies on it.

### 9.2 Published release validation

- Final Release uses the complete final release stack defined in Section 1.1.
- Release Candidate uses release-grade validation sufficient to prove Final readiness; exact byte-identical promotion may reuse still-applicable evidence under Section 1.2.
- Hotfix Release uses authoritative source/package integrity plus all acceptance required by the changed surface; untouched classes may reuse evidence only when unchanged boundaries are proven.
- Experimental Release uses explicit focused validation appropriate to its declared experiment and is never represented as production acceptance.

Historical 4/4 or other prior PASS records remain historical evidence and are not rewritten.

## 10. PROACTIVE_HARDENING_STANDARD

Project review proactively searches sibling systemic hardening classes instead of waiting for a reported bug. `PROACTIVE_TRANSACTIONAL_HARDENING` is a major subdomain of this umbrella standard.

Mandatory audit lenses include:

- transactional/failure atomicity and no partial success;
- staged prepare/validate then publish/commit ordering;
- ownership retention when rollback/cleanup is incomplete;
- retryable/idempotent teardown/shutdown/restore;
- quiescence and in-flight lifetime safety;
- state-vs-delivery coupling;
- snapshot/reload atomicity for config/store/runtime state;
- exact-size/no-partial-output serializer/buffer contracts;
- cross-resource restoration for environment/registry/CWD/page protection/hooks;
- persistent/session state isolation and rollback;
- release/build artifact publication atomicity.

These are audit lenses, not automatic defects. Findings require evidence and a violated invariant/contract. A valid pre-release fix path is retained under the automatic `REMEDIATION_READY` rule.

## 11. Repository Structure Contract

The release repository root is reserved for canonical entry/state files only, such as:

- `README.md`
- `CMakeLists.txt`
- `CHANGELOG.md`
- `SOURCE_REVISION.md`
- `RELEASE_STATE.json`
- `LICENSE`
- other explicitly justified canonical root entries.

Ownership rules:

- generated compiler/CMake build output -> root `build/`; this directory is a generated workspace, not a source-controlled orchestration directory, and its standard technical name remains `build/`;
- project-owned build launchers, PowerShell orchestration, environment preparation, tool binding, configure/build/finalization/acceptance orchestration -> `scripts/build_orchestration/`;
- `scripts/build/` is **SUPERSEDED** prospectively and is forbidden as the active project-owned build-script directory in newly published releases;
- the `scripts/build/` -> `scripts/build_orchestration/` migration must update launchers, QA gates, repository-contract checks, ownership metadata, documentation, CMake/script references and path-sensitive tests consistently, and must fail closed on stale active references;
- this path migration is repository-layout/naming work only and does not authorize unrelated build-system refactors or symbol/type/function renaming;
- QA/static gates -> owned `scripts/` / QA responsibility areas;
- tool/host applications -> `tools/`;
- active metadata -> `metadata/`;
- the release-packaged governance mirror -> **directly under `docs/PROJECT_GOVERNANCE_CONTRACT.md`**; no `docs/governance/` wrapper directory;
- `docs/` contains only current/live documentation needed to understand, build, validate, operate or release the current source state.

Official release source trees must **not** contain archive/research subtrees such as `docs/research/`, `docs/history/` or `docs/historical/`. Research, audit evidence, retired tests/source, historical snapshots and superseded release records belong in their persistent Library ownership areas, not inside the current release source tree.

Do not scatter governance/history/audit/research material into production source or repository root. Release QA must fail closed on forbidden archival/research documentation directories.

Root-level `BUILD_*.cmd` launchers remain explicitly **FORBIDDEN**. Historical W10-era `scripts/build/` placement is also superseded by the current `scripts/build_orchestration/` ownership rule. Future fixes must restore neither root-level launchers nor the superseded `scripts/build/` active directory.

Active scripts require a discoverable owner/purpose. One-off, scratch, duplicate, orphan, stage-named or historical active tooling is rejected. Script count itself is informational; there is no current fixed 42-script fail cap.

Production targets must not gain governance/history/research-only dependencies.

Release QA and repository-contract checks must not require narrative research reports, archived source/tests or historical release snapshots to exist inside the shipping source tree. Any contract required for automated validation must be represented by current code/tests, focused fixtures, or machine-readable `metadata/` contracts. Removing an obsolete archive/research document must not be blocked merely because QA was written to require its presence.

Source-health heuristic: 500+ lines alone is not an error; 500+ lines combined with three or more independently reviewable responsibilities triggers architecture review.


### 11.1 Project licensing contract

- Root `LICENSE` is a required canonical repository/release entry and must be present in every newly published official release.
- Project-owned `preloader_l` source and project-authored accompanying material are licensed under the **GNU General Public License, version 3 only (`GPL-3.0-only`)** unless a later explicit user decision changes the project license.
- Root `LICENSE` must contain the **complete, verbatim GNU General Public License version 3 text** (Version 3, 29 June 2007). A summary, placeholder, historical notice, inferred-license disclaimer, truncated text or rewritten license is not acceptable.
- The project license does not overwrite bundled third-party licenses. Material under `third_party/` or other explicitly vendored ownership boundaries remains governed by its own applicable license/notice, which must be preserved.
- Release/repository QA must fail closed if root `LICENSE` is missing, empty, truncated, replaced by a notice/summary, or no longer identifies the complete GNU GPL v3 text. QA must also ensure third-party license files are not removed or replaced by the project GPL text.
- License changes are release-significant documentation/legal changes. The applicable release notes/CHANGELOG and provenance must identify a project-license change; silently changing the licensing contract is forbidden.

### 11.2 HUMAN_INVOCABLE_SCRIPT_LAUNCHER_CONTRACT

Human-facing `.cmd` launchers are an explicit operator interface, not a blanket companion file for every script. Classification is **positive/default-deny**.

#### 11.2.1 Explicit human-entrypoint classification

- A script/tool is human-invocable only when it is explicitly registered in a machine-readable human-entrypoint registry/allowlist, or when it is the direct maintained successor of an already-established canonical operator entrypoint.
- Default classification is **NON_ENTRYPOINT**. The mere existence of a `.ps1`, `.py`, executable or other runnable file does not authorize or require a `.cmd`.
- Internal helpers, subordinate orchestration steps, library/module scripts, generated tooling, test implementation helpers, dependency bootstrap helpers and scripts invoked only by another script are not human entrypoints unless explicitly designated.
- Vendored/third-party ownership boundaries, including `third_party/`, are **launcher-forbidden by default**. A project-authored `.cmd` may not be injected into MinHook, TinyXML2 or another vendored dependency merely because an upstream/helper script is runnable. Any exception requires explicit user designation of that exact operation as a project operator entrypoint.

#### 11.2.2 Required launcher shape

- Every authorized human entrypoint has a **same-directory, same-semantic-stem `.cmd` launcher**. `.cmd` is the sole standard Windows human-launcher extension; parallel `.bat` variants are forbidden.
- Generic dispatcher layers such as `launcher.cmd`, `run.cmd`, `start.cmd`, `execute.cmd` or equivalent launcher-of-launcher abstractions are forbidden. The `.cmd` invokes its own authoritative same-stem target directly.
- The launcher is a thin invocation layer only. Authoritative operational/build/business/acceptance logic remains in the underlying `.ps1`, `.py`, executable or equivalent target; duplicated logic or divergent behavior paths are forbidden.
- The launcher resolves its target relative to its own location, works from arbitrary caller working directories, quotes paths safely, forwards all operator arguments unchanged, and preserves the **exact child exit status** without collapsing it to a generic code.
- The operator must not need shell-specific syntax, PowerShell execution-policy syntax, Python invocation syntax or a preparatory `cd`.

#### 11.2.3 Operator launcher UI compatibility

Existing operator-facing launcher presentation is a protected compatibility surface. Functional maintenance, security correction, path migration, naming cleanup or launcher-contract adoption does not authorize an unsolicited redesign.

- The established **3.2.3 MELOTRIX Production / Native Acceptance / CRT Acceptance / Debug launcher presentation family** is the compatibility baseline for those operator surfaces until the user explicitly authorizes a new presentation design.
- Byte identity is not required, but console title/banner structure, separators/layout, spacing, operation-description style, PASS/FAIL presentation, exit-code/result presentation and operator interaction flow are preserved except where a semantically necessary change requires a bounded edit.
- Security hardening, path migration, launcher fixes, repository cleanup, formatting cleanup, naming normalization or automation support alone are not UI-redesign authorization.

#### 11.2.4 Operator completion retention and automation separation

- Authorized human `.cmd` launchers are interactive operator interfaces and **must remain visibly open after the underlying operation completes** until the operator explicitly dismisses/continues the window. Automatic closure after build, acceptance, verification, evidence generation, diagnostics or equivalent operator work is forbidden.
- Before the hold/dismissal point, the launcher displays final PASS/FAIL, the exact child exit status, and the relevant report/log/evidence/output location when applicable.
- The launcher captures the child exit code before the operator-visible hold and preserves that original exact code when returning. Result visibility must not corrupt exit semantics.
- CI/non-interactive automation does not use the human `.cmd`; it invokes the authoritative underlying `.ps1`, `.py`, executable or equivalent implementation directly. Underlying implementations remain automation-safe and contain no unconditional blocking `pause`.

#### 11.2.5 Authorized launcher inventory and fail-closed QA

- The human-entrypoint registry/allowlist is the authoritative launcher inventory. Moving, renaming or removing an entrypoint updates the launcher, registry, ownership metadata, path-sensitive QA and documentation in the same change.
- QA fails closed both on **missing required launchers** and on **unauthorized extra launchers**. Unauthorized cases include helper/vendor launchers, generic dispatcher launchers, orphan/stale launchers, launcher-of-launcher chains, misnamed/path-broken launchers, broken argument forwarding, exit-code collapse, auto-closing human launchers, and unapproved UI redesign.
- The launcher and target represent one operation, not independently governed implementations.

### 11.3 SENSITIVE_MATERIAL_AND_SECRET_EXCLUSION_CONTRACT

Release, evidence, diagnostic, log, support-package and research artifacts must not disclose credentials, authentication tokens, private keys, signing secrets, environment secrets, user-specific absolute paths/profile names, or other unintentionally captured sensitive material.

- Packaging/publication workflows must perform a fail-closed sensitive-material scan before an artifact is accepted or published.
- At minimum, scanning covers project-owned source, metadata, logs, diagnostics, evidence bundles, support packages, generated reports and machine-readable manifests.
- Secret-like values must never be normalized into permanent test fixtures merely to make a scan pass. Required test credentials use clearly synthetic, non-functional fixtures.
- User-specific absolute paths, usernames/profile directories and local machine identifiers are removed, redacted or represented using documented neutral placeholders unless their presence is technically required evidence.
- Private signing material, credentials and tokens are never embedded in release/evidence archives or committed to source. Public certificates, public keys or non-secret identifiers may be retained only when technically required and explicitly classified as non-secret.
- A detected secret/sensitive-material hit blocks publication until reviewed and either removed/redacted or explicitly proven to be a safe false positive.
- Historical artifacts are immutable; newly discovered sensitive-material exposure is handled through access/remediation/provenance procedures rather than silently rewriting historical evidence.

## 12. Active filename/naming contract

- Production source/header pairs representing the same component use the same semantic stem.
- Single route handler: `<concept>_route.cpp`.
- Route group: `<concept>_routes.cpp`.
- Use a semantic discriminator when singular/plural commands would otherwise collide.
- Unit tests: `test_<concept>.cpp` or, only when required, `test_<concept>_<behavior>.cpp`.
- Static/source QA: `check_<concept>.py`.
- Executable/regression QA: `test_<concept>.py`.
- Build action: `build_<target>.ps1`.
- Active filenames do not contain transient HF/F/W/version/campaign/RVA bookkeeping; lineage belongs in provenance/evidence/history.
- Do not blindly replace `windows` with generic `platform`; remove irrelevant qualifiers or use the concrete responsibility.
- Prefer two or three semantic words; longer basenames require a real discriminator.
- Filename cleanup does not automatically authorize symbol/type/function/API renames.

### 12.1 ACTIVE_TREE_RELEASE_IDENTITY_HYGIENE_CONTRACT

Historical version/codename text is not automatically an error, but historical identity may not silently act as current authority.

- Historical identities are allowed in contextually valid surfaces such as `CHANGELOG.md`, approved naming registries, immutable provenance, migration ledgers and explicitly classified historical comparison/evidence.
- Current dependency documentation, project-authored current source comments, active QA contracts and active metadata must not present an old semantic version/codename as the current owner/identity unless the reference is explicitly required and classified.
- Active-tree lineage sweeps classify each historical version/codename hit as `CURRENT_REQUIRED`, `CROSS_RELEASE_REQUIRED`, `HISTORICAL_ALLOWED` or `STALE_REMOVE_OR_NORMALIZE`.
- Blind global replacement is forbidden. Stale project-authored identity is normalized to release-neutral/current terminology or moved to immutable Library provenance while preserving historical meaning.

### 12.2 VENDORED_DEPENDENCY_CURRENCY_AND_PINNING_CONTRACT

Every vendored/third-party dependency used by the project has a current machine-readable dependency record containing at least: upstream project/repository, exact pinned release/version/commit and source hash, project usage classification (`SHIPPING_LINKED`, `BUILD_ONLY`, `TEST_ONLY`, `REFERENCE_ONLY`), local modifications/backports, currently known upstream stable version, relevant known security/correctness changes, rationale for retaining the current pin, last review state and current disposition.

- Allowed dispositions include `KEEP_PINNED_COMPATIBILITY_PROVEN`, `UPDATE_READY`, `UPDATE_BLOCKED_COMPATIBILITY`, `UPDATE_REQUIRES_RESEARCH` and `REMOVE_UNUSED`.
- Neither automatic latest-version adoption nor indefinite stale pinning is allowed. Retaining an older dependency requires current source-level justification such as API/ABI/object-layout compatibility, required local patching, consumer compatibility, shipping-graph non-participation or a bounded backport strategy, with appropriate regression evidence.
- Dependency disposition is reviewed during each new semantic-release preparation and whenever materially relevant upstream security/correctness information appears.
- TinyXML2, MinHook and future registered vendored dependencies are governed by this contract; a historical release-era decision alone is not sufficient to retain a pin indefinitely.

### 12.3 THIRD_PARTY_UPDATE_NOTIFICATION_AND_APPROVAL_CONTRACT

Registered third-party dependencies neither auto-update nor silently age.

- When a newer upstream **stable release** than the currently pinned/reviewed version is verified, the user is notified and an update proposal/review is presented.
- The notice records dependency name, current pin, new upstream version/date, known security/correctness relevance, usage classification, local patches/backports, expected compatibility surface and a proposed disposition.
- Proposed dispositions include `RECOMMEND_UPDATE`, `RECOMMEND_RESEARCH_BEFORE_UPDATE`, `RECOMMEND_KEEP_PINNED`, `SECURITY_REVIEW_REQUIRED` and `NOT_APPLICABLE_TO_CURRENT_USAGE`.
- A newer release existing is not by itself authorization to update; a working current pin is not justification to ignore a newer release. Source-level comparison/research determines the recommendation.
- Any third-party version/commit/source-byte change requires **explicit user approval**. Notification, research PASS, recommendation or security relevance does not itself authorize dependency mutation.
- Security/memory-safety/parser/overflow/supply-chain fixes are surfaced with higher priority, but affectedness and update/backport/keep-pin options are still evaluated before user-approved source mutation.
- Every reviewed upstream release records a decision state such as `UPDATE_APPROVED`, `UPDATE_DECLINED`, `KEEP_PINNED_APPROVED`, `RESEARCH_PENDING`, `UPDATE_BLOCKED_COMPATIBILITY` or `NOT_APPLICABLE`. The same upstream release is not repeatedly re-presented unless the disposition changes or materially new evidence appears.

### 12.4 VENDORED_DEPENDENCY_IDENTITY_NEUTRALITY_CONTRACT

Current project-authored documentation/provenance inside vendored dependency areas must be release-neutral unless a release-specific reference is explicitly required.

- Historical origin may be retained as a provenance field (for example `origin_release: 3.2.3 MELOTRIX`) but must not make that historical release the current dependency owner/identity.
- Release-specific project-authored filenames/comments such as old-codename provenance documents are normalized to semantic dependency/provenance names when they remain current authority.
- Upstream-owned source/version/license/provenance text is not rewritten merely to remove project release terminology; project-authored and upstream-owned material must remain distinguishable.

### 12.5 ACTIVE_METADATA_LIVENESS_AND_OWNERSHIP_CONTRACT

`metadata/` is a current operational/contract surface, not a historical dumping ground. Every metadata file has explicit machine-readable ownership and liveness classification.

Required classifications include, as applicable: `ACTIVE_CURRENT`, `ACTIVE_CROSS_RELEASE_CONTRACT`, `BUILD_DERIVED`, `MIGRATION_BASELINE`, `HISTORICAL_PROVENANCE`. Each metadata item records its owner, consumers, purpose, authority state, retention reason and current-release relevance.

- `ACTIVE_CURRENT` metadata may not retain an old release identity as current owner.
- A contract that remains active across releases is classified `ACTIVE_CROSS_RELEASE_CONTRACT` and uses release-neutral current identity; its historical origin may remain provenance only.
- Migration/baseline metadata remains in the current tree only when it has independently proven current build/release/validation value. Purely historical baseline material belongs in immutable Library provenance.
- Orphaned, unconsumed, self-referential or unowned active metadata fails closed pending retain/normalize/rehome/remove disposition.

### 12.6 NO_SELF_JUSTIFYING_LEGACY_LAYER_CONTRACT

A legacy layer cannot justify its own continued existence.

- `legacy file exists -> QA checks legacy file -> therefore legacy file must remain` is prohibited circular ownership.
- Retained legacy source/metadata/evidence must have an independently demonstrable current function in product correctness, build correctness, release validation, protocol/ABI contract or active migration safety.
- If a legacy metadata file is consumed only by a legacy checker, the checker itself must first be justified by a current requirement. If no independent requirement exists, checker and metadata are retired/rehome-reviewed together.
- QA must not manufacture current authority for dead historical layers merely by depending on them.

## 13. Terminology migration safety

Project-owned vocabulary migrations use positive allowlists / approved rename manifests, preferably token/AST/symbol-aware. Blind raw substring replacement is prohibited.

Protect technical/external vocabulary including PowerShell variables, Win32 APIs, CMake keywords, CRT/STL, ABI/ISA names, environment variables, external protocol identifiers, XML tags/attributes, registry names, DLL/export/import names, filesystem/wire literals and configuration keys unless separately authorized.

Research terms such as donor/consumer/authority/parity may be used when technically meaningful. Legitimate technical terms are not banned merely because they resemble project vocabulary.

## 14. Documentation Preservation and Release-History Contract

Documentation preservation is fail-closed, but preservation does **not** mean embedding an archive inside every release. Governing principle: **one current documentation surface, one complete semantic CHANGELOG, immutable historical evidence in Library**.

### 14.1 Current `docs/` surface

- `docs/` is current/live release documentation, not an archive.
- The authoritative Library governance master is mirrored directly as `docs/PROJECT_GOVERNANCE_CONTRACT.md`; do not create `docs/governance/`.
- `docs/research/`, `docs/history/` and `docs/historical/` are forbidden in newly published official releases.
- Retired source files/tests, research reports, audit packages and historical release snapshots are preserved in Library under their owned areas rather than copied into each release source tree.
- Current release documents may include current build instructions, release notes, acceptance, provenance, architecture and Golden-protocol documentation when they are relevant to the current release.

### 14.2 `CHANGELOG.md` is the canonical in-source semantic history

- `CHANGELOG.md` is the canonical human-readable history of **material changes by semantic release**. It must not delegate essential release history to `docs/history/`, a prior package, or an external archived CHANGELOG.
- Each semantic-release section must preserve all material features, fixes, remediations, architecture changes, build/QA/governance changes and meaningful acceptance-status changes that define that release.
- Internal checkpoint/wave/hotfix-development labels may be normalized into clear semantic categories, but the underlying material technical changes must not disappear. Consolidation may remove duplication; it must not make a substantial release appear nearly empty.
- Exact log blobs, repetitive SHA inventories and low-level provenance may remain in release metadata or immutable Library artifacts; the CHANGELOG does not need to duplicate every evidence byte. However, the associated material change and its disposition must remain visible in the semantic release section.
- Historical source-package snapshots are not a substitute for CHANGELOG completeness.

### 14.3 Lossless history migration gate

`docs/history/` and equivalent archive trees are **migration sources first and deletion targets only after migration is proven complete**. Their removal is never authorized merely because the current documentation architecture forbids archival subtrees in newly published releases.

- Before deleting or omitting any in-source `docs/history/`, `docs/historical/` or equivalent archive tree from the next release, inventory **every human-readable file, heading and substantive release-history statement** it contains.
- Each inventoried history item must be accounted for against the owning semantic section in `CHANGELOG.md`. The preferred disposition is `MIGRATED_TO_CHANGELOG`; exact duplicates may be marked `DUPLICATE_ALREADY_PRESENT`; purely evidentiary/hash/log material may be marked `PROVENANCE_ONLY` only when the immutable Library artifact retaining the exact bytes is identified. No item may be silently dropped.
- A machine-readable migration ledger in `metadata/` or a Library evidence artifact must map each source history record to its CHANGELOG destination/disposition. Coverage must be **100% accounted-for** before archive removal is permitted.
- CHANGELOG migration does not require verbatim duplication of repetitive logs or hashes, but it **must preserve the complete substantive meaning** of every historical change record: feature/fix intent, architecture/lifecycle behavior, remediation/closure, build/QA/governance change, acceptance-state change, and any limitation/non-goal that materially defined the release.
- If any history text has no justified destination/disposition, if a substantial release becomes materially less descriptive after consolidation, or if the migration ledger is incomplete, documentation cleanup **fails closed** and the archive remains until the gap is resolved.
- The currently identified 3.2.3 MELOTRIX section is a mandatory backfill case. Its preserved records must be reconciled exhaustively into the MELOTRIX CHANGELOG section before `docs/history/3.2.3_MELOTRIX/` disappears from the next release. This includes, at minimum, MinHook-only 9/9 hook architecture, SAFE-FREEZE/NtGetNextThread behavior, loader-lock separation, rollback/trampoline ownership, PTD/shutdown lifecycle semantics, accumulated remediation/closure work, QA architecture changes, and Native/CRT/game acceptance-status history.
- Historical artifacts already published remain immutable in Library and serve as the exact-byte provenance source after migration; the next release need not carry duplicate archive trees once the 100% migration gate has passed.

### 14.4 Documentation consistency and QA

- `BUILD_ID`, `SOURCE_REVISION`, `BUILD_ME_FIRST`, `README`, `RELEASE_STATE`, `CHANGELOG`, release notes and acceptance docs must be mutually consistent for the current release.
- Repository/documentation QA must verify: no forbidden archive/research directories, governance mirror at the direct `docs/` path, CHANGELOG semantic-section presence and protected non-loss content, and no checker dependency on removed narrative archives.
- Documentation/history migration uses explicit comparison/ledger evidence where necessary, but the ledger itself belongs in current `metadata/` or Library evidence rather than a duplicated docs archive.

## 15. Production / Golden pipeline

Exact production pipeline and order:

**MSVC Build -> URSYNEX Golden Finalizer -> ARYRA Golden Layout -> MUSTAVEX Golden Metadata -> CORVEXON Golden Trail -> FALCION Golden Cleaning**

- MSVC Build includes link-policy audit; no separate Link-Policy Audit stage.
- URSYNEX is the bounded binary-mutating Golden stage.
- ARYRA is read-only structural PE/layout validation; no donor-exact layout requirement.
- MUSTAVEX is read-only metadata contract/allowlist enforcement.
- CORVEXON is read-only final-binary acceptance including exports/imports/entrypoint/ABI/security/invariants/hash trail.
- FALCION is final repository/release/package hygiene.
- No separate Canonical DLL Publication stage after FALCION.

### 15.1 TINYXML2_SINGLE_ENGINE_SEMANTIC_XML_CONTRACT — SQUALERIS HOTFIX11 and descendants

**Normative / fail-closed; user-approved on 2026-10-12.** Applies to `3.2.5 SQUALERIS HOTFIX 11` production implementation and all subsequent builds inheriting this architecture. It is a release-target architecture rule, **not** an assertion that HOTFIX11 is already implemented, tested or published. No earlier published artifact is retroactively changed. The currently published HOTFIX10 source and identity remain immutable until a separately verified HOTFIX11 publication transaction.

- **Exactly one production XML parser:** the pinned, licensed TinyXML2 source is the sole LSX request and XML Store input parser. Production code must link and execute TinyXML2; declaring it `test_reference_only` or linking it merely as a pre-validator while running an old scanner is forbidden. Build graph, runtime use and QA ownership must corroborate this. XML Store transactional/materialization semantics remain independently enforced; VDF/JSON parsing is out of scope.
- **No parallel legacy XML engine:** remove active custom XML tag scanners, child/attribute scanners, duplicated XML grammar/element traversal, and temporary compatibility/masking/normalization bridges, including processing-instruction masking fallback. A production code path must never validate XML with TinyXML2 then reinterpret or reparse it using another custom XML parser. No dormant production dependencies or test-only assertions that force obsolete parser retention. Historical artifacts are immutable provenance, not active build dependencies.
- **Parse once and traverse one owned DOM:** each complete logical XML document is parsed/de-entity-decoded exactly once and child/attribute/text access traverses that same `tinyxml2::XMLDocument`/DOM; do not reparse individual XML fragments or each child. Preserve existing LSX frame boundary/transport/session rules; TinyXML2 document parsing is not a substitute for wire framing.
- **Semantic-first value contract:** decoded TinyXML2 text/attributes go directly to handlers through explicit decoded/semantic value APIs (e.g. `assign_decoded`), without re-escaping to synthetic source XML and decoding again. Distinguish `RawXmlSource` from `DecodedSemanticValue` at type/API-contract level, including data lengths. A legacy `assign_source`/entity decoder may only accept genuine raw XML source, not values already decoded by TinyXML2. Double-decode and encode-then-decode compatibility round trips are forbidden; e.g. input `&amp;lt;` must yield the semantic string `&lt;`, never `<`.
- **Lifetime/ownership:** document/tree and derived strings/spans must remain valid through handler use; owned immutable decoded values or document-scoped leases are permitted. Reject dangling borrowed `TextSpan`/DOM pointers, stale references, hidden reparse caches and ownership-changing shortcuts. Preserve atomic output publication, failure handling, thread/transaction guarantees and resource release paths.
- **Serialization boundary:** preserve the necessary protocol-specific XML response construction/wire serialization, and XML-escape semantic values exactly once on output. Do not conflate safe output escaping with forbidden input re-encoding. Existing route behavior, request/response envelopes, public lifecycle ABI, LSX framing, encryption/session semantics, output size/error contracts, Store transactions and donor/Consumer authority remain in force.
- **Evidence-governed edge cases:** account explicitly for CDATA, text-first/first-text semantics, attributes/entities/numeric references, malformed XML, processing instructions, whitespace/UTF-8, duplicate attributes, nesting, empty inputs, bounds and failure atomicity. Resolve acceptance/rejection differences against the current Consumer + Donor1 hierarchy; where evidence is insufficient, record the boundary and fail closed rather than concealing a difference with a compatibility bridge. TinyXML2's mere presence does not establish complete Anadius parity. Preserve legitimate resource/depth/capacity safety limits.
- **Fail-closed tests/QA:** update all test and CMake/source-graph assertions from test-reference-only to verifiable production TinyXML2 ownership. Require positive and adversarial semantic-once/DOM-lifetime tests (including `&amp;lt;`), exact prior donor-derived XML regression fixtures, XML Store atomicity, no legacy scanner/bridge in compiled production source, one-pass instrumentation or equivalent proof, and full existing eight-shard/fresh-extract source QA. A temporary green focused GCC/Clang run, source ZIP integrity or user-reported game smoke test is not an official complete acceptance result.
- **Historical HOTFIX11 publication note (superseded for game gates):** this original checkpoint required complete TinyXML2 production integration and appropriate source/QA validation. Its former requirement for fresh CollegeFB27/Madden27 game acceptance before a Hotfix publication was **explicitly superseded by §0.0.2** and is NOT a current/future Hotfix release gate. Preserve the immutable historical checkpoint and actual evidence status; do not retroactively assert that a game test ran or passed.

### 15.2 MINHOOK_SINGLE_ENGINE_PRODUCTION_ARCHITECTURE_CONTRACT — current 9/9 hook topology and descendants

**Normative / fail-closed; user-approved on 2026-10-12.** Governs current 3.2.5 SQUALERIS MinHook production hooks and future builds inheriting this architecture. This Library master addition does **not** rewrite a previously published release, modify any shipping source, or itself claim renewed native/game acceptance. Its normative content must be mirrored with master revision/hash binding before the next official release publication.

- **Single hook engine:** the approved, hash-pinned MinHook implementation is the only production engine for inline patching, instruction relocation, trampoline creation and patch-time instruction-pointer relocation. Independently implemented custom inline patchers, trampoline/relocation engines, IAT/VEH/page-trap replacement backends and legacy MASM hook-tail wrappers must not re-enter the active production hook architecture or serve as fallbacks. Project-owned ticket/call-gate/atomic lifecycle components are allowed solely to manage ownership, original forwarding, installation transactions and safe quiescence; they must not implement a second patch engine.
- **Current exact nine-hook membership:** five registry detours (`RegOpenKeyExA`, `RegOpenKeyExW`, `RegQueryValueExA`, `RegQueryValueExW`, `RegCloseKey`), two Surge detours (`Process32NextW`, `_Mtx_trylock`), one `CryptQueryObject` detour and one destination-aware `ws2_32!connect` detour. This topology reflects the currently approved HF10 source; count/membership/target/owner changes other than the absolute prohibition below require explicit project authorization, documented justification and renewed relevant acceptance. Do not silently infer different membership from a historical hook diagram or compiler naming alias.
- **Absolute `htons` prohibition:** reintroducing the superseded process-wide `ws2_32!htons` detour is **completely and unconditionally forbidden**. No shipping, production, experimental, compatibility, recovery, fallback, diagnostic-live or alternate-hook-engine path may install, restore, replace or indirectly recreate a process-wide `htons` detour. There is **no exception or approval clause** in this contract. `ws2_32!connect` is the sole accepted destination-aware redirection hook, subject to the remaining architecture and acceptance requirements. Any `htons` detour reintroduction is an immediate fail-closed architecture/release blocker. Harmless references to the standard `htons` API or non-hook test data are not themselves forbidden; the banned behavior is the process-wide detour.
- **Loader-lock separation and startup transaction:** under `DllMain`, only loader-lock-safe scheduling/bookkeeping may occur; no hook patching, MinHook create/enable/disable, thread freeze or blocking wait. The runtime sequence is `DLL_PROCESS_ATTACH → schedule worker → return from DllMain → MinHook initialization → create/queue all nine hooks → one NtGetNextThread SAFE-FREEZE MH_ApplyQueued → transaction commit → PTD/config/service/network`. Incomplete/failed patching must not be published as initialized; retain every created ticket through commit or full rollback.
- **SAFE-FREEZE authority:** preserve the vetted `NtGetNextThread` SAFE-FREEZE integration used by MinHook's queue/apply operation. Do not silently replace it with an independent thread freezer, unsafe patch-without-freeze shortcut or custom patch backend. Any upstream/patched freeze implementation change needs source-bound instruction-pointer and thread-enumeration safety evidence and appropriate Windows/MSVC regression; game acceptance remains mandatory only for FINAL_RELEASE under §0.0.2.
- **Trampoline ownership and in-flight lifetime:** every detour keeps explicit MinHook ticket/original forwarding authority plus a call gate covering the original/trampoline call duration. Installation and draining states must avoid publishing unowned or freed trampoline pointers. `MH_RemoveHook` and trampoline memory release may occur only when appropriate disable/restore and active-call quiescence obligations are proven; late/in-flight wrappers must remain safely forwarded. On failed remove/restore, preserve live ownership and a retryable state rather than asserting clean teardown.
- **Failure-atomic install, rollback and teardown:** the lifecycle explicitly distinguishes `COMMITTED`, `ROLLED_BACK` and `ROLLBACK_INCOMPLETE`; the last is never success. Queued enablement, publication, rollback and partial cleanup may not leave unknown ownership silently discarded. Global disable must precede normal teardown; protected in-flight calls are drained before owned hooks are removed and MinHook uninitialized. Repeated startup/shutdown/reload must not leak patches, gates, tickets, forwarding targets or unsupported resource lifetime. Make incomplete restoration observable, block unsafe reinstallation and retain sufficient state for bounded retry.
- **Vendor/build pinning:** preserve the approved MinHook source pin, licensing and vendored ownership; source/binary identity and the project SAFE-FREEZE patch provenance must be verified in CMake/source graph and relevant native link policy. No silent upstream update, remote dependency substitution, unreviewed local patch or fallback engine. Any third-party version/commit/source change remains under the pre-existing explicit user approval and dependency-currentness rules.
- **Fail-closed validation/publication:** source and QA must check exact nine-hook inventory; `htons` detour absolute absence (including experimental and fallback paths); sole MinHook ownership; SAFE-FREEZE and DllMain separation; transaction commit/rollback-incomplete; call-gate/trampoline lifetime; partial remove retry; repeated shutdown; source/build pin and anti-legacy backend checks. Execute release-class-applicable deterministic eight-shard source QA, exact-package integrity and clean-extract replay, and independently required focused native/CRT safety checks where applicable. **CollegeFB27/Madden27 game acceptance is mandatory only for FINAL_RELEASE, regardless of any changed hook/transport surface; it must never block Hotfix/RC/Experimental publication.** Record actually executed evidence classes and exact candidate hashes truthfully. Missing required source/security evidence or a prohibited hook still blocks official publication. Historical HF10 status is preserved as a record, not a current pointer statement.

## 16. MUSTAVEX metadata recovery boundary

Metadata recovery/adoption is strictly allowlisted. Only numbered metadata table items **#1, #2, #3, #5, #6, #7, #10, #11, #13, #14, #18 and #36** may be recovered/adopted under the existing project contract unless explicitly changed later.

Authenticode/certificate material and exact PE addresses/sizes/layout values remain excluded from metadata recovery targets.

## 17. Authoritative release QA and evidence freshness

The release-validation contract separates **QA workload partitioning**, **package integrity**, and **clean-extract replay**. These are different operations and must not be conflated.

### 17.1 Source QA partitioning and schedule (see §0.1.1)

- Authoritative release/source QA is one logical QA universe partitioned across exactly **8 deterministic shards**; **when scheduled by §0.1.1** PASS requires all 8. Final schedules it at G0–G4; Hotfix/RC/Experimental schedule it **only at G4**; IDS/source-state **never** schedules it.
- The 8 shards exist to divide tests/gates/behavioral cases into deterministic subsets. They do **not** mean that unrelated package-level operations are repeated eight times.
- Behavioral regressions under release QA use the same deterministic partitioning principle for expensive cases. Cheap shared preflight/static assertions may run once outside the shard fan-out when this preserves equivalent coverage and fail-closed behavior.
- No monolithic/no-shard invocation is an authoritative substitute for the partitioned QA universe.
- Timeout/interruption is neither PASS nor FAIL; report and rerun the exact unresolved shard/gate.

### 17.2 Package integrity — once per exact package

- ZIP/package structure, member set, path safety, embedded manifest/hash consistency, package integrity and final package SHA-256 are validated **once per exact package artifact** unless a specific failure investigation requires a rerun.
- The exact validated package identity/hash is then reused by the clean-extract replay. Repeating the same package-integrity operation once per QA shard is unnecessary and must not be required by governance.

### 17.3 Scheduled clean-extract replay — one extraction, eight QA shards

- The exact release package is extracted **once** into a new empty temporary directory.
- When class/stage scheduling requires a full clean-extract replay (Final G2 and G4; RC/Hotfix/Experimental G4 only), that single extracted tree is the source root for shards 0..7. IDS/source-state has no full-shard clean-extract run; its bounded clean-extract integrity/QA remains required.
- Therefore `fresh-extract 8/8` means **one clean extraction + eight distinct QA partitions against that one extracted tree**. It never means eight independent extractions.
- A new extraction is required only when the exact package bytes change or when the previous extraction cannot be trusted as clean/complete.

### 17.4 Validation-class separation

- Source/static QA, package integrity, clean-extract replay, compile/link, final-binary/Golden, native and game/network acceptance are separate validation classes. Passing one class does not silently substitute for another.
- Shipping-impacting changes invalidate inherited acceptance only for the relevant changed surfaces; required freshness must be re-established before release claims.
- Behavior-preserving refactors require observable semantic regression coverage; compile/syntax alone is insufficient.

### 17.5 ACCEPTANCE_FRESHNESS_AND_OWNERSHIP_LEDGER

Every acceptance class used to support a release/readiness claim must be represented in a machine-readable acceptance ledger.

Each ledger entry must record at least:

- acceptance class and status;
- exact source and/or artifact SHA-256 to which the evidence applies;
- evidence artifact/log/report that proves the status;
- validation owner/responsible workflow;
- freshness state;
- explicit invalidation triggers for the class;
- carry-forward source, unchanged-boundary proof and rationale when evidence is inherited rather than rerun.

Allowed status vocabulary must distinguish at minimum `PENDING`, `PASS_FRESH`, `PASS_CARRIED_FORWARD`, `FAIL`, `NOT_APPLICABLE` and `BLOCKED` or equivalent machine-readable states with the same semantics. A carried-forward PASS must never be represented as a fresh exact-build/native/game PASS.

Changes to source, packaging, build inputs, QA/acceptance infrastructure, runtime-relevant configuration, ABI/protocol/lifecycle behavior or other declared invalidation surfaces must invalidate only the affected acceptance classes, but invalidation must be explicit in the ledger before release claims are made. Missing, stale, ambiguous or unbound acceptance evidence blocks any claim that depends on it.

### 17.6 CANONICAL_ARTIFACT_PUBLICATION_TRANSACTION_CONTRACT

Canonical/current artifact publication is a transactional operation. Authorization under Sections 1.7-1.8 controls **whether** publication may occur; this contract controls **how** it must occur safely.

- Prepare and upload the complete candidate publication set first: artifact, sidecar/hash record, required evidence/provenance and supporting metadata.
- Re-read or otherwise independently verify the uploaded bytes and bind the exact artifact/evidence hashes before changing any canonical/current pointer.
- Only after the complete candidate set verifies successfully may `CURRENT_ARTIFACT` or equivalent canonical/current pointer be advanced.
- The current-pointer update is the commit point. Prior to that commit, the previous canonical artifact remains authoritative.
- A failed, interrupted or partially uploaded publication must not leave the new artifact marked current/canonical and must not mark the previous artifact superseded.
- Supersession metadata for the previous artifact is written only after the new exact artifact is fully verified and the canonical pointer commit succeeds.
- Publication retry must be idempotent with respect to canonical identity: rerunning after a partial failure must not create duplicate release identities or contradictory current pointers.
- Post-commit verification must confirm that the canonical pointer resolves to the intended exact artifact SHA-256 and that its provenance/sidecar/evidence set is internally consistent. Failure after commit is a publication-integrity incident and must be reconciled explicitly; false success is forbidden.

## 18. Research/evidence packaging

- Research/audit evidence remains separate from current release source artifacts.
- Research and audit delivery parts target **200,000,000–300,000,000 bytes**, with a hard **300,000,000-byte maximum per independent standard ZIP**. The older 150 MB Consumer Research cap is superseded by the explicit user decision of 2026-10-16. A genuine smaller final tail or a whole small workstream/campaign is allowed; never pad or duplicate data to reach 200 MB. Section 18.2 governs consolidation and cleanup.
- Preserve complete relevant function bodies/caller-callee neighborhoods/xrefs/strings/tables/RTTI/raw bytes with PE/RVA/file-offset mapping/parser/serializer/helper/CFG context/comparisons/verifier/manifest/report where applicable.
- Do not promote research evidence into shipping behavior implicitly.

### 18.1 EVIDENCE_DERIVATION_AND_PROVENANCE_CONTRACT

Every research/audit/evidence package must preserve a machine-readable derivation chain sufficient to re-establish where a finding came from:

`source/input -> extraction/acquisition -> transformation/analysis -> retained evidence -> report/finding`.

The derivation record must identify, where applicable:

- original source/input identity and exact hash;
- source role/authority classification;
- acquisition/extraction method;
- tool names and versions materially affecting the derivation;
- filters, transformations, normalization or bounded-selection rules applied;
- resulting retained evidence files and hashes;
- report/finding IDs that consume the evidence;
- explicit exclusions, limitations and non-retained material.

If raw proprietary or otherwise non-redistributable material is intentionally not retained, the package must say so and preserve the strongest lawful bounded evidence needed to reconnect the retained analysis to the original exact source without pretending the omitted raw material is embedded. Derived screenshots, excerpts, hashes, offsets/mappings, structured observations or verifier anchors must remain within applicable legal/project boundaries.

A finding whose derivation cannot be rebound to its declared source/input and retained evidence is provenance-incomplete and may not be promoted as stronger evidence than the remaining verifiable chain supports.

### 18.2 RESEARCH_AUDIT_CONTENT_DEDUP_AND_CAMPAIGN_COMPACTION_CONTRACT

**Normative / fail-closed / effective immediately. Explicit user decision: 2026-10-16.** Applies to current and future research, audits, stage/campaign evidence, checkpoints, handoffs and their retained supporting outputs. The assistant owns final packaging, consolidation and verified cleanup; this is a standing workflow requirement, not an optional later housekeeping task.

**No recursive history or repeated input payloads.** A new package must not embed its predecessor research/audit ZIP, a growing master/history ZIP, previous campaign ZIPs, or another cumulative chain. Store each needed baseline, donor, binary or common evidence object once at its canonical content-bound location; later slices reference its identity, SHA-256 and retained location. A genuinely needed archival input/test fixture is retained once as an input rather than multiplied inside successive evidence bundles. Resolve referenced dependencies before claiming completeness or deleting a source.

**Content-level uniqueness.** Open nested source archives as needed and compare actual file bytes using SHA-256. Container hashes alone do not establish content uniqueness. Preserve every distinct file, including different bytes under the same filename; retain original paths, aliases, source archive identity/hash and source-to-evidence-to-finding derivation in a machine-readable index. Within a canonical consolidated collection, identical payload hashes occur once. A retired original wrapper's archive hash remains provenance; do not claim that the old wrapper bytes are still retained or exactly reproducible when only its unique file contents were saved.

**Independent delivery parts.** Small evidence/delta outputs may be produced while work progresses. They must later be consolidated into clean standard ZIP parts targeting 200–300 MB (decimal bytes), each no larger than 300,000,000 bytes. A smaller final part or a whole campaign/workstream with less data is valid; do not wait forever, add filler, or duplicate files to reach a minimum. Use project/workstream or campaign names with sequential part numbers. Each ZIP opens independently with a normal ZIP reader; no split files, joined volumes, custom merge tool or required reconstruction program. Keep one clear authoritative index identifying every part and content location.

**Research completion.** Accumulate bounded new evidence without carrying forward old ZIP payloads. At a useful consolidation boundary and at workstream closure, gather all unique evidence, reports, source-linked analyses, proposed fixes, acceptance records, limitations and DEBT; issue the verified canonical collection and retire the superseded small containers. Immediate REMEDIATION_READY retention under Section 6 remains required and follows this same no-nesting and later-compaction rule.

**Audit campaign completion.** Stage evidence may be stored temporarily as small indexed deltas in the audit's single owned root. When a campaign's stages finish, its consolidated package set must retain **all unique stage files**, reports, findings/dispositions, exact-source/toolchain binding, test/acceptance evidence, limitations and DEBT. Stage IDs, campaign ownership, chronology and original paths remain recoverable from the index even when physical payloads are shared. After verified campaign publication, remove superseded small stage/checkpoint ZIPs and obsolete sidecars; do not keep or nest them as an additional history layer. Later campaigns reference earlier canonical evidence without embedding it. Checkpoints store indexed progress, DEBT and content references, not another full accumulated evidence copy.

**Mandatory publish-before-delete transaction.**

1. Bind source stable identity, file/version identity, size, modification time and source SHA-256. Establish 100% coverage of unique leaf bytes and required provenance/paths, findings, logs, licenses, acceptance scope and DEBT.
2. Validate new ZIP path safety, CRC, member SHA-256 manifests, zero repeated payload SHA-256 across the canonical collection, per-part size and normal independent opening.
3. Save every replacement part and mapping/index to its authoritative location. Verify the persisted bytes/readback hashes and resolve all retained dependencies before any source removal.
4. Update stage/campaign/content-location and supersession references. Then delete fully covered superseded small research/audit packages and obsolete duplicate sidecars. Keep one cleanup receipt with original source IDs/hashes and destination mappings. This user decision supplies continuing cleanup authorization; do not repeatedly ask for deletion permission once these guards pass. Any missing or ambiguous unique content blocks deletion of that source.
5. Inspect each mutation result. Move an active failed deletion target to `/bogus` if permitted and record the outcome. If it is no longer active/available, update the `/bogus` checklist; do not restore a trashed item merely to move it. A failure without confirmed success is not counted as reclaimed space. Report gross confirmed removals, new canonical bytes and net active-byte reduction separately; physical quota recovery requires separate evidence.

**Boundaries.** Historical official release ZIPs and independently required exact release-bound receipts/provenance are not deleted or rewritten by this rule. Keep a genuinely required authoritative baseline/current runnable source state once and reference it; IDS exact-parent/full-current-state discipline and truthful native/game acceptance remain unchanged. Compaction does not authorize a production source change, a release, semantic-version advancement, current-pointer promotion or audit activation. It supersedes older unconditional retention of small research/audit wrapper containers, including the former permanent per-stage-ZIP interpretation, only after verified content preservation. Future release governance mirrors import this normative contract; existing immutable release mirrors remain at their recorded publication revision.

## 19. Library ownership map

Persistent project areas:

- `/DLL R&D Football/governance/` — release-independent governance recovery master.
- `/DLL R&D Football/releases/` — immutable published artifacts of the four official release classes and release-bound provenance/history; historical release evidence lives here/other Library evidence areas rather than inside current release `docs/`.
- `/DLL R&D Football/Remediations/` — self-contained REMEDIATION_READY/fix-line artifacts.
- `/DLL R&D Football/research/` — research/evidence packages.
- `/DLL R&D Football/Audits/` and `/audit/` — audit evidence/campaign material.
- `/DLL R&D Football/Experiments/` — explicitly experimental/non-canonical work.

The Library governance master is the **authoritative release-independent governance source** and the first recovery source after context loss. Governance embedded in a release is a hash-bound release snapshot/mirror, not a competing authority. Every newly published official release must regenerate/synchronize that mirror from the current Library master, record the master revision/hash in release provenance, and fail closed on any unresolved conflict.

