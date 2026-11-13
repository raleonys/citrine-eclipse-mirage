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

## CMake source declaration inventory

The repository-contract checker compares application `src/` declarations in
`cmake/Sources.cmake` and `CMakeLists.txt` with the actual translation units.
Its lexical scan excludes CMake line and matching equal-sign bracket comments;
a commented-out path cannot satisfy source ownership. Quoted arguments, escaped
hashes/quotes and opaque bracket arguments retain their lexical context, while
the existing complete path boundaries exclude vendor and external `src/` paths.
Regression fixtures cover comment delimiters, line endings, literal contexts and
restoration of active declarations. This is a bounded textual ownership check,
not evaluation of CMake variables, conditions or generator expressions. CMake
configuration and build/test validation remain separate checks.

## Historical layout schema validation

The layout-migration contract pointer is checked for safe paths, valid JSON and
an exact historical schema identity. The checker requires an ASCII string with
a fixed SHA-256 digest; a matching suffix with another namespace is insufficient.
The digest keeps the archived source-identity label out of active code. Tests
read the canonical schema from the retained evidence and reject forged prefixes,
malformed Unicode and non-string values. The independent evidence-retention
policy still pins the complete archived file and its membership; schema checking
does not replace whole-file byte validation or modify historical records.

## Historical evidence path validation

The repository-contract checker inspects the metadata directory, archive root
and retention-policy file with `lstat` before reading policy contents. It rejects
symlinks and Windows reparse points using attributes available on Python 3.9,
without relying on newer `Path.is_junction()` support. Retained member names use
the shared canonical source-path validator, including Windows alias restrictions.
Archive traversal classifies entries without following redirected directories and
rejects links or nonregular entries before reading any evidence payload. Regular
files still require the exact retained membership and SHA-256 identities.

The root-ownership regression module runs in existing Linux and Windows CTest.
It covers changed pinned-file aliases, README/policy/parent links, nested traversal,
read tripwires, recovery and modeled Windows reparse attributes. A Windows-only
fixture creates owned junctions to targets both inside and outside its repository
root, removes only each junction entry and verifies restored evidence bytes.
A Linux skip of that fixture is not native Windows acceptance. These are
point-in-time path checks, not race-free isolation against concurrent retargeting;
the selected checkout root remains the caller's trust boundary. Historical
records and retention-policy bytes remain unchanged.

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

## Human launcher and build-label inventory

The launcher contract discovers regular CMD files without case while retaining
exact registered path spelling. A CMD-named directory is not a launcher; a
registered uppercase launcher retains the same summary-forwarding and final
exit-command checks as lowercase launchers. Top-level metadata JSON extensions
are also matched without case for retired build-label screening. Ownership,
launcher registration and wording remain separate checks, and source inventories
are still responsible for complete file-set and byte identity.
