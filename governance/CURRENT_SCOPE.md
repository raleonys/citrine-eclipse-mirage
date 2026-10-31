# Current scope and source authority

Current owner PR-readiness rule: never create draft pull requests, convert PRs
to draft or leave PRs in draft. Open ready-for-review PRs after the required
local validation passes; document pending checks, remaining work and merge
blockers in the description. Ready status does not certify native acceptance,
approve changes or authorize a merge. The owner alone merges reviewed PRs.
This does not change the separate owner-authorized weekly draft-release policy.

Current owner source-validation correction: prevent repeated PR inventory errors.
SOURCE_FILES and ACTIVE_SOURCE_MANIFEST are separate records; review and update
only approved first-party entries before explicitly refreshing the whole-source
manifest. Require complete local source QA on the committed source before push/PR,
using the tracked read-only pre-push gate in every clone. Do not auto-refresh pins,
bypass failures or add automatic Windows builds. The local hook is a contributor
safeguard, not enforced server protection. See [workflow](../docs/GITHUB_WORKFLOW.md).

Current owner branch decision: agents must never edit, commit, push or update
main directly, including through file-content APIs. Only the owner merges
reviewed PRs into main. Reuse the permanent bugfix branch for routine fixes,
dependency updates, documentation, tests and small maintenance; retain and
synchronize it after each merge without rewriting shared history. Dedicated
descriptively named branches are for explicitly scoped, sustained audits/projects,
not one or two small PRs. Temporary exceptions require an owner instruction.
Retire completed branches only after owner confirmation, work-preservation and
open-PR checks, recording their tip SHAs; never delete main/bugfix or active audit
branches. See the mandatory rules at the start of [AGENTS.md](../AGENTS.md) and
the [GitHub workflow](../docs/GITHUB_WORKFLOW.md).

Current owner release instruction: prepare draft candidates every Friday at
16:30 Europe/Istanbul when new commits exist. The owner publishes manually.
The weekly workflow runs the full source/portable/native MSVC/Production gates;
this is the explicit scheduled exception to the manual-only Windows CI rule.
Release series and revisions are external tag/package labels supplied through
manual start_tag and continued from draft state, never product-code defaults.
See [weekly draft releases](../docs/WEEKLY_RELEASE.md).

Current owner CI decision: keep the repository private and reduce automatic
hosted-runner use. Source QA and portable tests stay automatic on PRs/main;
Windows Native and Production/Golden run only on manual workflow dispatch.
Local build/acceptance scripts and product/vendor contracts remain unchanged.
Skipped Windows jobs do not count as native acceptance. See the
[review workflow](../docs/GITHUB_WORKFLOW.md).

Current owner task: vendor the newly uploaded Asio source archive byte-for-byte.
The header version remains 1.38.2; the snapshot is pinned by its ZIP hash and
commit annotation. Preserve the other three vendors and product policies.
Prepare a separate PR; only the owner merges it.
[Scope](../docs/ASIO_UPDATE.md).

Current owner decision: the source archive was imported into the initial
GitHub source tree byte-identically. CI and PR workflow rules are prepared
on a separate branch/PR; only the owner merges. This does not authorize
changes to runtime or vendor behavior.
[Workflow](../docs/GITHUB_WORKFLOW.md).

Current owner decision: retain the existing embedded C++ avatar method;
replace only the profile avatar with the latest supplied PNG. Do not
change the external-file requirement, fallback, CFG precedence, or file
write behavior. [Scope](../docs/AVATAR_UPDATE.md).

Current continuation: correct the allocation-injection defect in
`net_failure_contracts` reported by the owner's Windows/MSVC logs.
Intercept the pinned Asio library's actual MSVC aligned-CRT allocation
path in the test executable. Do not change Production/vendor bytes or
options. The four earlier successful profiles and Native 204/205
result remain historical evidence; request acceptance of the corrected
native test separately. Earlier scopes are retained below.
[Review](../docs/NATIVE_ALLOCATION_FIXTURE.md).

Current owner instruction: implement the verified four-TU Production/reference
partition, narrow DC-005–011 tool/test/expression cleanup, and
`crypt_observed` Debug TLS gate together. Preserve source API/test scope
and all shutdown/native ownership corrections. The DC-012–019 design
candidates are excluded from this change.
[Current scope](../docs/BUILD_CLEANUP.md). The earlier tasks below
remain historical authorization/scope records; donor retirement is not
approved.

Latest continuation: native socket ownership and I/O retirement correction.
Retry a documented would-block result using the exact handle; do not blindly
reclose after an uncertain outcome. Retain the owner and image lease.
Accept shutdown based on native records and callback drain.
[Current scope](../docs/SOCKET_RETIREMENT.md). Donor retirement remains
unapproved; prior open findings below retain their historical scopes.

Previous continuation: change the inner hook cleanup loop to a single
attempt, test failures against the actual DLL worker/application/binding,
contain timers, and compare native socket-close ownership.
[Current decision](../docs/SHUTDOWN_RECOVERY_AUDIT.md).
The socket-close failure path remained unresolved; donor retirement was BLOCKED.

Current scope: correct verified transport/companion-DLL ownership defects,
add regression tests, and continue the Brownfield/Bluefield shutdown review.
**Donor retirement is not approved.** Reviewing selected branches does not
establish semantic closure across all source files. Current decisions and
evidence:
[review](../docs/DONOR_RECOVERY.md),
[validation](../metadata/evidence/recovery/VALIDATION.json),
[machine record](../metadata/RECOVERY_REVIEW.json).
Earlier task descriptions below retain their historical authorization
scope; `RUNTIME_CURRENT_STATUS.json` determines current status.

Current work item: **SURGE-RE-COPY-20261029**. The owner requested recovery
of the missing general copy helper from the original DLL.
Bodies 048d20/04895c/0489f4 are reconstructed from pseudocode and assembly;
the avatar mode-2 path connects to the full body. This is a bounded
compatibility addition for filesystem copying. Importing a custom CRT,
changing vendors, or modifying other hooks is outside scope.
[Recovery and validation](../docs/RE_COPY_RECOVERY.md).

Earlier work item: **SURGE-IDENTITY-20261029**. The owner requested
consistent correction of remaining old names in product code.
Configuration/data/avatar/provider names are tied to a shared
product definition; errors name `surge.cfg`. Source-level shutdown
status names become `DllShutdownStatus` / `DLL_SHUTDOWN_*`,
values remain 0..4, and three exports/ordinals are preserved.
Provenance comments, historical reports, and existing JSON schema
identities remain historical records.
[Naming correction](../docs/PRODUCT_IDENTITY_FIX.md).

Earlier work item: **YF-WINDOWS-IO-20261029**. The owner requested
correction of Windows filesystem differences and complete removal
of Gulrak, which had no other active uses. Restore the RE standard
filesystem/fstream adapter and remove the Gulrak vendor tree and
CMake/API dependencies. Preserve the other four vendors and
TinyXML2 limit 500. The Asio Windows target-definition fix is included.
The owner's export/log questions are resolved: the final Brownfield
three-export set remains in Production, and only Debug writes
`surge.log`. Also verify that Release does not modify an existing
Debug log. The owner confirmed game startup with the previous package.
[Current record](../docs/WINDOWS_IO_FIX.md).

Earlier work item: **YF-ASIO-WINDOWS-TARGET-20261029**. The owner noted
that Brownfield had already addressed the Production Asio diagnostics.
Take the missing `_WIN32_WINNT` and `WINVER=0x0601` build definitions
from the supplied Brownfield reference; add no vendor patches or general
warning suppression. Do not change the PE metadata profile. The owner
confirmed game startup with the previous package, but actual MSVC testing
with the current setting requires separate evidence.
[Correction](../docs/ASIO_WINDOWS_TARGET_FIX.md).

Earlier work item: **YF-STARTUP-BUILD-20261029**. The owner corrected
five build headings and cache JSON output, then reported that the game
did not start with the distribution DLL. Keep configuration optional.
Demonstrate the missing native registry-wait Guard/Pause dependency from
Bluefield FIX19 using a controlled failing test and adapt it.
Do not introduce a new file-search policy, change vendors, or claim
game PASS. Also correct the independent test-fixture NOMINMAX
error from native logs. [Findings and limits](../docs/STARTUP_BUILD_FIX.md).

Earlier work item: **YF-BLUEFIELD-ALIGN-20261029**. The owner selected
the Bluefield eleventh-hook implementation as the correct reference.
Adapt protected filename reading, UTF-16 selector/fallback, observation
of the original call, batch hook activation, and unload/thread guards.
The earlier RE trap selection is superseded. The owner also requested
comparison of all compiled metadata, including Compilation Timestamp,
against Brownfield: choose all declared fields from MUSTAVEX/ARYRA/URSYNEX.
Timestamp 0xCD268EB1, linker 14.0, no Rich header, and recomputed checksum.
Earlier preserve-compiler instructions remain historical. Preserve five
exports, static CRYPT32, no PNG PE resource, standard CRT, and immutable
vendor rules.
[Hook alignment](../docs/HOOK_ALIGNMENT.md),
[metadata audit](../docs/REFERENCE_METADATA_AUDIT.md).

Earlier work item: **YF-PRODUCTION-20261029**. The owner requested
creation of the final file in dist and retention of the static CRYPT32
import for hook 11, as in Brownfield/Bluefield. This supersedes the
earlier dynamic-only lookup decision: use the direct SDK target reference
and `crypt32.lib`. Preserve the callback/selector and five exports.
The new native log showed Golden PASS for the first five gates and a
FALCION Python-cache FAIL at the last gate. Propagate
`-B` / `PYTHONDONTWRITEBYTECODE` to subprocess interpreters and remove
only known manifest-owned generated cache files; do not relax FALCION.
After all six Golden checks and runtime cycles against the final-named DLL,
write `dist/preloader_l.dll`, SHA-256, and receipt; this is not a release.
Earlier items remain historical work records.
[Correction](../docs/PRODUCTION_IMPORT_FIX.md).

Earlier work item: **YF-GOLDEN-AVATAR-20261029**. The owner requested
completion of four missing Golden targets. Bind targets to Yellowfield
source/export/import/CRT contracts, retain timestamp/linker/Rich/OS identity,
and finalize only the checksum in the copied file. Fixture PASS is not
actual MSVC acceptance. The explicit avatar decision, translated, was:
"Do not make it a PE resource; replace the existing Anadius avatar data."
Profile PNG becomes ordinary read-only C++ data, with a separate original
fallback preserved from RE data. VERSIONINFO remains unchanged.
Restore the original environment-variable-reading implementation instead
of the previously introduced COM known-folder dependency, removing OLE32.
Fix Native C2589 by propagating NOMINMAX to core consumers. Supply a
matching SHA-256 sidecar with the new ZIP. Following items retain
earlier historical scope.

Correction at that checkpoint: **YF-MSVC-LOOKUP-20261029**.
Five owner logs showed Compile/Debug/CRT PASS, including CRT 6/6.
The Native `unexpected()` C2668 collision is corrected by qualifying
the test fixture namespace. All twelve calls are explicitly bound;
no scenarios are skipped. A matching global declaration under non-MSVC
compilers remains a compilation regression guard. Application/vendor/
resource data and five exports are unchanged. Golden BLOCKED/exit 3
remains the recorded state.

Correction at that checkpoint: **YF-MSVC-PROFILES-20261028**.
Two confirmed incompatibilities in the owner's five native profile logs
are corrected: an extra MSVC DLL manifest and test-only `__int128`.
Do not relax the product resource allowlist. Apply `/MANIFEST:NO` to all
DLL profiles; use standard 64-bit arithmetic in the test oracle while
retaining tests. CRT builds only selected test targets. If Golden targets
are missing, report BLOCKED/exit 3: do not invent target values or
activate them from fixture policy. Application, vendors, resource bytes,
and exports remain unchanged. A native MSVC rerun of the revised
source had not yet occurred.

Correction at that checkpoint: **YF-WIN-ZIP-20261028**.
Address the Asio 1607/1607 vendor verification error during the owner's
Windows/MSVC configuration. Without changing pristine vendor hashes,
store the two locked Asio aliases in ZIP as regular files containing
the exact relative target descriptors, removing Windows symlink
permission as a prerequisite. Accept only those exact two descriptors;
report actual paths and hashes on mismatch. Preserve application C++,
exports, and TinyXML2=500.

Naming correction at that checkpoint: **YF-DLL-NAMES-20261028**.
At the owner's instruction, readiness and port exports were named
`DllWaitReady` and `DllServerPort`. Signatures, behavior, and
ordinals 4/5 were preserved, and all calling tools updated.
A subsequent instruction removed duplicate `YellowfieldShutdown`
and ordinal 6, leaving one public `DllShutdown`. Shutdown order
and status codes remained unchanged. Five exports were present
at that checkpoint; earlier source ZIPs are retained.

Continuation work item: **YF-PUBLIC-ABI-20261028**.
After reviewing open work, the owner authorized continuation.
Six C exports of the application DLL were pinned and C++ source
APIs moved to a separate development DLL. An actual
startup/socket/shutdown/unload tool was added. The SURGE marker
adapted RE's RET-only behavior; `DllShutdown` was connected to
actual cleanup. The earlier 548-byte ABI claim was withdrawn for
lack of evidence. New export ordinals 4–6 belonged to Yellowfield;
original Consumer imports and EXE were not changed.
[Scope and limits](../docs/PUBLIC_DLL_ABI.md).

Work item: **YF-RUNTIME-BIND-20261028**.
The owner's correction, translated as "those bindings should already
have been present," required completion of concrete integrations
missing from the earlier product target: actual Asio/ghc application
paths, complete existing configuration application path, source
constructor fields, XML/crypto/router/persistence, hook/provider,
and DLL startup/shutdown ownership. The earlier report-only limit
applied only to the comparison pass, not this implementation task.
Keep vendor bytes and TinyXML2=500 unchanged; do not import Bluefield
application code.
[Details and behavioral differences](../docs/RUNTIME_BINDING.md).

Additional correction, translated: "The metadata under Brownfield
resources was missing and should have been included." Import
Brownfield `resources/preloader_version.rc` VERSIONINFO byte-identically.
Do not invent a fixed SURGE profile name/version. The former RCDATA
decision was later superseded by the owner's avatar correction.
Other PE header/ABI decisions are not automatically selected
by importing this resource.

Subsequent correction, translated: "There are still strange nested
folders under src and include." Apply Brownfield's one-level module
structure. Move 177 files, verify 261 source bodies byte-identical
except for include lines. Record paths and evidence in
[LAYOUT_MIGRATION.json](../metadata/evidence/LAYOUT_MIGRATION.json).

Earlier work item: **YF-COMPARE-20261028**.
The owner requested two separate difference/omission/deviation tables
for Bluefield and RE sources, then authorized continuation.
That pass only covered review, source evidence, documentation, and
packaging; no application behavior/code changes. Bluefield decisions
do not automatically become product instructions.
Tables: [Bluefield](../docs/COMPARISON_HOOK_REFERENCE.md),
[RE](../docs/COMPARISON_RE.md).

Earlier implementation work item: **YF-VDF-PEGTL-20261028**.
After correcting the record, translated as "the PEGTL VDF grammar
already existed," the owner authorized binding that RE grammar to
PEGTL. Authorization covers changing the parser backend to PEGTL
while preserving existing Config2/VDF rules and action semantics.
Bluefield remains comparison input, not application code.
[Details](../docs/VDF_PARSER.md).

Earlier **YF-VENDOR-20261028** and **YF-VENDOR-CONTINUE-20261028**
work installed five original vendor archives and verified them
using compatible LLVM-MinGW 20251118 / Clang 21.1.6. Preserve vendor
bytes, versions, and TinyXML2's default **500**; do not apply
the patch to 100. Do not choose a new storage/transport policy.

Earlier work item: **YF-INFRA-20261028**.
Adapt Brownfield infrastructure for Yellowfield, as separately
authorized when the owner agreed to proceed from the missing-transfer
list. That authorization covers the infrastructure work below without
renewed approval for the same operation. The initial package's
report-only phase does not block this work.

## Scope

Documentation system, Golden tools/pipeline, root layout, metadata
ownership, resource verification, scripts, local build/test orchestration,
compatible tests, command failure/status visibility, and source packaging.
The original requirements are in
[INTEGRATION_REQUIREMENTS.json](../metadata/INTEGRATION_REQUIREMENTS.json);
the current transfer record is
[BROWNFIELD_TRANSFER_MATRIX_V2.json](../metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json).

No application `.cpp`/header semantics were changed by the infrastructure
task. The new CRT probe host is a tool testing the standard compiler CRT,
not a recovered custom CRT. Six old hosts requiring the Brownfield
application ABI were not imported unchanged. The existing Yellowfield
source host and CTest targets use their actual interfaces.

## Standing targets

- First-party namespace `preloader_l`; visible identity SURGE;
  owner-provided artwork preserved byte-for-byte.
- Reverse Engineering is the base function source; Bluefield is explicitly
  selected as donor only for hook 11 and required hook lifecycle;
  other modules remain comparison scope. Frozen inputs are unchanged.
- Eleven-hook MinHook target; runtime patch/rollback/quiescence requires
  separate native acceptance.
- Four active vendors: Asio, MinHook, PEGTL, TinyXML2. The unused Gulrak
  vendor was removed by owner correction. Preserve versions and byte pins;
  copying does not establish active runtime acceptance.
  Current status: `metadata/RUNTIME_CURRENT_STATUS.json`.
- Standard CRT; no automatic fallback to a custom CRT.
- `%LOCALAPPDATA%\SURGE` and Debug-only `surge.log`.
  The runtime adapter follows RE's EXE-relative main -> override
  sequence using `surge.cfg` / `surge_override.cfg`.
  PEGTL implements the existing RE Config2/VDF grammar.
- Optional `preloader_l_online.dll` follows the RE version2/factory/
  virtual-slot contract. Acceptance of actual external provider/native
  compatibility remains open.
- Library synchronization, publication, and official release are
  outside this task's scope.

## Evidence boundaries

Source QA, synthetic parser tests, cross-DLL linking, Wine execution,
actual Windows/MSVC, and game/Consumer acceptance are different evidence
classes. PASS in one is not proof in another. If Golden product values
are unknown, tools fail without changing the file. Brownfield metadata
values have since been explicitly selected by the owner; preserve three
public exports and other owner exceptions.

Additional export instruction: preserve Brownfield's
`DllMain@1`, `SURGE@2`, `DllShutdown@3` ordinal layout.
Remove `DllWaitReady`/`DllServerPort` added for testing/management;
tools use existing EALsxPort/LSX behavior. Imported DLL descriptor order
is separate. At the owner's later explicit instruction, select the
CRYPT32, WS2_32, KERNEL32, USER32, SHELL32, MSWSOCK order only in the
finalized copy. Do not alter raw compiler output; retain read-only
Golden validation.

The final **SURGE-FIDELITY-FIX-20261029** instruction addressed all
four audit findings: Bluefield's narrow network counter and shared
callback Guard scope, RE type-2 event mutex contract, Debug diagnostic
content, and current-status pointers. Preserve standard CRT, Debug-only
file logging, vendor bytes, and Golden policies. Do not rewrite
historical runtime reports; record new status separately.

Delivery version, codename, and revision appear only in the external
archive filename and accompanying checksum file, not in source,
tools, build headings, receipts, or metadata/documentation inside
the package. The packaging tool receives the output path from
its caller.

## Whole-source review continuation

The owner requires review across all application and retained model sources, not
only the latest input delta or an arbitrary defect count. Review mapped source
differences, owned adapters, native boundary call chains, ownership/exception and
concurrency failure paths, and suitable test contributions. Adopt proven fixes and
record concrete pre/post evidence and remaining native/domain limits. Preserve
existing product policies and frozen evidence; do not import unrelated private
CRT or controller restrictions. Current review: docs/BROAD_SOURCE_AUDIT.md.
