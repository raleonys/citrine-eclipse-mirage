# Local development and operator commands

Windows requires Visual Studio C++ x64 tools/SDK, CMake >= 3.20, and Python
3.9 or later. Project C++ targets use C++20; C sources remain C17.
The optional version-3 `CMakePresets.json` requires CMake 3.21 or later;
ordinary `cmake -S ... -B ...` builds retain the 3.20 minimum.
The selected owner-supplied PEGTL 4.x snapshot supports this standard.
Reconfigure existing build directories after changing dependency snapshots.
Production and static-test builds use the compiler's standard CRT:
MSVC Release `/MT` and Debug `/MTd`. The separate C++ development DLL and its
host use `/MD` and `/MDd`, with their own matching vendor builds, because STL
objects cross that module boundary. Linux portable or Wine results do not
establish PowerShell/CMD or native MSVC execution; inspect the selected
commit's Windows results separately.

Human-facing commands under `scripts/build_orchestration/`:

The first-party namespace, include root and build target prefix are
`citrine_eclipse_mirage`. Raw application output is
`citrine_eclipse_mirage.unsigned.dll`; Production promotes the validated copy to
`citrine_eclipse_mirage.dll`. The separate development API is
`citrine_eclipse_mirage.source.dll`, and the optional online provider is
`citrine_eclipse_mirage_online.dll`. Reconfigure in a fresh build directory after
this rename; external C++ consumers and provider deployments must migrate too.
Public C exports and ordinals remain unchanged.

`resources/version.rc` uses a generated `project_version.h`. The default semantic
version is recorded in `metadata/BRANDING_AND_IDENTITY.json`. Set
`CITRINE_ECLIPSE_MIRAGE_VERSION=3.2.7` for the entire configure/build/validation
session to select another release version. Numeric FILEVERSION and PRODUCTVERSION
are `3,2,7,0`; FileVersion and ProductVersion strings are `3.2.7`. The revision
component is always zero. Reconfigure after changing the environment value.
Build receipts and Golden checks use the same selection and reject stale values.
Weekly Windows jobs receive the version from the approved planned release tag.

The CryptQuery compatibility target retains its historical filename, independently
of the new product DLL name. Retained evidence and vendor sources remain exact
historical inputs; they are not renamed product source.

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
Each CMD launcher passes the internal `-LauncherOwnsSummary` presentation
switch so that its closing panel prints the selected profile verdict once.
Direct PowerShell/CI calls retain the default result line. DLL paths, hashes,
failure diagnostics and Golden blocked-report guidance remain visible in
both routes; exit codes and validation steps are unchanged.
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
the CMake option `-DCITRINE_ECLIPSE_MIRAGE_WINE_EXECUTABLE=/path/to/wine64` selects
`scripts/run_wine_test.py`, which preserves the actual process exit code.
Output descriptors inherited by Wine services do not determine test success
or timeout behavior.

Current application C interface and source DLL separation:
[PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md).
Historical interface verification:
[PUBLIC_ABI_STATUS.json](../metadata/evidence/PUBLIC_ABI_STATUS.json).

The earlier owner-log Native build failure and its source correction are
documented in [MSVC_NATIVE_LOOKUP_FIX.md](MSVC_NATIVE_LOOKUP_FIX.md); that
checkpoint is not a current build blocker. Current validation belongs to the
selected head's GitHub Actions results. PR creation runs source integrity;
full Source QA, portable, Windows/MSVC and Golden checks follow integration
of current main or a later verified synchronization request. See
[GITHUB_WORKFLOW.md](GITHUB_WORKFLOW.md) for the eleven required contexts
and request sequence. A historical PASS does not validate a changed head.

Current five-launcher and cache-cleanup record:
[STARTUP_BUILD_FIX.md](STARTUP_BUILD_FIX.md).
Successful cleanup does not print JSON to the console; details are stored
in `PYTHON_CACHE_CLEANUP.json` in the selected build directory.
Failures stop the build with a nonzero exit code.

The focused operator output fixtures exercise all five CMD/PowerShell routes,
including failure, missing-script and color-renderer fallback paths. They use
injected build results and do not compile or establish native DLL/Golden
acceptance. Run them on Windows with:

```sh
python -B tests/infrastructure/test_infrastructure.py OperatorOutputTests
```

To request just these Windows fixtures remotely, manually dispatch their
dedicated workflow on the selected branch:

```sh
gh workflow run operator-output.yml --ref <branch>
```

This optional job does not run on PR creation or ordinary pushes and does not
replace the later eleven-check full validation gate.
