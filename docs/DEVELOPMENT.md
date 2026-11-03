# Local development and operator commands

Windows requires Visual Studio C++ x64 tools/SDK, CMake >= 3.20, and Python
3.9 or later. Production and static-test builds use the compiler's standard CRT:
MSVC Release `/MT` and Debug `/MTd`. The separate C++ development DLL and its
host use `/MD` and `/MDd`, with their own matching vendor builds, because STL
objects cross that module boundary. This Linux session does not claim PowerShell/CMD or MSVC
execution.

Human-facing commands under `scripts/build_orchestration/`:

| Command | Output / verification |
|---|---|
| `build_unstable.cmd` | Release application DLL; checks exports/resources and actual load/socket/shutdown/unload |
| `build_debug.cmd` | Debug application DLL with the same public API checks; debugger symbols are retained |
| `build_native_acceptance.cmd` | Citrine Eclipse Mirage CTest source/adapter/CRT tests on Windows |
| `build_crt_acceptance.cmd` | Builds and runs only selected compiler CRT/public DLL/source smoke targets |
| `build_production.cmd` | Native MSVC Golden link, six-stage verification and validated `dist` artifacts |

The human-facing CMD window colors the final result, shows the exit code,
and waits before closing. Automation should invoke the matching `.ps1`
script directly without waiting. Arguments include
`-BuildDir build\...` and `-Config Release|RelWithDebInfo|Debug`.
The sole profile executor is `build_profile.ps1`; `build_common.ps1` is the
shared helper. Each profile uses its own build directory, lock, toolchain
hash record, and build receipt. MSVC environment variables are restored
after execution; test failures are never converted into success.
When Golden product targets are missing, the operator reports `BLOCKED`
and returns exit code 3. Receipts distinguish build success from profile
validation. See [MSVC_PROFILE_FIX.md](MSVC_PROFILE_FIX.md) for the MSVC
resource/test compatibility correction.

Build evidence is written to `BUILD_RECEIPT.json` in the selected build
directory. Production verification writes `golden/GOLDEN_REPORT.json` and,
after all required checks pass, `dist/DIST_RECEIPT.json` beside the DLL and
checksum. CI and weekly draft packaging consume these same paths. Older
generated receipts are not reused as current evidence; regenerate the selected
profile after upgrading. Publication state is recorded separately as `DRAFT`
by weekly release preparation.

Source QA (does not require Windows):

```sh
python -B scripts/run_source_qa.py --report build/qa/SOURCE_QA.json
```

After an intentional source change, update `SOURCE_FILES.json` and the
corresponding provenance record. Explicitly refresh the complete source
identity, then verify it again:

```sh
python -B scripts/check_manifest.py --refresh
python -B scripts/run_source_qa.py
python -B scripts/package_source.py --output ../citrine-eclipse-mirage-source.zip
python -B scripts/check_falcion_golden_cleaning.py --package ../citrine-eclipse-mirage-source.zip
```

Packaging does not refresh the manifest automatically. The Golden command
returns a nonzero exit code if required product targets are missing;
[GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md) documents that case. Building
and testing use the same source and binary checks whether started from an
operator launcher or GitHub Actions.

## Current concrete runtime

Application ownership, actual vendor I/O, and DLL worker startup are
described in [RUNTIME_BINDING.md](RUNTIME_BINDING.md).
Normal Windows/MSVC builds do not use Wine. For LLVM-MinGW/Wine cross-tests,
the CMake option `-DPRELOADER_L_WINE_EXECUTABLE=/path/to/wine64` selects
`scripts/run_wine_test.py`, which preserves the actual process exit code.
Output descriptors inherited by Wine services do not determine test success
or timeout behavior.

Current application C interface and source DLL separation:
[PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md).
Current verification:
[PUBLIC_ABI_STATUS.json](../metadata/evidence/PUBLIC_ABI_STATUS.json).

The latest owner-provided native logs report Compile/Debug/CRT PASS;
the remaining Native test build correction is documented in
[MSVC_NATIVE_LOOKUP_FIX.md](MSVC_NATIVE_LOOKUP_FIX.md).

Current five-launcher and cache-cleanup record:
[STARTUP_BUILD_FIX.md](STARTUP_BUILD_FIX.md).
Successful cleanup does not print JSON to the console; details are stored
in `PYTHON_CACHE_CLEANUP.json` in the selected build directory.
Failures stop the build with a nonzero exit code.
