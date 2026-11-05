# Early registry hook publication — 2026-11-04

The actual OriginalCrt source route already permits a single published general registry callback while host setup is incomplete. No additional production readiness gate should be removed on this evidence. A typed callback exercised from actual hook enable remains an integration verification target; test source is not a test result.

## Fresh native observations

REA6.8.0 server identity and the original SHA-256 were verified before opening Ghidra12.1.4. Target SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Seven complete fresh Evidence records are retained in `rea-evidence-bundle.json` and individual full tool replies. Session closed after export. Function queries report Ghidra's default auto-analysis completed; no DLL execution occurred.

| Native location | Observation |
| --- | --- |
| 0x18004691a, create helper 0466e0 | Successful create writes the trampoline address from R8 to the requested original output cell [R15], when that cell is nonnull. |
| 0x18000f14a / 0x18000f163 | Installer 00f120 calls named creation046970, checks status, then enables046a10. The enabled-target vector is updated after successful enable. |
| 0x180046a10 / 0x180046a15 | Enable wrapper sets EDX=1 and tail-transfers to046230. |
| 0x18000eeaf / 0x18000eece / 0x18000eeed / 0x18000ef0c / 0x18000ef2b | Host setup00e310 installs registry hooks successively using original output cells a3840/a3848/a3850/a3858/a3860. Names are identified by matching the retained native setup analysis and current source descriptor order. |
| 0x18000f823 | RegOpenKeyExA callback00f800 unconditionally calls the published original slot a3840 before configured-name comparisons; examined body has no all-hooks-ready condition. |

Static order establishes pointer-before-enable, not that a particular Windows implementation actually invokes a callback during enable, permits arbitrary reentry, or safely supports concurrent shared-set mutation. Provider terminal-flow/no-return warnings remain provider analysis assumptions.

## Current production source

- `anadius-host-platform-source/assembly.cpp`: actual OriginalCrt install_host selects the per-callback successful-create publication proof and publishing OriginalProvider. It omits the cooperative providers_ready proof.
- `anadius-host-platform-source/platform.cpp`: create_hook calls after_create before returning to the source installer, which calls enable_hook separately. The publication receipt therefore exists before enable enters its engine.
- `anadius-original-provider-source/originals.cpp`: Publication::load accepts a published selected slot in Setup as well as Ready. Its setup path reads a sealed copy while other raw slots may still be populated. The legacy Bound provider's controller-only restriction does not govern this selected provider.
- `anadius-foreign-callback-source/foreign.cpp`: enter selects per-hook Checking before the cooperative !ready_ rejection; a published slot becomes Full and enters CallbackThreads. A single controller or foreign actor can therefore enter before host completion.
- `anadius-thread-domain-source/domain.cpp`: CallbackThreads borrows an existing non-sentinel FLS/PTD, including the initialization thread's, or constructs an owned foreign-thread PTD. There is no blanket controller-identity rejection.
- `anadius-native-context-source/context.cpp`: setup begins admission then publication; completion seals publication then admission.

These are source observations, independently reviewed by a subagent. Existing `foreign_tests.cpp` has an incomplete-setup Full callback scenario using a folder token; that does not by itself test typed registry dispatch during actual host enable.

## Limits and next target

General callback overlap/reentry, missing selected receipt, construction sentinel, poison/closing and failed PTD acquisition are distinct constraints. Null registry query names still access shared handle sets; they are not argument-only forwarding candidates. A registry query's client-path native wait(-1) does not establish that the whole callback body is a pure SDK call.

No production readiness rewrite, new unload requirement or numerical DLL-completion credit is justified here. Integration source additions, if completed, are recorded separately with exact changed-file hashes and explicitly remain uncompiled/unexecuted. Existing delivered DLL/ZIP remain the last actual build.
