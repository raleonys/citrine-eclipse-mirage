# Yellowfield

The current owner instruction requires a Brownfield-style application project
root and excludes unrelated reverse-engineering projects from the application.

- First-party C++ lives in src/ and include/preloader_l/, namespace preloader_l.
  Follow the owner-corrected Brownfield-style one-level module layout:
  src/<module>/<file> and include/preloader_l/<module>/<file>. Do not recreate
  transport/native/hooks/windows/achievements/time subfolders inside modules.
- Tests and fixtures live only in tests/. Dependencies live only in third_party/.
- Application implementations originate from the supplied anadius64 RE sources.
  The explicitly authorized CryptQueryObject hook is a separately documented
  exception; do not import its surrounding preloader/version/Once/mutex project.
- Brownfield supplies the required cleanly adapted documentation, Golden tools,
  root layout, metadata/resource ownership, scripts/build orchestration, compatible
  tests and operator tooling. This scope is mandatory, not limited to three helpers.
  Track every item in metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json.
  Bluefield is the explicitly selected donor for hook11 and its required native
  hook lifecycle dependencies only; other application modules remain comparative.
- Private CRT recovery and unfinished historical B2-B6 composition experiments
  are reference inputs outside this product tree. Use the compiler's standard CRT.
- The DLL owns concrete Application/Service startup on a Windows worker. Its
  three-entry C interface is fixed in exports/preloader_l.def and
  metadata/PUBLIC_DLL_ABI.json. C++ source APIs belong only to the separate
  preloader.source.dll development target. The earlier 548-byte ABI claim had
  no supplied RE layout and is withdrawn; do not invent that layout or stubs.
  Original Consumer imports use different module/symbol names, and real
  Windows/game acceptance remains unverified. See docs/PUBLIC_DLL_ABI.md.
- Preserve the original archives and earlier source packages. Record source
  origin, path/namespace adaptations and fresh verification in metadata/.
- The owner explicitly authorized continuing Brownfield infrastructure adaptation
  after reviewing the missing-transfer list. See governance/CURRENT_SCOPE.md.
- Preserve the original integration requirements in metadata/INTEGRATION_REQUIREMENTS.json.
  The owner now authorizes vendoring the five archives supplied under 3RD PARTY,
  excluding Brownfield copies. Keep all vendor bytes and TinyXML2's default 500
  unchanged. Do not add the previously proposed 100-depth compatibility patch.
  Vendor public API compatibility is required by default, including builds with
  tests disabled. LLVM-MinGW 20251118 / Clang 21.1.6 is the tested compatible
  cross-toolchain; LLVM23/libc++23 is diagnostic-only for this PEGTL snapshot.
  Distinguish vendored/build-tested dependencies from final active runtime use.
  For Windows ZIP extraction, only the two VENDOR_LOCK Asio aliases may use
  regular files containing their exact original relative target bytes instead
  of real symlinks. Source ZIPs emit these portable descriptors. Keep all member
  hashes unchanged; never accept altered descriptors or arbitrary symlinks.
  Library synchronization/publication requires a separate explicit owner request.
- The owner authorized executing the existing RE Config2/VDF grammar through PEGTL.
  Preserve its documented syntax, actions, partial state and owned error positions.
  Do not treat this as a new filename/precedence policy or import Bluefield code.
- Golden parser fixtures are tests only. The owner explicitly adopts the supplied
  Brownfield production metadata policies; never describe fixture PASS as final-DLL PASS.
  The latest owner follow-up explicitly adopts resources/preloader_version.rc
  VERSIONINFO from Brownfield unchanged; see metadata/RESOURCE_METADATA_CONTRACT.json.
  The latest owner correction additionally adopts the full declared Brownfield PE
  metadata profile; the five-entry public C ABI remains the user-selected exception.
- Latest YF-GOLDEN-AVATAR-20261029 owner correction: PNG must replace the original
  Anadius profile-avatar data in read-only C++ storage, never PE RCDATA; preserve
  the separate RE fallback avatar. VERSIONINFO bytes remain unchanged.
  Restore RE GetEnvironmentVariableW LocalAppData/ProgramData lookup, not the
  previously introduced SHGetKnownFolderPath/COM allocator dependency.
  Owner now requires the Brownfield metadata targets on a finalized copy:
  timestamp0xCD268EB1, linker14.0, Rich absent, OS/subsystem6.0 and recomputed checksum. Require exact five exports, original five import modules plus the owner-required
  hook11 CRYPT32!CryptQueryObject static import, static CRT /GS
  and genuine MSVC build evidence. Configuration readiness is not Golden PASS.
  Every newly delivered source ZIP must have an adjacent .zip.sha256 file.
- Windows PowerShell/CMD execution, Wine execution, source QA and game acceptance
  are distinct evidence classes. Record each result without inheriting donor PASS.
- Use python -B for project scripts. Check the whole-source manifest before packaging;
  refreshing it is an explicit recording action, never an automatic repair of drift.

- Latest YF-RUNTIME-BIND-20261028 owner correction requires completing the runtime
  bindings already expected by the product goal. All five user vendors now have
  concrete application paths. Preserve their bytes and record real validation.

- Latest YF-PRODUCTION-20261029 owner correction requires a real static CRYPT32
  import (crypt32.lib + SDK &CryptQueryObject target), following the supplied
  Brownfield/Bluefield hook11 integration. Dynamic-only lookup is superseded.
  Latest owner correction selects Bluefield callback/selection and lifetime
  behavior; retain the five public exports unchanged.
  Golden's six gates plus final-name runtime cycles must pass before writing
  dist/preloader_l.dll, its .sha256 and LOCAL_DIST_RECEIPT.json. Other profiles
  never promote to dist; failed attempts retain previous valid output/receipt.
  Prevent bytecode in every nested Python gate. Before build remove only known
  generated caches for manifest-owned first-party Python tools; never ignore
  caches in FALCION or refresh manifests to conceal source drift.

- Latest YF-BLUEFIELD-ALIGN-20261029: owner explicitly selects the supplied
  Bluefield CryptQueryObject implementation and its hook lifecycle dependencies.
  This is a scoped code-donor exception to comparison-only rules: preserve the
  Bluefield protected reader, UTF16 selector/fallback, queued activation, native
  call observation and unload/thread guard. Do not keep the RE trap variant.
  Owner also rejects preserving compiler metadata and requires the full supplied
  Brownfield metadata profile: timestamp0xCD268EB1, linker14.0, Rich absent and
  all other declared MUSTAVEX/ARYRA fields. Earlier PRESERVE_INPUT instructions
  are superseded. Retain the latest five exports, static CRYPT32 import, user
  PNG in C++ data, immutable vendors and standard CRT. Finalize a copy; keep raw
  compiler output distinct from validated dist.

- Latest YF-STARTUP-BUILD-20261029: owner reports no visible game startup with
  dist/preloader_l.dll; optional config is required. Fix reproduced registry
  startup-wait lock inversion using the supplied Bluefield FIX19 Guard/Pause
  dependency. Preserve binding leases, original registry algorithm and LastError.
  Do not silently switch config/provider path policy or claim game acceptance
  from host/Wine tests. Restore the five ordinary build-profile titles, keep
  successful cache cleanup quiet with a build-directory JSON report, and fix
  missing NOMINMAX on the independent MinHook test target. Keep prior archives.

- YF-ASIO-WINDOWS-TARGET-20261029 restores the supplied Brownfield explicit
  _WIN32_WINNT=0x0601 and WINVER=0x0601 definitions through the Windows Asio
  interface target. Do not patch vendor bytes or suppress warnings globally.
  PE OS/subsystem metadata6.0 stays separate. Owner confirmed game startup on
  the preceding startup-build-fix package; do not erase or overgeneralize that result.

- Latest YF-WINDOWS-IO-20261029 owner correction supersedes the earlier five-active-
  vendor requirement for Gulrak only: restore supplied Windows std::filesystem/
  standard fstream paths and remove unused ghc vendor/target/include bindings.
  Keep original filesystem-master.zip outside the product and all earlier ZIPs.
  Remaining four vendor payloads/defaults/pins stay immutable. Retain optional
  config, original registry/hook fix, five public exports and Debug-only surge.log.
  Release must never append to an existing Debug log; verify both configurations.

- Latest explicit owner export correction: restore Brownfield production
  DllMain@1, SURGE@2, DllShutdown@3. Remove DllWaitReady/DllServerPort and slots4/5,
  including unused management implementation. Adapt hosts to existing EALsxPort/LSX
  observation rather than retaining test exports. Keep Brownfield ordinals intact.
  DLL import-descriptor display order is separate and is not a new mutation target.

- Latest owner import-order instruction: finalized production descriptors must be
  CRYPT32, WS2_32, KERNEL32, USER32, SHELL32, MSWSOCK. This is distinct from export
  ordinals. URSYNEX may permute only exact complete descriptor records in a copy;
  preserve all name/thunk/IAT bytes and directory locations, recompute checksum,
  require read-only CORVEXON exact-order validation. Keep original build untouched.

- Latest SURGE-IDENTITY-20261029 owner correction requires consistent product
  naming in application code, diagnostics, source-level shutdown declarations
  and their tests. Use core/product_identity.hpp for runtime config/data/avatar/
  provider names; projections must use the existing SURGE runtime policy.
  DllShutdownStatus / DLL_SHUTDOWN_* retain numeric values0..4; three production
  exports and Brownfield ordinals remain unchanged. Keep original filenames in
  provenance comments, historical reports and input references. Existing
  yellowfield.* JSON schema identifiers remain stable document-format IDs, not
  runtime branding. Do not rewrite frozen evidence or vendor payloads.

- Latest SURGE-RE-COPY-20261029: owner asks to recover the missing full copy
  helper from the original DLL, beyond the supplied source's mode2 projection.
  Reconstruct048d20 plus04895c identity fallback and0489f4 time query from the
  pinned original pseudocode/assembly, bind the avatar through the complete
  helper with mode2, and verify options/errors/reverse close/fatal behavior.
  This is a scoped public filesystem-copy compatibility addition, not private
  CRT recovery. Keep standard IO elsewhere, four immutable vendors, three fixed
  exports, existing import module order and all Golden metadata policies.

- Latest SURGE-FIDELITY-FIX-20261029 owner authorizes resolving all four audit
  findings. Connect must use Bluefield narrow attempts synchronization, never
  serialize a startup wait or original connect. Match Bluefield common Guard
  scope (exclude only connect/crypt), retaining registry Pause, leases and LastError.
  Event Mutex must expose the RE type2 reentry error3/count1 using standard CRT
  storage; do not import private CRT. Debug diagnostics must retain actual
  request/response/event/XML/decryption/config values and their existing gates;
  Release never creates/appends a log. Current status pointers must reference
  RUNTIME_CURRENT_STATUS.json; preserve old runtime evidence as historical.
  Keep four byte-exact vendors, three production ordinals and all Golden policies.

- Latest owner packaging boundary: delivery version, codename and revision belong
  only to the external archive filename and adjacent checksum file. Do not
  propagate them into packaged source/docs/metadata/tests, CMake, build titles,
  tools or receipts. Keep the packager generic with a caller-supplied output
  path. Preserve the separately pinned PE metadata profile and previous archives.

- Latest owner identity cleanup allows original source/workstream names only in
  documents. Remove them from source comments, tools, tests, current metadata
  identifiers/keys and report schemas. Migrate internal schemas and consumers
  together; preserve raw prior reports as documents under docs/evidence. This
  explicitly supersedes the earlier stable-schema naming exception, not vendor
  byte pins, ABI ordinals, fixed PE policy or external-only delivery labels.

- Latest owner hook-hardening instruction adopts Brownfield NtGetNextThread
  SAFE-FREEZE and archive commit traceability. This explicitly authorizes the
  pinned patch in an owned build copy only; original third_party bytes remain
  immutable. Preserve queued single-apply activation outside DllMain, retained
  transaction ownership after incomplete rollback, trampoline drain and retryable
  cleanup. No new production exports. Record donor provenance in documents and
  verify patched output plus every unpatched staged member before compilation.

- Latest owner listener instruction adopts Brownfield's ascending automatic
  3226-3236 range. Remove ServerPort override application from both config paths;
  an old key is ignored without numeric parsing. Keep the selected live port in
  runtime state and EALsxPort, preserve connect/IPv6 hook behavior and never fall
  back to an arbitrary OS-selected port. Exhaustion fails startup. This is an
  explicit scoped product-policy departure from RE port configuration. Preserve
  every vendor byte and the current hook/ABI/Golden policies.

- Latest continuation addresses the confirmed inherited unload-guard ownership
  defect. Retain the exact thread and snapshot handles after incomplete cleanup;
  repay an owned suspension before enumerating/suspending again, and retry a
  failed close without another resume. Serialize scans without blocking peers.
  Keep the existing conservative stack/register scan, fixed port policy, hook
  callbacks, public ABI and vendor patch pins. This scoped recovery correction
  supersedes byte-identity with the Bluefield unload guard, not its scan policy.

- Latest owner SDK instruction pins numeric identity 10.6.2.31 until a new
  explicit owner update request. The latest correction removes the build suffix
  and supersedes the earlier full-string pin; wire identity is 10,6,2,31.
  Remove SDK config application in both paths and the mutable config-state field.
  Compose the greeting from core/protocol_identity.hpp, not a CFG/provider input.
  Do not infer SDK updates from vendors, compiler/Windows SDK or PE resources.
  This is owner-provided identity data, not independently verified consumer bytes.
  Keep ChallengeResponse protocol negotiation, PE metadata and public ABI intact.

- Latest owner runtime-resilience instruction explicitly adopts exact IPv4
  127.0.0.1:3216 and IPv6 [::1]:3216 destination filtering with a complete const
  address copy, checked atomic achievement XML/INI persistence with record/event
  rollback, and two owned handler workers with a bounded queue and per-session
  FIFO processing. Preserve callback leases/narrow attempts/LastError, optional
  config, fixed listener/SDK identities, immutable vendors and the public ABI.
  Keep failed file rollback backups owned for retry and drain active handlers on
  stop; do not claim a power-loss atomic multi-file transaction. Other RE binary
  IO paths and unrelated proposed network policies are outside this change.

- Latest owner session-transport instruction explicitly adopts full-send FIFO,
  event reserve/commit/retry, asynchronous greeting, monotonic receive/frame/send
  deadlines and bounded sessions/frames/output. Preserve existing protocol bytes,
  atomic achievement storage, worker ownership, fixed SDK/listener, vendor bytes
  and public ABI. Application defaults are 128 sessions, 1 MiB input frame,
  8 MiB combined output/greeting and 30/10/5 second deadlines; no CFG overrides.
  Keep pending write buffers alive through cancellation, never acknowledge partial
  output, and prevent stale timers from closing renewed sessions. Event completion
  must not block IO behind the persistence mutex; this supersedes the earlier
  eager-dequeue/next-read-after-handler transport policy, not the pure RE fixtures.

- Latest owner hook11 instruction adopts a 32768-unit UTF16 scan cap including
  NUL and removes path-scan C++ heap allocation. Retain protected per-unit reads,
  full-unit pointer bounds and the existing valid-input suffix rules without a
  second caller-memory dereference. Invalid/unreadable/over-limit input forwards
  the original object. Replace the internal string-output host boundary with a
  noexcept probe; preserve LastError, real SDK outputs, failure policy, static
  CRYPT32, hook lifetime, fixed identity/transport and three production exports.

- Latest owner build-audit instruction authorizes splitting production inputs from
  retained reference/model/test/source-API inputs. Do not delete the 74 reference
  TUs or call them globally dead; preserve all runtime C++ and header bytes.
  Enforce one-way dependencies and actual linker-map exclusion, including stale
  archive members. Production-only integration tests must not link reference
  libraries. Keep source API exports and reference tests; record comparative
  full tests and native MSVC/game acceptance separately. See
  metadata/BUILD_PARTITION_POLICY.json and docs/BUILD_PARTITION.md.

- Latest owner reference refresh adopts the newly supplied DLL VERSIONINFO and
  compilation timestamp through RESOURCE_METADATA_CONTRACT/MUSTAVEX/URSYNEX.
  This supersedes the previous reference values only; retain the current public
  ABI, static CRT, import descriptor order and debug/signature exclusions. Use
  the new owner PNG byte-for-byte as profile C++ data; keep fallback and no-PNG-
  resource policy. Record owner-confirmed Windows/Golden/game acceptance of the
  pre-update build separately from acceptance of these changes. Fix the reported
  native fixture hash using binary test writes and the live-port sharing defect
  with SO_EXCLUSIVEADDRUSE in the concrete Windows listener adapter; preserve
  the pure RE server flag and 3226-3236 selection/exhaustion policy.

- Latest owner source-review instruction authorizes proven RE delta bug fixes and
  suitable regression-test contributions only. Preserve existing product policies;
  do not import private CRT domains or restrictive controller admission. See
  metadata/CONFIG_PLATFORM_POLICY.json and docs/SOURCE_REVIEW.md.

- Latest owner optimization instruction authorizes the whole first-party compiler
  audit and proven, measured dead-code/allocation improvements. See
  metadata/OPTIMIZATION_REVIEW.json and docs/CODEBASE_OPTIMIZATION.md. Do not
  equate an absent AST syntax reference with dead code; preserve virtual/native
  callback/lifetime/development API roots and all 74 retained reference TUs. Keep
  Golden linker policy, vendors, PE/ABI and runtime policies fixed. The previous
  owner-accepted DLL is historical baseline evidence; changed code requires fresh
  native Windows/Golden and game acceptance. Delivery labels remain external only.

- Latest owner cleanup instruction applies the independently reviewed DC-001–011
  changes and the Debug-only crypt_observed TLS body together. Production now
  owns60 CPP and the retained reference/API group78, total138; no recovered body
  or public header is deleted. The old config/startup.cpp body is retained
  byte-exact as config/config_startup.cpp to avoid ambiguous startup.obj evidence
  beside production runtime/startup.cpp on Windows. Keep the split hash oracle
  and startup/storage model tests; production tests cannot link reference_models.
  Preserve all native owner/retry/quarantine/IO-drain contracts and3346 current
  development export names. DC-012–019 are deferred measured/API design topics,
  not authorized blanket deletions. See docs/BUILD_CLEANUP.md and
  metadata/BUILD_CLEANUP_REVIEW.json. The TLS cleanup is not a proven VirusTotal
  label cause or promise of a label change; native MSVC/Golden/game acceptance
  remains separate from local cross-toolchain validation.

- Latest owner Windows logs exposed a net_failure_contracts fixture gap: the
  pinned Asio MSVC memory helper uses aligned CRT allocation, outside the old
  scalar-new injector. Correct only the test boundary; preserve all product and
  vendor bytes and production compile definitions. Pre-parse dependencies before
  the scoped helper mapping; assert route/hit count and retain the pending-flag
  rollback check. MinGW also runs an isolated MSVC-memory-helper replay; this is
  not native MSVC compiler acceptance. Record the owner's four successful build
  profiles and204/205 native result separately from the corrected test rerun.
  See docs/NATIVE_ALLOCATION_FIXTURE.md and metadata/NATIVE_FIXTURE_REVIEW.json.

- Latest owner avatar clarification retains the current read-only C++ PNG
  embedding and RE write/config override behavior. Replace only the profile
  asset with the latest supplied208x208 PNG; preserve the separate original
  fallback and no-PNG-PE-resource policy. Do not switch to mandatory external
  avatars. See docs/AVATAR_UPDATE.md.

- Latest owner collaboration instruction moves development to GitHub from the
  exact archived source baseline. Work in a separate branch/worktree and open
  a focused PR with scope and validation evidence. The owner reviews and merges;
  do not merge, enable auto-merge, approve your own PR, bypass failed checks or
  publish releases. Preserve all existing product/vendor/ABI policies and exact
  source bytes. See docs/GITHUB_WORKFLOW.md.

- Latest owner dependency instruction replaces only Asio with the newly supplied
  archive snapshot pinned in VENDOR_LOCK.json. Its header still reports1.38.2;
  identify it by the archive SHA256 and commit comment, not a fabricated release.
  Preserve exact new vendor bytes and both portable alias descriptors. Keep
  MinHook/PEGTL/TinyXML2 pins, SDK/listener/transport policies and the public ABI
  unchanged. Develop in a separate PR; do not merge. See docs/ASIO_UPDATE.md.

- Latest owner CI quota instruction keeps this repository private. Automatic
  PR/main runs retain source QA and portable tests; both Windows Native and
  Production/Golden jobs run only on explicit workflow_dispatch. Preserve their
  existing scripts, assertions and budgets. Skipped jobs are not native PASS;
  review exact-source Windows/game evidence separately. This supersedes automatic
  Windows execution, not the owner-only merge policy or local acceptance checks.

- Latest owner repository-language instruction requires English for all future
  commit messages, PR titles/descriptions, and newly authored repository-facing
  text. Keep user chat in the user's chosen language. The English README rewrite
  must be previewed and explicitly approved before pushing; this extra approval
  requirement applies to README changes, not all otherwise authorized work.

- Latest owner release instruction explicitly authorizes a weekly draft-candidate
  workflow on Friday at 16:30 Europe/Istanbul, plus manual dispatch. Validate the
  same committed source with source QA, portable tests, native MSVC and the full
  Production/Golden profile before attaching source/binary ZIPs and SHA256 files.
  This is a scoped exception to manual-only Windows CI. Only drafts may be created
  or retried; the owner publishes. Skip unchanged sources, retain retryable partial
  drafts and never overwrite published releases/tags. Start or change the external
  series only through explicit start_tag input; continue its R01-R99 revisions
  without injecting release labels into product metadata or source defaults.
  No new code/version identity, game acceptance, merge, or Library publication is
  implied. See docs/WEEKLY_RELEASE.md.
