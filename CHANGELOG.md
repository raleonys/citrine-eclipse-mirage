# Changelog

This file distinguishes source imports, integrated pull requests and published releases. A GitHub merge is a source change, not an R-numbered release or release tag.

## Unreleased — post-import GitHub development (2026-10-30 to 2026-10-31)

The 52 entries below are the merged pull requests verified through PR #62, linked to their individual review records. The GitHub Releases collection was empty at this checkpoint, so these entries must not be read as published releases or evidence of Windows/native/game acceptance.

### Additional unreleased changes

- Audit C34-F004: confine Python cache-cleanup report files to the owned build tree and publish them through exclusive temporary files; reject unsafe, directory and missing-parent destinations before deleting caches. Cover linked-directory escapes, hardlinked sources and failed report publication.

- Pin Windows build-output path components before the exclusive lock and retain them through the build profile. Add controlled cross-process junction replacement, rename, ACL-denial, rollback and output-confinement regressions; release pins in reverse order and restore environment state even after cleanup faults. Use directory-list access so Windows enforces the no-delete sharing constraint, and run the same native fixture before long CI builds for early feedback. Document the selected-component trust boundary; runtime and vendor sources are unchanged.

- Exercise the production Registry callbacks and SDK original-call adapters against isolated real Windows HKCU keys in A/W lifecycle tests. Cover native and fabricated opens, query/close overlap during startup waits, binding leases, untracked SDK opens and bounded numeric reuse observations. Preserve runtime behavior and distinguish these callback fixtures from MinHook and Consumer provenance acceptance.

- Recheck the historical Hooks of Pioneering Registry ownership research against accepted main: raw HKEY provenance and retirement remain design/evidence gaps, while prior HeapFree failure receipts do not describe the current operator-new/delete backend. Record a source-linked applicability and proof matrix, synchronized with the allocation/insert rollback accepted in PR #98, without changing hooks, runtime code, tests, vendors or ABI.

- Roll back fabricated Registry key allocation when either owning-set or route-set insertion throws, restoring the caller output and removing only the new ownership entry before releasing the allocation. Preserve successful A/W effect order and cover real container allocation failures, logging exceptions, allocation failure and ordinary close on Linux and Windows; native handle provenance remains separate research.

- Add a reviewed Config2/VDF depth-limit candidate: reject structural nesting beyond 128 before recursive PEGTL actions. Extend boundary, conversion, byte/comment, error-position and retained-State regressions; preserve the frozen VDF corpus. The new input policy requires owner compatibility approval before merge.

- Reconcile all ten B49-B54 source-audit findings with accepted main, distinguishing covered startup/accept/module-lease scenarios from open late-completion and foreign-callback lifetime proof. Preserve the historical donor comparison and identify unavailable donor-byte evidence explicitly; no runtime or test policy changes.

- Add bounded, best-effort IPv4/IPv6 listener owning-PID diagnostics to the public DLL lifecycle test host. Keep diagnostic allocation, API and output failures separate from authoritative shutdown assertions; cover fault paths and table validation in Windows tests. Production code, shutdown budgets and pass/fail criteria are unchanged.

- Extend source-included unload scanner coverage for non-TLS R11 image references, balanced suspension/handle cleanup, cleared references and shutdown-caller exclusion. Preserve the exact validated TLS data-marker exemption and run the added cases in the existing Windows Debug/Release native and shutdown gates without a duplicate workflow; production code is unchanged.

- Cover ordered LSX send completion anomalies with three receipt-safety regressions: over-reported first/final chunk lengths and full-length completions carrying socket errors. Run the session-flow CTest on genuine Windows MSVC Debug/Release in the existing reference acceptance workflow; no production runtime behavior changed.

- Add actual Asio IPv4/IPv6 pending-accept shutdown regression scenarios, with both unconnected and already-connected peers, to distinguish cancellation intent from callback finality and prevent post-stop rearm. Include the native net-failure test in focused Windows MSVC Debug/Release lifecycle CI; no production behavior changed. Continue focused cancellation checks after a lifecycle failure only when the native build succeeded.

- Add a native Windows online-provider physical image-lease regression: after an injected FreeLibrary failure, verify the DLL remains mapped and callable until exact retry succeeds, then verify actual unload and no double release. Separate MSVC Debug/Release acceptance from future foreign-callback concurrency validation; production provider code remains unchanged.

- Repair the reference socket cancellation gate's exception path: release the selected engine gate when category resolution or cancellation rejects unsupported source input, while preserving normal close order and retaining the socket/control state on failure. Add focused injected-failure regression checks with unchanged strong/weak owner counts, and keep active-source terminology within the enforced policy; shipping Asio is unchanged. Add focused Windows Debug/Release execution of socket_closing_contracts to reference CI, and stack this change on the queued shutdown fix in PR #89.

- Defer application-owned Asio accept callbacks until provider/profile startup admission is published, while retaining the early bound listener and OS TCP backlog. Add a source-linked native startup regression proving an early connecting peer stays connected until the subsequent greeting. Standalone Asio services and internal failure-injection fixtures retain immediate admission by default; full native MSVC and Consumer validation remain required.

- Restore the process-wide Windows error mode if optional online-provider path conversion or DLL loading throws, preserving the originating exception even when restoration fails. Extend portable provider failure-injection contracts for the four pre-load stages and restoration fault. The reconstruction packet's factory-failure scenarios informed this review; native/Golden/Consumer acceptance is pending.

- [PR #28](https://github.com/raleonys/citrine-eclipse-mirage/pull/28): Read online-provider object and method-table fields through byte copies, reject missing tables or methods before indirect calls, and match the observed single-object destruction call. Add malformed-slot, unaligned-storage and module-cleanup regressions.

- Restore owner-approved 34-name LUTRAVEX weekly semantic version queue from project Library and Citrine_Eclipse_Mirage_x.y.z_CODENAME.zip naming; retire -RNN prospectively without changing ABI or release history.

- Run actual Windows Native Release/Debug, Production/Golden Release, CRT Release, Debug, Compile Only, full Windows portable Debug/Release, and reference graph automatically for every PR. Keep existing required Windows native/production check names and preserve weekly draft-only publication; no runtime source or ABI change.

- Give the unchanged 192-case socket pool release reference matrix a bounded 180-second native Debug test budget. Preserve its 60-second non-Debug budget, existing Wine Debug override, checked CRT, payloads and failure assertions.

- Fix the MSVC Debug unload scan false positive caused by an idle Windows peer retaining the compiler `_tls_end` data marker in R11. Validate the exact PE TLS endpoint, its non-executable section and actual image page before exempting data registers/stack slots; keep every in-image RIP unsafe. Add native rejection-boundary tests and run unload_guard_contracts alongside repeated lifecycle acceptance.

- Extend queued shutdown acceptance with two persistent cleanup failures: both requests are consumed, the sole owner remains retained, and cleanup resumes only after a third explicit request.

- Fix lost DllShutdown retries when two manual-reset event requests coalesce before the worker reset: consume one atomic request ticket per public call, preserving a separate retry after an incomplete first cleanup. A deterministic real DLL worker fixture reproduced the original failure on Windows Debug and Release before this correction; focused MSVC regression and existing native fault modes must pass on the patched head.

- Fix the owner-scoped native callback error policy lifetime: borrow it only under a live callback lease, keep the lease during terminal exception reporting, and clear the cached policy after successful unbinding. Add isolated Windows post-close and live-throw regressions, including a competing close during error reporting and guarded storage after policy destruction. The external reconstruction packet informed this audit; native/Golden/game acceptance remains pending.

- Add a PR #23 post-merge Windows listener-shutdown follow-up with repeated Debug/Release CTest evidence and an explicit unpinned local Compile Only failure; production runtime remains unchanged.

- Make the native CreateProcessA argument-forwarding fixture use explicit caller buffers for pointer identity, removing its dependence on string-literal pooling in MSVC Debug while retaining all ten argument assertions. Production callbacks and connect behavior are unchanged; fresh native Debug/Release validation remains pending.

- Guard native connect address inspection with a bounded readable snapshot, forwarding malformed addresses to Winsock with their original pointer/length and error behavior. Preserve exact loopback redirection, add portable snapshot/no-listener checks, and register focused Windows Debug/Release callback and real Winsock equivalence coverage; native validation remains pending.

- Audit C36-F003: keep unknown-entitlement diagnostics nonmodal and redacted, repair native test namespace scope, and cover Debug output plus Release preservation of existing logs. Add focused Windows MSVC Debug/Release acceptance; fresh native execution remains pending.

- Correct single-hook disable IP relocation in the pinned MinHook build copy, preserve upstream vendor bytes, and add exact patch, source, and focused Windows MSVC Debug/Release regression checks. The focused fixture passed; genuine suspended-peer and full Native/Golden acceptance remain pending.

- Isolate the MSVC C++ development DLL and host in a shared compiler-CRT graph, including separately compiled vendor dependencies, to avoid cross-module static Debug CRT ownership during parser-state destruction. Production retains its static CRT, sources and three-export ABI; native Debug/Release validation remains required.

- Report source-host smoke success only after local C++ destruction and successful application shutdown; preserve failure codes and Debug timeout diagnostics.

- Investigate the separate Windows MSVC Debug source-host timeout with test-only startup, LSX and shutdown stage markers; do not alter production runtime behavior.
- Repair 12 PR #71 C++ interface/adapter identity collisions in the source-linked reference graph, add bounded type-identity regression checks and Windows Debug/Release reference-object compilation gates; shipping DLL exports and vendor bytes remain unchanged.

- Correct the Windows MSVC Debug configuration-defaults test harness by injecting allocation failures at throwing map insertion boundaries while preserving non-MSVC-Debug constructor coverage; retain diagnostic probes and add full Windows x64 Debug/Release portable CTest acceptance (28/28 each). No production, DLL ABI, native release or game acceptance is implied.

- Replace 58 research-era labels in first-party `src/` and `include/` comments and five diagnostic messages with technical wording; enforce a terminology regression check without modifying runtime control flow, file names or ABI identifiers.

- Normalize first-party implementation identifiers, add SPDX ownership headers, place Python before CMake and keep the CI README badge last.

- Add a CMake 3.20+ README build-system badge linked to the build configuration; avoid unverified release or test-status badges.
- Add a Python 3.9+ README prerequisites badge linked to the development instructions.
- Apply the previously approved `GPL-3.0-only` licensing policy with the complete GNU GPLv3 text in the root `LICENSE` and governance; preserve independent vendor licenses and add a README license badge.
- Correct the owner-supplied README hero PNG to exactly 1400 x 1100 pixels, update its SHA-256 in branding and source manifest, and retain the existing README image link.
- Link Windows x64 and C++20 README badges to their source documentation and use a general Windows icon.
- Remove redundant project-name declarations and branding-transition commentary from current documentation; existing independent technical contracts are unchanged.
- Integrate the owner's final campaign artwork into the README hero, with exact PNG identity pinned in the branding contract and active source manifest.
- Update repository URLs to the renamed GitHub project.

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
- See [hook lifecycle contract and provenance](docs/HOOK_ENGINE_HARDENING.md) and [fresh evidence](docs/evidence/HOOK_HARDENING.json).

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
- Adapt Brownfield local build launchers; keep publication and Golden acceptance separate.
