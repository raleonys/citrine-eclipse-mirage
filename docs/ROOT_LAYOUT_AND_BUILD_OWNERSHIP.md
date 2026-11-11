# Repository layout and ownership

| Path | Responsibility |
|---|---|
| `src/`, `include/citrine_eclipse_mirage/` | Application sources selected from the RE inputs; detailed provenance in SOURCE_LAYOUT.json |
| `cmake/` | Explicit source lists, Windows DLL, portable targets, and infrastructure test targets |
| `scripts/build_orchestration/` | Five registered human-facing launchers and automation-ready PowerShell profiles |
| `scripts/` | Golden tools, source QA, manifest handling, and package validation |
| `tools/` | Application C API loader, separate source host, compiler CRT host, and read-only PE/resource reporting |
| `exports/` | The application's fixed C export table of three names/ordinals |
| `tests/unit/`, `tests/native/` | Current Citrine Eclipse Mirage source and adapter tests |
| `tests/infrastructure/`, `tests/support/golden/` | Adapted infrastructure tests and explicitly synthetic policies |
| `resources/branding/` | Repository campaign logo plus separate SURGE/profile and fallback avatars; avatar bytes are read-only C++ data, not PNG PE resources |
| `third_party/` | Four immutable owner-supplied vendor trees: Asio, MinHook, PEGTL and pugixml; retired archive provenance is retained separately |
| `docs/`, `metadata/` | Current guides, explicitly historical checkpoints and machine-readable records; see `docs/INDEX.md`; root `GOVERNANCE.md` and `AGENTS.md` own policy and agent workflow |
| `build/`, `dist/` | Transient local outputs; excluded from source ZIP packages |

The complete Brownfield application tree and research archives are not imported
into this root. Source packages use a flat root: `CMakeLists.txt` is directly
accessible after extraction. See [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md)
for the Golden pipeline, [DEVELOPMENT.md](DEVELOPMENT.md) for operator commands,
and [VALIDATION.md](VALIDATION.md) for acceptance boundaries.

The [verified source extractor](../scripts/extract_source.py) requires a fresh
destination. It rejects ancestor symlinks and, when the host Python provides
`Path.is_junction()`, ancestor junctions before creating parent directories or
staging extraction. Ordinary fresh paths retain ZIP integrity,
membership, type and source-hash validation. This ancestor precheck does not
provide race-free isolation against concurrent parent retargeting.

# Current file layout

The main source/header trees follow Brownfield's one-level module layout:

```text
src/net/asio_service.cpp
src/net/transport_acceptor.cpp
src/platform/hook_setup.cpp
src/platform/dll_entry.cpp
src/runtime/achievement_storage.cpp
src/runtime/calendar.cpp
src/xml/reader.cpp

include/citrine_eclipse_mirage/net/asio_service.hpp
include/citrine_eclipse_mirage/platform/hook_setup.hpp
include/citrine_eclipse_mirage/runtime/achievement_storage.hpp
include/citrine_eclipse_mirage/runtime/calendar.hpp
include/citrine_eclipse_mirage/xml/reader.hpp
```

No additional module subdirectories exist beneath `src/<module>/<file>` or
`include/citrine_eclipse_mirage/<module>/<file>`. Former `transport`, `session`, `hooks`,
`hook_engine`, `windows`, `achievements`, `events`, `time`, `files`, and `native`
layers are represented by filenames. The pre-migration source package is
retained. A total of 177 files were moved, and 261 source/header bodies were
verified byte-identical except for include directives.
[Complete path mapping](../metadata/evidence/LAYOUT_MIGRATION.json).
