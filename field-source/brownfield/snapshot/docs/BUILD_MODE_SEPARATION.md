# Build and publication separation (2026-10-18; user-authorized)

Compilation and test binaries must not require a release identity, Library access, publication intent, G0/G1/G2/G4 receipt, or authoritative 8-shard QA. A missing external master must never prevent invoking the compiler.

## Operator build modes

- `scripts/build_orchestration/build_unstable.cmd`: **COMPILE_ONLY**, raw unsigned local `build/preloader_l_compile_msvc/preloader.unsigned.dll`; no Golden finalization; no `dist/` mutation and no official release claim. CMake/MSVC configure, compile and link errors remain fatal.
- `build_debug.cmd`, `build_native_acceptance.cmd`, `build_crt_acceptance.cmd`: local build/validation, without governance dependency. Actual native/CRT test failures remain fatal.
- `build_production.cmd`: local Golden pipeline, still enforcing URSYNEX, ARYRA, MUSTAVEX, CORVEXON, FALCION and binary integrity. It writes only a LOCAL `dist/preloader_l.dll`. This is **not** Library publication and must not update `CURRENT_ARTIFACT`.

## Canonical publication: separate action

Only a distinct, explicitly user-authorized release transaction may promote an artifact into the Library. Publication remains fail-closed on independent source/master/parent/intent SHA pins, exact G0/G1/G2 receipt integrity, relevant G3 applicability, authoritative 8/8 source plus a single fresh-extract 8/8 at Hotfix G4, CRC, readback, and atomic/current-pointer transaction.

No development build command is permitted to call the publication transaction. Compile PASS or Golden PASS cannot be re-labeled official release PASS. Build failures and release failures must be separately reported. Prior HF17 publishing receipts and HF18 Windows acceptance are not retroactively changed.
