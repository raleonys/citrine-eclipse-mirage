# HF12: Permanent static CRT migration (user authorized)

Current source lineage only. Prior release/historical CRT decisions in docs/CRT_TRANSITION.md and docs/CORVEXON_GOLDEN_TRAIL.md are immutable provenance; this contract **supersedes them prospectively** for this HF12-derived source. It is not a published HOTFIX12.

- Shipping DLL, MinHook and TinyXML2: MSVC `MultiThreaded` `/MT`.
- Native and release hosts: `/MT`; debug DLL/debug helpers: `MultiThreadedDebug` `/MTd`.
- No `/MD` or `/MDd` flags, `/NODEFAULTLIB`, private STL helpers, manual CRT entrypoint, or vendor edits.
- Normal compiler/linker-selected `_DllMainCRTStartup`; the three named shipping exports and `/GS` image-backed writable cookie are unchanged.
- Final PE must not import `MSVCP*`, `MSVCR*`, `VCRUNTIME*`, `UCRTBASE*`, `API-MS-WIN-CRT-*`; OS API dependencies remain governed separately.
- All static libraries in a given link must have matching runtime flags; enforce via source and generated build checks.
- No C++ exception/allocator/storage ownership across the host/DLL public ABI. No shared CRT state assumptions. Run Windows CRT startup/static init, multiple load/unload, TLS/FLS, worker shutdown, failure-atomic close and native scenarios with actual MSVC.
- `PRELOADER_L_COMPATIBILITY_CUSTOM_ENTRY=ON` fails configuration; this old legacy variant is preserved exclusively in earlier lineage artifacts.
- Golden chain `URSYNEX -> ARYRA -> MUSTAVEX -> CORVEXON -> FALCION` remains unchanged; CORVEXON checks the **absence** of CRT DLL imports rather than requiring them.
- No Asio socket ownership changes in this migration; Windows release() 10045 remains separate and unaccepted.
- Validation on Linux cannot establish Windows PE import success or native CRT compatibility. Final release and canonical baseline remain unmodified pending exact Windows acceptance and authoritative QA.
