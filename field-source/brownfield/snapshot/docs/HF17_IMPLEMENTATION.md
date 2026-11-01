# preloader_l 3.2.5 SQUALERIS HOTFIX 17 — PREPUBLICATION IMPLEMENTATION CANDIDATE

**Parent (immutable official):** HF16 ZIP SHA-256 `9b380e4d953dffd03ace8b762e1199f11ca8ca4f373d86a0c9b3c074d647082c`.
**Status:** HF17 source code integrated; Windows/MSVC, native, game, security and Golden release QA pending. Not the canonical published release.

## Runtime implementation

- Preserves all seven HF16 MinHook detours: Registry A/W Open, Registry A/W Query, Close, connect, CryptQueryObject.
- Adds four post-loader MinHook detours in `optional_hooks.cpp`: `CreateProcessA/W` and `SHGetFolderPathA/W`. Uses the same MinHook ticket/call gate/queue and single global transaction; no alternative hook engine.
- `BlockReruns=true|1` and an exact ASCII case-insensitive configured `ClientPath` match block named CreateProcess. Missing/NULL, non-ASCII, invalid or unmatched paths forward to original. Block sets `ERROR_ACCESS_DENIED` (**project error-policy adaptation**, not asserted exact donor LastError behavior).
- `Documents` config and exact `CSIDL_PERSONAL=5` select local SHGetFolderPath A/W path output (bounded MAX_PATH; UTF-8->UTF-16->ACP A-path with no substitution); other requests forward to original.
- Hooks physically installed as 11 together; runtime-config-gated behavior starts only after Config2 publication. This is **NOT** evidence for donor's conditional physical installation or 7/9/9/11 physical patch counts.
- Source fixes Config2 Game-section normalization for `BlockReruns` and `Documents`.
- B opaque HKEY ownership TEST_ONLY prototype is deliberately not integrated; HF16 Registry implementations stay unchanged.

## Pending acceptance (NOT RUN / NO CLAIM)

Exact HF17 MSVC x64 compile/link; 11-hook post-loader transaction; optional hook actual Win32 API contracts and LastError/CRT A/W semantics; 7/9/9/11 *effective* policy matrix; partial rollback/teardown; both original Consumer game scenarios; all Golden/release gates; full donor parity; B provenance P0 debt.

No HF17 DLL is built, no shipping source promoted, and no Library CURRENT_ARTIFACT update is authorized by this package.

## Prepublication governance migration (non-runtime)
HF17 identity is now checked by an explicit prepublication-only gate. The original HF16 official publication contract and HF16 historic acceptance records remain intact. The operator Windows-build helper is a separate handoff artifact, so no unauthorized root CMD/Python tools enter the shipping source tree. No 8/8 release QA, MSVC binary, native, CRT, Golden, or game PASS is inherited.
