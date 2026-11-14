# Documentation index and authority

This catalog reviews the first-party Markdown documents and companion CSVs
against accepted `main@b6d3cfc48b1cbd0cca560d7f6f8931d7951be21c` and the changes
proposed here. It covers the root guides, PR template, tools guide and every
`docs/*.md` / `docs/*.csv` file. Vendor documentation and retained
`metadata/evidence/` material keep their original bytes and evidence limits.
This is documentation/source-contract reconciliation, not a new native audit,
complete donor semantic proof or release/game acceptance.

## How to read the documents

- **Current**: current navigation, policy or implementation guidance. Normative
  authority remains the owner's decisions, root governance and executable contracts.
- **Checkpoint**: a historical comparison, correction or test plan. Its filenames,
  counts, PASS/NOT_RUN fields and future-work instructions apply to its source
  snapshot. Use the linked current guide before acting on it.
- **Mixed**: an active implementation/diagnostic guide with clearly bounded
  historical results. Preserve those results and follow its current contract links.
- **Review**: a dated source/provenance review with accepted follow-ups and still
  open questions. A closed test gap does not close every donor/provenance issue.

[AGENTS.md](../AGENTS.md) and [GOVERNANCE.md](../GOVERNANCE.md) own contribution
rules; [DEVELOPMENT.md](DEVELOPMENT.md) owns commands;
[CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) summarizes runtime bindings;
[VALIDATION.md](VALIDATION.md) separates evidence classes. The catalog does not
turn an old receipt or pending status into a current acceptance result.

## Current facts checked against source

These values describe the accepted contracts at this review baseline, not
performance rankings or executed-test totals. Update them alongside the owning
contract if a later approved change alters the implementation.

| Fact | Value | Authority |
| --- | --- | --- |
| Production C++ TUs | `62` | [partition policy](../metadata/BUILD_PARTITION_POLICY.json): 60 objects + VDF + DLL entry |
| Reference C++ TUs | `78` | [partition policy](../metadata/BUILD_PARTITION_POLICY.json); retained, not globally dead |
| Production C exports | `DllMain@1, SURGE@2, DllShutdown@3` | [public ABI](../metadata/PUBLIC_DLL_ABI.json) |
| CryptQuery compatibility suffix | `preloader_l.dll` | [selector header](../include/laureondo/platform/crypt_query.hpp); independent of the shipping DLL name |
| Additional PE resource assets | `0` | [resource policy](../metadata/RESOURCE_METADATA_CONTRACT.json); VERSIONINFO only, both avatars are read-only C++ data |
| Manual operator fixture workflow | `operator-output.yml` | [workflow](../.github/workflows/operator-output.yml); no `operator_output` input on `ci.yml` |

The current vendors, archive hashes and compiler compatibility boundary are in
[VENDORING.md](VENDORING.md). Golden preserves the native compilation timestamp
and normalizes the selected import descriptor order; earlier fixed-timestamp
and raw-linker-order descriptions are historical. Ordinary PR opening runs
source integrity; full validation uses the accepted synchronization policy.
Accepted-main pushes separately run only the three CodeQL analyses in that
workflow, as integrated in PR #353; no Windows/Golden build is added to that
event. A proposed workflow change is not active until owner integration.

`metadata/RUNTIME_CURRENT_STATUS.json` combines current binding fields with
historical validation checkpoints. Its corrected 62/78 source counters agree
with the partition policy; old migration NOT_RUN fields do not negate later
exact-head Windows results. Do not rewrite retained receipts or infer global
PASS from that summary. Issue #16's accepted component-pinning scope is closed;
Issue #109's Registry provenance and original-donor retirement remain open.

## Complete catalog

| Document | Role | Current guidance or implementation owner |
| --- | --- | --- |
| [.github/pull_request_template.md](../.github/pull_request_template.md) | Current | [GITHUB_WORKFLOW.md](GITHUB_WORKFLOW.md) |
| [AGENTS.md](../AGENTS.md) | Current | [../GOVERNANCE.md](../GOVERNANCE.md) |
| [CHANGELOG.md](../CHANGELOG.md) | Mixed | [../GOVERNANCE.md#changelog-maintenance](../GOVERNANCE.md#changelog-maintenance) |
| [GOVERNANCE.md](../GOVERNANCE.md) | Current | [../AGENTS.md](../AGENTS.md) |
| [README.md](../README.md) | Current | [DEVELOPMENT.md](DEVELOPMENT.md) |
| [tools/README.md](../tools/README.md) | Current | [../tools/inspect_dll.py](../tools/inspect_dll.py) |
| [API_IMPORT_DIFFERENCES.md](API_IMPORT_DIFFERENCES.md) | Checkpoint | [RE_COPY_RECOVERY.md](RE_COPY_RECOVERY.md) |
| [API_IMPORT_RESEARCH.md](API_IMPORT_RESEARCH.md) | Checkpoint | [RE_COPY_RECOVERY.md](RE_COPY_RECOVERY.md) |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Current | [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) |
| [ASIO_UPDATE.md](ASIO_UPDATE.md) | Checkpoint | [VENDORING.md](VENDORING.md) |
| [ASIO_WINDOWS_TARGET_FIX.md](ASIO_WINDOWS_TARGET_FIX.md) | Checkpoint | [VENDORING.md](VENDORING.md) |
| [AUTOMATIC_LISTENER.md](AUTOMATIC_LISTENER.md) | Current | [../metadata/LISTENER_PORT_POLICY.json](../metadata/LISTENER_PORT_POLICY.json) |
| [AVATAR_UPDATE.md](AVATAR_UPDATE.md) | Current | [../metadata/AVATAR_DATA_CONTRACT.json](../metadata/AVATAR_DATA_CONTRACT.json) |
| [BROAD_SOURCE_AUDIT.md](BROAD_SOURCE_AUDIT.md) | Checkpoint | [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) |
| [BUILD_CLEANUP.md](BUILD_CLEANUP.md) | Checkpoint | [BUILD_PARTITION.md](BUILD_PARTITION.md) |
| [BUILD_PARTITION.md](BUILD_PARTITION.md) | Current | [../metadata/BUILD_PARTITION_POLICY.json](../metadata/BUILD_PARTITION_POLICY.json) |
| [BUILD_PATH_TRUST.md](BUILD_PATH_TRUST.md) | Current | [../scripts/build_orchestration/build_common.ps1](../scripts/build_orchestration/build_common.ps1) |
| [CODEBASE_OPTIMIZATION.md](CODEBASE_OPTIMIZATION.md) | Mixed | [BUILD_PARTITION.md](BUILD_PARTITION.md) |
| [COMPARISON_HOOK_REFERENCE.csv](COMPARISON_HOOK_REFERENCE.csv) | Checkpoint | [COMPARISON_HOOK_REFERENCE.md](COMPARISON_HOOK_REFERENCE.md) |
| [COMPARISON_HOOK_REFERENCE.md](COMPARISON_HOOK_REFERENCE.md) | Checkpoint | [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) |
| [COMPARISON_RE.csv](COMPARISON_RE.csv) | Checkpoint | [COMPARISON_RE.md](COMPARISON_RE.md) |
| [COMPARISON_RE.md](COMPARISON_RE.md) | Checkpoint | [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) |
| [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) | Current | [../metadata/RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json) |
| [DEVELOPMENT.md](DEVELOPMENT.md) | Current | [../scripts/build_orchestration/build_profile.ps1](../scripts/build_orchestration/build_profile.ps1) |
| [DLL_SIZE_AND_NATIVE_ACCEPTANCE.md](DLL_SIZE_AND_NATIVE_ACCEPTANCE.md) | Checkpoint | [VALIDATION.md](VALIDATION.md) |
| [DONOR_RECOVERY.md](DONOR_RECOVERY.md) | Checkpoint | [REMAINING_WORK.md](REMAINING_WORK.md) |
| [ENGLISH_ONLY_AUDIT.md](ENGLISH_ONLY_AUDIT.md) | Mixed | [../scripts/check_english_only.py](../scripts/check_english_only.py) |
| [ENGLISH_ONLY_CPP_TEST_REVIEW.md](ENGLISH_ONLY_CPP_TEST_REVIEW.md) | Checkpoint | [ENGLISH_ONLY_AUDIT.md](ENGLISH_ONLY_AUDIT.md) |
| [FILESYSTEM_BACKEND_COMPARISON.md](FILESYSTEM_BACKEND_COMPARISON.md) | Checkpoint | [WINDOWS_IO_FIX.md](WINDOWS_IO_FIX.md) |
| [GITHUB_WORKFLOW.md](GITHUB_WORKFLOW.md) | Current | [../AGENTS.md](../AGENTS.md) |
| [GOLDEN_AVATAR_FIX.md](GOLDEN_AVATAR_FIX.md) | Checkpoint | [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md) |
| [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md) | Current | [../scripts/run_golden_pipeline.py](../scripts/run_golden_pipeline.py) |
| [HOOK_ALIGNMENT.md](HOOK_ALIGNMENT.md) | Checkpoint | [HOOK_PATH_SCAN.md](HOOK_PATH_SCAN.md) |
| [HOOK_ENGINE_HARDENING.md](HOOK_ENGINE_HARDENING.md) | Checkpoint | [MITHRILHOOK_INTEGRATION.md](MITHRILHOOK_INTEGRATION.md) |
| [MITHRILHOOK_INTEGRATION.md](MITHRILHOOK_INTEGRATION.md) | Current | [../metadata/HOOK_ENGINE.json](../metadata/HOOK_ENGINE.json) |
| [HOOK_PATH_SCAN.md](HOOK_PATH_SCAN.md) | Current | [../include/laureondo/platform/crypt_query.hpp](../include/laureondo/platform/crypt_query.hpp) |
| [HOOK_PIONEERING_REVIEW.md](HOOK_PIONEERING_REVIEW.md) | Review | [RE_A11_09_REGISTRY_HKEY_BOUNDARY.md](RE_A11_09_REGISTRY_HKEY_BOUNDARY.md) |
| [IDENTIFIER_POLICY.md](IDENTIFIER_POLICY.md) | Current | [../scripts/check_repository_contract.py](../scripts/check_repository_contract.py) |
| [INDEX.md](INDEX.md) | Current | [../GOVERNANCE.md](../GOVERNANCE.md) |
| [INFRASTRUCTURE_ADAPTATION.md](INFRASTRUCTURE_ADAPTATION.md) | Checkpoint | [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md) |
| [MESSAGE_ERROR_COMPATIBILITY.md](MESSAGE_ERROR_COMPATIBILITY.md) | Current | [../src/protocol/messages.cpp](../src/protocol/messages.cpp) |
| [MSVC_NATIVE_LOOKUP_FIX.md](MSVC_NATIVE_LOOKUP_FIX.md) | Checkpoint | [DEVELOPMENT.md](DEVELOPMENT.md) |
| [MSVC_PROFILE_FIX.md](MSVC_PROFILE_FIX.md) | Checkpoint | [DEVELOPMENT.md](DEVELOPMENT.md) |
| [NATIVE_ALLOCATION_FIXTURE.md](NATIVE_ALLOCATION_FIXTURE.md) | Mixed | [../tests/native/net_failure_tests.cpp](../tests/native/net_failure_tests.cpp) |
| [PR23_POSTMERGE_SHUTDOWN_ACCEPTANCE.md](PR23_POSTMERGE_SHUTDOWN_ACCEPTANCE.md) | Checkpoint | [GITHUB_WORKFLOW.md](GITHUB_WORKFLOW.md) |
| [PRODUCTION_IMPORT_FIX.md](PRODUCTION_IMPORT_FIX.md) | Checkpoint | [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md) |
| [PRODUCT_IDENTITY_FIX.md](PRODUCT_IDENTITY_FIX.md) | Checkpoint | [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) |
| [PROJECT_BRANDING.md](PROJECT_BRANDING.md) | Current | [../resources/branding/laureondo.png](../resources/branding/laureondo.png) |
| [PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md) | Current | [../metadata/PUBLIC_DLL_ABI.json](../metadata/PUBLIC_DLL_ABI.json) |
| [REFERENCE_METADATA_AUDIT.md](REFERENCE_METADATA_AUDIT.md) | Checkpoint | [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md) |
| [REFERENCE_METADATA_AVATAR.md](REFERENCE_METADATA_AVATAR.md) | Checkpoint | [AVATAR_UPDATE.md](AVATAR_UPDATE.md) |
| [REMAINING_WORK.md](REMAINING_WORK.md) | Review | [SOURCE_DIFFERENTIAL_B49_B54.md](SOURCE_DIFFERENTIAL_B49_B54.md) |
| [REPOSITORY_BADGES_AND_OPENING_CI.md](REPOSITORY_BADGES_AND_OPENING_CI.md) | Current | [../.github/workflows/repository-badges.yml](../.github/workflows/repository-badges.yml) |
| [RE_A11_09_REGISTRY_HKEY_BOUNDARY.md](RE_A11_09_REGISTRY_HKEY_BOUNDARY.md) | Review | [../src/platform/registry.cpp](../src/platform/registry.cpp) |
| [A12_DEVIATIONS.md](A12_DEVIATIONS.md) | Review | [XML_BACKEND_COMPARISON.md](XML_BACKEND_COMPARISON.md) |
| [RE_COPY_RECOVERY.md](RE_COPY_RECOVERY.md) | Current | [../src/platform/file_copy.cpp](../src/platform/file_copy.cpp) |
| [RE_INPUT_REVIEW.md](RE_INPUT_REVIEW.md) | Checkpoint | [REMAINING_WORK.md](REMAINING_WORK.md) |
| [ROOT_LAYOUT_AND_BUILD_OWNERSHIP.md](ROOT_LAYOUT_AND_BUILD_OWNERSHIP.md) | Current | [../CMakeLists.txt](../CMakeLists.txt) |
| [RUNTIME_BENCHMARKS.md](RUNTIME_BENCHMARKS.md) | Current | [../tests/diagnostics/runtime_load/run.py](../tests/diagnostics/runtime_load/run.py) |
| [RUNTIME_BINDING.md](RUNTIME_BINDING.md) | Checkpoint | [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) |
| [RUNTIME_CONNECTIONS.csv](RUNTIME_CONNECTIONS.csv) | Checkpoint | [RUNTIME_CONNECTIONS.md](RUNTIME_CONNECTIONS.md) |
| [RUNTIME_CONNECTIONS.md](RUNTIME_CONNECTIONS.md) | Checkpoint | [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) |
| [RUNTIME_RESILIENCE.md](RUNTIME_RESILIENCE.md) | Mixed | [SESSION_TRANSPORT.md](SESSION_TRANSPORT.md) |
| [SDK_IDENTITY.md](SDK_IDENTITY.md) | Current | [../metadata/SDK_IDENTITY_POLICY.json](../metadata/SDK_IDENTITY_POLICY.json) |
| [SESSION_TRANSPORT.md](SESSION_TRANSPORT.md) | Current | [../metadata/SESSION_TRANSPORT_POLICY.json](../metadata/SESSION_TRANSPORT_POLICY.json) |
| [SHUTDOWN_RECOVERY_AUDIT.md](SHUTDOWN_RECOVERY_AUDIT.md) | Checkpoint | [SOCKET_RETIREMENT.md](SOCKET_RETIREMENT.md) |
| [SOCKET_RETIREMENT.md](SOCKET_RETIREMENT.md) | Mixed | [../src/net/asio_service.cpp](../src/net/asio_service.cpp) |
| [SOURCE_DIFFERENTIAL_B49_B54.md](SOURCE_DIFFERENTIAL_B49_B54.md) | Review | [REMAINING_WORK.md](REMAINING_WORK.md) |
| [SOURCE_REVIEW.md](SOURCE_REVIEW.md) | Checkpoint | [REMAINING_WORK.md](REMAINING_WORK.md) |
| [SOURCE_SCOPE.md](SOURCE_SCOPE.md) | Mixed | [BUILD_PARTITION.md](BUILD_PARTITION.md) |
| [STARTUP_BUILD_FIX.md](STARTUP_BUILD_FIX.md) | Mixed | [DEVELOPMENT.md](DEVELOPMENT.md) |
| [TRANSLATION_UNIT_USAGE.csv](TRANSLATION_UNIT_USAGE.csv) | Checkpoint | [BUILD_PARTITION.md](BUILD_PARTITION.md) |
| [UNLOAD_RECOVERY.md](UNLOAD_RECOVERY.md) | Mixed | [../metadata/UNLOAD_RECOVERY_CONTRACT.json](../metadata/UNLOAD_RECOVERY_CONTRACT.json) |
| [VALIDATION.md](VALIDATION.md) | Current | [GITHUB_WORKFLOW.md](GITHUB_WORKFLOW.md) |
| [VDF_PARSER.md](VDF_PARSER.md) | Mixed | [../src/config/parser.cpp](../src/config/parser.cpp) |
| [VENDORING.md](VENDORING.md) | Mixed | [../metadata/VENDOR_LOCK.json](../metadata/VENDOR_LOCK.json) |
| [WEEKLY_RELEASE.md](WEEKLY_RELEASE.md) | Current | [../metadata/WEEKLY_NAMING_POLICY.json](../metadata/WEEKLY_NAMING_POLICY.json) |
| [WINDOWS_IO_FIX.md](WINDOWS_IO_FIX.md) | Mixed | [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md) |
| [XML_BACKEND_COMPARISON.md](XML_BACKEND_COMPARISON.md) | Mixed | [../tests/diagnostics/xml_backend/run.py](../tests/diagnostics/xml_backend/run.py) |

## Maintenance boundary

Review the matching current guide when code, workflow, metadata or CLI changes.
Keep the catalog complete when adding documents. Retain original checkpoint
hashes, test totals and archive names; add a supersession note rather than
rewriting a historical observation as a new result. Relative file links and the
critical current facts above are checked by the host infrastructure suite.

A Markdown/link/contract check cannot decide whether every explanatory sentence
is semantically current. Continue source-backed review after relevant changes.
The remaining donor and Consumer questions are recorded in
[REMAINING_WORK.md](REMAINING_WORK.md), not erased by this documentation pass.
