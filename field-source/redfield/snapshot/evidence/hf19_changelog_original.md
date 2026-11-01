# CHANGELOG — preloader_l 3.2.5 SQUALERIS HOTFIX 19

- Fixed inherited addon shell32 FreeLibrary failure: retained HMODULE/retry across shutdown and install rollback.
- Fixed staged Config2 model-policy failed release: durable retry owner and serialized candidate state.
- Reconciled active HF19 identity/metadata and current versus historical release documentation.
- Production exports, route handlers, vendor and ABI surface unchanged.
- Status: source prepared; G4 and canonical publication not asserted.

---

# CHANGELOG

## 3.2.5 SQUALERIS HOTFIX 18 — SOURCE PREPARATION, NOT_RELEASE (2026-10-18)

- Technical source uses user-approved HF18 Candidate02 FIX01; Windows retention 176/176 and CRT/Golden 18/18 previously passed on FIX01 only. No additional production C++/CMake changes in this source stage.
- Development build entrypoints remain independent of G0; standalone compile-only operator renamed to `build_unstable.cmd/.ps1` without changing its behavior.
- Added HF18 external SHA-pinned G0 path and migrated core current-source metadata to official HF17 immediate parent; G1 governance mirror/QA catalog are still blocked.
- Corrected shipping source contract to exact FIX01 `bootstrap_thread.cpp` hash; shipping source graph and negative tests retain independent contract SHA requirements.
- No official HF18 release, no CURRENT_ARTIFACT commit, no new game acceptance, no G4 source/fresh-extract 8/8.

## 3.2.5 SQUALERIS HOTFIX 17

- HF17 prepublication governance cleanup (source-only): moved unapproved root-only Windows helper into a separately packaged handoff, retained the four sanctioned orchestration launchers, registered the two scoped HF17 QA scripts as internal owned checks, and repaired repository publication-state distinction without converting HF16 inherited Windows/Golden records into HF17 PASS. Runtime semantics unchanged in this governance slice.
 — PREPUBLICATION (2026-10-17)

- Source implementation: four new CreateProcessA/W and SHGetFolderPathA/W optional compatibility MinHook wrappers added alongside the seven HF16 hooks in one post-loader transaction.
- Config2 BlockReruns and Documents policy support; bounded A/W paths; conservative fallback and lifecycle ticket ownership.
- B HKEY opaque ownership research excluded from production candidate. Actual Windows/MSVC/game/Golden and donor registration parity pending. HF16 remains published.


## preloader_l 3.2.5 SQUALERIS HOTFIX 16 — 2026-10-16

- Official user-authorized next Hotfix baseline on exact published HF15 SHA-256 `d396109c90b7054fa592f4143a9aa511c687eaff879fc1c418051236e26f6235`; internal IDS05→IDS08 stages are supporting provenance only.
- Production Asio 1.38.2 Windows IOCP transport integrated with 30/10/5s receive/frame/send timeout budgets, cancellation, reconnect, shutdown/callback drain; WinSock raw-socket compatibility removed from shipping graph while WSA lifecycle and Native/Debug test instrumentation retained.
- IDS08 source QA 8/8 and clean extract 8/8; IDS08 Windows path/locale 4/4; IDS07 real Windows IOCP send-stall `/MT`+`/MTd` and MSVC x64 Golden/Native/Debug/CRT PASS. Prior DLL SHA `5f442bc1fd740e3bac907d67142be50a50ce5dfce9120796c0e4a0760179d939` carried via binary-input parity, not recompiled HF16 claim.
- Protected 0x548 ABI, LSX wire contract, CMake/shipping build inputs and seven MinHook hooks unchanged from IDS07; all user-reported game acceptance remains smoke only, full Consumer event ACK matrix not claimed.
- Synchronized HF16 release docs, version QA and governance normative mirror from 2026-10-15.1; exact HF16 8/8 + clean extraction must be validated after packaging and recorded with sealed-package SHA externally.

## preloader_l 3.2.5 SQUALERIS HOTFIX 15

- Official HF15 cleanup publication authorized 2026-10-15. Immediate immutable parent: HOTFIX14 d9a48bed9f0ffedd897ce9839fbfac0924b8d515da81354aadb9fcb2560112ce.
- Retired 21 superseded development-stage source records from active docs/metadata after preserving exact bytes in Library research provenance. Introduced docs/ASIO_PRODUCTION.md for active IOCP transport boundaries.
- Aligned current Asio/adoption, release state, source QA ownership and Hotfix15 metadata to the real shipping backend; historical release evidence remains unchanged.
- Normalized fourteen active, numbered test/vendor/metadata/doc filenames to their semantic ownership stems; tests and dependency path references updated without renaming technical symbols or immutable archived evidence.
- Binary-relevant source, CMake, vendored dependencies, golden policies and Windows runtime ABI unchanged; compatibility/mock WinSock worker sources retained because still consumed by native acceptance and ABI tests.
- Additional orphan historic remediation-state/record JSON sources retired to immutable Library research evidence; current acceptance matrix uses HF15 authority and exact HF14 Windows carry-forward.
- Open: shipping Asio deadline policy, worker/compatibility migration, CollegeFB27/Madden27 consumer tests.

## preloader_l 3.2.5 SQUALERIS HOTFIX 14 — 2026-10-15

- Official user-authorized corrective Hotfix publication for accumulated Asio IOCP ReleaseServer lifecycle transition from the exact HF13 technical source. Includes listener/service/session management, fail-closed service retirement and retry, F02/Wave4 native acceptance correction, Golden MSWSOCK import validation and `/MT` profile-specific data-footprint policy.
- Removes legacy Process32NextW and _Mtx_trylock hook implementations and their native QA ownership; remaining MinHook topology preserved as seven hooks.
- Includes route family naming remediation, upstream pinned-build publication date governance, transparent exact-source provenance and synchronized project governance revision 2026-10-12.3.
- Windows Production Golden, Debug, Native, Static CRT tests reported PASS for runtime-identical immediately prior HF13 source. CollegeFB27/Madden27 game/network test status PENDING; official Hotfix identity does not imply Final acceptance completeness.
- No shipping source or PE ABI change during the exact HF13→HF14 designation step.

## Prior semantic and technical history

## HF13 Asio service retirement fail-closed gate — NOT_RELEASE (2026-10-14)

The exact immutable HF13 Wave4 parent (SHA-256 `605f2f666141de38b140433b402738481a5693887d3b0f8aa7b1b0f66727fdba`) passes Windows Production Golden and Debug but the Native and Static CRT `wave4_resource_retry` Generation 2 case reports exit **225**. In the host, `225 = 225 + (first_shutdown_return & 0x0F)` where documented shutdown result codes are 0..12: the first `DllShutdown` reported **0** instead of required **10 (`service_cleanup_pending`)** with the Asio listener close fault still armed.

**Source-level root:** `shutdown_backend()` correctly propagates a failed `shutdown_release_asio_windows(context)`, but the bootstrap failure owner cleanup then reaches `shutdown_published_owner_if_idle()` without checking `adapter->asio_bridge`. Its raw adapter/service retirement could continue with a still-live Asio IOCP owner. The new gate runs `shutdown_release_asio_windows(adapter)` **before** clearing event/config/store state or releasing the permanent service alias. On failed physical close or outstanding IOCP drain it returns `resource_cleanup_pending`, retaining the exact adapter and permanent owner for an outside-loader-lock `DllShutdown` retry. On successful drain, the existing bridge shutdown alone deletes the bridge and nulls the pointer; the remaining service release path proceeds unchanged.

Scope: the service retirement C++ translation unit, a targeted source regression, script ownership, and cumulative source documents; no changes to Asio upstream, WinSock compatibility, F02/ Wave4 host expectations, Golden rules, public exports, 0x548 ABI, hook set or network response semantics. The gate applies to both bootstrap-failure and normal outside-loader-lock service cleanup. This is a **source remediation candidate only**: full Windows `/MT` + `/MTd`, Wave4 callback ordering/first-stop=10/second-stop=0, native/CRT, game acceptance and 8/8 release QA require new evidence; not published.

## HF13 internal F02 Asio timer API Windows build remediation — 2026-10-14 — NOT_RELEASE

- Exact immediate parent: `HF13_ASIO_F02_ASYNC_STALL_REMEDIATION_INTERNAL_SOURCE_NOT_RELEASE.zip`, SHA-256 `2affe8d209901f08efc9b30b204571d809da88cfa734e7a43d8195b100492c7d`.
- User Windows Native and Static CRT builds failed compiling `src/runtime/asio_session_owner.cpp` line 259 in the native-only F02 timer cancellation path. Production Golden and Debug on the same parent passed.
- Pinned upstream Asio 1.38.2 `basic_waitable_timer::cancel()` has no `cancel(asio::error_code&)` overload. Change the F02-only code to call `cancel()` inside a catch-all (because `stop()` is `noexcept`), preserving the outstanding wait and transaction until callback retirement. Cancellation failure records `Status::resource_error` rather than reporting successful shutdown.
- Added a fail-closed F02 native-branch compile replay with actual pinned Asio headers under GCC and Clang; the previous `cancel(error_code)` invocation is negatively verified to fail. Test-only branch is replayed on a portable host without changing shipping or creating mock Windows SDK headers.
- `src/runtime/asio_session_owner.cpp` is the only shipping-graph C++ delta; the altered lines are guarded by `_WIN32 && PRELOADER_L_NATIVE_ACCEPTANCE` so Production binary behavior is unchanged. No changes to CMake, exports, public ABI, WinSock compatibility, MinHook, Golden, vendor tree or protocol.
- **Windows x64 Native/CRT must be rerun** to verify compilation and F02 runtime behavior; full QA and game/runtime acceptance not asserted. The immutable parent is not rewritten; this is a new NOT_RELEASE candidate.

## HF13 Asio F02 session-owned stall remediation (2026-10-14) — NOT_RELEASE

- Exact immutable parent: `HF13_ASIO_NATIVE_HEADER_COLLISION_REMEDIATION_INTERNAL_SOURCE_NOT_RELEASE.zip` SHA-256 `d5e6883b2d03ec46ef9c41e1bc3af00af9a1a7aa36d8391743a536e544ab4a89`.
- Windows MSVC Production Golden and Debug PASS; Native and Static CRT fail at `f02_delivery_stall_parallel`, exit 196 = 192 + (0x2C04 & 63). F02 sends ShowIGO but native-only stall counters originally sit only in WinSock worker_send, which Asio native sessions bypass. This is an acceptance-injection gap, not proof that normal async delivery is failing.
- Add Windows NATIVE_ACCEPTANCE-only per-Asio-session artificial async F02 stall before the first prepared write. The owning session waits with `asio::steady_timer`; other sessions can continue. No global sleeps/delivery mutex; cancellation delivers failed completion rather than a false commit. Existing F02 control API and host two-client scenario remain intact.
- Correct the native F02 host completion observation: close the second persistent LSX client after its GetConfig response, then await its real retirement while the first session remains stalled. Reconnect for the post-stall no-early-IGO-event check. An open LSX session must not be counted as completed solely because one response arrived.
- Timer cancellation delivers failed transaction completion and waits for deferred `resume_transaction()` to retire the transaction before reporting service quiescence.
- Preserve the historical WinSock injection for injected SocketApi paths. Shipping Asio behavior, public ABI, exports, vendor code and CORVEXON policy are unchanged. Native/CRT full scenario and game acceptance remain NOT_RUN for this candidate; source-only QA is not native PASS.

## HF13 Native/CRT Windows SDK header collision remediation (2026-10-14) — NOT_RELEASE

- Immutable parent: `HF13_ASIO_NATIVE_ACCEPTANCE_METRICS_INTERNAL_SOURCE_NOT_RELEASE.zip`, SHA-256 `083006e5c23d34766cd535bb9176f99e93b636c6d6034c029a594ce8ee94c52b`.
- User Windows/MSVC `/MT` Native and CRT acceptance builds failed compiling `asio_service_owner.cpp` after test-only include of `release_server_io_internal.hpp` (first C2377 SOCKET redefinition at line 25; cascading Winsock/Win32 C21xx/C27xx errors). Other Windows Golden/Debug builds reportedly PASS on same parent source.
- Smallest root fix: avoid importing the private hand-declared WinSock ABI shim into a TU using Asio/Windows SDK. Retain seven native-only metric function declarations in AsioServiceOwner guarded by `_WIN32` and `PRELOADER_L_NATIVE_ACCEPTANCE`; implementations and counters remain in `release_server_lifecycle.cpp` and authoritative declarations stay in the private legacy header for its original users.
- Add fail-closed source regression to detect reintroducing that include and to verify signature correspondence/no duplicate counter storage.
- Not a runtime or shipping behavior change. No change to Golden, Asio, CMake, vendor tree, hook inventory, ABI, or release pointers. Windows native/CRT MSVC rerun and the earlier `scenario one (42)` remain open; full 8-shard QA/Consumer game acceptance are not claimed.

## HF13 CORVEXON MSWSOCK IOCP import-policy remediation — 2026-10-14 — NOT_RELEASE

- Parent full-source ZIP is immutable, SHA-256 `1b01307d196ebac62a2b59a081b48d46f15bc9237476aa9873a7b7a9e83f50a5`.
- Windows MSVC x64 `/MT` builds compile and link `AcceptEx` and `GetAcceptExSockaddrs` from `MSWSOCK.DLL`; previously CORVEXON rejected their containing module.
- Add only `MSWSOCK.DLL` to optional import modules and constrain its **entire** import-name inventory to these two exact functions; ordinals, extras, missing names and casing drift fail closed.
- Keep current required modules, static-CRT forbiddance, ABI/export invariants, Golden read-only SHA policy and `.fptable` profile unchanged. No production C++/CMake/Asio/vendor/ABI modification.
- Added a source-backed MSWSOCK import-name regression and current policy drift checks. It can be replayed against a linked PE with `python scripts/test_corvexon_mswsock_imports.py --dll path/to/linked.dll`.
- Windows Golden and Native Acceptance remain pending rerun on this exact package. Historical Native `one` exit 42 remains a separate finding, not a PASS. This artifact is NOT_RELEASE.

## HF13 internal continuation — full legacy hook removal, pinned upstream date governance, Asio session (NOT_RELEASE, 2026-10-14)

- Cumulative from the exact **HF13 listener-ownership internal source** SHA-256 `b38afd5c34801f4dced5f3d5f9b3826216f20595f59fc42638bbf181c4fe5708` only; immutable HF12 Stage04E lineage retained.
- Physically remove the `Process32NextW` and `_Mtx_trylock` hook code and corresponding dedicated tests, status/event bindings and bootstrap install/rollback/uninstall wiring. Explicit source/QA contracts pin a 7-hook MinHook topology and preserve surviving hooks.
- Add machine-readable release-date governance for pinned Asio, PEGTL, filesystem, MinHook and TinyXML2; dates for upstream tag releases are never silently used as dates of untagged pinned commits.
- Add a standalone owned Asio session implementation and source-linked loopback test, sharing original LSX parsing/transaction mechanisms. **Production ReleaseServer still uses WinSock; Windows/IOCP and game acceptance open.**
- This is a source/remediation checkpoint, not an official Hotfix Release or canonical publication.

## HF13 internal workstream (unpublished, Asio listener ownership)

- Compiled a standalone pinned-Asio listener owner with strand-serialized
  async accept, 128-session admission, cancel/close retry and callback drain.
  The live WinSock service remains unchanged; no Asio shipping cutover is claimed.
- Added portable real loopback regression and exact source contract metadata.
- Parent HF12 Stage04E Golden/Naming source remains immutable.

# CHANGELOG
## HF12 Stage04E follow-up — naming and static CRT Golden gate (NOT_RELEASE, 2026-10-14)

- Exact parent Stage04E source ZIP SHA-256 `ad0b8b1343593330b2bca020af85206f63cd73c5f997a2e2fd2247e909aa5b17` stays immutable.
- Move previously donor-branded eight route handlers to integration/store/group-info/UTC-time owners and isolate QueryPresence header; update all CMake, registry, test and metadata active paths without route/ABI/protocol changes.
- Enforce a fail-closed 0x1800 .data raw budget only for exact static /MT seven-section MSVC .fptable PE with no dynamic CRT imports, while preserving the old 0x1000 historical /MD comparison ceiling and universal 0x400000 virtual limit.
- Run adversarial PE and filename tests; Windows Golden and native/game acceptance remain pending on the new exact source.

## 3.2.5 SQUALERIS

### HOTFIX 10 — Achievement B separate Response/Event framing and retry (2026-10-12)

- **Identity and exact parent:** `preloader_l 3.2.5 SQUALERIS HOTFIX 10` (HOTFIX_RELEASE); immediate source parent HOTFIX09 full ZIP SHA-256 `f23eb4dd9da31ce2e6973a75fff42cce20f498c2c77e98acc150d2f816790bf9`. Historical HOTFIX09 content below remains unchanged.
- **Authorized production scope (3 files):** `src/protocol/socket_exchange_transaction.cpp`, `src/runtime/event_queue.cpp`, and `include/preloader_l/runtime/event_queue.hpp`. No changes to active route inventory, ABI/export, Store/Config2 parser, Local Social handlers, Auth/Connectivity, build graph, Golden input, or resources.
- **Selected B policy:** render/encode both Response and Event into separate complete encrypted AES/hex+NUL frames before any send; Response is first. An incomplete Response rolls back Grant and prepared events; a complete Response commits Grant even when Event later fails. Event success consumes reserved FIFO nodes; partial/failed Event restores original queue ownership and retryability without losing older FIFO position. Does not establish application ACK, crash-durable event or exactly-once delivery.
- **Validation:** new source-linked GCC+Clang fault-injection 218/218 each, authoritative working/fresh-extract 8/8 source QA required before promotion. Prior HF09 Windows W02/W03 source-linked MSVC test-only success belongs to HF09, not a new HF10 shipping DLL test. HF10 Windows Production/Golden/Native/CRT/Debug/CollegeFB27/Madden27 runtime acceptance NOT_RUN; canonical pointer unchanged.

### HOTFIX 09 — QueryPresence / QueryEntitlements Anadius donor alignment (2026-10-11)

- **Official identity:** `preloader_l 3.2.5 SQUALERIS HOTFIX 09`. Exact immediate source parent `preloader_l 3.2.5 SQUALERIS HOTFIX 08` ZIP SHA-256 `197ab6bf0f95302835547c69d265c312df396da463b90ca0a128ef59bc6e9d55`; no new semantic version or codename. Old HOTFIX08 is immutable historical ancestry, not replaced or reissued.
- **Production delta — strictly two C++ files:** `src/handlers/query_presence_route.cpp` accepts the donor's first valid text or CDATA node even when followed by other XML nodes; element-first/textless unsafe cases still fail closed. `src/runtime/service_state_parser.cpp` enforces `depth==2` for Entitlements Group/Version/Type/EntitlementTag assignment, avoiding nested-field overrides. Former QueryPresence comment corrected.
- **Evidence:** QP01 donor 29-fixture study, QCE03 x86/x64 + QCE04 macOS x86_64/ARM64 static donor studies, JR01 GCC/Clang combined regression (cross-route 21/21; QueryPresence 29/29; entitlement nested 21/21; lifecycle 30/30; inherited unit 3/3 per compiler). These are research/source-level, not Windows native/game acceptance.
- **Non-delta:** RouteId/42 routes, Social scope, Auth/Connectivity, Achievement B policy choice, XML Store/Config2 authority division, MSVC production graph, public ABI and Golden chain unchanged.
- **Acceptance:** HF08 Windows/Golden/Native/CRT/Debug and user-reported games are **HISTORICAL ONLY**; changed source means HF09 requires exact-new-binary Windows and both games tests before canonical publication. No CURRENT_ARTIFACT pointer change in package preparation.

### HOTFIX 08 — 42-route Local Social + Anadius integration; governance convergence (2026-10-11)

- **Official identity:** `preloader_l 3.2.5 SQUALERIS HOTFIX 08` (Hotfix Release, single canonical identity). Exact immediate accepted source ancestor: `preloader_l 3.2.5 SQUALERIS HOTFIX 07`, ZIP SHA-256 `a21490ffe382508d75746ad27bca219d74bc3a42436d1dbc30a23e2e4bdb689f`. The full-development source parent for this documentation/QA-only governance revision is `6358cd413b5c6c8c7956f7559c2921677b39b864b80ed94312ee961aa18038f9`; no production C++/ABI/config/exports/resource/CMake behavior is changed by the governance revision.
- **Runtime:** expanded the active normal LSX route inventory to 42/42. Anadius-derived extension handlers: GoOnline, EnableVoip, SetDownloaderUtilization, SetPresenceVisibility, GetGroupInfo, SelectStore, GetWalletBalance, GetUTCTime, QueryPresence. Five newly bound Local Social routes: BlockUser, UnblockUser, RequestFriend, RemoveFriend, SendInvite. Retired GetAuthToken from active RouteId. ChallengeResponse remains handshake-only. 26 of 27 common RouteId numeric assignments changed as part of the project-approved Anadius-aligned internal ordering; equality with HF07 numeric IDs is not a project compatibility requirement. Unknown external numeric consumers remain outside proven scope, not an active defect.
- **Local Social correctness:** Config2-verified principal and explicitly registered local recipients, bounded invitee list and encoded invitation payload, committed actor snapshot for QueryFriends/GetBlockList, atomic local reserve/rollback, and prepare→encode→send→commit transaction gating; no false remote delivery/Consumer ACK, exactly-once replay or crash-atomic guarantee. Unbound LocalSocialSession preserves historical empty-list response, but stale bound sessions fail closed.
- **CRT compatibility:** legacy custom-entry comparator may contain a separately audited `.CRT` section; strict six-section ARYRA shipping Golden invariant remains unchanged. MSVC short-path handoff prevents long-path C1083 acceptance-host build failure without altering shipping build policy.
- **Acceptance:** exact pre-governance-change HF08 source ZIP `6358cd413b5c6c8c7956f7559c2921677b39b864b80ed94312ee961aa18038f9` was tested under Windows: Production/Golden, Native (including frame_coalescing), CRT transition and Debug all PASS (result ZIP `HF08_SHORTPATH_RESULTS_ba7aa0f3.zip`, SHA-256 `483a44b6d4623657b43e83bf15ee7ff2d5843ebe94fb107b679eb7b47d8823a0`). Production DLL SHA-256 `5930f0131d60d040bdf2d0c9f4e1beb421f7b8f8023dc36f9f5e3cb2f87b9d36`. User explicitly reports CollegeFB27 and Madden27 game tests PASS, but machine logs and exact tested game-build/hash bindings were not provided in this work session; record as user-reported, not independent fresh evidence. Final canonical publication remains a separate atomic Library transaction.
- **Release documentation/governance:** synchronized current identity, active metadata and QA authority to Library `PROJECT_RULES_MASTER` revision `2026-10-09.1` (normative-only mirror; debt excluded). Historical hotfix and development-stage records remain historical facts, not competing active release identities. Complete canonical display/filename grammar applies prospectively; shipping donor-facing PE VERSIONINFO is unchanged.

### HOTFIX 08 — acceptance/governance hygiene reconciliation (2026-10-11)

- Reconciled current Local Social security/delivery/integration metadata to **42 registered routes** and exact Windows Native/CRT/Production/Debug **PASS** while preserving W05/W06 pre-activation records under explicit historical provenance keys; no claim of remote backend, ACK or exactly-once semantics.
- Updated source QA and single fresh-extraction status from obsolete PENDING to **8/8 PASS**; package hash is bound by **post-seal external sidecar/receipt**, not embedded cyclically in the source archive. Game callback evidence remains **USER_REPORTED_PASS** without independent per-game machine logs.
- Corrected stale W01 documentation comment in `route_inventory.hpp` without changing any C++ token, RouteId value, executable function, CMake, or Golden policy. Anadius-aligned internal RouteId numbering remains intentional.
- Canonical publication remains **PRECOMMIT** until the exact release-identity/Library ancestry transaction is resolved. This hygiene revision does not itself promote CURRENT_ARTIFACT or rewrite historical HF07.

### Historical HF08 development notes (non-release; retained provenance)

**Historical HF08 internal development material (not release identities; superseded by HOTFIX 08):**

- **Historical HF08 delivery model, test-only:** add a compiled local social delivery failure-model to test no-byte rollback, partial-write ambiguity, inconsistent send receipts, and post-write commit failure without modifying shipping route registration, production build graph, handshake, protocol wire output, or social dispatch. Original HF07 immutable; 37 active normal routes and 5 excluded social IDs unchanged. This is not a semantic/hotfix release.

**Historical HF08 internal milestone — isolated SocialState read snapshot (not a release):**

- Add allocation-free copy-out snapshot with epoch/generation identity and fail-closed lease checks. No mutation on rejected read, including stale epoch and unknown actor.
- Add *test-only* mutual friendship fixture and direct executable coverage of both sides of RemoveFriend, pending-to-friend, local inbox, precommit visibility and rollback.
- Preserve all live 37 route bindings and exclude five social routes 38–42. No socket trust, live reader, XML writer schema, inbox ACK, or transaction activation.
- Record exact source parent W03 and original HF07. This is not a final HF08 release.

**Historical HF08 internal milestone — local state model (not a release):**

- Parent: exact cumulative W02 candidate SHA256 `4fdf4e2bc633d6905b6f4df3a006e3e60347843bf5433c59ccde26f4165c9cc1`, derived from exact selected HF07.
- Added bounded in-process LocalSocialStore model with test-only actor lease, epoch and reservation-serial replay validation, rollback and no-fail local commit.
- Added the five local-only social handler source boundaries; they fail closed with negative ErrorSuccess when no authenticated principal exists.
- Preserved governance-excluded shipping lookup: all five social route IDs 38–42 remain unregistered; 37 earlier route bindings remain active.
- Trusted runtime principal, local inbox ACK, atomic social readers, exact XML writer grammar and socket send/commit wiring remain implementation gates.
- No canonical HF07 mutation, MSVC build, native/CRT/game acceptance or HF08 release claim.

**Historical HF08 internal milestone — Anadius extension work (not a release):**

- Immediate parent: verified HF08 W01 (SHA-256 `690c19aa3ef7c698419c4748575b24020a45bd50ede6762f0f840b695a537040`); original source ancestor exact HF07 (SHA-256 `a21490ffe382508d75746ad27bca219d74bc3a42436d1dbc30a23e2e4bdb689f`).
- Implemented nine evidence-bounded Anadius handlers: GoOnline, EnableVoip, SetDownloaderUtilization, SetPresenceVisibility, GetGroupInfo, SelectStore, GetWalletBalance, GetUTCTime, QueryPresence. Unproven service-side mutation and online/auth side effects not invented.
- Dense RouteId 1–42 unchanged from W01; 37 normal routes bound, five local-only social route names remain unbound pending W03. Handshake remains separate; no GetAuthToken.
- Dedicated QueryPresence parser follows 13 attribute order and escaped direct text, rejects unsupported textless/mixed/nested instead of unsafe NULL dereference. Reader lease held through render.
- Production source graph explicitly reviewed and repinned for the two added .cpp units; native acceptance pending.



### HOTFIX07 — Remediation closure, curio metadata and operator UI (2026-10-11)


- Explicit user-authorized production/publication scope: `preloader_l 3.2.5 SQUALERIS HOTFIX 07`, exact immediate official HOTFIX06 parent SHA-256 `daa58dff566762acd9255666c0077fdbcacb777798a93cd6b31c089c867af442`. Historical HOTFIX06/FInal artifacts remain immutable; no new semantic version.
- Shipping fixes: MinHook SAFE-FREEZE rare failed-cursor-close next-HANDLE ownership (S0181) and `SemanticValue` owned-buffer alias rejection (S0291), including the end/terminator boundary previously missed. Minimal scope; no new public exports or route additions.
- QA fixes: S0196 fake Win32 API stub, S0221 registry-detour regression, S0271 PTD private-tail test, S0885 behavioral-test invocation inspection and S0887 shipping-source-graph CMake membership inspection; S0015 human release provenance correction. S0955 is a separate audit-ledger metadata concern and not silently closed by this release.
- Four interactive CMD launchers color **only** terminal PASS green / FAIL red; WARN/PARTIAL is absent, output logs and exit codes are preserved.
- User-supplied reference PE SHA-256 `a9d964beec1a82b13e3172228f9b03cbd661bcd0123eca1fca7bf7ef5fd9ba8b`: recover only MUSTAVEX-allowlisted VERSIONINFO fixed numeric fields and TimeDateStamp; Authenticode, debug directory, resource type 24, exact PE layout and signing are explicitly excluded. This input DLL is not a compiled HOTFIX07 artifact.
- Governance mirror: Library master `2026-10-09.1` normative-only; Library operational debt excluded. Changed Golden/source/MinHook surfaces invalidate inherited Windows acceptance as documented in `metadata/ACCEPTANCE_FRESHNESS_LEDGER.json`.
- **Publication is fail-closed**: fresh Windows/MSVC Production→URSYNEX→ARYRA→MUSTAVEX→CORVEXON→FALCION and affected native acceptance remain PENDING until executed. Source QA PASS alone does not assert a published HOTFIX07 shipping DLL or Consumer acceptance.


### HOTFIX06 — Approved CMD console UI refinements (2026-10-08)

Immediate official HOTFIX05 parent SHA-256 `5e6a0f980bab3b7be26e9c287f0b859a9630bb131c06e84ebd329aaf386722e9`. Four operator CMD launchers receive the explicitly approved preview design: default build-output color (no global cyan), color-coded green PASS/red FAIL only at the final result block, numeric exit status on both paths, and retained interactive pause. Extraneous output-directory and artifact-path text is removed, including the known incorrect Debug output-dir hint. Integrated QA rejects reintroduction of global cyan, stale output hints, missing results, broken exit propagation and missing pause. All shipping/runtime C++, PowerShell build scripts, CMake, exports, resources and vendored dependencies are unchanged. Acceptance is carried forward from accepted HOTFIX05 lineage.

### HOTFIX05 — Human operator CMD UI restoration (2026-10-08)
- Exact parent is published HOTFIX04 source ZIP SHA-256 `f2ebac88e7c4cebaafdd52adb72fb9a6a61cffbaf202595d5966afd6135aefda`.
- Restored the historical four-launcher banner, per-operation explanation, visible numerical exit code, real output locations, PASS/FAIL display, and explicit pause.
- Added a negative-tested fail-closed UI regression gate to authoritative 8-shard QA. No runtime C++/shipping build PowerShell changes.
- HF04 and previous release artifacts remain immutable.


### HOTFIX04 — Official bounded dependency/governance correction (2026-10-08)
- Exact cumulative source parent is HF03 source-state SHA-256 `7a41fc853a6659ad0acfd7edb5653071229af492ceccccc4179cb188db8c3392`; HF01–HF03 runtime acceptance was explicitly confirmed by the user for this release decision.
- Four Windows human-operator build launchers directly invoke their matching PowerShell scripts and preserve exit codes and PASS/FAIL visibility.
- TinyXML2 v11.0.0 imported as SHA-pinned TEST/REFERENCE_ONLY; MinHook version unchanged; F14 security regression retained.
- Project-owned stale identity, metadata ownership, current governance mirror revision 2026-10-08.1, dependency/provenance and release metadata corrected.
- Shipping runtime behavior and public exports unchanged from HF03; native/game acceptance uses user-attested cumulative lineage, not a newly manufactured HOTFIX04 execution result.
- Historical 3.2.5 FINAL artifact remains preserved; HOTFIX04 has distinct official release identity and external canonical publication pointer.


### Audit3 remediation
- Removed the dead compiled private release-service `runtime_source` adapter while preserving the live store materialization sources.
- Bound `docs/RELEASE_ACCEPTANCE.md` hash roles and status claims to `metadata/RELEASE_ACCEPTANCE.json` in executable source QA.
- Golden PE tooling now rejects fixed fields/data directories that fall outside the declared `SizeOfOptionalHeader`, with negative fixtures for URSYNEX, ARYRA, MUSTAVEX and CORVEXON.
- Corrected the Donor1 cross-architecture route inventory from 35 to 39 as an evidence/provenance-only correction; social-route shipping authorization remains unchanged.
- **Final artifact governance/license convergence:** the root project license is now the complete verbatim GNU GPL v3 (`GPL-3.0-only`) text; bundled third-party license material remains unchanged.
- **Governance mirror:** synchronized to Library `PROJECT_RULES_MASTER` revision **2026-10-07.8** using normative rules only; Library-only debt/backlog bookkeeping is excluded from packaged governance.
- **Build entrypoint naming:** active one-click Windows launchers are `build_production.cmd`, `build_native_acceptance.cmd`, `build_crt_acceptance.cmd`, and `build_debug.cmd` under `scripts/build_orchestration/`; historical uppercase launcher names remain only in semantic history/provenance.
- **Release identity immutability:** artifact maintenance remains the same `3.2.5 SQUALERIS` `FINAL_RELEASE`; correction/repack terminology is provenance-only and not part of the release name/type.
- **QA hardening:** release/repository QA now fails closed on incomplete project license text, stale governance revision/debt mirroring, stale current-state ownership, uppercase active launcher names, and repack-style release identity modifiers.

### Final release identity and acceptance
- **3.2.5 SQUALERIS** is the current **FINAL RELEASE**, explicitly published by user command on 2026-10-07 from the exact cumulative HF13 Correction1 parent SHA-256 `2ae20e96f2f7c0b7256b7e122ac32953c15c65532b82a04989815087f8f5ac71`.
- Windows Production Build completed the canonical `MSVC Build -> URSYNEX -> ARYRA -> MUSTAVEX -> CORVEXON -> FALCION` chain; accepted shipping DLL SHA-256 is `00623382ebedd6a91eb895d548368847fbfbb7f67e84f2161bdbe7f7e74852d3`.
- Native Acceptance passed on test DLL SHA-256 `e75f089e58c14063ffc822d6804c46fa21b7f44e0bcb38442f5ebf6b154497de` with host SHA-256 `9c672be74a1a929a2506bcef9d2d940f4605a9076ba279bc39f8cba006bbd96e`; CRT Transition Acceptance passed with compatibility artifact SHA-256 `54ffcb6a2ee97e4a2f8d2f1dc37b3954184989da026e65059cea0cea9a545f6c` and the same normalized shipping DLL; Debug Build passed with DLL SHA-256 `9711e015df43978bfc99c345ef34d5fce20bb82eb525bc2861b82e758bb00b0a`.
- CollegeFB27 and Madden27 normal game/network acceptance are user-confirmed PASS on the validated runtime lineage. Source/repository QA is separately authoritative at exactly 8 deterministic shards and is rerun on the final package/fresh extract.

### Audit2 remediation convergence carried into SQUALERIS
- **Build/link truthfulness — S0083/S0095/S0096:** linker-policy evaluation consumes only the selected link invocation and referenced response closure, rejects every effective incremental-link form except `/INCREMENTAL:NO`, and executable build wrappers preserve real nonzero child exit codes without downstream continuation.
- **Release publication I/O and identity — S0104/S0116/S0120:** finalizer publication uses same-directory temporary output plus atomic replacement; the validated candidate identity is preserved across Golden gates/publication; CLI/policy input errors produce controlled fail-closed outcomes rather than uncontrolled traceback/partial publication.
- **Parser/package integrity — S0108/S0127/S0135:** PE resource traversal is bounded for cycles/depth/entry/range work, nested archive/research payloads are rejected from release packages, and package identity is bound to the verified active-source manifest.
- **CRT matrix — S0081:** shipping MinHook remains normalized `/MD`, while the debug prototype uses its separate `/MDd` MinHook target so incompatible CRT families are not mixed.
- **Bootstrap and batch atomicity — S0160/S0166/S0191:** pre-runner failures publish through the common bootstrap failure state; stop requests are checked before/among heavy runtime acquisitions; MinHook batch validation completes before any gate-state mutation.
- **MinHook peer-thread ownership — S0181/S0183/S0195:** peer suspend/context outcome is explicit, failed `ResumeThread`/`CloseHandle` obligations retain ownership for retry, and successful MinHook API return is not treated as proof that every peer operation succeeded.
- **Service/native unload ownership — S0235/S0241:** failed service-block physical release retains a durable retry owner, and acceptance-host DLL unload is permitted only after successful explicit shutdown/teardown postconditions.
- **Native configuration transaction — S0329/S0352/S0366:** nonempty Windows string-conversion sizing failure is distinct from empty success; config open errors preserve `GetLastError` classification; fallible clear stages/detaches old policy ownership and commits all logical domains/generation atomically.
- **Portable parser/test correctness — S0371/S0381/S0446/S0689:** stale unit fixtures were corrected and made authoritative; full XML source validity is checked beyond the first semantic mismatch; socket endpoint tests reset listener state before intended assertions; store depth counts retained TinyXML2 document/OPEN ancestry correctly.
- **Native session/oracle correctness — S0421/S0439/S0471:** handshake acceptance proves complete frames and an independent established-session request/response outcome; receive nonprogress has a finite monotonic budget; achievement acceptance identifies separate primary and Event extents with order/cardinality/semantic-ID checks.
- **Debug state/concurrency — S0470/S0718:** recoverable capacity failure preserves the complete pending input and does not count required bytes as delivered bytes; reused trace-ring slot payload publication/snapshot copying is synchronized as one ownership unit.
- **Transient native owner retention — S0489/S0518:** temporary response/render allocations and SemanticValue scratch allocations transfer failed physical frees into durable intrusive pending owners; retry performs only raw release and never repeats dispatch/render/parser/callback/commit side effects.
- **HF13 Correction1:** the production PowerShell candidate-identity message uses safe `${Label}` interpolation; native shutdown stress rejects the `0xffffffff` metric-failure sentinel and independently bounds started-worker versus live-owner readiness, keeping real owner loss visible instead of masking it as success.

### Governance, documentation and repository convergence
- The packaged governance mirror is synchronized byte-for-byte from Library `PROJECT_RULES_MASTER` revision **2026-10-07.4** at `docs/PROJECT_GOVERNANCE_CONTRACT.md` and `metadata/PROJECT_GOVERNANCE_INVARIANTS.json`; `metadata/GOVERNANCE_BINDING.json` records the exact master hashes.
- Current release documentation is deliberately small and live-only. `docs/history/`, `docs/historical/`, `docs/research/`, and the old `docs/governance/` wrapper are absent from the release source tree.
- Exact removed documentation/research/history bytes are retained outside the release source in immutable Library provenance archive `SQUALERIS_3.2.5_DOCUMENTATION_MIGRATION_SOURCE.zip`; `metadata/DOCUMENTATION_HISTORY_MIGRATION.json` accounts for every migrated historical file/heading/substantive record and binds the archive SHA/Library ID.
- `CHANGELOG.md` is now the canonical in-source semantic history. The previously under-described 3.2.3 MELOTRIX section is exhaustively backfilled for hook architecture, lifecycle/ownership, pre-audit remediation, QA/repository changes and Windows/native/CRT/game acceptance history.
- Project-owned build orchestration lives under `scripts/build_orchestration/`; the superseded `scripts/build/` path is absent. Script bytes are preserved across the directory migration and active references/checkers use the new path.
- Root `LICENSE` remains the canonical complete GNU GPL v3 (`GPL-3.0-only`) text; vendored third-party licenses remain separately preserved.

## 3.2.4 RANQUEL

### HOTFIX01 — governance and release-history convergence
- Derives from the exact user-accepted 3.2.4 RANQUEL Final parent SHA-256 `b7f041048d77ccf35e515cf5f7a9ffa3c5735d461a38524bea2fc7ea5ded08b7`.
- Synchronizes the packaged governance mirror byte-for-byte from Library `PROJECT_RULES_MASTER` revision `2026-10-06.3` and binds the master MD/JSON SHA-256 values in `metadata/GOVERNANCE_BINDING.json`.
- Reclassifies current release truth under the four-class governance taxonomy as **HOTFIX RELEASE 01**; obsolete `FULL SOURCE`, `FULL HISTORY`, `SOURCE CANDIDATE`, and named-Wave concepts are not current release classes.
- Consolidates current release history by semantic identity while preserving the complete pre-HOTFIX01 CHANGELOG and parent current-truth snapshot under `docs/history/3.2.4_RANQUEL/FINAL_PARENT_EXCEPTION/`.
- Removes named-Wave/current-wave bookkeeping from active `RELEASE_STATE.json`; exact historical lineage remains in the preserved parent snapshot and provenance surfaces.
- Updates release/repository/FALCION QA so stale governance mirrors or obsolete current release-class terminology fail closed.
- **Runtime boundary:** documentation, governance, metadata and QA-policy correction only. Production `src/`, public export ABI, protocol behavior, MinHook membership and build targets are unchanged from the accepted 3.2.4 Final parent.

### Final Release baseline
- 3.2.4 RANQUEL was explicitly accepted by the user as a one-time **FINAL RELEASE** despite incomplete exact Windows/MSVC Production, Native Acceptance, CRT Transition, CollegeFB27 and Madden27 execution for that package.
- That exception is release-specific and is not part of standing Library governance. Future releases remain governed by the current Library master.
- The Final parent inherited the Audit 01 remediation closure, Store FeatureDesc/FeatureFlag interleaving correction, repository-wide naming/reference closure, and the 9/9 MinHook SAFE-FREEZE topology.

## 3.2.3 MELOTRIX

### Release identity and semantic boundary
- 3.2.3 MELOTRIX was promoted as the stable semantic successor to 3.2.2 MUSTORIN. Working RC, Wave, candidate and correction labels from its development are provenance only and are not separate semantic releases.
- The release preserved the established MUSTORIN protocol/route/config/XML/Store/socket/session/public-export behavior except for explicitly bounded pre-audit remediations. Public shipping exports remained `DllMain @1`, `SURGE @2`, `DllShutdown @3`.

### MinHook-only SAFE-FREEZE architecture
- MinHook became the sole production inline-detour mechanism for **9/9 hooks**; the legacy project-owned inline/IAT/VEH production hook engine was removed from the active production graph.
- `DllMain` only schedules bootstrap work. Executable hook apply/freeze occurs post-loader, outside loader lock.
- All nine hooks are created/queued before one global `MH_ApplyQueued` transaction. The transaction is classified as committed, rolled back, or rollback-incomplete; rollback-incomplete is never treated as initialization success and potentially-live trampoline ownership is retained for later teardown retry.
- SAFE-FREEZE replaced Toolhelp thread-snapshot enumeration with dynamically resolved `NtGetNextThread` stable-handle enumeration while retaining MinHook suspend/context/instruction-pointer relocation/resume semantics.
- PTD initialization failure disables/rolls back the entire active hook surface before removal. Normal shutdown performs one global disable transaction, drains call gates, removes subsystem tickets and only then uninitializes MinHook.
- Upstream MinHook commit `8af6b4acae5a9388fd742b56fa79ece89d96f823` is physically vendored and manifest-bound; configure-time network/external-source fallback and upstream MinHook CMake execution are forbidden. SAFE-FREEZE is applied only to a build-tree copy.

### Pre-audit remediation and ownership/lifecycle convergence
- The cumulative pre-audit line retained the earlier failure-atomicity work and closed F11 QA/evidence truthfulness, F14 TinyXML2 test/reference/security provenance, arena decode lifetime, exact model-XML release-size ownership, and transactional generic event drain.
- Retryable shutdown was corrected so client quiescence stops/joins workers without prematurely destroying jthread/HANDLE ownership; physical retirement remains in the fail-propagating retirement preflight so one-shot close failure produces `service_cleanup_pending` and retains exact retry ownership.
- Achievement coherence reused the existing GrantAchievement delivery gate for measure/preview and QueryAchievements reads, closing in-flight Grant-to-query and competing-Grant coherence without adding a second synchronization primitive or changing protocol/XML/FIFO/session topology.
- Store FeatureDesc and FeatureFlag storage was separated so per-type encounter order survives arbitrary interleaving. Audit 01 remediation also tightened complete production-target source membership, acceptance-freshness governance clauses, script owner/purpose registration, architecture-review registration, MinHook standalone syntax closure and build-selector reset protection.
- MinHook vendoring/recovery exports only the pinned tracked Git tree and normalizes ordinary Windows separators deterministically.

### Repository, naming and QA architecture
- The project-wide filename/path migration applied exactly **288 approved project-owned path decisions** with CMake/include/QA/docs/metadata rebinding and no authorized symbol/type/function semantic rename. Historical release/evidence identities remained immutable.
- A follow-up naming/reference correction rebound the four build launchers, optional debug-host CMake source template, current documentation references and fail-closed filename QA; no new runtime finding was imported by that correction.
- Authoritative source validation transitioned to exactly **8 deterministic shards**. Both the frozen working tree and exact fresh-extract release package passed 8/8; historical 4/4 records remained immutable historical evidence.

### Acceptance-oracle convergence and Windows closure
- Native acceptance was aligned with shipping lifecycle: duplicated per-hook teardown logic was removed; controlled teardown delegates to release shutdown; two-generation shutdown/unload/reload behavior became the acceptance contract.
- The acceptance host reuses production session/message codecs and performs `Challenge -> ChallengeResponse -> ChallengeAccepted -> encrypted request/response` sequencing; required ADVAPI32 IAT imports are retained so fallback-restore tests cannot be dead-stripped.
- Stale acceptance expectations were corrected without production runtime semantic changes: GetVoipStatus envelope handling, QueryAchievements `<AchievementSets>` recognition, and GrantAchievement completion Event validation in the same decoded compound frame.
- Accepted Windows source snapshot SHA-256 `d642eff468e4193112d0a2e41a8cea6cff0b572cee4dc0d0f0bb83df725e828a` passed Native Acceptance. Native test DLL SHA-256 was `65497e04709184a221cbd245e28cc007881df24b9e8beaa65fc532d70f154fe5` before/after; host SHA-256 was `09e27c5051f5187978550b216c42eeb13dbb8c67e2933031ddbba81152797a28`.
- CRT Transition Acceptance passed on the same source snapshot with compatibility custom-entry SHA-256 `cb7cd5ccc19e9749d7d966342d6eb449d0e1fe47cac8c0062baba588e763ff5d` and normalized `/MD + /GS + default _DllMainCRTStartup` shipping DLL SHA-256 `d229c4289b835428d4d345faf5fae6ad9b93da59313b6db43020455b64ac2121`; shipping link policy, shipping-blackbox, native CRT scenarios and URSYNEX/ARYRA/MUSTAVEX/CORVEXON gates passed.
- Production/Debug and normal CollegeFB27/Madden27 Windows/game operation were user-confirmed on the runtime-equivalent stabilized lineage; final MELOTRIX promotion changed documentation/metadata/source-QA release-state surfaces only and did not change production runtime source/build/Native-CRT harness inputs.

### Historical limitations and non-goals
- Intermediate Wave/RC/candidate names represented cumulative development provenance, not additional releases. Their exact hashes/logs remain immutable external provenance and are not reproduced in the current release tree.
- MELOTRIX did not claim donor-exact PE layout, new public ABI, or broad protocol redesign; its central architectural change was the MinHook-only post-loader SAFE-FREEZE transaction plus the bounded remediation/acceptance convergence above.

## 3.2.2 MUSTORIN
### Added
- Establishes **3.2.2 MUSTORIN** as the sole current semantic-release identity, with immutable 3.2.1 CERVRAX as semantic/source parent.
- Adds a source-candidate versus released-state boundary requiring external exact-hash acceptance attestation before promotion.
- Adds generation-pinned Store readers and generation-consistent runtime configuration/behavioral publication for the third remediation wave.
### Changed
- Cumulatively applies the fixed **25 + 14 + 9** remediation partition: Infra/Release/Golden/Acceptance, Ownership/Lifecycle/Runtime-Safety, then Protocol/Runtime-Semantics.
- Hardens environment state preservation, command-line quoted-token parsing, XML special-node framing, malformed-frame/ciphertext isolation, debug socket ownership and entitlement-filter prevalidation.
- Preserves Store borrowed-view lifetime across generation replacement and fails config/behavioral replacement closed when an active reader prevents an atomic generation publication.
- Retains the first- and second-wave build, Golden, ownership and teardown hardening unchanged.
- Repairs the `iat_fallback_restore` acceptance-host IAT-retention surface by forcing all five required ADVAPI32 registry imports to remain linked; fresh Windows execution remains required.
- Removes the independent CRT release-host `/Od` override so the Release host uses `/O2` and `/DEBUG:NONE`.
### Preserved
- Public DLL ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Compatibility-route inventory remains unchanged; social mutation routes remain evidence-blocked and unregistered.
- Consumer + Donor1 remains the effective behavioral authority.
- Authoritative source/release QA uses exactly **8 deterministic shards**; historical 4/4 records remain historical evidence.
### Validation
- Current working-tree release QA: **8/8 deterministic shards PASS**, behavioral **92-case inventory via 8/8**, unit syntax **207/207 PASS** plus Surge VM runtime.
- Production graph **136/136 PASS**, portable syntax **129/129 PASS**, shipping build policy **55/55 PASS**, MSVC link-policy self-test **14/14 PASS**, native worker/config source surfaces **31/31 + 14/14 PASS**.
- Source-package fresh-extract release QA: **8/8 deterministic shards PASS** after metadata/manifest convergence.
- Final promotion requires fresh Windows/MSVC production build, `URSYNEX -> ARYRA -> MUSTAVEX -> CORVEXON -> FALCION`, Native Acceptance, CRT Transition Acceptance, game/network acceptance and exact rebuilt DLL SHA binding.
## 3.2.1 CERVRAX
### Added
- Establishes **3.2.1 CERVRAX** as the sole current semantic-release identity, with 3.2.0 SCIURONYX as the immutable semantic/source parent.
- Adds the evidence-backed `GetBlockList` compatibility route. Its bounded observable contract is `GetBlockListResponse Return="Success"` with zero `User` children, no block-store allocation, no mutation, no event publication and no connectivity gate.
- Adds an active QueryChunkStatus authority-precedence record without rewriting historical evidence archives.
### Changed
- Makes ShowIGO/ShowIGOWindow event delivery failure-atomic on the shipping socket path: the `IGOEvent State="DOWN"` candidate is staged with the response, committed only after successful `send_all()`, and rolled back on allocation/encode/send failure while preserving any older FIFO head. Direct-dispatch behavior remains unchanged.
- Extends the separate compatibility inventory from five to six entries while preserving the 22-entry historical recovered inventory unchanged.
### Fixed
- Corrects the active research interpretation of the inherited OriginSDK `QueryChunkStatus -> QueryContentResponse` donor attribution: the captured range crossed the terminal handler boundary at `0x180030EB0` into the separate `QueryContent` function at `0x180030EE0`. Runtime behavior remains the established `no_response_required` contract.
### Preserved
- `ConsumeEntitlement`, `RequestLicense`, and social mutation routes remain evidence-gated and unregistered.
- Public DLL ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Authoritative source/release QA uses exactly **8 deterministic shards**; historical 4/4 records remain historical evidence. Fresh Windows/MSVC/native/final-binary/game-network acceptance is required for the 3.2.1 runtime delta.
## 3.2.0 SCIURONYX
### Added
- Establishes **3.2.0 SCIURONYX** as the sole current semantic-release identity.
- Adds a fail-closed Documentation Preservation Contract that distinguishes semantic release identity from working provenance identity.
- Adds machine-readable preservation and migration ledgers so documentation loss or destructive consolidation fails release QA.
- Adds the source-packaged Project Governance Contract and binds its critical release, evidence, validation, repository, terminology, and immutability rules into fail-closed repository QA.
- Adds a typed Rust-derived `OriginError` taxonomy with success/pending classification, area/level/local-code decoding, stable diagnostic names, and a separate route-local `ErrorSuccess` description-contract layer.
- Adds a five-entry evidence-backed compatibility inventory for `GetPresenceVisibility`, `AreChunksInstalled`, `RestartGame`, `GetVoipStatus`, and `QueryChunkStatus` while preserving the 22-entry Primary-exact inventory unchanged.
- Adds a bounded social-route error-contract research record for `RequestFriend`, `RemoveFriend`, `BlockUser`, `UnblockUser`, and `SendInvite`; the five routes remain evidence-blocked and intentionally absent from shipping registration because no Tier-1 server-side result/mutation contract is available.
- Adds the exact eight-shard release-QA governance contract and fail-closed repository enforcement for release QA, behavioral QA, BUILD_ME_FIRST, and fresh-extract validation.
### Changed
- Consolidates prior remediation work at the semantic-release level without retaining intermediate working identities as CHANGELOG release/subrelease headings.
- Normalizes the release root and repository ownership model: canonical root documents remain at the root; build orchestration, metadata and governance documents remain in their owned directories.
- Preserves the Debug-only `preloader_l_debug` private include topology for `src/debug/platform`; the production target remains unchanged.
- Synchronizes the Project Governance Contract with the current explicit-route/error-convergence rules: accidental silence/timeout is not semantic equivalence, route recognition and intentional no-response are observable contract dimensions, and Rust numeric error meaning is separated from route-specific wire-description authority.
- Replaces the timeout-prone monolithic/four-shard current release-QA path with an authoritative **8/8 deterministic shard** contract. No-shard and non-8 invocations fail closed; historical 4/4 records remain preserved as historical evidence.
### Fixed
- **Presence/friends route convergence:** removes the route-local Online/offline error substitutions from `SetPresence` and `QueryFriends`. `SetPresence` now always uses the Donor1-aligned `ErrorSuccess Code="0" Description=""` contract. `QueryFriends` now always returns `QueryFriendsResponse Return="Success"` with zero `Friend` children, matching Donor1 exactly; no friend store, synthetic friend payload, presence event, RTM/network side effect, or unrelated route change is introduced.
- **Current bounded runtime remediation:** removes the route-local Online/offline substitution from `ShowIGO` and its `ShowIGOWindow` alias. Both now return route-local `Success(0)` and publish exactly one `IGOEvent State="DOWN"` after a real successful render, independent of the project Online setting. Measure/size-query passes remain side-effect free; global Online policy, `StartDownload`, `SetPresence`, public ABI, and unrelated routes are unchanged. Effective behavioral authority is Consumer + Donor1; Rust SDK/Maxima are supporting oracles and the historical `version.dll + winmm.dll` line is supplementary only.
- Adds `ShowIGOWindow` as a compatibility route-name alias to the existing `ShowIGO` / `RouteId::show_igo` path. The 22-entry Primary-exact inventory is unchanged; no rich ShowIGOWindow payload parsing or separate handler semantics are introduced.
- Implements `GetPresenceVisibility` as an explicit compatibility route returning `Visible="false"`, and `AreChunksInstalled` as the closed valid-football response/ChunkIds-echo contract.
- Implements `RestartGame` as route-local `ErrorSuccess Pending(1)` with the exact shutdown-wait description, and `GetVoipStatus` as route-local `ErrorSuccess NotImplemented(-1593835518)` with the exact not-implemented description.
- Implements `QueryChunkStatus` as a recognized first-class intentional-no-response route that writes zero response bytes and keeps the persistent session alive, rather than relying on accidental unknown-route behavior.
- Splits `StartDownload` from the generic online/offline ErrorSuccess helper: it now returns route-specific `Success(0)` with `Requested a download of a DLC offer.` independent of Online.
- GetAuthCode request remediation: requires the `@ClientId` XML attribute, accepts a present-empty value, and keeps ClientId out of Auth8 entropy/producer and network/backend policy; the existing offline AuthCode error remains unchanged.
- Offline route closure: removes the former offline-only error special cases from `GrantAchievement`, `QueryAchievements`, and `QueryOffers`, so these routes follow the same request/state/serialization semantics regardless of the Online flag.
- Corrects the Windows shipping black-box acceptance host protocol sequence: the host now consumes the mandatory server Challenge greeting, performs ChallengeResponse/ChallengeAccepted session establishment, sends an encoded GetConfig request, decodes the response, and validates `GetConfigResponse`.
- Reuses the project session/message codec implementation in the acceptance-host target instead of introducing a second crypto/protocol implementation.
- FNET-001 diagnostic evidence clears the prior CRT/loader/socket-readiness suspicion: listener, acceptor, worker and protocol entry progressed, while the previous host misclassified the initial Challenge greeting and closed the peer. The acceptance-host correction itself does not add a production runtime change.
### Preserved
- Detailed technical change records from prior remediation work remain in CHANGELOG beneath their owning semantic releases.
- Exact working identities, parent hashes and chronology remain available only in provenance/evidence/history and do not act as release identities.
- Historical semantic-release sections remain part of the authoritative CHANGELOG record.
### Validation status
- The last fully execution-accepted ancestor predates the shipping route-alias change. Subsequent source changes include shipping route acceptance and the prior compatibility/error-contract additions, so fresh Windows/MSVC/native/final-binary/game-network execution remains required. The current source carries the bounded ShowIGO/ShowIGOWindow runtime remediation described above while retaining the prior QA/governance improvements. Working-tree source QA passes **8/8 deterministic shards**, behavioral **91-case inventory via 8/8 shards**, unit syntax **201/201** plus Surge VM runtime, production graph **136/136**, portable syntax **129/129**, shipping build policy **42/42**, and active manifest **631/631**; the exact frozen package passes the same **8/8** after fresh extraction.
## 3.1.9 CASTORADRON
### Consolidated changes
#### Audit01 finding closure
- Closes all six findings from 3.1.9 CASTORADRON Audit01: `F319-P006-001`, `F319-P031-001`, `F319-P069-001`, `F319-P078-001`, `F319-P085-001`, and `F319-P111-001`.
- Restores `$ErrorActionPreference = "Stop"` on all five audited PowerShell build/acceptance entry surfaces and adds protected platform-identifier handling plus fail-closed regression coverage.
- Applies UTF-8/XML-character validation to raw `SemanticValue` aliases before zero-copy publication; malformed entity-free values cannot enter handshake/session or route semantics.
- Removes the same-hook recursive-entry drain deadlock by keeping draining-state hook calls counted until direct-target publication and quiescent retirement.
- Synchronizes `test_entry_flow.cpp`, `test_session_consistency.cpp`, `test_runtime_configuration_routes.cpp`, and `test_settings_query.cpp` with current runtime/config/settings contracts and gives them executable release-QA ownership.
- Corrects Store surrogate numeric-entity taxonomy to `invalid_xml` and permanently verifies committed-snapshot preservation.
- Working-tree source QA: 4/4 deterministic shards PASS; behavioral 88/88; active manifest 596/596. Public ABI remains `DllMain @1 / SURGE @2 / DllShutdown @3`. Windows/CRT/native/game/final-binary strict evidence is parent evidence; requires a fresh binary acceptance rerun after rebuild.
#### synchronization/ownership boundary closure
- Makes inline-hook page-trap/protection state TU-local and moves allocator owner-count bookkeeping from the trap lock to the existing allocator lifecycle lock, eliminating cross-domain lock borrowing without adding a new synchronization primitive.
- Makes release-server stop/acceptance atomic storage lifecycle-TU private; worker and acceptor paths use narrow intent operations.
- Removes broad mutable `HostBridgeState*` and behavioral `Store&` cross-TU accessors. Host wrappers dispatch through owner-TU operations; active behavioral state uses narrow owner operations while caller-owned candidate Store contexts remain explicit.
- Quiescence-gate borrows are retained intentionally because physical gate identity is part of wrapper admission/restore/drain correctness. Hook mechanism and F069 restore-then-retire ordering are unchanged.
- Focused validation: changed Windows TUs Werror syntax PASS; behavioral 87/87 via 4/4 shards; unit syntax 198/198 plus Surge VM runtime PASS; production graph 136/136; portable syntax 129/129; hardcap/model/native/F1420/F1425/shipping gates PASS. Final release/source QA is 4/4 deterministic shards PASS; fresh-extract candidate QA is 4/4 deterministic shards PASS.
#### residual cross-TU ownership closure
- Removes the remaining non-synchronization writable storage exports from `runtime_configuration_internal.hpp`, `store_entitlement_internal.hpp`, and `behavioral_service_state_internal.hpp`.
- Runtime-configuration state/current-directory mirror is private to `runtime_configuration.cpp`; runtime-store slots/active generation/status are private to `store_entitlement_runtime.cpp` with only a narrow status-update operation exposed to the model bridge; behavioral Store/locks are private to the storage TU with a borrowed Store accessor.
- No public API/ABI, parser grammar, transaction, allocator, locking, teardown or lifecycle policy change is intended. Existing inline-hook backend synchronization externs and release-server stop/acceptance atomics are retained deliberately as process-wide subsystem state.
- Repository policy fails closed if raw mutable storage returns to any of the three closed private contracts. Focused validation: behavioral 87/87 via 4/4 shards; unit syntax 198/198 plus Surge VM runtime PASS; production graph 136/136; portable syntax 129/129; hardcap 6/6; model/native semantic gates PASS; F1420/F1425 executable gates PASS; shipping policy 38/38. Final release/source QA is 4/4 deterministic shards PASS; fresh-extract candidate QA is 4/4 deterministic shards PASS.
#### Surge native-hook ownership encapsulation
- Removes raw cross-TU writable Surge hook storage from `surge_native_hooks_internal.hpp`. Wrapper original/policy state, inline ticket/quiescence state, IAT ledger state and façade counters each remain private to their owning translation unit.
- Cross-TU cooperation now uses narrow operations/accessors for original publication/query, quiescence-gate borrowing, inline installed-state queries, mutex-policy retirement and install-failure accounting.
- IAT recovery semantics, inline-hook rollback/remove ordering, wrapper leases, policy behavior and public API/ABI are unchanged by intent. Repository/behavioral QA fail closed on reintroduction of raw ownership symbols.
- Focused validation: Windows-target Werror syntax PASS for all four Surge hook TUs; behavioral 87/87 via 4/4 shards; production graph PASS; portable syntax 129/129; unit syntax 198/198 plus Surge VM runtime PASS; hardcap 6/6; model/native semantic gates PASS; shipping policy 38/38. Final release/source QA: 4/4 deterministic shards PASS on the metadata/manifest tree; fresh-extract candidate QA: 4/4 deterministic shards PASS.
#### Windows host-bridge ownership encapsulation
- Replaces raw cross-TU writable host-bridge globals with two explicit owner objects: host runtime/path/original storage in `host_bridge_windows.cpp`, and registry hook tickets/originals/inline tickets/quiescence/recovery/mode state in `host_bridge_hooks_windows.cpp`.
- `host_bridge_windows_internal.hpp` exposes operations/borrowed accessors rather than writable storage; wrapper TUs acquire quiescence gates and bridge state through bounded accessors, and replacement addresses are immutable function-local storage.
- Public host-bridge API/ABI, registry inline/IAT fallback policy, rollback/recovery semantics, original publication, and teardown ordering are unchanged by intent. Repository QA fails closed on reintroduction of raw cross-TU host-bridge storage.
- Focused validation: Windows-target Werror syntax PASS for all three host-bridge TUs; F1420 registry teardown 9/9 PASS; environment restore 9/9 PASS; behavioral 87/87 via 4/4 shards; production graph PASS; portable syntax 129/129; unit syntax 198/198; hardcap 6/6; model/native gates PASS; FALCION root=16/scripts=42 PASS. Final release/source QA: 4/4 deterministic shards PASS on the metadata/manifest tree; active manifest 596/596 PASS. Fresh-extract candidate QA: 4/4 deterministic shards PASS; final packaging bytes are rechecked after metadata closure.
3.1.9 is the semantic consolidation of the complete 3.1.8 CIVETAVEX remediation line. Intermediate working labels are provenance only and are not part of the 3.1.9 release identity.
#### explicit store parse ownership hardening
- Removes the cross-TU mutable `g_parse_store` parser context and `parse_store` accessor.
- Threads explicit `StoreState&` ownership through store document parsing, value/entitlement decoding, arena/reset helpers, runtime candidate staging and model-service staging.
- Public store API/ABI and serialized behavior are unchanged by intent; fail-closed repository/behavioral guards reject a return to implicit parse-store ownership.
- Validation: release/source QA 4/4 deterministic shards PASS; fresh-extract final candidate QA 4/4 shards PASS; behavioral 87/87 via 4/4 shards; unit syntax 198/198; production graph 136/136; portable syntax 129/129; hardcap 6/6; model/native gates PASS; shipping build policy 38/38; active manifest 596/596; FALCION root=16/scripts=42 PASS.
- Cumulative Windows Production/Debug/Native Acceptance/CRT/game validation remains pending.
#### achievement-route structural decomposition
- Split the former `src/handlers/achievement_routes.cpp` into a thin public façade, `achievement_grant_route.cpp`, `achievement_query_route.cpp`, and private `achievement_routes_internal.hpp`.
- GrantAchievement transactional mutation/deferred-delivery logic and QueryAchievements read/serialization policy remain behavior-preserving and independently owned.
- Public ABI/API is unchanged; repository/behavioral guards fail closed on responsibility collapse.
- Validation: release/source QA 4/4 deterministic shards PASS; behavioral 87/87 via 4/4 shards; unit syntax 198/198; production graph 136/136; portable syntax 129/129; hardcap/model/native gates PASS; shipping build policy 38/38; active manifest 596/596; FALCION root=16/scripts=42 PASS.
- Fresh-extract candidate validation: **4/4 deterministic release-QA shards PASS**.
- Cumulative Windows Production/Debug/Native Acceptance/CRT/game validation remains pending.
#### XML request-parser structural decomposition
- Splits the former 460-line `src/xml/request.cpp` into request-envelope/public-policy façade, bounded tag/attribute/close scanner, and direct-child traversal modules behind `request_internal.hpp`.
- Existing LSX frame cardinality, duplicate-attribute, TinyXML-compatible name/attribute, document-depth and direct-child semantics remain unchanged by intent; public XML request API/ABI is unchanged.
- F1425 executable/document-depth gates and behavioral link sets now consume the complete request source set; repository policy fails closed if scanning/child traversal collapses back into the façade.
- Source/portable validation: release/source QA **4/4 deterministic shards PASS**; behavioral **87/87** via 4/4 shards; unit syntax **198/198** plus linked surge VM runtime PASS; production graph **134/134**; portable syntax **127/127**; F1425 attribute grammar **15/15 executable**; F1425 document depth **9/9**; hardcap **6/6**; active manifest **593/593**; FALCION **root=16 / scripts=42 PASS**. Cumulative Windows Production/Debug/Native Acceptance/CRT/game validation remains pending.
#### Host policy/bridge structural decomposition
- Splits registry host policy into public policy façade, configuration/key matching, handle/pseudo ownership, and A/W value serialization modules behind `host_policy_internal.hpp`.
- Splits the Windows host bridge into public lifecycle façade, registry wrapper/quiescence, and inline/IAT hook ownership modules behind `host_bridge_windows_internal.hpp`. Existing registry matching, pseudo-HKEY teardown fence, fallback policy, rollback/recovery and original-publication semantics are retained by intent; public API/ABI are unchanged.
- Source/portable validation: release/source QA **4/4 deterministic shards PASS**; behavioral **87/87** via 4/4 shards; production graph **129/129**; portable syntax **122/122**; F1420 teardown **9/9**; active manifest **586/586**; cumulative Windows Production/Debug/Native Acceptance/CRT/game validation remains pending.
#### XML value-codec structural decomposition
- Splits the former 539-line value codec into decode/escape façade, UTF-8/entity core, normalization/comparison and SemanticValue ownership modules.
- Updates all executable QA link sets to consume the complete four-TU codec implementation.
- Public XML codec API and behavior are unchanged by intent; repository QA fail-closes the new source boundary.
- Source/portable QA: 4/4 release-QA shards PASS; production graph 132/132; portable syntax 125/125; active manifest 590/590.
- Cumulative Windows Production/Debug/Native Acceptance/CRT/game validation remains pending.
#### Registry host policy/bridge structural decomposition
- Splits registry host policy into thin façade plus config/classification, handle/pseudo-handle ownership, and A/W value serialization modules.
- Splits the Windows host bridge into lifecycle façade, wrappers/replacement-address publication, and hook install/remove/recovery ownership modules.
- Public APIs and runtime semantics are unchanged by intent; repository QA fail-closes the new ownership boundaries.
- Source/portable QA: 4/4 release-QA shards PASS; production graph 129/129; portable syntax 122/122; active manifest 586/586.
- Cumulative Windows Production/Debug/Native Acceptance/CRT/game validation remains pending.
#### Surge native-hook structural decomposition
- Splits `src/platform/surge_native_hooks_windows.cpp` into a thin install/upgrade/uninstall/snapshot façade plus dedicated wrapper/original-resolution, IAT transaction/recovery and process-wide inline-hook ownership translation units behind a private internal header.
- Existing Process32NextW/_Mtx_trylock policies, IAT protection-recovery ledger, quiescent inline-hook lifecycle, partial-install rollback and snapshot semantics are retained by intent. Public API/ABI are unchanged.
- Source/portable validation: release/source QA **4/4 deterministic shards PASS**; behavioral **87/87** via 4/4 shards; unit syntax **198/198** plus linked surge VM runtime PASS; production graph **124/124**; portable syntax **119/119**; hardcap **6/6**; model/native semantic gates PASS; shipping build policy **38/38**; active manifest **579/579**; FALCION **root=16 / scripts=42 PASS**. Windows Production/Debug/Native Acceptance/CRT/game validation remains pending.
#### Model release derived-state decomposition
- no intended runtime/public-ABI semantic change.
- Splits low-level MSVC ABI string/vector ownership, entitlement/store-vector materialization, runtime-store source adaptation and public derived-state orchestration into separate translation units.
- Public `model_release_service_derived_state.hpp` and `ReleaseServiceObject548` physical ABI remain unchanged.
- Adds fail-closed repository ownership guards so ABI storage/runtime adapter/materialization code cannot silently collapse back into the façade.
- Source/portable QA: 4/4 release-QA shards PASS; behavioral 87/87; hardcap 6/6; unit syntax 198/198 + linked surge VM PASS; production graph 121/121; portable syntax 119/119; model ABI gates PASS; shipping build policy 38/38; active manifest 575/575; FALCION root=16/scripts=42. Cumulative Windows validation remains pending.
#### Behavioral service state decomposition
- no intended runtime/protocol/public-ABI semantic change.
- Splits the behavioral service monolith into public façade, owned store/locking, Config2 projection, and GrantAchievement transaction translation units behind a private internal contract.
- Keeps Content/Entitlement/Achievement defaults, descendant policy, transactional candidate publication, delivery reservation, and GrantAchievement prepare/commit/rollback semantics unchanged.
- Updates hardcap/behavioral executable harnesses to link the full four-TU production source set and adds repository fail-closed ownership guards.
- Source/portable QA: 4/4 release-QA shards PASS; behavioral 87/87 via 4/4 shards; hardcap 6/6; unit syntax 198/198 plus surge VM linked runtime PASS; production graph 118/118; portable syntax 116/116; shipping build policy 38/38; active manifest 571/571; FALCION root=16/scripts=42. Cumulative Windows validation remains pending.
#### Acceptance/evidence topology closure
- no intended normalized shipping runtime or public-export ABI change.
- Moves native acceptance export implementation to `tests/native/native_acceptance_exports_windows.cpp`; CMake still adds it only when `PRELOADER_L_NATIVE_ACCEPTANCE=ON`.
- Moves evidence-only headers out of shipping public include topology into explicit `tests/support/...` or `src/debug/...` ownership.
- Retires `preloader_l_host`; its load/SURGE/shutdown/unload smoke is subsumed by `preloader_l_release_host`, which additionally exercises the real GetConfig socket path.
- Wires `preloader_l_release_host` into CRT acceptance as the mandatory `SHIPPING_BLACKBOX` step after shipping finalization and before transition/native scenario closure.
- Source/portable QA: 4/4 release-QA shards PASS; behavioral 87/87 via 4/4 shards; unit syntax 198/198 plus linked surge VM runtime PASS; production graph 115/115; portable syntax 113/113; hardcap 6/6; native worker/config source surfaces 25/25 and 14/14; shipping build policy 38/38; active manifest 567/567; FALCION root=16/scripts=42. Cumulative Windows validation remains pending.
#### Debug/evidence source-topology closure
- Moves 20 debug-prototype/evidence-only translation units from production-looking `src/runtime`, `src/net`, `src/handlers`, and `src/platform` locations into explicit `src/debug/runtime`, `src/debug/net`, `src/debug/handlers`, `src/debug/platform`, and `src/debug/evidence` ownership domains. No function body is intentionally changed.
- Moves the Crypt diagnostics private internal header alongside its debug implementation. Public/evidence-only headers under `include/` are intentionally left for the later header-topology cleanup.
- Normalized shipping `preloader_l` CMake membership is unchanged. Debug prototype CMake inputs and executable QA source references use the relocated paths. Repository policy fails closed on old-path return, production-graph leakage, missing relocated files, or duplicate/missing debug-graph references.
- Source/portable validation: release/source QA **4/4 deterministic shards PASS**; behavioral **87/87** via 4/4 shards; unit syntax **198/198** plus linked surge VM runtime PASS; production graph **115/115**; portable syntax **113/113**; hardcap **6/6**; model settings/config runtime + ABI PASS; model worker ABI **27/27**; native worker/config source surfaces **25/25 / 14/14 PASS**; active manifest **568/568**; FALCION **root=16 / scripts=42 PASS**. All 20 moved translation units plus the private Crypt diagnostic header are byte-identical to . Windows Production/Debug/Native Acceptance/game validation remains pending.
#### Surge VM research/test topology closure
- Moves `surge_vm_model.cpp/.hpp` from production-like `src/research` + public `include/preloader_l/research` locations into `tests/support/research/`; neither shipping nor debug runtime targets consume the model.
- Retains the existing VM entry/bootstrap/context-claim regression vectors but closes the dormant-test gap: mandatory unit QA now compiles, links and executes `test_surge_vm_model.cpp` together with the implementation instead of only syntax-checking a header consumer.
- Repository contract fails closed if the old production-like paths return or the linked runtime proof disappears. FALCION script count remains bounded at 42 by integrating the linked proof into the existing unit-syntax gate rather than adding another active script.
- Source/portable focused validation: linked surge VM runtime PASS; behavioral **87/87** via 4/4 shards; unit syntax **198/198**; production graph **115/115**; portable syntax **113/113**; FALCION **root=16 / scripts=42 PASS**. Windows Production/Debug/Native Acceptance/game validation remains pending.
#### Runtime configuration decomposition
- Reduces the former 522-line `src/config/runtime_configuration.cpp` monolith to a public lifecycle/model-service façade and moves Config2 parser policy plus transient storage/arena ownership into `runtime_configuration_parser.cpp` and `runtime_configuration_storage.cpp`.
- `runtime_configuration_internal.hpp` is the private cross-TU contract. Public runtime-configuration API, profile defaults, section bit semantics, configuration transaction behavior and current-directory/model-service policies are unchanged by intent.
- CMake and executable QA link the decomposed source set; repository/behavioral guards fail closed if parser/storage implementation returns to the façade.
- Source/portable QA: behavioral **87/87** via 4/4 shards; unit syntax **198/198**; production graph **115/115**; portable syntax **114/114**; hardcap **6/6**; model settings/config ABI + runtime PASS; native worker/config source surfaces **25/25 / 14/14 PASS**. Windows Production/Debug/Native Acceptance/game validation remains pending.
#### Store entitlement parser decomposition
- Replaces the former 930-line `src/config/store_entitlement_parser.cpp` with four responsibility-specific production translation units: `store_xml_parser.cpp`, `store_value_decode.cpp`, `store_entitlement_decode.cpp`, and `store_document_parser.cpp`.
- XML grammar/scanning, primitive decoding, entitlement/store construction and top-level document orchestration retain the existing algorithms; this working state intentionally changes source ownership only.
- The private `store_entitlement_internal.hpp` contract exposes only the cross-TU parser primitives required by the split. Public store-entitlement API, transactional snapshot behavior, AE00 auxiliary decode semantics and parser status mapping are unchanged by intent.
- CMake, hardcap convergence and repository/behavioral fail-closed checks now consume/enforce the decomposed parser graph.
- Source/portable QA: **4/4 release-QA shards PASS**; behavioral **87/87** via 4/4 shards; unit syntax **198/198**; production graph **113/113**; portable syntax **112/112**; hardcap **6/6**; active manifest **565/565**. Windows Production/Debug/Native Acceptance/game validation remains pending.
#### Crypt observer diagnostic extraction
- Reduces `src/platform/crypt_observer_windows.cpp` from 878 lines to a 201-line production observer focused on CryptQueryObject inline-hook ownership, effective-object selection and teardown.
- Moves optional `CryptMsgGetParam` / `CertFindCertificateInStore` IAT observers, module sweep/slot ownership, caller provenance, debug byte/hex formatting, reconstructed-identity trace accounting and diagnostic call counters into `src/debug/platform/crypt_observer_diagnostics_windows.cpp`.
- The diagnostics TU is present only in the `preloader_l_debug` target. The shipping `preloader_l` target does not compile or link it; a private internal façade is the only debug coupling left in the production observer source.
- Public `crypt_observer.hpp`, release CryptQueryObject behavior, inline-hook lifecycle and restore-then-retire semantics are unchanged by intent. Repository/behavioral QA fail closed if the diagnostic implementation returns to the production observer or enters the production source graph.
- Source/portable QA: behavioral **87/87 PASS** via **4/4** shards; unit syntax **198/198**; production graph **110/110**; portable syntax **109/109**; active manifest **562/562** after manifest refresh. Windows Production/Debug/Native Acceptance/game validation remains pending.
#### socket exchange structural decomposition
- Decomposes the former 619-line `src/protocol/socket_exchange.cpp` into session façade/policy, bounded wire framing/reassembly, and a shared response transaction implementation.
- Both shipping exchange entry paths now call the same `execute_response_transaction` primitive for measure → allocation → render → encode → send → deferred delivery commit. Persistent-session malformed/incomplete/unsupported/unknown routing policy remains in the session façade and is not generalized into the transaction primitive.
- Public `socket_exchange.hpp`, `ExchangeState`, exported behavior and protocol wire semantics are unchanged by intent. GrantAchievement delivery still commits only after successful send and rolls back on all failure paths.
- Repository/behavioral QA fail closed if transaction or wire implementation returns to the façade or if the two exchange paths stop sharing the transaction primitive.
- Source/portable QA: behavioral **87/87 PASS** via **4/4** shards; unit syntax **198/198**; production graph **110/110**; portable syntax **109/109**; model worker ABI **27/27**; active manifest **560/560**. Windows Production/Debug/Native Acceptance/game validation remains pending.
#### model release-service policy structural decomposition
- Splits the former 717-line `model_release_service_policy.cpp` into a thin public-policy façade plus dedicated value/path storage, settings-map, and XML-document ownership translation units sharing only a private internal contract.
- Public `model_release_service_policy.hpp`, model object ABI, allocation policy, transactional settings/document semantics, adapter binding, and F069 hook behavior are unchanged by intent.
- Model settings/config runtime tests now link the decomposed source set; repository contract checks fail closed if heavy implementation responsibility returns to the façade TU.
- Portable/source validation: 4/4 release-QA shards PASS; behavioral 87/87; unit syntax 198/198; production source graph 108/108; portable syntax 107/107; model settings ABI 15/15 + runtime PASS; model config ABI 19/19 + runtime PASS; active manifest 557/557.
- Windows Production/Debug/Native Acceptance/game validation for the cumulative + working state is pending.
#### F069 restore-then-retire inline-hook teardown
- Closes the F069 same-gate re-entry deadlock in quiescent inline-hook teardown by separating admission closure from retirement waiting.
- Teardown ordering is now `Draining -> target restore -> direct-original publication -> Direct -> active replacement-call drain -> trampoline/resource release`.
- Failed target restore reactivates the gate and retains trampoline/ticket ownership for retry, preserving the existing fail-closed recovery contract.
- No TLS/per-thread ownership state is introduced; the existing page-trap relocation path remains the native interleaving mechanism during target restore.
- Portable/source validation: behavioral regressions 4/4 shards PASS (87/87 inventory), unit syntax 198/198, production source graph 105/105, portable syntax 104/104, model config ABI 19/19, active manifest 553/553.
- Windows Production/Debug/Native Acceptance/game validation for is pending.
### Consolidated runtime/security fixes
- Carries all 3.1.8 lifecycle, ownership, protocol transactionality, network/hook hardening, parser/value-codec, XML framing, build/Golden and QA corrections through F1533-001/F1566-001/F1570-001.
- Loader attach remains fail-open while bootstrap functionality fails closed into explicit failed state.
- Persistent-session malformed input uses the bounded consecutive-frame policy; quote-aware LSX framing prevents quoted `</LSX>` data from terminating the outer frame.
- Public export ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
### 3.1.9 release corrections
- Fixes the CRT acceptance launcher failure caused by assignment to PowerShell's read-only `$Host`; the path variable is now `$AcceptanceHost`.
- FALCION's `05_FALCION_GOLDEN_CLEANING.log` now includes Data Virtual Footprint output and final result; the standalone footprint log is removed.
- Assigns CASTORADRON and moves CIVETAVEX into the historical reserved set.
- Embeds the Bakugan-style suffix-family pool and a three-release rotation window in LUTRAVEX naming governance.
#### Windows evidence carried from 3.1.8 CIVETAVEX
- Production build PASS; URSYNEX/ARYRA/MUSTAVEX/CORVEXON/FALCION PASS; shipping DLL SHA-256 `f69a7cf50e74a45114559bc501c425731d61bb31a732f163c92aaad24b171e10`.
- Debug build PASS; debug DLL SHA-256 `85cb275fee490985c35c7bd30ea3e6ac9ed5bb9ddfe280de431b053e38809e00`.
- Native Acceptance PASS; native test DLL SHA-256 `825173cc6f7d7d115622649ea01bfd34e62e409f49c0d92d8e48a464a0fc2d74`.
- Normal Windows/game operation user-confirmed PASS.
- CRT acceptance build/link/policy stages succeeded but the parent launcher stopped on the `$Host` automatic-variable collision; corrected 3.1.9 CRT acceptance rerun is pending.
#### Inline-hook backend architecture decomposition
- Cumulative within this semantic release; behavior-preserving decomposition of the inline-hook Windows backend.
- Keeps `inline_hook_backend_windows.cpp` as thin orchestration and moves quiescence, allocator ownership, page-trap/protection, and patch/trampoline responsibilities into dedicated modules.
- Adds private `inline_hook_backend_windows_internal.hpp`; public hook ABI and `InlineHookTicket` physical layout are unchanged.
- Existing allocator fail-closed shutdown, VEH retry, protection-segmentation, coordinator ownership and relocator semantics remain unchanged.
- Repository architecture QA now forbids implementation-heavy responsibilities from regrowing in the thin backend; no known 500+ line / 3+ responsibility blocker remains in the current registry.
- Exact Windows/MSVC/native/game rerun remains pending.
- portable/source validation: release QA 4/4 shards PASS; behavioral 74/74; unit syntax 198/198; production graph 106/106; portable syntax 104/104; hardcap 6/6; active manifest 552/552; FALCION scripts=42.
#### F1552 relocator trust-boundary remediation
- Cumulative within this semantic release; source-closes `F1552-001` relocator plan trust-boundary / robustness gap.
- `emit_x64_relocated_prologue` preflights caller-supplied plan geometry before any output mutation.
- Rejects malformed instruction counts, non-contiguous source/trampoline offsets, inconsistent aggregate lengths, per-kind emitted-length mismatches, and malformed RIP-relative metadata with `invalid_argument`.
- Exact F1552 adversarial shape (`trampoline_offset=64`, `trampoline_length=1`, caller capacity=1) is executable-regression covered and leaves output/output_size unchanged.
- Canonical `plan_x64_relocation` output and normal `output_too_small` behavior are preserved.
- Exact Windows/MSVC/native/game rerun remains pending.
- portable/source validation: release QA 4/4 shards PASS; behavioral 74/74; unit syntax 198/198; production graph 102/102; portable syntax 104/104; hardcap 6/6; active manifest 547/547; FALCION scripts=42.
#### F1550 hook-coordinator ownership remediation
- Cumulative within this semantic release; source-closes `F1550-001` hook-coordinator ownership gap.
- Failed backend install with a non-null durable current ticket is adopted into `HookSet` before rollback.
- Failed-current cleanup failure keeps the ticket published for later teardown retry instead of losing ownership outside the coordinator.
- Public HookBackend callback ABI and successful registry-hook semantics are unchanged.
- Exact Windows/MSVC/native/game rerun remains pending.
- portable/source validation: release QA 4/4 shards PASS; behavioral 73/73; unit syntax 197/197; production graph 102/102; portable syntax 104/104; hardcap 6/6; active manifest 546/546; FALCION scripts=42.
## 3.1.8 CIVETAVEX
### Consolidated changes
#### lifecycle + transport hardening
- Closes `F1533-001`, `F1566-001`, and `F1570-001`.
- Loader attach remains successful when bootstrap worker dispatch cannot be scheduled, while bootstrap state records the failure and no worker functionality becomes active.
- Persistent sessions accept up to 32 consecutive malformed logical frames, terminate on the 33rd, and reset the streak after a successful handshake or structurally valid dispatch.
- LSX framing is quote-aware: `</LSX>` inside quoted tag values no longer terminates the outer frame. Socket plaintext and decoded-payload boundary handling use the same scanner.
- Source/portable validation: release/source QA 4/4 deterministic shards PASS; behavioral 87/87; unit syntax 198/198; production graph 105/105; portable syntax 104/104; hardcap 6/6; model settings ABI 15/15; model worker ABI 27/27; model config ABI 19/19; native worker surface 25/25; native config surface 14/14; shipping build policy 34/34; active manifest 552/552; FALCION root=16 scripts=42.
- Public export ABI and `ExchangeState` layout are unchanged; cumulative Windows/MSVC/native/game/network rerun remains pending.
#### terminology-only structural refactor
- Applies one fixed project vocabulary across code/machine surfaces and file names; contextual synonyms are not permitted.
- Both historical recovery-form roots map to `curio`.
- Adds fail-closed repository terminology QA while leaving documentation history unrestricted.
- No intentional runtime/protocol/config/public-ABI/layout semantic delta.
#### parser/codec remediation
- Closes `F1571-001`, `F1573-001`, and `F1579-001`.
- Self-closing Request wrappers reject non-whitespace trailing content instead of returning `no_command` before document-cardinality validation.
- Raw XML-source decode/compare paths validate complete UTF-8 units and XML Char legality consistently with normalization/entity decoding.
- Duplicate attribute names are malformed across both the request parser and public attribute helper; first-match lookup cannot mask duplicates.
- Authority-blocked `F1570-001` is intentionally unchanged; framing and quoted raw-`<` compatibility remain outside .
- Portable/source validation: release/source QA 4/4 shards PASS; behavioral 87/87; unit syntax 198/198; production graph 105/105; portable syntax 104/104; hardcap 6/6; reference worker ABI 27/27; native worker surface 25/25; native config surface 14/14; shipping build policy 34/34 + MSVC link-policy self-test 14/14; active manifest 552/552; FALCION root=16 scripts=42.
- Exact cumulative Windows/MSVC production/debug/native/game/network rerun remains pending.
#### 12-root build/release + QA/provenance consolidation
- Closes `F1532-001`, `F1532-002`, `F1536-001`, `F1536-002`, `F1538-002`, `F1614-001`, `F1530-001`, `F1531-001`, `F1531-002`, `F1549-001`, `F1559-001`, `F1585-001`.
- URSYNEX now binds structured mutation values to MUSTAVEX authority and owns PE checksum recomputation after its approved post-link mutations; CORVEXON independently verifies the final checksum read-only.
- MUSTAVEX now enforces VersionInfo StringTable-key authority. Debug build language/CRT scoping and native-acceptance CWD robustness are fail-closed.
- FALCION rejects generated Python bytecode; semantic-release H2 headings are unique per version; stale current-shipping CRT provenance and dormant production diagnostic-TU debt are removed; entitlement QA reflects direct-child filter authority.
- `F1571-001`, `F1573-001`, and `F1579-001` are intentionally not part of this consolidation.
- Exact cumulative Windows/MSVC/native/game rerun remains pending.
- Portable/source validation: release/source QA 4/4 shards PASS; behavioral 87/87; unit syntax 198/198; production graph 105/105; portable syntax 104/104; hardcap 6/6; reference worker ABI 27/27; native worker surface 25/25; native config surface 14/14; shipping build policy 34/34 + MSVC link-policy self-test 14/14; active manifest 552/552; FALCION root=16 scripts=42.
#### F1610 primary-state executable coverage closure
- Closes `F1610-001` at QA/source level without changing production/runtime/protocol/config/build-authority/public-ABI source.
- Updates all 13 `tests/unit/test_primary_state_*.cpp` executables from stale historical parity/unresolved expectations to the current authority split: production unresolved/parity-open are zero; historical bit identities remain source-compatible; recovered/integrated/runtime-active roles remain explicit; exact/reference parity is asserted through its separate snapshot field.
- `scripts/test_behavioral_regressions.py` now fail-closes primary-state test-count drift and compiles+runs all 13 programs against `src/runtime/primary_runtime.cpp` + `src/runtime/parity_closure.cpp`. Behavioral inventory increases from 74 to 87.
- Exact cumulative Windows/MSVC/native/game rerun remains pending; itself has no production/runtime semantic delta.
- Portable/source validation: release/source QA 4/4 shards PASS; behavioral 87/87; unit syntax 198/198; production graph 106/106; portable syntax 104/104; hardcap 6/6; reference worker ABI 27/27; native worker surface 25/25; native config surface 14/14; active manifest 552/552; FALCION root=16 scripts=42.
#### build/finalization remediation
- Source/portable-QA remediation scope: `F1535-001`, `F1537-001`, `F1538-001`, `F1539-001`, `F1539-002`.
- Production publication is transactional: a stale prior `dist/preloader_l.dll` is invalidated before compile and a new DLL is published only after the full finalization/validation chain passes.
- ARYRA rejects raw section extents outside the file and overlapping loaded virtual section ranges without adopting exact-layout parity.
- MUSTAVEX enforces current-release identity exclusion in addition to historical release identities.
- CORVEXON requires executable entrypoint section semantics and writable `/GS` SecurityCookie section storage.
- Production/runtime/protocol/config/public ABI source is unchanged by this remediation. Exact Windows/MSVC/native/game rerun remains pending.
- Portable/source validation: release/source QA 4/4 shards PASS; behavioral 74/74; unit syntax 198/198; production graph 106/106; portable syntax 104/104; hardcap 6/6; reference worker ABI 27/27; native worker surface 25/25; native config surface 14/14; active manifest 552/552; FALCION root=16 scripts=42. Production/runtime/include/export/resource/CMake surface delta: 0 files. Exact Windows/MSVC/native/game rerun remains pending.
#### combined runtime remediation
- Source/portable-QA closes `F1598-001`, `F1599-001`, `F1601-001` and `F1602-001`.
- Adds a project-owned 128-live-client worker hard cap and bounds client-vector reserve to the same ceiling. Over-capacity and per-client allocation/thread admission failures are connection-local and traced; `0x5d02` no longer dispatches process-terminal policy.
- `reset_exchange` now securely zeroes the complete `ExchangeState` before reuse or heap release, including decoded request plaintext, serialized/plain response bytes, wire/ciphertext remnants and the session key.
- Keeps the recovered recursive-mutex ABI/layout intact while making owner/recursion observations and updates atomic around the existing SRW lock.
- Replaces the shipping MSVC outer-dispatch no-yield spinlock with a blocking SRW lock while retaining the two-pass correctness transaction and queue/state serialization.
- Extends executable/source regression coverage for all four roots. Portable/source validation: release QA 4/4 shards PASS; behavioral 74/74; unit syntax 198/198; production graph 106/106; portable syntax 104/104; hardcap 6/6; active manifest 552/552; FALCION scripts=42.
- Exact Windows/MSVC production, native acceptance and normal game/network rerun remains pending.
#### Release-server architecture decomposition
- Behavior-preserving decomposition of `src/runtime/release_server_windows.cpp` into worker, acceptor, and lifecycle translation units plus a private internal contract header.
- Preserves worker socket I/O, accept-loop/client ownership, stop/quiescence semantics, public release-server ABI, and all protocol/config behavior.
- Updates CMake production/native graphs, worker semantic QA, CRT source-policy paths, repository contracts, and active-manifest ownership to the split module graph; the legacy monolith is fail-closed absent.
- Source-health architecture review now leaves `src/platform/inline_hook_backend_windows.cpp` as the remaining known 500+ line / 3+ responsibility blocker.
- portable/source validation: release QA 4/4 shards PASS; behavioral 73/73; unit syntax 197/197; production graph 102/102; portable syntax 104/104; hardcap 6/6; active manifest 546/546; FALCION scripts=42.
- Exact Windows/MSVC/native/game rerun remains pending.
#### Store-entitlement architecture decomposition
- Behavior-preserving decomposition of the 1546-line `src/config/store_entitlement.cpp` monolith into parser, materialization, runtime-state, and reference-service bridge translation units plus a private internal contract header.
- Preserves the existing exact XML grammar and Store/Entitlement traversal; does not replace the retained parser with a generic XML library or alter donor/consumer authority semantics.
- Public `store_entitlement.hpp` ABI remains unchanged. CMake, hard-cap linkage, repository QA, and active-manifest ownership are updated to the new module graph.
- Adds source-health architecture-review reporting to existing repository QA: 500+ lines alone is not an error, while known 500+ line files with 3+ independent responsibilities are surfaced for review. No new top-level script; FALCION remains capped at 42 scripts.
- Exact Windows/MSVC/native/game rerun remains pending.
#### Architecture decomposition
- Removes `src/handlers/recovered_routes.cpp` and `include/preloader_l/handlers/recovered_routes.hpp`.
- Splits route implementations into functional game-info, profile/presence, auth, store-query, achievement, and image modules with domain headers plus neutral `route_handlers.hpp`.
- Keeps shared response/config/attribute mechanics in a private support header while achievement transaction/watcher helpers and entitlement-filter helpers remain owned by their domain TUs.
- Updates CMake, registry/dispatch/socket includes, unit tests, focused QA source lists and repository contracts to the new module graph; legacy recovered-route paths are fail-closed absent.
- Public export ABI, route signatures, protocol grammar, config authority and transactionality semantics are unchanged by intent. Behavioral inventory remains **73 cases** through 4 deterministic shards; FALCION script cap remains **42**.
- Exact Windows/MSVC production, native acceptance and normal game/network rerun is **pending** because the production TU/link graph changed.
#### F1568 state-delivery transactionality remediation
- Cumulative within this semantic release; source-closes `F1568-001` protocol transactionality/state-delivery divergence.
- Shipping socket GrantAchievement now prepares persistent achievement mutation and completion-event FIFO delivery without committing either before response delivery. Encoding/resource/send failure rolls both back; successful `send_all` commits the store mutation and reserved FIFO decision before the frame is consumed.
- Direct/non-socket GrantAchievement callers retain the established immediate-commit handler semantics. The watcher contract is unchanged: the projected callback remains pre-commit because external callback side effects are not rollbackable.
- Adds executable rollback/retry coverage for achievement state plus FIFO preservation, and fail-closed source assertions that both shipping socket paths commit only after successful `send_all`.
- Behavioral inventory is now **73 cases**, executed through 4 deterministic shards. Full release/source QA passed 4/4 deterministic shards and the FALCION 42-script cap is unchanged.
- exact Windows/MSVC production, native acceptance and normal game/network rerun is **pending**.
#### F1553/F1604 protection-ownership remediation
- Cumulative within this semantic release; source-closes `F1553-001` and `F1604-001`.
- Inline-hook source/trampoline protection changes are partitioned by the actual `VirtualQuery` memory regions; each region retains and restores its own original protection instead of restoring a heterogeneous cross-page span with one first-page value.
- Protection restoration that fails after executable-byte/protection mutation is process-terminal, preventing degraded protection provenance from escaping into a later rollback/retry.
- Adds portable protection-segment geometry coverage and fail-closed source assertions. Behavioral inventory is now **72 cases**, executed through 4 deterministic shards; behavioral 4/4 and full release/source QA 4/4 PASS on the portable validation host.
- exact Windows/MSVC production, native acceptance and normal game/network rerun is **pending**.
#### F1540/F1544 bootstrap and PTD/FLS remediation
- behavioral QA: the 71-case aggregate is executed through 4 deterministic shards; all 4 shards must pass before 71/71 is claimed. This is QA-only and does not alter shipping behavior. Full source QA is likewise shardable and passed 4/4 portable shards.
- Source-closes audit finding `F1540-001` by serializing the bootstrap phase/counter/snapshot state behind a short interlocked state lock. The accept-owner `mark_bootstrap_client_served` path and bootstrap-worker state transitions no longer access the shared bootstrap state concurrently through plain non-atomic reads/writes.
- Source-closes audit finding `F1544-001` by extending every successful post-acquire bootstrap-failure unwind through current-thread PTD detach, process-wide PTD/FLS shutdown, and explicit `ptd_fls_out_of_indexes` verification after temporary lease release, hook teardown, and permanent release-service owner shutdown. Failed `FlsFree` remains fail-closed/retryable.
- Adds executable multi-thread bootstrap-state regression coverage and strengthens aggregate behavioral QA for post-acquire PTD/FLS unwind ordering.
- exact Windows/MSVC production, native-acceptance and normal game/network rerun is **pending**. No Windows-native PASS is claimed by this source-side remediation working state.
### Supported MSVC runtime transition
- Canonical shipping C++ uses **`/GS` + `/MD` (`MultiThreadedDLL`)** and linker-selected **`_DllMainCRTStartup`**.
- Canonical `/NODEFAULTLIB`, custom `/ENTRY:DllMain`, `/Zl`, manual `msvcrt.lib`/`vcruntime.lib` closure, `memset`/`memcpy` alternatename shims and manual shipping `__security_init_cookie` bootstrap are removed.
- `DllMain @1` remains part of the public ABI and remains loader-safe, but is the application callback invoked through CRT startup rather than the PE entrypoint itself.
- The old 3.1.7 custom-entry `/GS` model remains available only through `PRELOADER_L_COMPATIBILITY_CUSTOM_ENTRY=ON` for A/B comparison; it is never canonical shipping.
- Project-owned production/native/release/debug developer hosts use normal CRT `main`; release/native hosts use `/MD`, debug hosts and debug DLL use `/MDd`.
- The freestanding `memset`/`memcpy` shim is outside the canonical production graph and retained only for legacy comparison.
### Golden / acceptance changes
- CORVEXON keeps exact exports `DllMain @1`, `SURGE @2`, `DllShutdown @3`, but requires the PE entrypoint to be **distinct from `DllMain @1` and resident in `.text`**.
- `/GS` remains fail-closed: CORVEXON requires a nonzero image-backed Load Config `SecurityCookie`.
- Approved runtime imports are limited to the supported `/MD` family: `VCRUNTIME140.DLL`, `VCRUNTIME140_1.DLL`, `UCRTBASE.DLL`, and `API-MS-WIN-CRT-*` when emitted. `MSVCP*` and legacy `MSVCR*` families remain forbidden.
- `BUILD_CRT_ACCEPTANCE.cmd` builds the superseded 3.1.7 custom-entry comparison and canonical 3.1.8 CRT image, compares PE/import/entrypoint surfaces, applies the canonical Golden chain and runs native scenarios.
- Fixes the CRT acceptance false-negative in the prior transition candidate: generated-file validation no longer requires literal `/MD` spelling. `MSVC_RUNTIME_LIBRARY=MultiThreadedDLL` is the source authority; generated flags fail closed on contradictory runtime selections and final binary/Golden/native checks prove the realized CRT model.
- Shipping pipeline remains **MSVC -> URSYNEX -> ARYRA -> MUSTAVEX -> CORVEXON -> FALCION**.
### Validation status
- User-confirmed runtime execution for the prior transition candidate: production build PASS, debug build PASS, native acceptance PASS and normal Windows/game operation PASS.
- The initial CRT acceptance run stopped only at the corrected literal-`/MD` QA false-negative before downstream Golden/binary/native stages; no runtime/link failure was observed in the canonical CRT build.
- Final release changes relative to the tested candidate are QA/release metadata only; production/runtime source is unchanged.
- Phase1450–1529 remains 18/18 source/QA closed with zero source-open findings.
- 3.1.8 CIVETAVEX is the official baseline for the next 100-phase audit.
### Naming / documentation
- **CIVETAVEX** remains the 3.1.8 current root; **GENETAVEX** remains historical/reserved.
- The machine-readable reserved namespace appears exactly once at the end of this CHANGELOG and is mirrored by the LUTRAVEX JSON registry.
---
## 3.1.7 GENETAVEX
### Build/security baseline changes
- Promotes MSVC **`/GS`** from the old opt-in candidate to the canonical shipping C++ mode. Legacy `/GS-` remains comparison-only and cannot be the canonical shipping artifact.
- Keeps `/NODEFAULTLIB` and the explicit `/ENTRY:DllMain` shipping model; `DllMain` performs the required `__security_init_cookie` bootstrap before entering `/GS`-protected project code.
- Adds explicit `msvcrt.lib` compiler-support linkage for GS cookie/handler support while CORVEXON continues to reject broad MSVCRT/MSVCP/UCRT runtime-import expansion.
- Keeps toolchain-default FH4. `VCRUNTIME140.DLL` and `VCRUNTIME140_1.DLL` remain the only approved compiler-runtime DLL families.
- Separates debug hosts from the shipping freestanding linker model: debug tools use the supported Debug CRT so `_RTC_*` and FH4 runtime helpers resolve normally.
- Corrects the production launcher banner to the canonical sequence **MSVC -> URSYNEX -> ARYRA -> MUSTAVEX -> CORVEXON -> FALCION**.
### Golden / validation changes
- `BUILD_GS_ACCEPTANCE.cmd` now validates canonical shipping `/GS`, an explicitly non-shipping legacy `/GS-` comparison, Golden acceptance and native shipping-mode scenarios.
- GS transition comparison permits only the intended Load Config (#10) and VCRUNTIME140/VCRUNTIME140_1 module deltas; ARYRA/MUSTAVEX/CORVEXON remain the final fail-closed authorities.
- Broad CRT import families remain forbidden even though compiler-support objects are explicitly linked.
### Documentation / naming cleanup
- Removes duplicated machine-readable reserved-root dumps from historical release sections.
- The reserved namespace now appears exactly once, at the **end of this CHANGELOG**, and is mirrored by the JSON naming registry.
- Assigns **GENETAVEX** to 3.1.7 (`GENET-` from New Latin *Genetta* / genet, plus synthetic `-AVEX`). `VIVERRAVEX` moves into the permanent historical reserved set.
### Validation status
- Bounded source QA PASS: release/naming/FALCION/Golden/repository/GS/production/freestanding/macro/hook/minimal-fixset policies; unit syntax 194/194; production syntax 93/93; portable syntax 96/96; hard-cap 6/6; settings 15/15 + runtime; worker 27/27; native worker/config surfaces 25/25 + 14/14; config 19/19 + runtime; F1420/F1425/Auth8/avatar PASS; F1451 executable support 5/5; retained F1485/F1499/F1500 focused PASS.
- Full 71-case monolithic behavioral aggregate is not claimed by the assistant because it exceeds the execution window.
- Exact 3.1.7 Windows production, GS acceptance, debug build, native acceptance and normal game/network regression remain required before final publication.
---
## 3.1.6 VIVERRAVEX
3.1.6 is the consolidated production release of the complete **3.1.5 MELAVEX remediation line**. The semantic-version bump itself introduces **no production/runtime source change**: `src/`, `include/`, `exports/`, `resources/`, `third_party/`, and `CMakeLists.txt` are byte-identical to the final 3.1.5 production/runtime state. Release documentation, QA identity, naming governance and package metadata are updated for the new canonical release.
### Consolidated runtime and lifecycle changes
- Carries forward the consumer-compatible exact-v3 session-key authority, true-unknown no-response persistent-session continuation and retained telemetry/entitlement authority decisions from the MELAVEX line.
- Carries forward lifecycle fixes for worker/listener/socket/config teardown, CWD mirror reset, Winsock mismatch-cleanup retry ownership, queued-event shutdown lifetime and same-process reload/unload discipline.
- Carries forward bounded 4 KiB stream reassembly for fragmented/coalesced LSX and encrypted frames, direct/unwrapped one-command cardinality, and well-formed UTF-8/XML 1.0 response-attribute validation.
- Carries forward lease-only Auth8 token access and retirement/correction of stale handshake/API/documentation surfaces.
### Audit closeout
- The Phase1450-1529 80-phase campaign contained **18 historical findings**.
- **2** closed in Phase1529 authority convergence: `F1477-001`, `F1491-002`.
- **8** closed in the first consolidated audit-remediation wave: `F1452-001`, `F1452-002`, `F1463-001`, `F1465-001`, `F1476-001`, `F1482-001`, `F1519-001`, `F1527-001`.
- **8** closed in the final consolidation wave: `F1485-001`, `F1499-001`, `F1500-001`, `F1451-001`, `F1452-003`, `F1491-001`, `F1492-001`, `F1514-001`.
- Current source-open audit finding count: **0**.
### Build and release governance
- Canonical Golden chain remains **MSVC -> URSYNEX Golden Finalizer -> ARYRA Golden Layout -> MUSTAVEX Golden Metadata -> CORVEXON Golden Trail -> FALCION Golden Cleaning**.
- Canonical x64 MSVC uses the toolchain-default FH4 model; the undocumented `/d2FH4-` escape hatch remains prohibited. `VCRUNTIME140_1.DLL` is accepted when emitted by MSVC.
- Test-support-only implementation units remain outside the production source graph with executable coverage.
- FULL SOURCE packaging is flat at archive root; `surge.cfg`, `surge.png`, FULL HISTORY and embedded research/evidence archives are excluded.
### Native and validation status
- The user confirms Windows/native acceptance and normal game operation PASS on the final audit-debt-zero runtime. 3.1.6 inherits that working state because its production/runtime source is byte-identical. The assistant did not independently inspect the Windows artifacts.
- Inherited source closure records unit syntax **194/194**, production syntax **93/93**, portable syntax **96/96**, hard-cap **6/6**, F1451 support coverage **5/5**, and focused Auth8/handshake/session/F1485/F1499/F1500 PASS.
- Behavioral inventory is **71 cases**. The assistant does not claim a complete 71/71 monolithic aggregate because that runner exceeds the available execution window.
### Naming provenance
The authoritative reserved namespace appears once at the end of this CHANGELOG and is mirrored by `metadata/LUTRAVEX_GOLDEN_NAMING_REGISTRY.json`; historical release sections do not embed duplicate dumps.
---
## 3.1.5 MELAVEX
### Consolidated changes
#### final source consolidation (native pending)
- Consolidates Golden authority separation plus source-closed **F1485-001**, **F1499-001**, and **F1500-001**.
- Final audit-debt closure also resolves the five remaining Phase1529 carry-forward findings: **F1451-001** test-support TU classification/executable coverage, **F1452-003** removal of undocumented `/d2FH4-`, **F1491-001** stale handshake-status retirement, **F1492-001** exact-v3 session-key documentation correction, and **F1514-001** removal of the unsafe Auth8 global-view API.
- Modern x64 MSVC FH4 is now the canonical exception model; `VCRUNTIME140_1.DLL` is an accepted CORVEXON import when emitted by the toolchain.
- Canonical Golden names remain exactly **URSYNEX Golden Finalizer**, **ARYRA Golden Layout**, **MUSTAVEX Golden Metadata**, **CORVEXON Golden Trail**, and **FALCION Golden Cleaning**.
- Final FULL SOURCE packaging is flat at archive root; `surge.cfg`, `surge.png`, embedded evidence/research archives, and FULL HISTORY remain excluded.
- Current source validation after final audit-debt cleanup: unit syntax 194/194, production syntax graph 93/93, portable syntax 96/96, F1451 implementation-linked support coverage 5/5, lease-only Auth8 PASS, handshake/session authority PASS, retained F1500/F1499/F1485 focused PASS, Golden/FALCION/LUTRAVEX/repository/build-policy gates PASS. Behavioral inventory is 71 cases; full monolithic aggregate remains a long-running execution boundary. Exact Windows/MSVC FH4 link/import behavior remains pending.
- The previously validated native/game state remains inherited evidence only. Exact Windows/MSVC build/link, final-DLL Golden verification, native acceptance, and real game/network regression remain pending.
#### F1500 UTF-8 attribute boundary
- Retains Golden authority work, F1485 stream reassembly, and F1499 direct-command cardinality unchanged.
- Source-closes F1500-001: semantic and parser-owned response attribute serialization now requires well-formed UTF-8 plus the existing XML 1.0 character subset before emitting markup.
- Preserves valid multi-byte UTF-8 and numeric entity decoding while rejecting lone continuation, truncated, overlong, surrogate, out-of-range and malformed-continuation encodings.
- Extends the existing response executable only; no standalone QA script or new behavioral case slot is added, so script hard-cap remains 42 and behavioral inventory remains 70 cases.
- Phase1500 evidence SHA-256: `ff61e3c8aec4cee7dd3fbf58d06d455c5fbbb7e0c3ad73b9067497ff532a245e`; verifier 4/4 PASS.
- Focused F1500 PASS with retained F1499/F1485 focused PASS; bounded repo-wide QA PASS. The long 70-case aggregate is not claimed as 70/70.
- Exact Windows/MSVC, final-DLL, native and real game/network acceptance remain pending. Documentation parity and final flat release consolidation remain after this bounded slice.
#### F1499 direct-command cardinality
- Retains Golden authority work and F1485 stream reassembly unchanged.
- Source-closes F1499-001: direct/unwrapped LSX requests now require whitespace-only remainder after the first command, matching the wrapped one-command cardinality and rejecting silently ignored trailing sibling commands.
- Covers both self-closing and explicitly closed first commands while preserving trailing-whitespace acceptance.
- Extends the existing request executable and behavioral aggregate without adding a standalone QA script; script hard-cap remains 42 and behavioral inventory advances to 70 cases.
- Focused F1499 execution PASS and retained F1485 focused execution PASS; full 70-case aggregate rerun hit the tool timeout, so aggregate PASS is not claimed.
- Phase1499 evidence SHA-256: `14f9f438be1b1674fda1e76eac1cdfa0892345ba69932f201e52437cbac03440`; verifier 4/4 PASS.
- F1500-001 remains pending. Exact Windows/MSVC, final-DLL, native and real game/network acceptance remain pending.
#### F1485 stream reassembly
- Retains Golden naming/authority/logging/flat-package work unchanged.
- Source-closes F1485-001 with bounded 4096-byte wire reassembly in `ExchangeState`; fragmented LSX/encrypted frames are retained across recv calls and coalesced frames are consumed independently.
- Encrypted framing uses terminal NUL when present and first valid AES/PKCS#7 frame boundaries for optional-NUL compatibility; no retired 8192-byte ingress accumulator returns.
- Adds executable split/coalesced framing regression coverage and advances the aggregate behavioral inventory to 69 cases; focused F1485 execution passes, while the full aggregate rerun remains tool-timeout pending.
- Phase1485 evidence SHA-256: `bcac7d28c4701a1f56db0f7fd6268146da959c1e9d09c0c1e0515eee9b982c30`; verifier 6/6 PASS.
- F1499-001 and F1500-001 remain pending. Exact Windows/MSVC, final-DLL, native and real game/network acceptance remain pending.
#### Golden authority separation
- Canonical Golden names are exactly **URSYNEX Golden Finalizer**, **ARYRA Golden Layout**, **MUSTAVEX Golden Metadata**, **CORVEXON Golden Trail**, and **FALCION Golden Cleaning**.
- URSYNEX is the sole bounded mutating post-link authority; ARYRA/MUSTAVEX/CORVEXON remain read-only.
- FALCION owns release cleaning and flat-package boundaries; flat packaging is not part of the protocol name.
- Golden logs are ordered `01` through `05` and carry authority/target/mode/hash/result fields.
- Golden authority separation changes no production/runtime source. F1485-001, F1499-001 and F1500-001 remain pending for later slices.
#### combined lifecycle + QA/acceptance hardening
- Source-closes three independent lifecycle roots from the completed adversarial audit: F1476-001 CWD mirror persistence, F1482-001 Winsock mismatch-cleanup ownership loss, and F1519-001 event FIFO shutdown lifetime.
- Closes the approved QA/proof debts F1452-001, F1452-002, F1463-001, F1465-001 and F1527-001 without widening the shipping export ABI.
- Production build is Release-only; native acceptance sanitizes inherited compiler/linker option environment; project-owned hosts require successful `DllShutdown` before final unload; acceptance adds same-process two-generation reload coverage; production graph fail-closes `exception_policy.cpp`.
- FALCION remains bounded to 42 active scripts; checks are integrated into existing gates.
- F1485-001, F1499-001 and F1500-001 remain intentionally deferred/outside .
- The previously validated native/game state remains inherited evidence only; exact Windows/MSVC, final-DLL and native/game/network acceptance are pending.
#### F1427 true-unknown dispatcher convergence + FALCION
- F1427-001 donor x86/x64 research proves true router misses are distinct from registered generic-unimplemented routes. removes the synthetic unknown-command `ErrorSuccess` response and restores no-normal-response persistent-session continuation.
- F1427-002 does not expand the offline shipping route surface: all 18 donor-only registrations remain research-only; the 9 CollegeFB27/Madden27 exposed donor-only routes remain a compatibility watchlist.
- Active release-hygiene terminology is migrated to **FALCION** (`docs/FALCION_GOLDEN_CLEANING.md`, `scripts/check_falcion_golden_cleaning.py`).
- `BUILD_GS_ACCEPTANCE.cmd` line endings are repaired. `/GS` remains deferred/rejected for the current `/NODEFAULTLIB` shipping contract.
- The latest previously validated native/game state remains evidence only; a fresh native acceptance run is required.
#### F1423 consumer session-key authority remediation
- Consumer recovery from CollegeFB27 and Madden27 proves version-3 session derivation consumes `ChallengeAccepted/@response[0:2]`.
- The donor-derived exact live one-byte `version="3"` incoming `ChallengeResponse/@key[0:2]` seed authority is superseded as consumer-incompatible.
- This remediation retains `atoi`-family version selection and handshake grammar, changes only exact one-byte `version="3"` to generated `ChallengeAccepted/@response[0:2]`, preserves missing/non-exact-v3 dynamic behavior, and adds an executable authority regression.
- F1429 raw telemetry string behavior and F1436 direct-child entitlement grammar are retained; consumer research found no incompatibility for those changes.
- F1445 runtime remains unchanged.
- Exact Windows/MSVC and native game/network acceptance remain pending.
#### donor convergence state, partially superseded by consumer evidence
- Introduced donor-derived changes for F1423, F1429 and F1436 plus F1445 provenance metadata correction.
- F1429 and F1436 remain retained after consumer validation.
- F1423's exact one-byte `version="3"` **seed source only** is superseded by consumer evidence; version parsing/grammar and other dynamic branches are retained.
#### F1416-003 /GS hardening candidate
- Added opt-in `PRELOADER_L_GS_HARDENING_CANDIDATE`; shipping default remains `/GS-`.
- Candidate initializes the MSVC security cookie at the custom `DllMain` entry before protected runtime flow and remains blocked on exact Windows acceptance.
#### bounded runtime/lifecycle remediation
- Closed F1415-003 at source level with completed-client owner reaping after kernel-signaled completion.
- Closed F1421-002 at source level with exact Winsock 2.2 negotiation and balanced mismatch cleanup.
- Retained native shutdown/htons acceptance surfaces and kept PROBE7 diagnostic-only.
#### F1411-001 CORVEXON Golden Trail
- Closes `F1411-001` at the build/QA layer with a permanent read-only final-binary acceptance controller.
- CORVEXON is used only as provenance-preserving derived gate lineage; this is not a reused release codename.
- MUSTAVEX remains the only bounded PE metadata finalizer. Corvexon runs after ARYRA/MUSTAVEX validation and never writes the DLL.
- Exact shipping exports/ordinals are checked (`DllMain @1`, `SURGE @2`, `DllShutdown @3`), and `DllMain` must equal PE `AddressOfEntryPoint`.
- Final binary machine/format/subsystem, import-module policy and exact MSVC shipping-link policy are fail-closed.
- Controller verifies SHA-256 immutability before/after all checks.
- Production/runtime source and shipping CMake target are unchanged.
#### dead/compatibility production cleanup
- Closes `F1445-004` by removing `crypt_guard.cpp` and `crypt_path_restore.cpp` from the shipping production target; they remain debug/test compatibility surfaces only where needed. The dead shipping `crypt_path_restore.hpp` include is removed from `crypt_observer_windows.cpp`.
- Closes the `F1444-002 / F1447-004` legacy planner debt by removing `x64_inline_hook_plan.cpp` from the shipping target. The shipping inline-hook backend now owns its narrow FF25 absolute-jump encoder locally and continues to use the recovered relocation engine for prologue planning/emission.
- The retained debug/test planner now records REX.W and decodes `MOVABS r64, imm64` as a 10-byte instruction; `test_inline_hook_plan.cpp` adds an executable MOVABS regression.
- `check_production_source_graph.py` fail-closes reintroduction of all three retired compatibility TUs into the shipping target. `test_behavioral_regressions.py` compile+executes the compatibility planner regression and raises active behavioral coverage from **41/41 to 42/42**.
- Shipping production graph is reduced from **96 to 93 translation units** by deliberate dead-TU removal; this is not a lost semantic implementation. Portable syntax remains **98/98** and unit syntax **189/189**.
- No new observable runtime behavior is intended, but the shipping binary byte/layout surface changes. A cumulative Windows/native rerun is required for this shipping binary/layout delta; the previously validated native Windows result remains evidence only until that rerun is completed.
#### metadata closure truthfulness
- **No production/runtime source, test-source, or QA-orchestration semantic changes.** This working state changes release identity/metadata/documentation only.
- Closes the metadata-truthfulness slice PIA315-002 / F1448-002: F1425/F1426 closure is now bounded to the actually recovered/tested ASCII shipping subset and does not claim full XML-spec or Unicode Name compliance.
- Adds an explicit closure truth model separating source-closed findings, native-pending deltas, authority/evidence-blocked findings, deferred hardening policy, and dead/compatibility cleanup.
- The previously validated native Windows state remains evidence only. The only subsequent production/runtime delta is F1442-001 single-`htons` authority; the later executable-QA work is QA-only and the metadata-closure work is metadata-only.
- Authority/evidence-blocked roots remain intentionally unmodified: F1423-001, F1427-001/002, F1436-001, F1445-003 and F1429-001.
- Cumulative native rerun remains intentionally deferred until the consolidated remediation line is ready for acceptance.
#### executable QA truthfulness convergence
- **No production/runtime source file changes.** This working state changes tests, active QA orchestration and release metadata only.
- Closes the known stale-fixture slices `M315-QA-001`, `F1429-002`, `F1432-003`, `F1434-002` and `F1440-002`; extends the active `F1437-003` QueryAchievements regression with exact-`false` offline and legacy `"0"` non-offline coverage.
- `test_environment_policy.cpp` now uses exact `Online=false` for offline projection and proves `Online=0` remains non-offline.
- `test_selector_pipeline.cpp` now expects current fallback provenance for IGO/telemetry selectors and is compile+executed by active QA.
- QueryFriends, CheckPermission, ShowIGO, StartDownload and SetPresence fixtures now exercise both exact `false` offline responses and `"0"` online behavior, and are compile+executed by active QA.
- `test_connectivity_auth_authority.cpp` is now compile+executed through the real `handle_get_auth_code` path, locking measurement/tiny/exact-capacity no-commit semantics plus successful session-token stability.
- Behavioral aggregate rises from **33/33 to 41/41**. This does not claim that every one of the 189 unit translation units is now executable; the broader F1447-001 coverage-ratio concern remains for later QA/dead-code convergence.
- Native status is unchanged from runtime ancestry: the previously validated native Windows PASS remains inherited evidence; exact-package native execution for the later QA states remains pending and is intentionally deferred until consolidated acceptance.
#### F1442-001 single `htons` authority
- Runtime scope is limited to F1442-001: the dedicated quiescent `htons_redirect` stage is the sole shipping `htons` authority.
- Generic `host_hook_count` is reduced from six to five and now covers only `RegOpenKeyExA`, `RegOpenKeyExW`, `RegQueryValueExA`, `RegQueryValueExW` and `RegCloseKey`; its module authority is registry-only `advapi32.dll`.
- Legacy `HtonsOriginal`, `HostBridgeState::selected_port`, `bridge_htons`, `preloader_l_htons`, `set_windows_host_selected_port` and the sixth HostBridge original/replacement slot are removed.
- Listener bootstrap now publishes the selected port only through `set_htons_selected_port` before installing `htons_redirect`.
- `test_hook_coordinator.cpp` and `test_host_bridge.cpp` are updated to the five-hook/registry-only contract. Active behavioral QA fail-closes any reintroduction of the legacy sixth authority and compile+executes the coordinator plus dedicated portable redirect regression, raising the aggregate to 33/33.
- F1442-002 is improved but not declared fully closed: exact Windows inline-wrapper/trampoline install→map→original→uninstall execution remains pending native acceptance.
#### PIA315-004 / F1426-001 XML minimality-parity
- Runtime scope is limited to the retained-authority XML overreach identified as PIA315-004 plus the residual F1426-001 rejection of XML-legal TAB/LF/CR semantic attribute values.
- `src/xml/request.cpp` preserves F1425 full-tag validation and ASCII Name-start enforcement while restoring retained TinyXML2 compatibility for adjacent quoted attributes and raw `<` inside quoted attribute values.
- `src/xml/value_codec.cpp` now accepts XML 1.0 TAB (`0x09`), LF (`0x0A`) and CR (`0x0D`) in semantic/source-normalized attribute values while still rejecting other non-XML C0 controls.
- The F1425 grammar gate remains executable and now locks the two retained-parser compatibility vectors as accepted input.
- The behavioral aggregate now compile+executes `tests/unit/test_response.cpp`, covering all five XML specials, TAB/LF/CR, source numeric-reference normalization, and rejection of invalid C0 controls.
- No Unicode Name broadening, entity-policy change, depth-policy change, route-authority change or unrelated TinyXML2 behavior is introduced.
#### F1425-001 XML ASCII Name-start grammar
- Runtime scope is intentionally limited to the residual F1425-001 / PIA315-001 Name-start defect; earlier audit corrections are retained unchanged.
- Element and attribute names now require the retained ASCII XML NameStartChar subset on the first character: `[A-Za-z_:]`.
- Existing continuation policy remains `[A-Za-z0-9_:.\-]`; digits, `-` and `.` are therefore rejected only at name position zero.
- No Unicode-name broadening is introduced and unrelated retained TinyXML2 grammar differences remain intentionally untouched for later review.
- The dedicated F1425 grammar gate now compiles and executes `tests/unit/test_request.cpp`, closing the prior static-only false-confidence gap for this boundary.
- Current release metadata no longer calls the whole start-tag grammar "complete" while separate TinyXML2 parity questions remain open.
#### M315-AUD-003 pre-service-acquire bootstrap unwind
- Scope is intentionally one new finding only: `M315-AUD-003`; the earlier `M315-AUD-001` and `M315-AUD-002` corrections are retained unchanged.
- A failed `acquire_release_service_once` no longer returns while bootstrap-installed host hooks and PTD/FLS authority remain published.
- Dedicated pre-acquire cleanup order is host-facing hooks -> residual published service owner if any -> current-thread PTD detach -> PTD/FLS shutdown.
- Hook teardown failure stops the unwind before service/PTD cleanup so retained runtime authority remains meaningful for explicit `DllShutdown` retry.
- Residual service owner cleanup accepts the expected `not_initialized` pre-acquire state and also closes a published owner if failure occurred after publication but before a usable temporary lease was acquired.
- PTD/FLS cleanup is successful only when the FLS index reaches the uninitialized sentinel; otherwise bootstrap reports `shutdown_failed`.
- Exact Windows release-service acquire-failure -> unwind injection remains pending native acceptance.
#### M315-AUD-002 allocator partial-release retry ownership
- Scope is intentionally one new finding only: `M315-AUD-002`; the earlier `M315-AUD-001` correction is retained unchanged.
- `allocator_shutdown` no longer unconditionally resets the complete allocator region vector after attempting `VirtualFree` on every region.
- Each successful physical region release now retires exactly that region's allocator metadata immediately.
- A failed `VirtualFree` leaves the region base/size and free-range metadata published, so a later explicit shutdown retry attempts only still-owned regions.
- The allocator vector backing storage is reset only when the remaining region count reaches zero.
- `M315-AUD-003` remains intentionally untouched.
- Portable/source validation proves ownership ordering and retry retention structure; exact Windows partial-release fault injection remains pending native acceptance.
#### M315-AUD-001 VEH unregister retry authority
- Scope is intentionally one finding only: `M315-AUD-001`.
- `shutdown_inline_hook_runtime` now treats successful VEH unregistration as the final teardown commit boundary.
- On `RemoveVectoredExceptionHandler` failure, `g_trap_veh` and the trap table remain published for retry; they are cleared only after successful unregister.
- Existing F1443-001 allocator live-ownership fence remains first in shutdown ordering.
- `M315-AUD-002` and `M315-AUD-003` are not modified by this working state.
- Portable/source validation can prove ordering and retained-handle structure; exact Windows unregister-failure -> retry execution remains pending native fault-injection acceptance.
3.1.5 is the consolidated production release of the complete **3.1.4 LEPORVEX –20 remediation line**. Production/runtime source is byte-identical to the canonical 3.1.4 LEPORVEX parent; this release changes package/documentation/QA identity and naming/metadata governance only. The twenty intermediate remediation states are consolidated into one semantic-release point while their exact chronology remains provenance evidence.
### 3.1.4 to MELAVEX non-loss comparison
- Base file count: **486**; /consolidation input file count: **505**.
- Files removed from 3.1.4 base: **0**.
- Files added: **19**, all dedicated regression tests.
- Files changed relative to the 3.1.4 base: **61**; files byte-identical: **425**.
- Production source graph passes in both trees and remains **96/96**.
- The original exports `DllMain @1` and `SURGE @2` are preserved; the remediation line adds `DllShutdown @3` without renumbering the inherited ordinals.
- `surge.cfg` and `surge.png` remain intentionally unbundled by release policy; their DLL-adjacent lookup/fallback code is retained.
### Consolidated remediation payload
The release carries the complete consolidated remediation state in one semantic release, including:
- lifecycle/unload/loader-lock closure: F1412-001, F1412-002, F1413-002, F1413-003;
- worker/network/PTD/allocator robustness: F1415-001, F1415-002, F1416-001, F1416-002, F1421-001, F1421-003;
- Config2/settings/path authority: F1417-001/002/003, F1418-002/003, F1419-001;
- Content and achievement-read semantics: F1435-001/002, F1437-001;
- registry/hook/PE robustness: F1441-001, F1443-001/002/003/004, F1444-001;
- inherited pre-3.1.4 remediation already present in the base: F1420-001/002, F1425-001/002/003, F1426-001/002/003, F1432-001, F1438-001/002, F1446-001/002 and associated active QA convergence.
No new runtime semantic change is introduced by the 3.1.5 packaging step itself. F1429-001 remains intentionally unresolved because retained evidence does not pin the canonical typed-boolean versus raw telemetry value-domain direction. Authority-blocked Phase1449 roots remain untouched.
### Native status
The inherited 3.1.3 MARTAVEX runtime remains the last user-confirmed Windows/native working state. The cumulative LEPORVEX –20 runtime delta, now consolidated as MELAVEX, still requires an exact-package Windows/MSVC compile/link, produced-DLL ARYRA/MUSTAVEX verification, native acceptance and real-host/game regression before MELAVEX can become the new native-confirmed working state.
### Source identity
- Immediate source parent: **3.1.4 LEPORVEX **.
- Production/runtime byte delta from parent: **0** for `src/`, `include/`, `exports/`, `resources/`, `third_party/`, and `CMakeLists.txt`.
- Release identity remains package/documentation-only and is not embedded as a project version/build tag in the shipping DLL.
### Naming provenance
The authoritative reserved namespace appears once at the end of this CHANGELOG and is mirrored by `metadata/LUTRAVEX_GOLDEN_NAMING_REGISTRY.json`; historical release sections do not embed duplicate dumps.
Important lineage records remain unchanged:
- `ARYRA` -> historical 2.6.0 identity -> **ARYRA Golden Layout**.
- `MUSTAVEX` -> historical 2.9.5 identity -> **MUSTAVEX Golden Metadata**.
- `LUTRAVEX` -> historical 3.0.9 identity -> **LUTRAVEX Golden Naming Scheme**.
- `LEPORVEX` -> historical 3.1.4 release identity, now permanently reserved.
## 3.1.4 LEPORVEX
### Consolidated changes
#### F1437-001 QueryAchievements `All` exact text authority
- Closes **F1437-001** by evaluating QueryAchievements `All` from the untrimmed direct-child content span instead of the shared trimmed child-text helper.
- Preserves semantic entity decoding: exact `true` and `tr&#117;e` enable zero-progress records, while leading/trailing whitespace, `True`, `1`, empty, and self-closing values do not.
- Preserves malformed/nested `All` rejection and leaves global child-text behavior for other routes unchanged.
- Adds `test_query_achievements_all_whitespace.cpp`; aggregate behavioral QA now reports **31/31** and closes the whitespace/case/value negative-coverage portion of **F1437-003**.
- Leaves **F1429-001** open: Phase1449 marks remediation-ready, but retained evidence does not pin whether telemetry's canonical shared value domain is raw or typed-boolean.
- Public lifecycle ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Immediate parent: **3.1.4 LEPORVEX **.
#### F1444-001 PE Import Directory extent bounds
- Closes **F1444-001** by bounding `find_pe_import_slot` and `find_pe_import_slots_by_current` descriptor traversal to the PE-declared Import Directory `[RVA, RVA+Size)` extent instead of `SizeOfImage`.
- Rejects an Import Directory whose declared extent exceeds the mapped image and refuses to consume a partial/out-of-directory 20-byte `IMAGE_IMPORT_DESCRIPTOR`.
- Preserves image-wide validation for imported DLL/name/thunk RVAs, because those structures are referenced from the descriptor rather than required to reside inside the descriptor table itself.
- Adds `test_pe_import_directory_bounds.cpp` using the exact Phase1444 `[0x300,0x314)` repro; aggregate behavioral QA now reports **30/30**.
- Public lifecycle ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Immediate parent: **3.1.4 LEPORVEX **.
#### F1443-004 Surge direct-IAT protection recovery
- Closes **F1443-004** by retaining a durable Surge slot ledger when install rollback restores the original function pointer but both attempts to restore the original page protection fail.
- Stores the recovered original protection value and marks protection-only recovery pending; re-install is fail-closed until explicit teardown completes that recovery.
- Preserves normal successful direct-IAT ownership, foreign-pointer non-overwrite behavior, process-wide inline upgrade, and the existing generic-IAT transaction policy.
- Adds `test_surge_iat_protection_recovery.cpp`; aggregate behavioral QA now reports **29/29**.
- Public lifecycle ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Immediate parent: **3.1.4 LEPORVEX **.
#### F1441-001 failed-RegOpen output-handle robustness
- Closes **F1441-001** by consuming retained RegOpenKeyExA/W output storage only after `host_error_success`; failed originals use defined local zero-handle state before policy/pseudo synthesis.
- Preserves original-first ordering, successful real-handle tracking, pseudo-HKEY allocation/classification, local pseudo close, and existing registry teardown fencing.
- Closes the failed-open portion of **F1441-002** active QA gap with `test_registry_failed_open_outparam.cpp` plus an aggregate source guard that fails closed if unconditional `*result` reads return.
- Aggregate behavioral QA now reports **28/28**.
- Public lifecycle ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Immediate parent: **3.1.4 LEPORVEX **.
#### F1435-002 Content descendant-field authority isolation
- Closes **F1435-002** by restricting recognized Content `Name`, `Version`, and `State` authority to direct fields of `Content/<id>` (`depth==2`).
- Preserves Content entry identity/default instantiation: nested-only entries remain valid records with retained Phase209 defaults `GAME / 0 / INSTALLED`, but descendant leaf values are non-authoritative.
- Prevents nested descendants such as `Content/A/Inner/Name` from overwriting valid direct `Content/A/Name` values.
- Completes the descendant-isolation portion of **F1435-003** active QA drift with `test_content_descendant_authority.cpp`; 's explicit-empty regression remains active.
- Aggregate behavioral QA now reports **27/27**.
- Public lifecycle ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Immediate parent: **3.1.4 LEPORVEX **.
#### F1435-001 Content empty-field/default semantics
- Closes **F1435-001** by preserving retained Phase209 Content defaults when Config2 provides explicit empty recognized fields. `Name=""`, `Version=""`, and `State=""` no longer replace `GAME`, `0`, and `INSTALLED`.
- Preserves non-empty direct-field overrides and the existing Content record creation/serialization model.
- Closes the empty-default portion of **F1435-003** active QA drift with `test_content_empty_defaults.cpp`, which exercises the real service-store Config2 adapter.
- **F1435-002 remains open**: deeper descendants under `Content/<id>` are intentionally not changed in this remediation work.
- Aggregate behavioral QA now reports **26/26**.
- Public lifecycle ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Immediate parent: **3.1.4 LEPORVEX **.
#### F1443-002/F1443-003 hook transaction/failure policy
- Closes **F1443-002** by allowing generic IAT recovery to distinguish an already-restored original pointer from a foreign mutation. A failed protection restore keeps durable tracking, and a later uninstall/retry can finish restoring the original page protection instead of becoming permanently stuck.
- Closes **F1443-003** by applying the existing D2/D4 terminal authority to `HookAllocatorReleaseStatus::metadata_exhausted` at setup/install rollback. Metadata exhaustion no longer escapes as `InlineHookInstallStatus::release_metadata_exhausted` and cannot cross into ordinary IAT fallback/recovery.
- Preserves non-terminal behavior for ordinary allocation, relocation, protection, and patch failures; this remediation work does not broaden the terminal class.
- Adds `test_hook_transaction_recovery.cpp` and strengthens `check_hook_terminal_policy.py`; aggregate behavioral QA now reports **25/25**.
- Public lifecycle ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Immediate parent: **3.1.4 LEPORVEX **.
#### F1421-001/F1421-003 Winsock failure authority
- Closes **F1421-001** by preserving the nonzero `WSAStartup` return value as the startup failure authority instead of replacing it with `WSAGetLastError`.
- Closes **F1421-003** by retaining failed cleanup state, retrying `WSACleanup` at the release-server shutdown boundary before service shutdown, and propagating persistent cleanup failure as `BootstrapRunResult::shutdown_failed`.
- Closes the associated **F1421-004** active QA gap with dedicated startup/cleanup failure injection plus bootstrap shutdown-propagation regressions.
- Adds `test_socket_failure_authority.cpp` and `test_bootstrap_shutdown_authority.cpp`; aggregate behavioral QA now reports **24/24**.
- Public lifecycle ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Immediate parent: **3.1.4 LEPORVEX **.
#### F1416-001 generic large-allocation release validation
- Closes **F1416-001** by validating generic >=0x1000 raw/aligned allocation metadata before forwarding a raw pointer to the heap backend.
- Rejects malformed null/forward raw pointers, insufficient metadata distance, recovered padding beyond `0x1F`, non-32-byte user alignment, and logical-size overflow/accounting mismatch with `AllocationReleaseStatus::malformed_large_allocation`.
- Preserves normal small release and valid recovered large-allocation release behavior; malformed metadata produces zero `heap_free` calls instead of an unsafe free.
- Adds `test_allocator_release_validation.cpp`; aggregate behavioral QA now reports **23/23**.
- Public lifecycle ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Immediate parent: **3.1.4 LEPORVEX **.
#### F1413-002/F1413-003 bootstrap failure transaction
- Treats non-`ok` process-environment publication as a fatal bootstrap condition instead of continuing into hook/listener startup.
- Rolls back every environment variable actually written before returning from the failed startup attempt.
- On post-acquire persistent-bootstrap failure, releases the temporary service lease, removes host-facing hooks while backing service/config authority remains alive, then releases the permanent published service owner and clears bound config/store/auth authority.
- If hook teardown or permanent-owner cleanup cannot complete, reports `shutdown_failed` and retains backing authority for a later `DllShutdown` retry rather than claiming successful cleanup.
- Adds `test_bootstrap_failure_unwind.cpp` plus aggregate source-order gates for F1413-002/F1413-003.
- Public lifecycle ABI remains `DllMain @1`, `SURGE @2`, `DllShutdown @3`.
- Immediate parent: **3.1.4 LEPORVEX **.
#### F1417-002/F1417-003 Config2 parser hardening
- Closes **F1417-002** by limiting flat runtime-setting publication to direct leaves under recovered top-level sections. Nested collection/object leaves remain structurally parseable but are non-authoritative and cannot collide with direct settings.
- Closes **F1417-003** by treating unterminated `/* ... EOF` as a lexical error. Candidate state is rejected transactionally, preserving the previously committed configuration.
- Preserves direct-section authority, duplicate direct-leaf last-write-wins behavior, valid line/block comments, and the lifecycle ABI (`DllMain @1`, `SURGE @2`, `DllShutdown @3`).
- Adds `test_config_parser_hardening.cpp`; aggregate behavioral QA now reports **21/21**. Windows/MSVC/native rerun remains required for cumulative –10.
#### F1412-002 loader-lock hygiene
- Closes **F1412-002** at source/QA level by reducing `DllMain` to loader bookkeeping plus one nonblocking bootstrap dispatch.
- Moves PTD/FLS initialization, reconstructed/debug identity initialization, main-module hook installation, process-wide hook upgrade, and CryptQueryObject observer installation into the post-loader bootstrap worker.
- Removes hook uninstall, inline-hook runtime shutdown, and PTD/FLS teardown from the loader-lock detach path; `DllShutdown` remains the outside-loader-lock teardown owner.
- Replaces suspended worker creation + `ResumeThread` with one nonblocking `CreateThread` dispatch while preserving automatic startup and module self-pin lifetime.
- Renames `preloader_l_shutdown @3` to **`DllShutdown @3`**. Ordinal 3 and shutdown ABI/semantics are unchanged; `DllMain @1` and `SURGE @2` remain unchanged.
- Aggregate behavioral QA now reports **20/20** and fails closed if heavy loader-lock operations, suspended/resume dispatch, or the superseded active export name reappear.
- Windows/MSVC/native rerun remains required for cumulative –09 runtime changes.
#### F1412-001 explicit unload lifecycle
- Closes **F1412-001** at source/QA level by adding an outside-loader-lock production lifecycle owner.
- Adds `preloader_l_shutdown @3` without changing `DllMain @1` or `SURGE @2`. A zero return is the safe-to-unload contract; nonzero statuses fail closed and remain retryable.
- The bootstrap worker no longer closes its own thread HANDLE. The external lifecycle owner waits on the real kernel thread object, proving worker exit and self-pin release rather than trusting the pre-exit `completed` flag.
- Teardown order is stop/quiesce -> hook uninstall/runtime shutdown -> permanent service-owner release -> PTD/FLS cleanup. Retained hook ownership therefore keeps shared service/config authority alive on failure.
- `DllMain(DLL_PROCESS_DETACH)` recognizes a completed external shutdown and does not duplicate hook/PTD teardown. F1412-002 loader-lock attach/detach hygiene remains open.
- Adds `test_shipping_shutdown_contract.cpp`, aggregate F1412 gates, and a native-acceptance `shipping_shutdown` scenario that calls the production export and then `FreeLibrary`.
- Windows/MSVC/native rerun remains required for cumulative –08 runtime changes.
#### F1415-001/F1415-002 shutdown liveness
- Closes **F1415-001** and **F1415-002** at the current shipping Windows worker/listener boundary without changing recovered service/jthread physical ABI or public DLL exports.
- Native listener and accepted client sockets are explicitly placed in nonblocking mode with `ioctlsocket(FIONBIO)`.
- Worker send/receive retains the existing 100 ms `select` readiness wait; nonblocking `send`/`recv` can now return `WSAEWOULDBLOCK` instead of entering an unbounded blocking syscall after readiness races or backpressure.
- Accept ownership now uses nonblocking listener + bounded readiness polling. Quiesce publishes inactive/stop state, joins the accept owner, and only then closes the listener, removing the previous concurrent `closesocket`-during-`accept` dependency.
- Adds `test_worker_shutdown_liveness.cpp` plus aggregate/static liveness gates. **F1415-004** remains a native acceptance coverage item until the fresh Windows run exercises the cumulative runtime delta.
- Windows/MSVC/native rerun remains required for cumulative –07 runtime changes.
#### F1419-001 Unicode-safe DLL-relative paths
- Closes **F1419-001** without changing the public DLL export ABI.
- Canonical startup `surge.cfg` lookup now acquires the loaded DLL path with `GetModuleFileNameW`, constructs the sibling path in UTF-16, and opens it through `CreateFileW`; the active startup path no longer depends on the Windows ANSI code page.
- QueryImage's DLL-adjacent `surge.png` fallback now acquires the module path in UTF-16 and materializes the exact sibling path as UTF-8 via `WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS)` before the existing semantic XML attribute escaper.
- Adds `test_unicode_sibling_paths.cpp`, exercising a controlled DLL directory containing both non-ACP `ğ` and XML-special `&`; active avatar/minimal-fixset QA no longer requires the ANSI module-path API.
- `surge.cfg` and `surge.png` remain intentionally unbundled; lookup/fallback semantics remain DLL-adjacent. Windows/MSVC/native rerun remains required for the cumulative –06 runtime delta.
#### F1418-003 shared language authority convergence
- Closes **F1418-003** without changing the public DLL export ABI.
- The existing exact `-emuGameLang <value>` process authority is now reverse-published into shared `SettingKey::language` during bootstrap after Config2 load.
- `GetGameInfo(INSTALLED_LANGUAGE)`, `GetSetting(LANGUAGE)`, aggregate `GetSettings.Language`, and host `EAGameLocale` publication now consume one shared language authority.
- CLI language retains precedence over Config2 `Game.Language`; no CLI override retracts the runtime override and restores Config2 authority; missing CLI + missing Config2 retains the inherited `en_US` consumer fallback.
- Repeated exact `-emuGameLang` switches preserve the existing last-write-wins parser semantics.
- Removes the selector-local direct CLI query so `INSTALLED_LANGUAGE` can no longer diverge from the shared provider.
- Adds `test_language_authority.cpp`, which executes the real command-line parser with controlled Win32 stubs and validates Config2 fallback, CLI publication, last-write-wins, retraction, LSX convergence, and `EAGameLocale` convergence. Aggregate behavioral QA now includes the same authority boundary.
- Windows/MSVC/native rerun remains required for the cumulative –05 runtime delta.
#### F1418-002 sole-Online manual-offline authority
- Closes **F1418-002** without changing the public DLL export ABI.
- `GetSetting(IS_MANUAL_OFFLINE)` and aggregate `GetSettings.IsManualOffline` now derive exclusively from canonical section-isolated Config2 `Emulator.Online`.
- The runtime settings provider no longer publishes Config2 `EALaunchOfflineMode` as `SettingKey::launch_offline_mode`; the legacy Config2 token remains parse-compatible but cannot override Online.
- Host process `EALaunchOfflineMode` remains an environment-variable projection derived from Online, so LSX connectivity/manual-offline and host environment state now share one authority.
- Contradictory legacy combinations are regression-locked in both directions: online + legacy true stays manual-offline=false; offline + legacy false stays manual-offline=true.
- F1418-003 (`-emuGameLang` reverse publication) remains open and is intentionally outside this bounded remediation work.
- Adds `test_manual_offline_authority.cpp`; aggregate behavioral QA now includes the exact duplicate-authority regression. Windows/MSVC/native rerun remains required for the cumulative –04 runtime delta.
#### F1417-001 Config2 section authority isolation
- Closes **F1417-001** without changing the public DLL export ABI.
- Runtime Config2 publication is now section-aware for shipping authority keys: `Game`, `Emulator`, `User`, and `Achievements` keep distinct top-level authority domains before values enter the existing flat runtime setting store.
- Root-level leaves and unknown top-level sections remain lexically parse-compatible but are non-authoritative; they cannot publish or overwrite runtime settings.
- Reserved shipping keys are accepted only from their owning section. This blocks `User.Online`, `Bogus.Online`, root-level `Online`, and `User.GameName` poisoning while preserving intended `Emulator.Online` and `Game.Name -> GameName` behavior.
- F1417-002 nested-subtree flattening and F1417-003 unterminated block-comment handling remain open and are intentionally outside this bounded remediation work.
- Adds `test_config_section_authority.cpp`; aggregate behavioral QA now includes the exact wrong-section/root-level/alias-overwrite regression surface. Windows/MSVC/native rerun remains required for the cumulative –03 runtime delta.
#### F1416-002 retryable PTD/FLS cleanup
- Closes **F1416-002** without changing the public DLL export ABI.
- `FlsFree` failure during PTD runtime shutdown no longer clears `g_fls_index`, platform operations, or shared bindings. The exact live FLS ownership remains represented and teardown can be retried.
- Initialization rollback now distinguishes cleanup-pending from initialized state. If rollback `FlsFree` fails, a later initialize call retries the retained cleanup first and only allocates a new FLS slot after that cleanup succeeds.
- Global PTD runtime state is reset only after successful `FlsFree`; 's F1443-001 retained-trampoline shutdown fence is inherited unchanged.
- Adds `test_ptd_flsfree_failure.cpp` and aggregate regression coverage for failed-shutdown retry plus failed-initialization-rollback retry. Windows/MSVC/native rerun is required for the cumulative +02 runtime delta.
#### F1443-001 retained-trampoline shutdown fence
- Closes **F1443-001** at the shipping inline-hook runtime boundary without changing the public DLL export ABI.
- Adds a serialized allocator lifecycle fence and exact live-trampoline allocation count. New hook allocations are rejected once runtime shutdown begins.
- A failed uninstall/release that retains trampoline ownership now makes `shutdown_inline_hook_runtime` fail closed **before** VEH removal, trap-table clearing, or `VirtualFree` of allocator regions. The retained runtime remains intact for a later uninstall/shutdown retry instead of creating a live-detour-to-freed-trampoline UAF.
- Successful shutdown still releases all allocator regions and then removes the patch-trap runtime; / publication and in-flight quiescence semantics are preserved.
- Aggregate behavioral QA adds an explicit F1443-001 ownership/order regression. Windows/MSVC/native rerun is required because this remediation work changes shipping Windows hook-runtime code.
3.1.4 is the consolidated production release of the cumulative MARTAVEX remediation line. **Production/runtime source and physical hook/runtime behavior are byte-identical to native-confirmed 3.1.3 MARTAVEX remediation work 21.** This release changes package/documentation/QA identity only, restores provenance/hygiene detail that had been over-compressed after 3.1.0, and replaces the per-remediation work CHANGELOG stack with one semantic-release record.
#### Runtime and lifecycle remediation inherited from MARTAVEX
- **F1420-002 registry teardown safety:** pseudo-HKEY publication is fenced during unhook; teardown refuses while pseudo handles remain outstanding; failed/partial uninstall keeps the fence engaged for retry.
- **F1420-001 environment rollback:** only process environment variables actually created by this DLL are removed; pre-existing host values are preserved; failed/incomplete restoration converts bootstrap success to shutdown failure.
- **F1425-001 XML start-tag grammar:** opening-tag structure and retained ASCII Name-start rules are validated before exposing spans; malformed trailing attributes/separators/quoting/self-close syntax and invalid first-name characters are rejected. Broader TinyXML2 grammar parity remains a separate audit concern.
- **F1425-003 document depth:** document-wide nesting matches the retained TinyXML2 boundary on the audited wrapped request (495 accepted, 496 rejected).
- **F1425-002 semantic entity consumption:** parser-owned source spans remain bounded/non-owning, while audited shipping semantic consumers decode entities exactly once; malformed entities fail request processing.
- **F1426-001/F1426-002 XML attribute serialization:** source-owned id/sender values normalize entity spelling exactly once; semantic runtime/config/service values escape all five XML attribute specials without structural injection or double escaping.
- **F1426-003 serializer capacity accounting:** StreamWriter-backed serializers continue full logical-length measurement after capacity exhaustion, invalidate undersized output as an empty C string, and require `required+1` bytes for successful NUL-terminated render.
- **F1432-001 GetAuthCode transactionality:** a fresh Auth8 candidate is staged under the session lock and committed only after successful serialization; measurement/tiny/exact-capacity failures do not create session state.
- **F1438-001 GrantAchievement failure atomicity:** response render and all fallible allocation/staging complete before no-fail persistent store/FIFO commit; failed render/event preparation/watcher rejection leaves internal achievement state and completion FIFO unchanged.
- **F1446-001 inline-hook publication ordering:** trampoline/original forwarding authority is published before target patch activation; clean failure retracts publication, while rollback-incomplete live-detour ownership retains forwarding authority.
- **F1446-002 successful-uninstall quiescence:** per-hook active-call gates drain in-flight replacement calls before target restore/trampoline release; late already-dispatched wrappers forward to the restored direct target. `InlineHookTicket` remains physically 0xA8. CryptQueryObject uses a call-through wrapper so its lease spans the complete original/trampoline call.
#### Windows/MSVC shipping-build convergence inherited from MARTAVEX
- Shipping link policy is fail-closed around `/DEBUG:NONE` and `/INCREMENTAL:NO`; `/Brepro` and other `/DEBUG*` forms are rejected.
- NMake/CMake audit no longer assumes `link.txt`; it can derive the target-local command from `build.make` and target response files.
- `/LTCG:OFF` and `/NOCOFFGRPINFO` suppress the observed POGO Debug Directory while preserving MUSTAVEX's zero-Debug-Directory policy.
- `/MANIFEST:NO` prevents the linker-generated `RT_MANIFEST` resource rather than weakening the MUSTAVEX resource allowlist.
- ARYRA Golden Layout and MUSTAVEX Golden Metadata remain the unchanged post-link acceptance authorities.
#### QA and authority convergence inherited from MARTAVEX
- GetAuthCode authority is machine-readable in `RELEASE_STATE.json`: project-adopted local session Auth8, 8 alphanumeric characters, session-stable after successful commit, Config2-independent, no ClientId semantic/entropy authority, and no donor/primary/backend-validity equivalence claim.
- Deterministic QA executes both transactional Auth8 rollback/commit behavior and the real `_WIN32` entropy branch with controlled Windows entropy stubs.
- GrantAchievement end-to-end QA covers size-query/tiny/exact-capacity failure, retry idempotency, completion-event allocation/FIFO consistency and watcher rejection; stale v306 authority checkers remain excluded.
- Inline-hook QA fail-closes publication-after-patch regressions and validates the real quiescence backend active-call accounting contract.
- remediation work 19 was user-confirmed native; the cumulative / inline-hook delta is now also **user-confirmed to compile and run natively without observed issues**. 3.1.4 itself preserves those production/runtime bytes unchanged.
### Documentation/provenance restoration in 3.1.4
No runtime defect was found in this comparison. The following documentation detail from 3.1.0 had been compressed out of later package text and is restored:
- retained primary reference identity `blaster12_1.dll` SHA-256;
- explicit statement that donor/reference binaries are provenance inputs, not runtime dependencies;
- detailed release-hygiene removed/retained payload inventory and the `runtime_memory.cpp` -> `freestanding_memory.cpp` relocation rationale;
- native acceptance requirement to confirm absence of unexpected runtime donor-DLL dependencies;
- failure-report checklist in `BUILD_ME_FIRST.txt`.
`surge.cfg` and `surge.png` remain intentionally unbundled; their DLL-adjacent runtime lookup/fallback code is retained. Their absence is policy, not documentation or runtime loss.
### Source identity
- Immediate source parent: **3.1.3 MARTAVEX **.
- Production/runtime byte delta from parent: **0** for `src/`, `include/`, `exports/`, `resources/`, `third_party/`, and `CMakeLists.txt`.
- Release identity remains package/documentation-only and is not embedded as project version/build tag in the shipping DLL.
### Naming provenance
The authoritative reserved namespace appears once at the end of this CHANGELOG and is mirrored by `metadata/LUTRAVEX_GOLDEN_NAMING_REGISTRY.json`; historical release sections do not embed duplicate dumps.
Important lineage records remain unchanged:
- `ARYRA` -> historical 2.6.0 identity -> **ARYRA Golden Layout**.
- `MUSTAVEX` -> historical 2.9.5 identity -> **MUSTAVEX Golden Metadata**.
- `LUTRAVEX` -> historical 3.0.9 identity -> **LUTRAVEX Golden Naming Scheme**.
- `CORVEXON` -> 3.1.1 canonical identity; 3.1.2 CORVEXON naming-only artifact is superseded by canonical 3.1.2 VULPHEXON.
## 3.1.3 MARTAVEX
### Added
- Established **LUTRAVEX Golden Naming Scheme** as the permanent naming-governance authority.
- Replaced the independent-namespace model with one global reserved root namespace plus explicit derived lineage.
- Preserved **ARYRA Golden Layout**, **MUSTAVEX Golden Metadata**, and **LUTRAVEX Golden Naming Scheme** as intentional provenance-preserving inherited authorities rather than collisions.
- Added `metadata/LUTRAVEX_GOLDEN_NAMING_REGISTRY.json` and a fail-closed naming QA gate.
- Corrected the earlier 135-identifier audit by adding omitted release roots **CUNYRA**, **FALCORYNTH**, **LYNXAVEX**, **MUSTELDRAX**, plus verified internal historical roots used as codename-like identities.
### Preserved
- No intended runtime or production-source semantic change from 3.1.2 VULPHEXON.
- ARYRA and MUSTAVEX post-link authority contracts remain unchanged.
- Historical naming anomalies are retained as provenance and are not rewritten.
### Naming registry transition
3.1.3 established the corrected 150-root historical baseline and assigned `MARTAVEX`. The current authoritative root registry is carried by the latest release entry and `metadata/LUTRAVEX_GOLDEN_NAMING_REGISTRY.json`. ARYRA, MUSTAVEX and LUTRAVEX lineage remains unchanged.
## 3.1.2 VULPHEXON
### Added
- Restored **ARYRA Golden Layout** as a permanent fail-closed post-link gate for the accepted AMD64/PE32+ six-section structural profile: `.text`, `.rdata`, `.data`, `.pdata`, `.rsrc`, `.reloc`.
- Added **MUSTAVEX Golden Metadata** as a separate read-only fail-closed shipping-metadata verifier.
- Added explicit authority files under `metadata/` so golden values are separated from verifier implementation.
### Preserved
- No intended production/runtime semantic change from 3.1.1.
- Exact PE RVA/size/raw-offset/layout parity remains explicitly outside ARYRA and metadata recovery scope.
- The 3.1.1 strict metadata allowlist, exact TimeDateStamp, stripped RSDS/REPRO policy, Authenticode exclusion, external project-release identity and unbundled `surge.cfg` / `surge.png` policy remain unchanged.
## 3.1.1 CORVEXON
### Changed
- Added the strict recovered PE metadata profile for the approved allowlist only: VERSIONINFO strings/fixed fields/translation, AMD64 PE32+, Windows GUI subsystem, OS/subsystem 6.0, linker version 14.0, exact TimeDateStamp `0x7669FC7A`, internal export name `preloader.unsigned.dll`, and Rich Header absence.
- Shipping DLL debug metadata is stripped: RSDS and REPRO are intentionally absent; no PDB is distributed.
- Project release identity is externalized: `3.1.1 CORVEXON` exists only in package/release documentation and is not embedded as a project version/tag in the DLL.
- Added a VERSIONINFO-only resource; no manifest/icon/other recovered resource class is added.
- Removed packaged `surge.cfg` and `surge.png` assets while retaining their existing DLL-adjacent runtime lookup/fallback code paths.
- Added PE-aware allowlist finalization and verification gates.
### Runtime
- No intended runtime semantic change from 3.1.0.
## 3.1.0 LUTRAVYX
### Changed
- Consolidated release documentation into `README.md`, `BUILD_ME_FIRST.txt`, `CHANGELOG.md`, `RELEASE_STATE.json` and a small `docs/` set.
- Removed historical `PACKAGE_STATUS_*`, `QA_*`, `FEATURE_COMPLETENESS_*`, `QUALITY_PROMOTIONS_*`, `PHASE*`, per-version report and evidence clutter from the FULL SOURCE package.
- Reduced the shipped QA script surface to active release/build/semantic checks instead of carrying every historical version gate.
- Moved the required freestanding `/NODEFAULTLIB` `memset`/`memcpy` shim from `src/debug/runtime_memory.cpp` to `src/runtime/freestanding_memory.cpp`; behavior is unchanged.
- Corrected stale README/test/native-acceptance metadata inherited from older releases.
- Consolidated project release identity in package/documentation surfaces; later releases fully externalize it from the DLL.
### Runtime
- No intended runtime semantic change from the final 3.0.9 state.
## 3.0.9 LUTRAVEX
### Final accumulated state
- Connectivity authority uses `Emulator.Online`; exact `false` selects offline and exact `true` selects online, while missing/unrecognized values retain the inherited default-enabled policy.
- Legacy `PretendConnected`, `FakeAuth` and authentication-success latch are not global connectivity authority in this line.
- Startup configuration is `surge.cfg` / Config2.
- `surge.cfg` lookup is DLL-directory-only in the final 3.0.9 source.
- `GetAuthCode` final state uses a session-stable 8-character `[0-9A-Za-z]` token, independent of `ClientId`.
- AuthCode state resets with the release-service lifetime.
- QueryImage no-Avatar fallback is `surge.png` beside the loaded DLL.
- The failed experimental early-Online-preflight / runtime-shadow branches are not part of the final canonical 3.0.9 source.
## 3.0.8 WORKING CLEAN FIXSET
- Internal clean working baseline used to reconstruct 3.0.9.
- Preserved the pre-3.0.9 connectivity/profile model used for comparison and rollback work.
## 3.0.7 FALCYNEX
- Removed trace implementation and other diagnostic/reconstructed bookkeeping from the canonical production graph while keeping an explicit optional debug target.
- Preserved primary-backed `CryptQueryObject` production behavior.
- Removed the unproven 4 MiB plaintext-response limit.
- Removed clean-only 128-byte Config2 canonical-key and Language ceilings in favor of exact-size storage.
- Retained donor-derived/project-adopted GrantAchievement behavior while recording the primary conflict separately.
## 3.0.6 ACCIPHEXON
- Corrected profile connection handling and GetAuthCode service delegation boundaries.
- Corrected QueryImage provider-disabled behavior and removed the `AvatarImage` alias from configured authority.
- Restored donor-exact FakeAuth handling without inventing a live online producer.
- Serialized measure/allocation/render under one outer transaction to prevent state drift.
- Corrected RegQueryValueEx A/W capacity handling.
- Hardened LSX XML framing against mismatched/trailing nested command structure.
- Added UTF-8-authority to UTF-16 case-insensitive registry matching.
- Disabled release trace implementation in production and retained diagnostics only in the optional debug target.
## 3.0.5 FELORYNTH
- Corrected the freestanding `__chkstk` link regression by removing large GrantAchievement route-local serializer buffers from the stack.
- Preserved `/Zl /NODEFAULTLIB` without adding CRT/UCRT helper dependencies.
- Retained the clean-source offline architecture and no runtime donor-DLL dependency.
## 3.0.4 CORVIONYX
- Integrated optional online companion/authentication lifecycle work, GetAuthCode service delegation, QueryImage provider materialization boundaries, GrantAchievement event/watcher boundaries and persistent unsupported-route continuation.
- This architecture was subsequently superseded by the corrected offline clean-source line.
## 3.0.3 MUSTELDRAX
- Raised the Config2 donor-service adapter depth from 8 to the canonical depth-32 contract.
- Kept QueryImage corrective research out of shipping source where provider authority remained incomplete.
## 3.0.2 LUTRAXION
- Corrected QueryImage outer request/provider semantics: required Width/Height/ImageId, no-colon ImageId continuation, signed Width handling and project fallback behavior.
- Corrected GrantAchievement signed numeric fields and mutation/completion ordering.
- Tightened quality-override provenance classification.
## 3.0.1 LYNXADRON
- Replaced raw/synthetic response approximations with typed service-owned content, entitlement and achievement stores where donor evidence closed the observable contract.
- Added exact QueryEntitlements synthetic identifier sequencing.
- Added persistent achievement mutation semantics.
- Added the bounded ShowIGO event queue behavior.
## 3.0.0 FELIVEXON
- Replaced the synthetic startup `Online` assumption with the recovered connectivity model based on authentication success, `PretendConnected` and `FakeAuth`.
- Kept `surge.cfg` as the startup configuration format.
## 2.9.9 NAUTILKHEON
- Replaced legacy startup XML configuration with donor-style Config2 under the project filename `surge.cfg`.
- Removed the `surge.xml` compatibility/migration path.
- Preserved LSX XML and Store XML where XML is the actual protocol/data format.
- Consolidated SURGE identity and profile/request representation corrections accumulated during the version.
## 2.9.8 ARANEZOLTH
- Implemented recovered GetProfile field order and required index gate.
- Applied profile identity/metadata authority mapping.
- Bound request `recipient` to Response `sender` authority.
- Preserved persistent LSX session continuation after unsupported routes without fabricating a response.
## 2.9.7 FALCORYNTH
- Promoted the four bounded quality corrections: strict GetSetting, strict GetGameInfo, minimal QueryFriends success and exact GetConfig service manifest.
- Avoided wholesale donor behavior promotion.
## 2.9.6 VULPORYX
- Identity/version release over MUSTAVEX with no intended runtime semantic delta.
## 2.9.5 MUSTAVEX
- Removed the obsolete WinMM proxy/resolver layer from shipping behavior.
- Consolidated settings-authority consistency and primary import/lifecycle parity corrections.
- Adopted the approved handshake greeting and GetAuthCode quality overrides.
- Kept direct `CRYPT32!CryptQueryObject`; production WS2 import surface excluded `getsockname` and `shutdown`.
## 2.9.4 PARDAVEX
- Made crypt diagnostic IAT observers reversible and teardown-safe.
- Corrected partial hook rollback ownership and released-trampoline pointer cleanup.
- Preserved allocator ownership on snapshot-release failure.
- Interlocked crypt telemetry counters and one-shot identity logging.
- Restricted default crypt observation to the primary-backed `CryptQueryObject` path.
## 2.9.3 URSYNEX
- Consolidated the Phase 1–170 authority-filtered research sequence and resolved the supplemental container-campaign phase-ID collision without rewriting historical artifacts.
- No runtime semantic delta solely from the research consolidation.
## 2.9.2 VULPHEXIS
- Advanced the container-primary research model through the Phase 101–110 campaign.
- Kept shipping runtime behavior unchanged aside from release identity/research metadata.
## 2.9.1 LYNXAVEX
- Native-validation/second-check roll-forward over CORVEXIS.
- No runtime semantic delta beyond release identity.
## 2.9.0 CORVEXIS
- Closed the freestanding Windows link surface: fatal-policy/exception runtime wiring, `_fltused` avoidance and heap-backed large worker protocol state.
- Completed a real Windows production compile/link and ARYRA/PDB/data-footprint acceptance for the recorded parent snapshot.
## 2.8.9 LUPHORYX
- Added exact release identity, authority-filtered Phase 1–100 integration ledger and retained-evidence identity reconciliation.
- Quarantined unmaterialized alternate identities from canonical promotion.
## 2.8.8 VELORYX
- Corrected TinyXML `StrPair` and XML object geometry for the retained config path.
- Canonicalized all 22 primary LSX route dispatch name/hash/RVA identities.
- Formalized payload/container/donor authority classes.
## 2.8.7 ORNORYX
- Tightened build/repository identity validation and separated portable/unit syntax from production Windows syntax gates.
- Consolidated the Settings/worker/UTF-16 path/XMLDocument authority metadata accumulated during the version.
## 2.8.6 NEXORYX
- Converged service worker ownership on recovered jthread/vector/mutex semantics.
- Added dynamic client-thread storage and recovered worker closure/lifetime behavior.
## 2.8.5 MERLORYX
- Recovered Settings unordered-map semantic ABI: FNV-1a lookup, duplicate update, growth/rehash and destruction behavior.
## 2.8.4 CAPRORYX
- Removed multiple clean-only hard caps from Config2/Store/settings/path handling.
- Added dynamic storage for settings, Store records/features, scratch arenas and current-directory capture.
- Raised parser depth to the recovered TinyXML2 500 boundary.
## 2.8.3 FALCORYX
- Established the early runtime/acceptance correction baseline used by the later semantic-ABI convergence line.
## Machine-readable reserved namespace

```text
ACCESSORS
ACCIPHEXON
ACCIPTRIX
AEGIS
AQUILIX
AQUILORYX
AQUORIN
AQUYRA
ARANEZOLTH
ARYRA
ATTRS
AUTHORYX
AXIS
BOREAL
BOVYRA
BUILDER
CALLBACK
CANVYRA
CAPRORYX
CAPRYRA
CASTORADRON
CERVRAX
CERVYRA
CHAIN
CINDER
CIVETAVEX
CONTEXT
CORVATRIX
CORVEXIS
CORVEXON
CORVIONYX
CORVODRAX
CORVYRA
CUNYRA
D7
DELPHYRA
DRACO
ECHO
EQUYRA
FAILSAFE
FAILURES
FALCION
FALCORYNTH
FALCORYX
FALCYNEX
FALCYRA
FALVEX
FELIVEXON
FELNIX
FELORYNTH
FELYRA
FIELDS
FORGE
FORMATTER
GENETAVEX
GLAIVE
HANDLERMAP
HEAP
HELIX
HELPERS
ION
JAVELIN
KESTREL
LANCER
LEONYRA
LEPORVEX
LUPHEON
LUPHORYX
LUPRAX
LUPYRA
LUTRAVEX
LUTRAVYX
LUTRAXION
LYCION
LYNCYRA
LYNX
LYNXADRON
LYNXAVEX
LYNXOR
LYNXORYX
MANTIS
MARTAVEX
MATERIALIZERS
MATRIX
MELAVEX
MELOTRIX
MERLORYX
MUSTAVEX
MUSTEL
MUSTELDRAX
MUSTORIN
NAUTILKHEON
NEXORYX
NEXUS
NEXYRA
NOVA
ORACLE
ORBIT
ORNORYX
OVYRA
PANTHORIX
PANTHYRA
PARDAVEX
PARDYX
PARSER
PATH
PHOENIX
POLICY
PORCYRA
PRIMITIVES
PRISM
PROFILE
PTDINIT
PTDTEARDOWN
PULSE
QUARTZ
RANQUEL
RAPTYRA
RAVEN
RELAY
RENDERERS
REPLAY
RETURN
ROUTES
SABLE
SCIURONYX
SELECTORS
SENTINEL
SINKS
SKYSECT
SPINE
SQUALERIS
STATECHAIN
STREAM
STRIXON
SUBMIT
TALON
TAURYRA
TIGRYRA
TRACER
UMBRA
UNWIND
URSORYX
URSVEX
URSYNEX
URSYRA
VARYRA
VECTOR
VEIL
VELORYX
VESPERA
VIGIL
VIVERRAVEX
VSTREAM
VULPATRIX
VULPAX
VULPHEXIS
VULPHEXON
VULPORYX
VULPYRA
WARDEN
WRAITH
ZENITH
```

## HF08 W06 (NONRELEASE SOURCE CANDIDATE; 2026-10-11)

Immediate parent is **W05 SHA-256 `f7d86345a682c7c700cf6ed6f835996112da5c70478d8c3ede3bb5e466e09973`**; exact original release ancestor is **HF07 SHA-256 `a21490ffe382508d75746ad27bca219d74bc3a42436d1dbc30a23e2e4bdb689f`**. No HF06 fallback.

**Production integration progress:** Host Config2 `UserId` is the local-only trust anchor after the successful loopback LSX handshake, never inbound `Request/@UserId` or peer IP. A per-session profile-generation lease is held in `ExchangeState`, old leases fail when Config2 reloads. Shared in-process LocalSocialStore has an atomic-flag serialization owner. Unknown local contacts are created only during successful store commit and not by prepare/rollback. A shipping socket transaction branch owns prepare, render, encode, send_all and post-send commit/rollback for a future social route lookup. Single-Invitees direct-child local-only SendInvite is bounded; multi-invitee requests currently fail-closed.

**NOT ACTIVATED:** IDs 38–42 still unregistered; no automatic 42/42 claim. QueryFriends/GetBlockList still use original fixed-empty responses; activating mutations before local readers and exact Consumer XML golden converge would be a coherence defect. No remote social service, ACK or delivery guarantee; post-send failed commit is indeterminate. Windows/MSVC/native/CRT/game acceptance remains NOT_RUN. This ZIP must not be published as FINAL HOTFIX08.


## HF08 completion — committed social reader guard (NONRELEASE, 2026-10-11)

Immediate working parent: HF08 completion trust-hardened full source ZIP SHA-256 `0367572c1104b5e8ff1300d7c22022344e787e00cee98145f7ad4156e629c6e1`; original HF07 release ancestor remains `a21490ffe382508d75746ad27bca219d74bc3a42436d1dbc30a23e2e4bdb689f`. No baseline substitution.

When a real socket dispatch carries a Config2-bound `LocalSocialSession`, `QueryFriends` and `GetBlockList` consult the same committed LocalSocialSnapshot via the serialized host reader. Valid empty snapshots preserve the previously verified exact empty response. Stale/untrusted snapshots and nonempty lists do not fall through to the fixed empty Success response. Until exact populated OriginFriendT XML wire grammar is independently confirmed, nonempty snapshots fail closed (`unsupported_route`) rather than synthesizing incompatible payloads. Direct/debug dispatch without a bound session keeps prior source behavior.

Test-only snapshot regression covers precommit invisibility, committed block visibility, route-specific choice, stale/untrusted failures and forged synthetic-contact lease. Live 5 social mutation routes remain shipping-unregistered (37/42) and this checkpoint is NOT a final HOTFIX08; Windows/MSVC/native/CollegeFB27/Madden27 acceptance not run.

### HF12 experimental Asio IOCP continuation Stage03 (source-only, NOT_RELEASE)
- Preserve exact Stage02A FIX01 parent and user-confirmed Windows x64 four-way
  ingress/continuation PASS. Add source-shared full Response/Event write-completion
  gate and tests; preserve Grant response commit / event retry / social full
  transaction ordering. Shipping transport remains WinSock. No new production
  async dispatcher/session-owner or release authorization.

## HF12 IOCP-owned Stage04A source integration — 2026-10-14 (NOT_RELEASE)

- Exact parent Stage03A source ZIP SHA-256 09b4afb05167aa596ec250bffcf9c409558e55f09ebfa9367ad6059f91620894; retains user-confirmed six x64 /MT+/MTd IOCP native tests, without inheriting them as Stage04A runtime evidence.
- PreparedResponseTransaction retains real response/event queue, GrantAchievement, local social reservations and both encoded frame buffers across possible async completion callbacks. WinSock shipping now consumes the same owner and completion path, not a second transaction algorithm. Allocation uses production retry-aware transient allocator.
- Test-only Stage04A source-linked callback and early-destruction model under GCC and Clang; real Windows async store/FIFO, Asio shipping listener, shutdown/failure injection, native/game acceptance and full eight-shard QA pending. IOCP always enabled; never use ASIO_DISABLE_IOCP; NOT_RELEASE.

HF12 STAGE04B RESEARCH/IMPL: source-shared async transaction completion driver
retains real Stage04A PreparedResponseTransaction across async Response/Event
write callbacks, cancellation intent, retry and terminal completion. IOCP remains
mandatory. Production listener is still WinSock; no complete shipping Asio
transport, native x64 Stage04B run, final Golden QA or game acceptance claimed.

### HF12 Stage04B Windows x64 native owner link FIX01 — NOT_RELEASE (2026-10-14)

From immutable Stage04B SHA-256 `08907b90e0dccaa329a05dbc11a524b9d8ba973713cc09f4435f1878003f6a0b`.
Windows x64 VS2026 `owner_mt` failed at LNK2019/LNK1120 (13 missing symbols), all from the native test including the complete production Response preparation/dispatcher translation unit. Compilation of IOCP test and actual async driver succeeded; owner_mtd was not run. Root cause is test source closure, not evidence of an IOCP runtime fault.

FIX01 keeps the single real production transaction implementation but separates the byte-identical Response/Event commit/retry and owner teardown functions into `socket_exchange_transaction_delivery.cpp`, with shared private `socket_exchange_transaction_owner_private.hpp` giving the identical owner layout to the production preparation and completion TUs. Both production and native CMake graphs include the real completion source. Native owner test includes this exact private header and links the actual delivery/allocator/PTD sources; no dispatcher or fake test-only handler/allocator is inserted to suppress these LNK2019 errors. Existing bounded external store/FIFO *recorders* remain as they were in Stage04B; true dispatcher/store native acceptance is still open.

IOCP remains mandatory; no `ASIO_DISABLE_IOCP`, reactor fallback, socket.release() or changes to upstream Asio, 548-byte ABI, exports or official release state. The old Stage04B source is immutable and superseded only as a test/link candidate. Actual FIX01 Windows x64 `/MT` and `/MTd` acceptance remains NOT_RUN until MSVC rerun.

### HF12 Stage04C Real FIFO IOCP Native Acceptance Source Candidate — NOT_RELEASE (2026-10-14)

- Research/acceptance-only delta from exact Stage04B FIX01 source SHA-256 `a6e6a266a1d44ece6a45834ede4d82c6874eed06c70333cf16fd3b6a7cc8f80a`.
- Added real `event_queue.cpp` source-linking to Asio-owned IOCP completion native FIFO test, plus GCC/Clang real-FIFO callback regression. Stage04B owner and shipping WinSock unaffected; source stage not promoted.
- Stage04C FIFO demonstrates only genuine queue reservation/commit/retry. Grant store and dispatch remain unbound/mocked in this fixture. Native Windows x64 Stage04C, full source release QA, Golden, failed close, session lifecycle, and games NOT_RUN.
- Kept 548-byte ABI, exports, vendor sources, CRT, Windows IOCP-required policy unchanged; no `ASIO_DISABLE_IOCP` or socket `release()` handoff.

## HF12 Stage04C FIFO native test-length FIX01 (2026-10-14) — NOT_RELEASE

Exact immediate source parent Stage04C SHA-256
`194cb3d5fe6757599950ebb2b956f7ccbf9197618d71908c0153cb236cea2789`.
User's first Windows x64 `/MT` FIFO run compiled but failed at `checks=7`:
8-byte expected `"RESPONSE"` versus only 7 bytes prepared/sent in native fixture.
`fifo_mtd` was not run. Corrected only native/portable fixture Response byte
lengths (now derived from one constant), added failure diagnostics and a focused
fixture-length source assertion. Shipping, CMake, vendored libraries, public ABI,
CRT and export identity unchanged. Source tests are distinct from actual
Windows FIX01 `fifo_mt/mtd`, full 8/8 source and clean-extract QA, Golden,
real Grant store/dispatcher, failed-close and game acceptance (all pending).
**NOT_RELEASE**; IOCP must remain enabled and production WinSock unchanged.


## HF12 Stage04D source-linked persistent store and FIFO (NOT_RELEASE)

New bounded tests link actual Grant service-state, real FIFO and transaction callback owner. Portable GCC/Clang pass is not Windows IOCP native acceptance; `store_mt`/`store_mtd`, full dispatcher/handler, shipping Asio session, full QA/Golden/game remain open. Production sources/vendor/ABI unchanged. See docs/ASIO_IOCP_GRANT_STORE_ACCEPTANCE.md.

## Stage04E — real Grant/XML/AES callback research checkpoint (NOT_RELEASE)

From immutable Stage04D source SHA-256 c715bb649d4fe8cc76dca77246a214cac56f28b205e4a2e5f6fa95a3befe07bb,
added test-only full dispatcher/GrantAchievement persistent-state and
AES/hex Response/Event source-linked callback acceptance, plus an x64-only
Windows Asio IOCP `dispatch` native test mode. No production, vendor,
public ABI or shipping networking changes. Stage04E native Windows/Golden/game
acceptance remains pending; see docs/ASIO_IOCP_FULL_GRANT_DISPATCH_ACCEPTANCE.md.

HF13 internal cumulative continuation: AsioServiceOwner bridges listener/session admission, lifecycle stop, and callback drain. Parent SHA 820573098b64e0721cd0dd38db6a80b50ed1efbc9f89b5fb8c01811cb4b801bf; shipping ReleaseServer remains WinSock; Windows native/Golden/game NOT_RUN; NOT_RELEASE.


HF13 internal source cutover (not a release): native ReleaseServer listener,
serve, stop and shutdown paths now route into AsioServiceOwner through
asio_release_bridge; adds abort-before-start failure atomicity and counts
Asio-accepted bootstrap clients. Real Windows/IOCP acceptance pending.


## HF13 native/CRT host prerequisite compile correction — internal, NOT_RELEASE (2026-10-14)

- Exact immutable parent: `HF13_ASIO_RELEASESERVER_CUTOVER_INTERNAL_SOURCE_NOT_RELEASE.zip` SHA-256 `a2d205a4d4f75c88e9467ecb0117d6cf9d942f3e9d7485d5b0e140549db8b43d`.
- Windows user logs for the earlier HF13 Service Ownership parent confirmed Production Golden **PASS**, Debug **PASS**, but Native Acceptance and Static CRT Acceptance **FAIL** at the host compile stage (MSVC C2039).
- The two native/release hosts still referenced the removed `HookPrerequisiteModules::msvcp` even though only `advapi` remains following the approved `_Mtx_trylock` hook removal. Corrected both predicates to check `advapi` only; the actual ADVAPI32 module/Reg* availability fail-closed check and unload path are unchanged.
- Added source/negative-regression coverage for the exact host field layout and stale member; no WinSock/Asio/production or ABI source touched.
- New host acceptance on Windows/MSVC remains **NOT_RUN**; historic failed source log does not prove the immediate new source has compiled.

### HF13 native Asio acceptance metrics — bounded remediation candidate (NOT_RELEASE)

- Exact immutable parent HF13 CORVEXON MSWSOCK source SHA-256 `1f83a5319011c1e404e7a988af7b85075617c9c9dee32f26aecde6179ba53bb2`.
- Windows evidence: Production Golden and Debug PASS; Native/Static CRT fail in scenario `one` at exit 42. This specifically means the native host observed no completed legacy worker metric (selector 3) in 5 seconds, **after** authentic LSX handshake and GetConfig response completed. It does not prove failed IOCP transport.
- Root: acceptance metric selectors 1/2/3/4/7 and 5/6 were attached only to historical WinSock jthread lifecycle; the Asio service owns accepted sessions without those worker threads. The new Native Acceptance-only observation records real Asio accepted/started/completed/reaped and nonempty peer events, plus actual live session registry count and the existing 128-session cap. It does not fabricate `ModelJthread24` ownership or loosen host timeouts/required counts.
- Source paths: `src/runtime/asio_service_owner.cpp/.hpp`, `src/runtime/release_server_lifecycle.cpp`, `src/runtime/release_server_io_internal.hpp`; extended `scripts/test_native_worker_acceptance_surface.py` for fail-closed source QA. All counter calls and the extra SessionEntry state are compiled only in native-acceptance Windows builds.
- Explicit non-goals: no change to Production LSX dispatcher, Asio IOCP socket or callback lifetime, public 548-byte ABI, hooks, CMake, Golden allowlist, or vendor code; do not reinterpret other Native Acceptance cases as PASS.
- Windows acceptance, especially real callback completion, partial-send/F02 fault injection, shutdown/reload and the actual Windows IOCP close paths, remains pending. No formal release or canonical pointer mutation is authorized.

### HF13 Wave4 Asio resource retry research/remediation candidate (NOT_RELEASE, 2026-10-14)
Exact parent ZIP SHA-256 e6b52091312a115308f3a8511d4bd9fa2f83904e215c91db83ad8a715ef52bb8. User Windows Golden/Debug PASS; Native/CRT scenario wave4_resource_retry exit 215. Original fault/retention tests targeted legacy WinSock/jthread owners, not active Asio socket/acceptor owners. Native-acceptance-only test controls now hold actual Asio owners until explicit disarm and successful retry, keeping historical mock WinSock behavior intact. The exact host first-shutdown return 10 contract is retained, and Windows/MSVC Native/CRT has NOT been rerun on this candidate. See docs/ASIO_RESOURCE_RETRY_ACCEPTANCE.md. No official release/pointer mutation.

## HF15 Asio Stage02 shipping legacy retirement — 2026-10-15 (NOT_RELEASE)
Source-only continuation of immutable timeout candidate SHA-256 `32657ea5a9ae8d79cb87fd34eab18f7f9ab10707712561f3da8863a719eef7df`. Drop legacy WinSock-jthread worker/acceptor from Production/Debug CMake, retain only in native-acceptance test target. Remove unreachable native fallback in ReleaseServer. Preserve 0x548 ABI and fail closed on unexpected nonempty legacy ownership. Mock SocketApi and WSA lifecycle remain; all Windows native/Golden/game gates PENDING. See docs/ASIO_LEGACY_RETIREMENT.md.


## Stage03 — Legacy mock listener/endpoint removed from Production (NOT_RELEASE)

- Exact parent is the Stage02 Legacy Retirement source candidate (SHA-256 `58ae8d72566c7da175bebef5f3826cb077d61dbe487971105b6f182e22794f33`).
- Normal Production CMake excludes `src/net/socket_listener.cpp` and `src/net/socket_endpoint.cpp`, reducing production source graph 134 → 132 TUs. These implementation files remain present and explicitly compiled **only** for instrumented native-acceptance fault injection and separately enabled Debug socket diagnostics.
- Production/Debug `ReleaseServer` explicitly rejects injected non-native `SocketApi` without invoking the historical bind/accept path. Only Asio IOCP native listener/session remains selectable. The test-only injected backend and deterministic acceptance callback semantics survive under `PRELOADER_L_NATIVE_ACCEPTANCE`.
- Shipping release server initializes historical listener/client model slots without unresolved reference to retired `reset_socket_listener`/`reset_socket_endpoint`. Unexpected physical model slots still fail closed during service retirement.
- `SocketRuntime`, `native_winsock_api`, explicit WSAStartup/WSACleanup and generic `TransportEndpoint` protocol utilities are **still shipping**; no all-legacy-free claim. They require separate architecture proof and native Windows validation before further removal.
- The 0x548 model ABI, headers, exports, MinHook, vendored sources and Stage01 timeouts are unchanged. Windows x64 /MT and /MTd, Golden, 8/8 QA, and CollegeFB27/Madden27 are NOT_RUN.

IDS Stage04 (2026-10-15, NOT_RELEASE): Stage03 parent `100a97a68582c0ae9c200e813a74e62263706d4c3f5922484ed265fb0c52d27a`; remove shipping generic socket_transport.cpp (132->131 TUs); same endpoint validation now local to synchronous exchange; retain Native/Debug helpers and explicit WSA lifecycle; Windows/Golden/game/8-shard gates deferred. See docs/ASIO_TRANSPORT_UTILITY_RETIREMENT.md.

## Internal IDS Stage05 — shipping Winsock lifecycle isolation (NOT_RELEASE)

Exact parent: IDS Stage04 source SHA-256 `288442bdd78ff6c4bbf4ac5956f1ab7b600a152bbe300b8a80de8b8acf07fb06`.
Production links `winsock_lifecycle_native.cpp` rather than the raw socket implementation
`winsock_native.cpp`. The new provider preserves the existing SocketApi identity,
WSAStartup version contract, paired WSACleanup, retry ownership, and last-error.
Its other callbacks fail closed: they cannot create raw shipping listeners or
sockets. Native fault injection and Debug keep the byte-identical original full
adapter. Asio still owns actual accept/session/IOCP handles. SocketRuntime
logic, the 0x548 service ABI, and public headers remain byte-identical.

This is an INTERNAL SOURCE STATE, not a fifth release category and not an
HF16 artifact. Windows native/Golden/CRT/game acceptance is deferred; changing
CMake shipping inputs invalidates affected carried-forward runtime evidence.

## IDS Stage06 — lifecycle-only shipping API (INTERNAL / NOT_RELEASE)

- Parent: IDS Stage05 exact SHA-256 `83eef7d17c7065efe6157094e5a4e6324f9ed981dbcdeb5e133b4e409f9f63fa`.
- Native/Debug full SocketApi remains; Production callback table has only WSA lifecycle/error slots. Shared runtime accounting preserves WSACleanup failure retry and negotiated 2.2 semantics. Asio bridge selects lifecycle-only entrypoint; source graph remains 131 TUs.
- New Windows-only acceptance host/runner covers receive idle 30s, immutable partial-frame 10s, reconnect, shutdown, without pretending to test real send stalls. Real native Windows send-idle/transaction callback evidence and final 8/8 QA remain pending. No HF16/release/pointer mutation. See docs/ASIO_LIFECYCLE_ONLY_RETIREMENT.md.

## Internal IDS07 — 2026-10-15 (NOT_RELEASE)

- Exact parent IDS06 source hash `5f0970f9dad316ac707ad77d3c709af16e748c02f2de8f7d1f4e80677f0904eb`.
- Native-only 5-second suspended prepared write and exact send_timeout counter; separate large kernel-pending Asio async_write backpressure test. No Production runtime delta is intended; acceptance remains Windows-dependent.
- Corrected IDS06 Python frame timeout runner incorporated from test-only FIX01.

### HF17 governance-first canonical-target convergence (2026-10-17)

- The active HF17 release class is `HOTFIX_RELEASE` under Library master 2026-10-17.1; no new subordinate candidate release taxonomy is created.
- HF16 official current pointer and HF16 carried acceptance remain historical, not HF17 acceptance. HF17 G2/G3/G4 and Windows/CollegeFB27/Madden27 remain blocked pending fresh evidence.
- All eleven runtime hook sources retained byte-identical to the earlier governance-ready HF17 working candidate.
