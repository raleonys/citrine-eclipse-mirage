> **HF12 prospective supersession (2026-10-13):** Authorized current HF12 source uses static `/MT` shipping and `/MTd` debug CRT. Historical detailed records below remain immutable lineage. See `docs/CRT_STATIC_POLICY.md`. This is NOT_RELEASE until exact Windows acceptance.

# CRT transition — retained authority plus 3.2.0 path note

For 3.2.0, the CRT acceptance launcher is `scripts/build_orchestration/build_crt_acceptance.cmd`. The detailed transition record follows unchanged.

<!-- DOC_PRESERVE_BEGIN:HF22_V5_CRT_TRANSITION -->
# 3.1.8 CRT Transition

## Canonical model

Shipping uses MSVC `/MD` + `/GS` and the default linker-selected `_DllMainCRTStartup`. CRT/VCRuntime owns security-cookie initialization, EH/FH4 runtime support, static initialization and termination. Exported `DllMain @1` remains the project loader callback but is not the PE entrypoint.

## Legacy comparison

`PRELOADER_L_LEGACY_CUSTOM_ENTRY=ON` reconstructs the superseded 3.1.7 custom-entry boundary with `/NODEFAULTLIB`, `/ENTRY:DllMain`, `/Zl`, manual `__security_init_cookie()` and the local freestanding memory shim. It exists only for A/B comparison and can never satisfy canonical shipping CORVEXON acceptance.

## ABI rule

No CRT/STL ownership crosses the public DLL boundary. Exports continue to use primitive values/caller-owned buffers/opaque module lifetime semantics. This avoids allocator and CRT-state coupling between the DLL and host.

## Runtime-selection authority

Canonical CMake selects `MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"`. The acceptance harness does **not** require a literal `/MD` token to appear in generator-specific `flags.make`, `build.make` or response-file text. Source policy proves the CMake runtime selection; generated artifacts reject contradictory `/MT`, `/MTd` or `/MDd` selections; final binary imports, entrypoint relation, CORVEXON and native scenarios prove the realized runtime model. This closes the CANDIDATE01 false-negative `canonical CRT build missing /MD`.

## Acceptance

`scripts/build_orchestration/build_crt_acceptance.cmd` demonstrates: source runtime policy passes; both comparison and canonical builds compile; canonical generated flags contain `/GS` and no conflicting runtime selection; canonical link command contains neither `/NODEFAULTLIB` nor custom `/ENTRY`; Golden chain passes on the canonical image; CRT import growth stays within the allowlist; `AddressOfEntryPoint` differs from `DllMain @1`; `/GS` Load Config SecurityCookie is valid; native scenarios pass.

## Debug build-policy clarification

Debug C++ compiler switches are generator-expression scoped to `COMPILE_LANGUAGE:CXX` so MASM does not consume C++-only options. `MultiThreadedDebugDLL` (`/MDd`) is the sole debug CRT selection authority; the debug DLL no longer manually names release `vcruntime`.
<!-- DOC_PRESERVE_END:HF22_V5_CRT_TRANSITION -->
