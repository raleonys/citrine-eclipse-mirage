# Repository layout and ownership

| Path | Responsibility |
|---|---|
| `src/`, `include/preloader_l/` | Application sources selected from the RE inputs; detailed provenance in SOURCE_LAYOUT.json |
| `cmake/` | Explicit source lists, Windows DLL, portable targets, and infrastructure test targets |
| `scripts/build_orchestration/` | Five registered human-facing launchers and automation-ready PowerShell profiles |
| `scripts/` | Golden tools, source QA, manifest handling, and package validation |
| `tools/` | Application C API loader, separate source host, compiler CRT host, and read-only PE/resource reporting |
| `exports/` | The application's fixed C export table of three names/ordinals |
| `tests/unit/`, `tests/native/` | Current Yellowfield source and adapter tests |
| `tests/infrastructure/`, `tests/support/golden/` | Adapted infrastructure tests and explicitly synthetic policies |
| `resources/branding/` | Owner-supplied, byte-pinned SURGE PNG; RCDATA 101 |
| `third_party/` | Exact source trees from the five original owner-supplied archives; Brownfield vendor trees are not copied |
| `docs/`, `metadata/` | Current technical documents and machine-readable records; root `GOVERNANCE.md` and `AGENTS.md` own policy and agent workflow |
| `build/`, `dist/` | Transient local outputs; excluded from source ZIP packages |

The complete Brownfield application tree and research archives are not imported
into this root. Source packages use a flat root: `CMakeLists.txt` is directly
accessible after extraction. See [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md)
for the Golden pipeline, [DEVELOPMENT.md](DEVELOPMENT.md) for operator commands,
and [VALIDATION.md](VALIDATION.md) for acceptance boundaries.

# Current file layout

The main source/header trees follow Brownfield's one-level module layout:

```text
src/net/asio_service.cpp
src/net/transport_acceptor.cpp
src/platform/hook_setup.cpp
src/platform/dll_entry.cpp
src/runtime/achievement_storage.cpp
src/runtime/calendar.cpp
src/xml/native_document.cpp

include/preloader_l/net/asio_service.hpp
include/preloader_l/platform/hook_setup.hpp
include/preloader_l/runtime/achievement_storage.hpp
include/preloader_l/runtime/calendar.hpp
include/preloader_l/xml/native_document.hpp
```

No additional module subdirectories exist beneath `src/<module>/<file>` or
`include/preloader_l/<module>/<file>`. Former `transport`, `session`, `hooks`,
`hook_engine`, `windows`, `achievements`, `events`, `time`, `files`, and `native`
layers are represented by filenames. The pre-migration source package is
retained. A total of 177 files were moved, and 261 source/header bodies were
verified byte-identical except for include directives.
[Complete path mapping](../metadata/evidence/LAYOUT_MIGRATION.json).
