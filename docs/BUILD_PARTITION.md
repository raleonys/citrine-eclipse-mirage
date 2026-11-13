# Production and retained reference sources

The application now has 62 first-party C++ translation units in its production
inputs. Another 78 translation units remain in the source tree, reference tests
and the separate development DLL. They are **not globally dead code**. Four more
test/model compositions were removed from the production link without deleting
their sources or public APIs. The accompanying narrow implementation cleanup is
recorded in [BUILD_CLEANUP.md](BUILD_CLEANUP.md).

| Target | Inputs and purpose |
| --- | --- |
| `citrine_eclipse_mirage_objects`, `citrine_eclipse_mirage_vdf_objects` | Production application, native callbacks and parser |
| `citrine_eclipse_mirage_core` | Production-only static library for integration tests |
| `citrine_eclipse_mirage` | Production objects, entry point, resources and three fixed exports |
| `citrine_eclipse_mirage_reference_objects` | 52 transport models, 16 XML models and ten reference adapters/compositions |
| `citrine_eclipse_mirage_reference_models` | Retained models; may depend on the production core |
| `citrine_eclipse_mirage_source_api` | Both source sets, preserving the development C++ API |
| `citrine_eclipse_mirage_source_minhook`, `citrine_eclipse_mirage_source_pugixml` | MSVC shared-CRT vendor builds used only by the C++ development DLL |

MSVC compiles the complete development DLL source set separately with `/MD`
(Release) or `/MDd` (Debug); its C++ host and vendor libraries use the same CRT.
The source host constructs parser states that the DLL mutates, receives strings,
and catches parser exceptions, so separate static CRT instances are unsafe at
this boundary. The Debug timeout was observed immediately before destruction of
the invalid-input parser state, after all functional checks completed and before
shutdown. This identifies a destructor boundary, not a captured native stack.
The diagnosis above describes the correction's original checkpoint; current
native Debug/Release acceptance belongs to the selected head's CI results.
Production and static reference/test targets retain `/MT` or `/MTd` and their
existing objects. The MSVC development DLL must not link those static-CRT
objects or archives. Non-MSVC development builds retain their existing graph.

Production must never depend on a reference target or the source API DLL. With
`BUILD_TESTING=OFF` and `CITRINE_ECLIPSE_MIRAGE_BUILD_SOURCE_API_DLL=OFF`, reference targets
are not configured. The owner-selected pugixml archive replaces TinyXML2; all current vendor
pins are maintained in [VENDORING.md](VENDORING.md). `xml/reader.cpp`, `xml/reader_policy.cpp`,
`xml/reader_values.cpp` and `xml/memory_printer.cpp` are production inputs.
Production and test targets link `citrine_eclipse_mirage_pugixml`; the MSVC development
DLL links `citrine_eclipse_mirage_source_pugixml`. No legacy XML vendor target aliases
are provided.
The evaluated production graph rejects `third_party/tinyxml2/` sources.
The external TinyXML2 control exists only in the standalone comparison.

## Portable model checks

`portable_bootstrap_model` and `portable_thread_model` compile the retained
bootstrap/thread source models with their unchanged controlled-effects fixtures.
Both portable CTest targets keep the existing native fixtures' 60-second budgets;
`bootstrap_contracts` and `crt_thread_contracts` remain in the Windows/reference
graph. Portable coverage checks model ordering, context/handle ownership and
cleanup contracts without a Windows SDK or actual OS threads, module/FLS lifetime
or private CRT execution. Production source lists and linkage remain unchanged;
Application continues to use its owned standard-library workers and compiler CRT.

## Evidence and limits

The matrices and measurements below describe the historical first partition of
74 sources. The later four-source review, split test coverage and its checkpoint
results are in [build-cleanup/VALIDATION.json](../metadata/evidence/build-cleanup/VALIDATION.json).
The historical 898-symbol count and development export count are not current
totals and are not used as current acceptance gates.

[The per-file JSON matrix](../metadata/evidence/TRANSLATION_UNIT_USAGE.json) and
[the CSV table](TRANSLATION_UNIT_USAGE.csv) record external COFF definitions,
references, vtables, CRT/TLS sections, direct/transitive test references and source
API exports for every retained TU. All 74 have transitive test references and
development DLL exports. The audited production objects have no undefined
references to the 898 symbols defined only by those reference objects; none of
the 74 has a nonempty CRT/TLS initializer section in the audited cross-build.

This is a symbol and build-input audit, not exhaustive dynamic coverage. Tests
referencing a symbol do not prove that every branch executes. Static/inline code
and compiler-generated code can have different symbol ownership. The inspected
callback resolver returns the eleven concrete native entry functions; thread
entries and concrete virtual implementations remain in the production set.
`GetProcAddress` sites resolve operating-system/provider functions, rather than
loading reference model symbols from the application DLL. The matrix includes
those binding sites so future changes can be reviewed.

The before/after validation and outstanding acceptance checks are recorded in
[BUILD_PARTITION.json](../metadata/evidence/BUILD_PARTITION.json). Native MSVC/Golden and real
game acceptance remain separate required evidence. Passing Wine fixtures must
not be reported as 100% proof of every game path.

[The DLL comparison](../metadata/evidence/BUILD_PARTITION_DLL_PARITY.json) uses the same
[diagnostic probe](../metadata/evidence/BUILD_PARTITION_DLL_PROBE.cpp.txt) against the preserved
baseline, candidate and clean testing-disabled DLLs. Each runs three public-ABI
load/start/request/shutdown/unload cycles. Full XML replies are transported as hex
in the diagnostic output, decoded and compared; only validated wall-clock fields
are normalized. The isolated avatar, fallback avatar, achievement XML and license
files are compared too. This fixture covers nine cases per cycle, including
malformed XML recovery and encrypted profile/event output. It does not load a
real game or provider. Provider and broader runtime fixtures are separate tests.

[Binary measurements](../metadata/evidence/BUILD_PARTITION_BINARIES.json) describe raw
diagnostic builds. The production `.text` section decreases from 578,726 to
462,886 bytes (about 20%); development exports remain 3,336 with no removed or
added names. Three diagnostic C++ runtime imports disappear with the excluded
objects. These MinGW imports are separate from native MSVC shipping acceptance.

## Enforced build boundary

`metadata/BUILD_PARTITION_POLICY.json` explicitly owns all 140 C++ source files
and classifies executable targets. Configure rejects overlaps and unowned source
files. CMake emits an evaluated target graph during generation, including on the
first clean configure. The build produces a production linker map and checks:

1. The production graph and production-only tests cannot reach reference or
   source API targets.
2. Actual production source sets equal the declared partition.
3. No reference object or reference archive appears in the production map,
   including a stale reference member inside a production-named archive.
4. Entry-point and protected-reader host objects have positive map evidence.
5. Reference tests and the development DLL retain their reference dependencies.

The generated `partition.<CONFIG>.targets` reader requires each target record
to contain exactly `TARGET`, `TYPE`, `DEPS`, `SOURCES` and `END`, in that order.
Target names and types must be nonempty; duplicate targets/fields, missing or
reordered fields, unmatched terminators and unfinished records fail closed.
Empty dependency/source lists remain valid when their field records are present.
A target enters the parsed graph only after its complete record closes, so a
later field cannot silently overwrite its executable classification or edges.
This validates generated evidence structure; the closure, source, linker-map and
native acceptance checks still establish their separate boundary properties.

The check runs after linking and as an incremental `ALL` target. Unsupported or
empty maps fail. Both LLVM-MinGW and MSVC map syntax have negative fixtures;
actual MSVC linker acceptance is recorded by the selected head's Windows
jobs rather than these historical fixtures. Raw compiler DLLs and maps stay
in the build directory; no dist promotion or publication is performed here.

The original callback mixed tests remain intact. Additional `production_callback_contracts`
and `production_network_hook_contracts` compile their native callback/connect
fixtures with only the production core; reference-only adapter cases stay in the
original mixed targets. This prevents reference libraries from masking a missing
runtime binding in these tests.

`asio_thread_launch_contracts` is also a production-core test target. Its private
launcher selection exercises actual Asio worker rollback without linking any
reference models or the development DLL. Register new executable tests in the
partition policy as well as CTest; the generated Windows target graph rejects
unclassified targets before native tests can run.

## Reproduce

Use the existing native acceptance launcher or the documented compatible cross
toolchain. The usual build automatically verifies the partition. For a configured
Windows build directory:

```text
cmake --build <build-directory> --config Release
ctest --test-dir <build-directory> -C Release --output-on-failure
ctest --test-dir <build-directory> -C Release -L production --output-on-failure
```

Repeat for Debug. A separate clean production-only configuration must set both
`BUILD_TESTING=OFF` and `CITRINE_ECLIPSE_MIRAGE_BUILD_SOURCE_API_DLL=OFF`. Do not reuse a
testing build as the sole proof that production can build independently.

To regenerate the symbol matrix from a built Ninja cross-build:

```text
python -B scripts/audit_translation_units.py --build-dir <build-directory> --config Release --nm <llvm-nm> --objdump <llvm-objdump> --readobj <llvm-readobj> --output <build-directory>/usage.json --csv <build-directory>/usage.csv
```

The audit currently supports CMake's Ninja object layout and LLVM inspection
tools; it fails for unsupported layouts rather than silently omitting files.
Object lookup resolves the selected checkout/build roots, rejects source
parent traversal and absolute sources outside that checkout, and refuses
redirected source/object paths or invalid target directory components. This
prevents attributing an unrelated COFF object to the selected translation unit;
it does not prove runtime reachability or Consumer coverage.
Source QA runs the current boundary regressions in
`tests/infrastructure/test_build_partition.py`, separately from native tests.
The earlier fixed test count is not a current inventory.

Five exhaustive reference fixtures have a 600-second test-process budget only
for single-configuration Debug cross-builds under Wine (the process runner has
660 seconds for cleanup). Their matrices encrypt large messages repeatedly with
unoptimized code. The original 60-second budget was exceeded in the full audit.
Release/native test budgets and every application session/shutdown deadline stay
unchanged; assertions and cases are not removed. Exact retries and timings are
recorded in the validation evidence.

Before final acceptance, compare the same actual consumer on native Windows
with the prior and current DLL: missing/main/override config, offline and provider
startup, plaintext/encrypted LSX, profile/content/avatar, achievement persistence
and retry, registry/process/folder/connect and hook11 paths, concurrent sessions,
shutdown retry/unload and Release logging. Record failures and toolchain/linker
evidence; do not replace these checks with import-list similarity.
