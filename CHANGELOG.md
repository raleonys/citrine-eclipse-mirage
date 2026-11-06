# Changelog

This file distinguishes integrated source changes, imported source history and published releases. A GitHub merge does not create an R-numbered release or release tag. Validation belongs to its exact source commit and evidence class.

## Unreleased — GitHub development

### This PR

- Keep Golden subprocess and runtime diagnostics in the complete on-disk report while printing only the final result, executed-stage count and runtime exit state. Preserve current GOLDEN_PASS criteria, all six stages, per-stage announcements, exit codes and distribution recovery; cover successful, blocked, stage-failure and runtime-failure console/report contracts.

- Register machine-specific Windows path masks before checkout/tool setup in the current full-validation Windows profiles, portable and reference jobs, plus the manual operator fixture. Normalize single backslashes to forward slashes, reject CR/LF before emitting logging commands and escape literal percent characters. Preserve integrity-only opening CI, the eleven-check synchronization gate, build/test commands and unmodified diagnostic artifacts; masking affects subsequent hosted console presentation.

- Exercise A11-02 startup flag admission with preseeded/duplicate and unrelated publications, both worker publication orders and observed worker payload values. Reopen the controlled sleep gate after each incomplete phase so early returns cannot hide behind the gate; retain the 10 ms test clock, bounded failure cleanup, cold-cache checks and 120000 contended critical sections. The control gate and worker joins also synchronize payloads, so this is bounded scenario coverage rather than an isolated memory-ordering proof. No production code or test timeout changes.

- Target the later provider FreeLibrary call by its exact ordinal in the staged module cleanup test, so the second attempt actually retires the extra lease before the injected provider failure. Preserve all physical image, counter, retry and final-unload assertions.

- Add an A11-06 source-linked native staged-cleanup regression: an extra-library FreeLibrary failure prevents provider retirement, the next explicit retry releases the extra lease before a separate provider FreeLibrary failure, and final/idempotent retries release exactly the remaining module without double-free. Test-only; no shipping owner, ABI or vendor behavior change.

- Cover C15 initial async-accept submission failure after port/readiness publication: require the source-defined error propagation and no continued IO run, while preserving the unresolved native startup-readiness contract.

- Adopt Archive 12's bound-server-port boundary cases (0, 80, 443 and 65535) into the actual portable server source test. Verify decimal EALsxPort publication, readiness-before-accept ordering, balanced source locks and ignored environment-write failure without changing production behavior. Original DLL/native execution remains separate evidence.

- Restore the owner-selected reference NoResponse behavior: consume only the shared typed no-response terminal before response framing/encryption/event selection, keep the connection open and admit its next command. Bind both Application emitters to that type, retain unrelated error handling, and test consecutive silent commands, sequential/coalesced plaintext/encrypted frames, event retention/delivery and a valid same-session follow-up. Native Windows sockets/Golden and real Consumer acceptance remain separate from portable source tests.

- Own a missing Config2 lookup key before traversal rehash can invalidate an aliased string_view, and erase its provisional child if table insertion throws. Cover long borrowed prefixes at 8-to-64 growth, independent traversal ordering, existing-key stability and failed-insert retry while retaining the recursive assignment allocation-failure campaign. Synchronize the source-file inventory and its enclosing manifest digest for packaging and Golden verification.

- Preserve QueryImage owner ID and signed width/208 through the production avatar provider call. Keep the legacy path-only Effects interface available to source adapters, select the complete call in the application, and test real slot5 argument forwarding and nonzero-byte return handling. Make foreign-table byte offsets explicit with unsigned-character storage/pointers, retaining memcpy reads and unaligned-slot regression coverage; resolve all three PR #161 CodeQL sizeof-scaling findings without suppressing the rule. Preserve the accepted provider-auth Registry hook fixture and unsafe avatar-component rejection in both provider routes.

- Add process-interruption recovery for Golden dist promotion: retain the current DLL/checksum/DIST_RECEIPT.json names and receipt schema, publish a verified previous-bundle journal before replacement, and recover under a persistent OS-held writer lock. Preserve current Golden output safety checks and test hard exits during promotion/recovery, damaged snapshots, lock contention and failed-run recovery. Flush owned staging copies with Windows write access and restore the previous bundle after a handled journal-retirement failure, with canonical temporary paths in Windows fault injection; power-loss durability and atomic visibility to direct readers remain outside this guarantee.

- Extend the native Application fixture with a real provider auth getter that reenters the active registry hooks after startup readiness. Assert configured ClientPath, one completed open/query/close probe, disabled-probe controls and single provider retirement after client reads drain; retain the provider ABI and fixture-only advapi32 dependency. Shipping behavior and vendor sources are unchanged.

- Audit F-C24-001: route license-ticket output through opt-in write/close status checks so nonthrowing late failures cannot report ticket success. Preserve the recovered initial-open-only legacy writer, other file-write callers and public DLL ABI; add deterministic failure coverage and a portable file-I/O target. Isolate the checked-write native fixture from the existing 8193-byte partial-read/EOF fixture so both scenarios retain their own input bytes.

- Audit C36-F002: reject unsafe Config2 ContentId filename components before license-path construction, and reject empty-ID license writes before the license callback. Preserve supported ASCII IDs and no-license empty-ID configurations; retain source-linked boundary regressions. This conservative input policy still requires owner compatibility review; lexical checks do not establish reparse-safe filesystem confinement.

- Reject an invalid native binary-input `tellg()` position instead of converting its failure sentinel into a huge unsigned file length; add EOF-state regression coverage using the existing generated file fixture.

- Preserve the last online-profile cache when a provider-supplied persona ID, user ID or username contains the unescaped semicolon delimiter: the existing offline reader only accepts exactly three fields. Keep the authenticated in-memory identity, normal valid profile cache bytes, and the original offline parser unchanged. Add source-linked online tests for each delimiter position, cache enabled/disabled and safe round-trip inputs; this is deliberate product hardening rather than a claim of donor binary parity. Keep the source-file inventory and its enclosing manifest hash synchronized for package and Golden verification.

- Reject ASCII control characters U+0000–U+001F and DEL in canonical vendor alias targets while retaining the existing relative-path, escape and malformed-record checks. Preserve the locked aliases and all vendor bytes/pins.
- Retain the control-character rejection/valid-descriptor regression alongside the malformed alias-table/target-type regression integrated in #218. This changes source validation, not runtime behavior or dependency versions.

### Accepted-main checkpoint — 2026-11-05

Verified against [`main@2ef4ea9f8c2613540566f2dab6a208cfaf6b438e`](https://github.com/raleonys/citrine-eclipse-mirage/commit/2ef4ea9f8c2613540566f2dab6a208cfaf6b438e). All 82 PRs below have matching GitHub merge records and merge commits reachable from that source checkpoint after the earlier #62 checkpoint. Dates use Europe/Istanbul. The list records integrations, not passing tests, releases or unmerged proposals. Later entries supersede earlier implementation and policy decisions.

The current XML backend is pugixml behind the project compatibility layer (#149/#150), with the self-aliasing printer correction (#193). MinHook uses the existing hook-engine adapter (#148); PEGTL grammar is private behind the parser interface (#231). The native completion fixtures cover held receive (#107), send/output (#114) and accept (#130) packets across bounded stop retries; these controlled schedules do not establish exhaustive kernel identity or Consumer equivalence.

Full validation follows actual main integration or a later verified synchronization/label request (#212/#225); opening a PR runs lightweight integrity checks. The eleven required checks remain mandatory. Single-PR branches are temporary; the continuing research and owner-designated audit exceptions from #103/#110 supersede #100's broader original wording. Weekly release preparation remains owner-published and separate from PR integration.


### Integrated 2026-11-05

- [PR #142](https://github.com/raleonys/citrine-eclipse-mirage/pull/142): reject missing and header-overlapping symbol-table pointers in ordinary COFF and BigObj size evidence, preserving valid section accounting and existing unsupported-object reporting.
- [PR #187](https://github.com/raleonys/citrine-eclipse-mirage/pull/187): preserve committed Config2 state across recursive assignment failures; repair the checked-MSVC allocation fixture without changing iterator settings or timeouts. Validate actual-source rollback, sanitizer coverage and exact-head Windows Debug/native acceptance.
- [PR #165](https://github.com/raleonys/citrine-eclipse-mirage/pull/165): repair 24 displayed source-path labels in historical comparison tables, including the private PEGTL grammar, while preserving finding text, link targets, fragments and donor references.
- [PR #232](https://github.com/raleonys/citrine-eclipse-mirage/pull/232): update the exact owner campaign logo and provenance, reconcile the development changelog through #115, and correct current documentation for vendor/resource ownership, VDF policy, completion coverage and validation evidence.
- [PR #115](https://github.com/raleonys/citrine-eclipse-mirage/pull/115): distinguish the retained Archive 11 ZIP (`db394434...`) from the historical B49–B54 same-named donor (`4b98177c...`). Preserve both evidence identities as `PROVENANCE_OPEN` and require per-file paths, byte lengths and SHA-256 verification before transferring donor-specific assertions; no donor equivalence or runtime change is established.
- [PR #130](https://github.com/raleonys/citrine-eclipse-mirage/pull/130): test: hold a real accept IOCP completion across shutdown retries (A11-05/B49).
- [PR #114](https://github.com/raleonys/citrine-eclipse-mirage/pull/114): test: retain a real WSASend completion and output lease across shutdown retries (B50).
- [PR #107](https://github.com/raleonys/citrine-eclipse-mirage/pull/107): test: retain real socket completions across bounded shutdown retries.
- [PR #193](https://github.com/raleonys/citrine-eclipse-mirage/pull/193): fix: prevent self-aliased XML writer use-after-free.
- [PR #231](https://github.com/raleonys/citrine-eclipse-mirage/pull/231): refactor: isolate PEGTL grammar behind the public parser interface.
- [PR #226](https://github.com/raleonys/citrine-eclipse-mirage/pull/226): Update owner logo master, development changelog and vendor provenance.
- [PR #188](https://github.com/raleonys/citrine-eclipse-mirage/pull/188): fix: reject achievement watcher INI section injection (A11-09).
- [PR #203](https://github.com/raleonys/citrine-eclipse-mirage/pull/203): fix: atomically refresh source manifest without truncating hardlinks.
- [PR #206](https://github.com/raleonys/citrine-eclipse-mirage/pull/206): fix: use filename UTC format for licensing request tickets.
- [PR #229](https://github.com/raleonys/citrine-eclipse-mirage/pull/229): fix: print operator build verdicts once across all five profiles.
- [PR #216](https://github.com/raleonys/citrine-eclipse-mirage/pull/216): qa: unify weekly JUnit structure and test-count validation.
- [PR #205](https://github.com/raleonys/citrine-eclipse-mirage/pull/205): fix: reject out-of-range achievement XML numbers (A11-09).

### Integrated 2026-11-04

- [PR #218](https://github.com/raleonys/citrine-eclipse-mirage/pull/218): fix: reject malformed vendor alias record types.
- [PR #154](https://github.com/raleonys/citrine-eclipse-mirage/pull/154): fix: contain exceptions from crypt callback diagnostics.
- [PR #163](https://github.com/raleonys/citrine-eclipse-mirage/pull/163): test: cover actual Environment license key boundaries.
- [PR #223](https://github.com/raleonys/citrine-eclipse-mirage/pull/223): fix: escape license XML text before encryption (A11).
- [PR #179](https://github.com/raleonys/citrine-eclipse-mirage/pull/179): fix: confine A11-09 achievement XML and watcher path components.
- [PR #175](https://github.com/raleonys/citrine-eclipse-mirage/pull/175): fix: avoid reading caller output during native UTF-16 conversion.
- [PR #180](https://github.com/raleonys/citrine-eclipse-mirage/pull/180): test: keep invalid LSX process failures isolated from queued events (A11-09).
- [PR #72](https://github.com/raleonys/citrine-eclipse-mirage/pull/72): audit: guard QueryImage avatar provider path components (C36-F001).
- [PR #225](https://github.com/raleonys/citrine-eclipse-mirage/pull/225): ci: require later synchronization before accepting validation labels.
- [PR #70](https://github.com/raleonys/citrine-eclipse-mirage/pull/70): audit: prevent Golden output symlink overwrite (C35-F002).
- [PR #212](https://github.com/raleonys/citrine-eclipse-mirage/pull/212): ci: defer full validation until main synchronization or manual request.
- [PR #144](https://github.com/raleonys/citrine-eclipse-mirage/pull/144): test: retain grant XML/INI rollback after native locked destination (A11-09).
- [PR #69](https://github.com/raleonys/citrine-eclipse-mirage/pull/69): audit: retain buffered Debug log entries after failed file open (F-C28-001).
- [PR #156](https://github.com/raleonys/citrine-eclipse-mirage/pull/156): docs: refresh current XML dependency and changelog scope.

### Integrated 2026-11-03

- [PR #117](https://github.com/raleonys/citrine-eclipse-mirage/pull/117): test: pin LSX multi-command first-child dispatch behavior (C21).
- [PR #155](https://github.com/raleonys/citrine-eclipse-mirage/pull/155): fix: validate source manifest byte counts alongside hashes.
- [PR #152](https://github.com/raleonys/citrine-eclipse-mirage/pull/152): chore: remove obsolete build labels from operator tools and reports.
- [PR #124](https://github.com/raleonys/citrine-eclipse-mirage/pull/124): test: verify queued and all-hook MinHook IP relocation (D326-F0006).
- [PR #151](https://github.com/raleonys/citrine-eclipse-mirage/pull/151): test: automate production DLL XML contracts and paired performance reports.
- [PR #138](https://github.com/raleonys/citrine-eclipse-mirage/pull/138): test: retain hook ownership after MH_ERROR_DISABLED and drain retries (A11-07).
- [PR #150](https://github.com/raleonys/citrine-eclipse-mirage/pull/150): build: remove obsolete TinyXML2 target aliases.
- [PR #149](https://github.com/raleonys/citrine-eclipse-mirage/pull/149): feat: migrate production XML to immutable pugixml with compatibility policy.
- [PR #135](https://github.com/raleonys/citrine-eclipse-mirage/pull/135): test: reject MinHook peer context write failures (A11-07).
- [PR #148](https://github.com/raleonys/citrine-eclipse-mirage/pull/148): refactor: isolate MinHook behind the existing hook engine API.
- [PR #116](https://github.com/raleonys/citrine-eclipse-mirage/pull/116): ci: retain CTest JUnit reports at their expected artifact paths.
- [PR #131](https://github.com/raleonys/citrine-eclipse-mirage/pull/131): test: run caller Git isolation in Windows infrastructure suites.
- [PR #137](https://github.com/raleonys/citrine-eclipse-mirage/pull/137): fix: publish Source QA reports without truncating hardlinked files.
- [PR #127](https://github.com/raleonys/citrine-eclipse-mirage/pull/127): fix: isolate root-ownership Git fixtures from hook repository selectors.
- [PR #68](https://github.com/raleonys/citrine-eclipse-mirage/pull/68): audit: recover source ZIP when checksum publication fails (C34-F002).
- [PR #67](https://github.com/raleonys/citrine-eclipse-mirage/pull/67): audit: block source package symlink partial attacks (C34-F001).
- [PR #66](https://github.com/raleonys/citrine-eclipse-mirage/pull/66): audit: reject invalid source manifest headers (C34-F005).
- [PR #65](https://github.com/raleonys/citrine-eclipse-mirage/pull/65): audit: enforce ownership ledger schemas and role contracts (C34-F003).
- [PR #146](https://github.com/raleonys/citrine-eclipse-mirage/pull/146): chore: refresh project campaign logo from the new owner-supplied PNG.
- [PR #64](https://github.com/raleonys/citrine-eclipse-mirage/pull/64): fix: confine cache-cleanup reports before deleting caches.

### Integrated 2026-11-02

- [PR #110](https://github.com/raleonys/citrine-eclipse-mirage/pull/110): policy: preserve owner-authorized Draft research PR #81.
- [PR #103](https://github.com/raleonys/citrine-eclipse-mirage/pull/103): policy: preserve Reverse Engineering projects, keep routine audits temporary.
- [PR #100](https://github.com/raleonys/citrine-eclipse-mirage/pull/100): chore: update project logo and adopt temporary PR branches.
- [PR #57](https://github.com/raleonys/citrine-eclipse-mirage/pull/57): fix: pin Windows build-output components through profile cleanup.
- [PR #99](https://github.com/raleonys/citrine-eclipse-mirage/pull/99): test: validate registry callback lifecycle with real Windows keys.
- [PR #97](https://github.com/raleonys/citrine-eclipse-mirage/pull/97): audit: reassess Hooks of Pioneering registry provenance on current main.
- [PR #98](https://github.com/raleonys/citrine-eclipse-mirage/pull/98): fix: roll back fabricated registry keys when registration fails.
- [PR #77](https://github.com/raleonys/citrine-eclipse-mirage/pull/77): fix: bound Config2/VDF nesting at 128 before PEGTL parsing.
- [PR #82](https://github.com/raleonys/citrine-eclipse-mirage/pull/82): audit: reconcile B49-B54 findings with accepted native coverage.
- [PR #91](https://github.com/raleonys/citrine-eclipse-mirage/pull/91): test: make shutdown listener diagnostics bounded and exception-safe.
- [PR #94](https://github.com/raleonys/citrine-eclipse-mirage/pull/94): test: cover R11 unload ownership with existing native gates (D54).
- [PR #85](https://github.com/raleonys/citrine-eclipse-mirage/pull/85): test: malformed ordered-session completion must not commit receipts (packet R-02).
- [PR #86](https://github.com/raleonys/citrine-eclipse-mirage/pull/86): test: dual-stack Asio accept cancellation finality (packet D49).
- [PR #88](https://github.com/raleonys/citrine-eclipse-mirage/pull/88): test: verify online provider physical DLL lease across failed cleanup (packet D54).
- [PR #87](https://github.com/raleonys/citrine-eclipse-mirage/pull/87): fix: balance reference socket cancellation gate on exceptions (packet D50).
- [PR #90](https://github.com/raleonys/citrine-eclipse-mirage/pull/90): fix: defer early TCP accept until application startup readiness (B48).
- [PR #96](https://github.com/raleonys/citrine-eclipse-mirage/pull/96): fix: restore process error mode on provider load exceptions (packet B53).
- [PR #28](https://github.com/raleonys/citrine-eclipse-mirage/pull/28): Read foreign provider slots as bytes and match the destroy call.
- [PR #93](https://github.com/raleonys/citrine-eclipse-mirage/pull/93): policy: restore LUTRAVEX-approved weekly semver and Citrine release archives.
- [PR #89](https://github.com/raleonys/citrine-eclipse-mirage/pull/89): fix: preserve shutdown retries and avoid TLS marker false positives (R-03).
- [PR #95](https://github.com/raleonys/citrine-eclipse-mirage/pull/95): ci: run full Windows Native, Golden, CRT, Debug and portable QA on every PR.
- [PR #84](https://github.com/raleonys/citrine-eclipse-mirage/pull/84): fix: prevent stale native callback error-policy use (packet R-01).

### Integrated 2026-11-01

- [PR #83](https://github.com/raleonys/citrine-eclipse-mirage/pull/83): test: PR23 post-merge Windows listener shutdown acceptance (draft).
- [PR #23](https://github.com/raleonys/citrine-eclipse-mirage/pull/23): Guard connect sockaddr inspection without changing original Winsock errors.
- [PR #73](https://github.com/raleonys/citrine-eclipse-mirage/pull/73): audit: remove blocking entitlement modal from shared handler lock (C36-F003).
- [PR #19](https://github.com/raleonys/citrine-eclipse-mirage/pull/19): fix: correct MinHook single-disable peer IP relocation (D326-F0006).
- [PR #80](https://github.com/raleonys/citrine-eclipse-mirage/pull/80): fix: restore PR71 reference graph type identities and add Windows acceptance gate.
- [PR #76](https://github.com/raleonys/citrine-eclipse-mirage/pull/76): test: fix MSVC Debug fault injection and validate Windows portable checks.
- [PR #79](https://github.com/raleonys/citrine-eclipse-mirage/pull/79): chore: retire recovery-era wording from active C++ source.
- [PR #71](https://github.com/raleonys/citrine-eclipse-mirage/pull/71): refactor: normalize internal identifiers and enforce SPDX headers.
- [PR #63](https://github.com/raleonys/citrine-eclipse-mirage/pull/63): docs/license: adopt GPLv3 and correct campaign artwork with build badges.

### Historical merged-PR checkpoint through #62

The following 52 entries were verified through PR #62. The GitHub Releases collection was empty at that checkpoint; this historical count and release observation do not describe the current repository state.

### 2026-10-31

- [PR #62](https://github.com/raleonys/citrine-eclipse-mirage/pull/62): docs/assets: display final campaign artwork and simplify README.
- [PR #61](https://github.com/raleonys/citrine-eclipse-mirage/pull/61): ci/docs: require changelog coverage and pin Citrine Eclipse Mirage branding.
- [PR #60](https://github.com/raleonys/citrine-eclipse-mirage/pull/60): docs: establish Citrine Eclipse Mirage identity and repair stale changelog.
- [PR #59](https://github.com/raleonys/citrine-eclipse-mirage/pull/59): refactor: retire duplicate root release-state records.
- [PR #58](https://github.com/raleonys/citrine-eclipse-mirage/pull/58): docs: consolidate governance and retire historical scope ledger.
- [PR #56](https://github.com/raleonys/citrine-eclipse-mirage/pull/56): docs: introduce yellowfield governance and approved README.
- [PR #55](https://github.com/raleonys/citrine-eclipse-mirage/pull/55): build: enforce English-only diagnostics and first-party language policy.
- [PR #53](https://github.com/raleonys/citrine-eclipse-mirage/pull/53): qa: audit C++ and test language signals with ASCII diagnostic regression.
- [PR #52](https://github.com/raleonys/citrine-eclipse-mirage/pull/52): test: resolve Native MSVC C4244/C4312/C4805/C4834 fixture warnings.
- [PR #54](https://github.com/raleonys/citrine-eclipse-mirage/pull/54): fix: canonical import descriptor order in every unsigned MSVC application DLL.
- [PR #51](https://github.com/raleonys/citrine-eclipse-mirage/pull/51): qa: translate build diagnostics and protect English-only error messages.
- [PR #50](https://github.com/raleonys/citrine-eclipse-mirage/pull/50): fix: register configuration defaults test targets in build partition policy.
- [PR #49](https://github.com/raleonys/citrine-eclipse-mirage/pull/49): assets: update SURGE profile avatar from owner-supplied PNG.
- [PR #48](https://github.com/raleonys/citrine-eclipse-mirage/pull/48): qa/docs: audit first-party English-only content and enforce prose checks.
- [PR #47](https://github.com/raleonys/citrine-eclipse-mirage/pull/47): docs: translate socket retirement and shutdown recovery audits.
- [PR #46](https://github.com/raleonys/citrine-eclipse-mirage/pull/46): docs: translate donor recovery, Golden pipeline and vendor records.
- [PR #45](https://github.com/raleonys/citrine-eclipse-mirage/pull/45): docs: translate API import research and source audit records.
- [PR #44](https://github.com/raleonys/citrine-eclipse-mirage/pull/44): docs: translate historical comparison tables and build cleanup.
- [PR #43](https://github.com/raleonys/citrine-eclipse-mirage/pull/43): docs: translate source review and native acceptance records to English.
- [PR #42](https://github.com/raleonys/citrine-eclipse-mirage/pull/42): docs: complete English translation of the Bluefield comparison CSV.
- [PR #41](https://github.com/raleonys/citrine-eclipse-mirage/pull/41): docs: translate governance scope and two structured comparison records.
- [PR #40](https://github.com/raleonys/citrine-eclipse-mirage/pull/40): docs: translate runtime and build evidence guides to English.
- [PR #39](https://github.com/raleonys/citrine-eclipse-mirage/pull/39): docs: extend English-only migration to native integration and ABI guides.
- [PR #38](https://github.com/raleonys/citrine-eclipse-mirage/pull/38): docs: continue English-only migration across project guides.
- [PR #37](https://github.com/raleonys/citrine-eclipse-mirage/pull/37): docs: establish English-only documentation baseline.
- [PR #35](https://github.com/raleonys/citrine-eclipse-mirage/pull/35): Test Debug log record integrity and the Release logging gate.
- [PR #34](https://github.com/raleonys/citrine-eclipse-mirage/pull/34): Test native accept and receive ownership through cancellation.
- [PR #33](https://github.com/raleonys/citrine-eclipse-mirage/pull/33): test: recover listener admission after accepted-handler factory failure.
- [PR #32](https://github.com/raleonys/citrine-eclipse-mirage/pull/32): test: cover B37-style multi-client accept/rearm and session-cap behavior.
- [PR #31](https://github.com/raleonys/citrine-eclipse-mirage/pull/31): qa: reject stale or unsafe layout migration contract references.
- [PR #29](https://github.com/raleonys/citrine-eclipse-mirage/pull/29): Publish the cold performance-frequency cache atomically.
- [PR #27](https://github.com/raleonys/citrine-eclipse-mirage/pull/27): Release synchronous greeting storage before starting reads.
- [PR #26](https://github.com/raleonys/citrine-eclipse-mirage/pull/26): Fix online cookie temporary lifetime before provider queries.
- [PR #30](https://github.com/raleonys/citrine-eclipse-mirage/pull/30): Test defaults through configuration and serialized responses.
- [PR #36](https://github.com/raleonys/citrine-eclipse-mirage/pull/36): Require ready-for-review pull requests for all agents.

### 2026-10-30

- [PR #25](https://github.com/raleonys/citrine-eclipse-mirage/pull/25): Retain the online provider module after a failed FreeLibrary.
- [PR #21](https://github.com/raleonys/citrine-eclipse-mirage/pull/21): docs: remove obsolete bootstrap/export notes and curate evidence.
- [PR #24](https://github.com/raleonys/citrine-eclipse-mirage/pull/24): fix: standardize DLL import descriptor ordering across active policies and reports.
- [PR #18](https://github.com/raleonys/citrine-eclipse-mirage/pull/18): Fix audit F-C03-002 unresolved forbidden CMake dependency checks.
- [PR #8](https://github.com/raleonys/citrine-eclipse-mirage/pull/8): fix: worktree-safe root guard, source inventory and Windows CTest integration.
- [PR #15](https://github.com/raleonys/citrine-eclipse-mirage/pull/15): Fix audit F-C04-001 COFF .ctors initializer inventory.
- [PR #13](https://github.com/raleonys/citrine-eclipse-mirage/pull/13): Fix audit F-C03-001 uppercase C++ source partition detection.
- [PR #9](https://github.com/raleonys/citrine-eclipse-mirage/pull/9): Reject Windows-unsafe source archive member names.
- [PR #11](https://github.com/raleonys/citrine-eclipse-mirage/pull/11): Prevent incomplete source inventory updates before pushing.
- [PR #10](https://github.com/raleonys/citrine-eclipse-mirage/pull/10): Define mandatory branch rules for agents and contributors.
- [PR #7](https://github.com/raleonys/citrine-eclipse-mirage/pull/7): Reuse a shared bugfix branch for routine maintenance.
- [PR #6](https://github.com/raleonys/citrine-eclipse-mirage/pull/6): Prepare weekly draft releases with verified source and Windows artifacts.
- [PR #5](https://github.com/raleonys/citrine-eclipse-mirage/pull/5): Rewrite README as an English project guide.
- [PR #4](https://github.com/raleonys/citrine-eclipse-mirage/pull/4): Make Windows CI validation manual for the private repository.
- [PR #3](https://github.com/raleonys/citrine-eclipse-mirage/pull/3): Update Asio to the owner-provided source snapshot.
- [PR #2](https://github.com/raleonys/citrine-eclipse-mirage/pull/2): Add the Source and Windows checks badge to README.
- [PR #1](https://github.com/raleonys/citrine-eclipse-mirage/pull/1): Set up GitHub CI, manual PR reviews, and Windows test-path fixes.

## R-series source provenance (historical reconciliation required)

- **R24:** Source baseline imported 2026-10-30 by commit `50b6a293522bf63ebdd1fccc900b061abc964362` (`Import verified R24 source baseline`). This identifies the source archive ancestry, not a GitHub release publication.
- **R01–R23 and any other pre-GitHub revisions:** Exact revision-by-revision names, dates, artifact checksums, status and deltas have not been established in the available repository records. These entries require independent artifact-backed reconciliation. Do not invent missing milestones, or conflate research/audit R01/R02 checkpoints with product releases.
- Imported pre-GitHub engineering notes are retained below for traceability, in their original order. They are **unversioned notes**, not a verified replacement for the missing R-series release ledger.

## Imported pre-GitHub development notes (historical, unversioned)

The following sections are preserved verbatim from the R24 source-import changelog. Some linked historic evidence paths may have been retired in later repository maintenance.

## Numeric SDK greeting format

- Apply the owner's suffix-removal correction: advertise exactly four numeric SDK components, retaining the fixed config-independent identity policy.
- Require the exact quoted numeric value in the actual socket greeting despite conflicting main/override CFG values.

## Owner-pinned SDK greeting identity

- Pin the complete owner-provided SDK identity in LSX greeting format, retaining its build suffix until an explicit owner update.
- Remove the mutable SDK config field, both CFG assignment paths and obsolete normalization; main/override SDK values are ignored.
- Check the actual greeting against an independent expected value despite conflicting configs; preserve separate client negotiation and PE metadata.

## Unload guard ownership recovery

- Retain failed peer-resume and handle-close owners across shutdown retries; never close a scan-owned suspended live peer.
- Release a previous suspension before a new scan and retry close without repeating a successful resume, preserving other owners' suspend counts.
- Retain snapshot-close failures, reject overlapping scans without blocking and cover the actual production scanner with injected Windows API failures.

## Bounded automatic listener selection

- Select the first usable listener port from 3226 through 3236, inclusive; fail startup on exhaustion without an arbitrary-port fallback.
- Ignore obsolete ServerPort configuration in both application routes while retaining actual bound-port state and EALsxPort publication.
- Preserve existing connect/IPv6 hook behavior and verify range exhaustion, endpoint ownership and recovery with real socket tests.

## Hook lifecycle ownership and vendor commit provenance

- Record the four owner-archive commit comments alongside immutable archive/member SHA-256 pins, distinguishing archive claims from independent Git verification.
- Build MinHook with the selected NtGetNextThread SAFE-FREEZE patch in a separately verified build copy; preserve every original vendor member.
- Add explicit transaction states, rollback ownership retention, call drain and retryable DLL cleanup without changing the three production exports.
- Correct the donor helper's second-handle retention on simultaneous array allocation and two close failures; cover the actual patched engine with injected-failure tests.
- See [hook lifecycle contract and provenance](docs/HOOK_ENGINE_HARDENING.md) and [historical evidence](https://github.com/raleonys/citrine-eclipse-mirage/blob/50b6a293522bf63ebdd1fccc900b061abc964362/docs/evidence/HOOK_HARDENING.json).

## Owner-restored Windows standard IO — 2026-10-29

- Restore RE standard filesystem/fstream for Windows file IO, directory operations and Debug log stream.
- Remove unused Gulrak vendor/target/include dependencies; retain standard Unicode/binary IO coverage and the four remaining immutable vendors.
- Include Brownfield Asio Windows target definitions and verify old Debug log remains unchanged in Release.
- Restore Brownfield DllMain@1/SURGE@2/DllShutdown@3; remove diagnostic export slots4/5 and observe readiness via existing EALsxPort/LSX.
- Normalize the owner-selected six-module import descriptor order in the final copy; enforce exact record permutation and read-only final order checks.


## Asio Windows target definition — 2026-10-29

- Restore supplied Brownfield _WIN32_WINNT=0x0601/WINVER=0x0601 through the Windows Asio interface dependency.
- Remove the cause of the Asio missing-target diagnostic without warning suppression or vendor edits.
- Preserve PE metadata targets and record the owner-confirmed game startup on the parent package.


## Startup wait and build console correction — 2026-10-29

- Reproduce a native registry startup wait blocking peer callbacks, then bind the supplied Bluefield FIX19 Guard/Pause dependency; preserve original registry logic and LastError.
- Add missing NOMINMAX to the independent MinHook fixture target reported by the user MSVC log.
- Restore the five ordinary build-profile titles and keep successful Python cache cleanup quiet with a JSON report under build.
- Keep config optional and record the reported game no-start separately from Wine/native-host validation.


## Owner-selected Bluefield hook and full Brownfield metadata — 2026-10-29

- Adopt Bluefield protected UTF16 selection, original-call observation, queued hook activation and native unload guard within the authorized scope.
- Replace preserve-compiler policy with the supplied Brownfield timestamp0xCD268EB1/linker14.0/Rich-absent profile and all declared metadata/layout checks.
- Preserve raw linker DLL, five C exports, static CRYPT32, read-only avatar data, exact VERSIONINFO and all five vendors.
- Verify Release Wine173/173, affected Debug7/7, final-name DLL3cycles and 19 policy groups. Preserve initial2 socket timeouts and passing reruns; native MSVC/game acceptance pending.


## RE VDF grammar executed by PEGTL — 2026-10-28

- Replace the handwritten parser backend with PEGTL rules derived from the supplied RE grammar/action evidence.
- Bind the dedicated VDF object owner to the real DLL and existing config pipeline; preserve the public parser interface.
- Retain the 115 existing parser assertions and add 23,417 frozen-baseline corpus observations.
- Exercise nested VDF, duplicate actions, BOM and failure position through the dynamically loaded DLL.
- Preserve vendor bytes, TinyXML2 500, filename/precedence policy and comparison-only donor roles.

## Vendor compatibility follow-up — 2026-10-28

- Keep all five owner-archive vendor trees and TinyXML2 depth 500 unchanged.
- Verify PEGTL public headers with LLVM-MinGW 20251118 / Clang 21.1.6; retain the Clang 23 incompatibility record.
- Require PEGTL compatibility at configure time even with BUILD_TESTING=OFF; explicit diagnostic-only opt-out remains visible.
- Add real Asio loopback I/O/EOF/cancellation and ghc::filesystem Unicode/binary I/O fixtures.
- Preserve application implementations, grammar, storage policy and runtime ownership decisions.

## Exact owner-archive vendoring — 2026-10-28

- Materialize all five supplied archive trees and preserve the two Asio symlinks.
- Replace old RE MinHook/TinyXML2 copies; take no Brownfield vendor files.
- Verify archive pins and every member; keep all vendor defaults and bytes unchanged.
- Use TinyXML2's default 500; record the difference from the older RE profile 100.
- Add offline CMake targets, API fixtures and separate PEGTL toolchain diagnostics.
- Preserve typed links through flat packaging and safe extraction; reject alias tampering.

## Brownfield infrastructure adaptation — 2026-10-28

- Restore the broad integration mandate and track all ten transfer categories.
- Adapt four PE Golden tools, the MSVC link audit and compatible negative suites.
- Add a gated six-stage local pipeline with exact build/DLL/source binding.
- Add five registered operator profiles and compiler CRT/runtime probes.
- Add metadata/script ownership, target-drift guards and a visible source-QA runner.
- Package a deterministic flat source tree against the verified source manifest.
- Verify the exact owner SURGE image in the real DLL; preserve application sources.
- Keep final Golden product values, Windows/MSVC execution and game acceptance explicit.

## Local source layout correction — 2026-10-28

- Replace the raw RE-module tree with functional application directories.
- Separate public headers, tests, third-party sources and operator tooling.
- Remove whole preloader/version and private CRT research packages from build.
- Decouple the approved hook-11 selector from the preloader project.
- Add an actual development DLL target and a dynamically linked source host.
- Adapt Brownfield build launchers; keep publication and Golden acceptance separate.
