# Current outstanding work

Recent changes are documented in
[CODEBASE_OPTIMIZATION.md](CODEBASE_OPTIMIZATION.md); local test results are
in the [validation record](../metadata/evidence/optimization/VALIDATION.json).

Five Windows profiles and Native 193/193 PASS were confirmed by logs for
the previously uploaded DLL; the owner reported game acceptance. The
[pinned acceptance record](../metadata/evidence/OWNER_NATIVE_ACCEPTANCE.json)
retains these results. Acceptance of that earlier DLL does not cover
subsequent XML/Release source changes.

| Outstanding item | Next action | Evidence boundary |
|---|---|---|
| New Windows/MSVC acceptance | Run the five normal build profiles and Native Acceptance on a clean source checkout. | LLVM-MinGW/Wine and sanitizer evidence exists for new source; MSVC was not executed. |
| New game acceptance | Verify the applicable game scenarios and shutdown with the Production `dist/preloader_l.dll`. | The earlier game PASS does not replace retesting of new source. |
| Native size attribution | Build before/after with identical MSVC/SDK/Release settings; collect DLL/MAP/OBJ evidence using `collect_size_evidence.py`. | The earlier native DLL/log exists, but its same-build native OBJ/MAP was not uploaded. |
| Exceptional Asio teardown | Inject native faults in the `stop()` post/join/resource-failure paths while preserving ownership. | A destructor-exception warning remains open; no exception-swallowing change was introduced without proof of a safe fix. |

An absent AST reference is not proof that a file can be removed; virtual,
callback, lifetime, and development API roots remain in scope. The earlier
notes below are historical and must not replace the current local results
or outstanding-work list.

## Historical acceptance notes

Current note (`YF-BLUEFIELD-ALIGN-20261029`): the Bluefield eleventh
hook/lifecycle and all declared Brownfield metadata targets are connected
at source level. Current verification is recorded in
[BLUEFIELD_ALIGNMENT_STATUS.json](../metadata/evidence/HOOK_ALIGNMENT_STATUS.json).
Actual Windows/MSVC Native/Production and game acceptance on the new source
remain required. The earlier acceptance details below are historical.

Current note (`YF-GOLDEN-AVATAR-20261029`): all four Golden product targets
are now defined, resolving the former missing-target `BLOCKED` status.
The remaining task is actual Windows Native/Golden acceptance on new source.
See [current record](GOLDEN_AVATAR_FIX.md). The list below preserves
historical acceptance details.

# Outstanding acceptance tasks

This list distinguishes missing application code from acceptance tests
that were not executed. Current source, the fixed C interface, and Wine
results are in [PUBLIC_ABI_STATUS.json](../metadata/evidence/PUBLIC_ABI_STATUS.json).

| Item | Concrete next action | Current limit |
|---|---|---|
| Windows/MSVC | Rerun the Native profile with the namespace-collision fix and retain full CTest results. | Latest owner logs show Compile/Debug runtime and CRT 6/6 PASS. The complete Native test suite did not run because of C2668 in an asynchronous receive test. |
| Windows command interface | Exercise error codes, final result, and console pause behavior for Compile/Debug/Native/CRT entrypoints using actual PowerShell/CMD. | Source and negative contract checks passed; the Windows console behavior was not exercised. |
| Actual online provider | Exercise version 2/factory/virtual-slot and error/cleanup paths using a `preloader_l_online.dll` with a known hash and version. | Current tests use a controlled fixture DLL; an original external provider has not been supplied or accepted. |
| Named/versioned Consumer/game | Pin the exact host and how it loads the branded DLL; observe actual startup, LSX requests, hook conditions, and shutdown. | Static imports in the supplied original EXE reference `anadius64!anadius`. This source package does not modify the EXE or claim automatic compatibility under other names. |
| Complete Golden pipeline | Prove remaining section/header/import/CRT targets after actual MSVC linking, then run all six stages against the real DLL. | Resource profile and five C exports are pinned. All product targets are now defined; actual MSVC six-gate acceptance and final-name runtime checks remain required for new source. Fixture results are not an MSVC Golden substitute. |

The 548-byte legacy ABI was removed from the proven outstanding-work list;
the supplied RE sources had no authoritative interface definition of that
size. The SURGE marker and actual DllShutdown were implemented; see
[PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md). Library synchronization, publication,
and official release were outside that work's scope.
