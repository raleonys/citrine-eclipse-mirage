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

The verified extractor checks ZIP header-reported uncompressed sizes before
CRC decompression or whole-member reads: at most 64 MiB per member and
256 MiB across all members. Budget rejection removes temporary extraction
files and leaves the destination absent. These byte limits do not establish
complete CPU/memory isolation for arbitrary ZIP input.

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

## Historical layout schema validation

The layout-migration contract pointer is checked for safe paths, valid JSON and
an exact historical schema identity. The checker requires an ASCII string with
a fixed SHA-256 digest; a matching suffix with another namespace is insufficient.
The digest keeps the archived source-identity label out of active code. Tests
read the canonical schema from the retained evidence and reject forged prefixes,
malformed Unicode and non-string values. The independent evidence-retention
policy still pins the complete archived file and its membership; schema checking
does not replace whole-file byte validation or modify historical records.

## Ownership ledger input validation

Metadata and script ledgers require nonblank string owner/purpose fields and a
string role from their existing allowed sets. Both use the shared strict UTF-8
JSON loader: duplicate keys are rejected at every nesting level, even when
repeated values are identical or the final value would otherwise be valid.
The CLI reports failure and can pass again after an operator repairs the file.
Inventory discovery compares JSON and script extensions without case, while
ownership keys retain their exact filename/path case. Unowned uppercase files
are rejected; correctly registered uppercase files remain valid. These checks
validate ownership metadata independently of the active-source byte inventory;
they do not replace package identity, runtime or native acceptance checks.
