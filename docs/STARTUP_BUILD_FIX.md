# Startup lock and build entrypoints

The owner reported that the game did not start at all with
`dist/preloader_l.dll` built from the last
`yellowfield-bluefield-aligned-source.zip` package, while Bluefield performed
the same product task. `surge.cfg` must remain optional. A successful build
or test does not establish real game compatibility.

## Confirmed startup defect

During a native registry query for tracked `ClientValue`, the
`registry_gate` lock that we had introduced remained held while waiting
for startup server/provider/extra phases. If another thread needed to
enter a registry callback to advance startup, both sides could wait
on each other. Bluefield addressed this in FIX19 through
`callback_lock::Guard/Pause`, but that required component was missing
from the earlier Yellowfield adaptation.

The new regression invokes actual native callback entrypoints on two
threads. While the first query waits for its phase, the second thread's
callback must be able to advance. The previous code did not advance
within a controlled five-second observation and exited with code 1.
The test then released and joined both threads; this failure evidence
is retained.

The correction uses the supplied Bluefield header byte-identically.
Only the registry callback's tracked recursive locks are released
during the wait and reacquired afterwards. The native binding lease
remains live throughout the wait; original-call, match/wait/write
ordering, results, and LastError remain unchanged. `registry.cpp`
is unchanged. The separate network callback lock scope is not
modified by this correction.

This demonstrates a source defect capable of blocking startup.
It does not establish that this was the only cause of the owner's
game-startup failure or that the game now starts.

## Other failure in the build log

The log identified the earlier
`yellowfield-production-import-fix-source` checkout; a folder name
alone is not exact identity evidence without a ZIP hash. The build
stopped at C2589 in test-only `fake_win32.cpp:159/160` before CTest.
Because `hook_engine_fixture_tests` links MinHook independently,
it did not receive the core target's PUBLIC NOMINMAX definition.
PRIVATE NOMINMAX was added to this target. Vendor sources and test
bodies were not patched to address this error.

## Five command headings

| Command | Heading | Purpose |
|---|---|---|
| `build_unstable.cmd` | COMPILE ONLY | Compile unsigned DLL, verify startup/shutdown |
| `build_debug.cmd` | DEBUG BUILD | Compile Debug DLL, verify startup/shutdown |
| `build_native_acceptance.cmd` | NATIVE ACCEPTANCE | Compile/run native Windows tests |
| `build_crt_acceptance.cmd` | STATIC CRT ACCEPTANCE | Static CRT and DLL-interface tests |
| `build_production.cmd` | PRODUCTION BUILD | Compile shipping DLL, Golden validation and dist output |

The headings come from the supplied Brownfield profile names. Former
HF12 labels and descriptions unrelated to current work were not copied.
Exit codes, colored result, pause, argument propagation, and Golden
gates were retained. Automatic publication was not added.

`CLEARED_KNOWN_GENERATED_CACHE_ONLY` is the outcome of the helper that
removes known first-party Python bytecode caches to prevent the earlier
FALCION failure. `removed: []` indicates that nothing was removed.
The build flow no longer prints this on successful completion; details
are written to `PYTHON_CACHE_CLEANUP.json` in the selected build
directory. Cleanup errors stop the upstream build via stderr and
nonzero exit code; silence on success does not conceal errors.

## Outstanding game investigation

Startup without configuration using defaults is preserved.
Bluefield resolves its configuration/provider paths relative to
the DLL; Yellowfield uses the EXE. This deployment difference is
recorded, and no new search or precedence policy was added without
validating file placement. The SURGE/DllMain/DllShutdown exports
are compatible; the two additional Yellowfield management exports
in that snapshot came from a previous explicit owner instruction.

Actual Windows/MSVC Production execution and a game rerun are
required for the changed source. Status and raw test results:
`metadata/evidence/STARTUP_BUILD_FIX_STATUS.json`.

Cross-validation passed Release 173/173, affected Debug 8/8,
infrastructure 29/29, and three actual DLL runtime cycles without
configuration. The original lock test's exit-1 evidence is retained.
No game rerun or native MSVC execution of this revised source was
performed in that session.
