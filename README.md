# SURGE

[![Source and Windows checks](https://github.com/cyanoleon/yellowfield/actions/workflows/ci.yml/badge.svg)](https://github.com/cyanoleon/yellowfield/actions/workflows/ci.yml)

SURGE is a Windows x64 C++20 DLL providing a local LSX service, Windows API
hooks, configuration loading, and persistent achievement and event handling.
The application DLL, development API, tests, and build tools have separate
source and build boundaries.

## Build on Windows

Install Visual Studio with the C++ x64 build tools and Windows SDK, CMake 3.20
or later, and Python 3. Python 3.13 is used in CI.

From the repository root, run the production profile:

```powershell
.\scripts\build_orchestration\build_production.ps1
```

The script locates the MSVC toolchain, builds the Release DLL, and validates
its exports, resources, PE metadata, and runtime lifecycle. After every gate
passes, the validated output is available in:

```text
dist/preloader_l.dll
dist/preloader_l.dll.sha256
dist/LOCAL_DIST_RECEIPT.json
```

The raw linker output remains in the selected build directory as
`preloader.unsigned.dll`. A successful production build creates a local
candidate; it does not publish a release.

Other build profiles are available for development and validation:

| Profile | PowerShell entry point |
| --- | --- |
| Compile and runtime checks | `scripts/build_orchestration/build_unstable.ps1` |
| Debug build | `scripts/build_orchestration/build_debug.ps1` |
| Native Windows acceptance | `scripts/build_orchestration/build_native_acceptance.ps1` |
| Standard CRT acceptance | `scripts/build_orchestration/build_crt_acceptance.ps1` |

Each profile also has a matching `.cmd` launcher for interactive use. See
[development commands](docs/DEVELOPMENT.md) and the
[production validation pipeline](docs/GOLDEN_PROTOCOLS.md).

## Runtime configuration

Configuration is optional. SURGE reads `surge.cfg` from the host executable's
folder, followed by `surge_override.cfg`. Built-in defaults apply when these
files are absent. Runtime data is stored under `%LOCALAPPDATA%\SURGE`.
Detailed `surge.log` diagnostics are enabled only in Debug builds.

The listener selects the first available port in `3226-3236`. IPv4 and IPv6
loopback connections to the consumer port `3216` are redirected to the selected
listener. The SDK identity is fixed at `10.6.2.31`; legacy `ServerPort` and `SDK`
configuration keys do not override these policies.

The application DLL exposes `DllMain` at ordinal 1, `SURGE` at ordinal 2,
and `DllShutdown` at ordinal 3. C++ development APIs belong to the separate
`preloader.source.dll` target. Hosts must retain their DLL reference until
shutdown succeeds. See the [public DLL interface](docs/PUBLIC_DLL_ABI.md) and
[runtime architecture](docs/RUNTIME_BINDING.md).

## Tests and source integrity

Run the source and dependency checks without a Windows toolchain:

```sh
python -B scripts/run_source_qa.py --report build/qa/SOURCE_QA.json
```

Build and run the portable tests:

```sh
cmake -S . -B build/portable -DPRELOADER_L_PORTABLE_CHECKS_ONLY=ON
cmake --build build/portable --parallel 2
ctest --test-dir build/portable --output-on-failure
```

The **Source and Windows checks** workflow runs source QA and portable tests
automatically on pull requests and main pushes. Its Windows jobs run only when
started with **Actions > Source and Windows checks > Run workflow**. The separate
[weekly draft release workflow](docs/WEEKLY_RELEASE.md) also runs native Windows
acceptance and Production/Golden for new release candidates. Build receipts and
test reports identify the source and toolchain used. Game acceptance is recorded
separately from CI.

Source and vendor manifests verify exact file contents. Record intentional
source changes explicitly before packaging. See
[dependency ownership](docs/VENDORING.md),
[current runtime and validation records](metadata/RUNTIME_CURRENT_STATUS.json),
and the [GitHub review workflow](docs/GITHUB_WORKFLOW.md).

## Repository layout

| Path | Contents |
| --- | --- |
| `src/`, `include/preloader_l/` | Application and retained reference sources and headers |
| `tests/` | Unit, integration, native, and infrastructure tests |
| `third_party/` | Pinned Asio, MinHook, PEGTL, and TinyXML2 sources |
| `scripts/`, `tools/` | Build, validation, packaging, and development tools |
| `resources/`, `exports/` | Branding assets, VERSIONINFO, and the public export table |
| `metadata/` | Source provenance, manifests, and current contracts |
| `docs/`, `governance/` | Architecture, review evidence, and project scope |

For design and maintenance details, see the [source scope](docs/SOURCE_SCOPE.md),
[build boundaries](docs/BUILD_PARTITION.md),
[hook lifecycle](docs/HOOK_ENGINE_HARDENING.md), and
[cleanup review](docs/BUILD_CLEANUP.md).

## Licensing

See [LICENSE](LICENSE) and the license files retained with each dependency.
