# Production and retained reference sources

The application now has 62 first-party C++ translation units in its production
inputs. Another 78 translation units remain in the source tree, reference tests
and the separate development DLL. They are **not globally dead code**. Four more
test/model compositions were removed from the production link without deleting
their sources or public APIs. The accompanying narrow implementation cleanup is
recorded in [BUILD_CLEANUP.md](BUILD_CLEANUP.md).

| Target | Inputs and purpose |
| --- | --- |
| `preloader_l_objects`, `preloader_l_vdf_objects` | Production application, native callbacks and parser |
| `preloader_l_core` | Production-only static library for integration tests |
| `preloader_l` | Production objects, entry point, resources and three fixed exports |
| `preloader_l_reference_objects` | 52 transport models, 16 XML models and ten reference adapters/compositions |
| `preloader_l_reference_models` | Retained models; may depend on the production core |
| `preloader_l_source_api` | Both source sets, preserving the development C++ API |
| `preloader_l_source_minhook`, `preloader_l_source_pugixml` | MSVC shared-CRT vendor builds used only by the C++ development DLL |

MSVC compiles the complete development DLL source set separately with `/MD`
(Release) or `/MDd` (Debug); its C++ host and vendor libraries use the same CRT.
The source host constructs parser states that the DLL mutates, receives strings,
and catches parser exceptions, so separate static CRT instances are unsafe at
this boundary. The Debug timeout was observed immediately before destruction of
the invalid-input parser state, after all functional checks completed and before
shutdown. This identifies a destructor boundary, not a captured native stack.
The shared-CRT correction still requires fresh native Debug/Release execution.
Production and static reference/test targets retain `/MT` or `/MTd` and their
existing objects. The MSVC development DLL must not link those static-CRT
objects or archives. Non-MSVC development builds retain their existing graph.

Production must never depend on a reference target or the source API DLL. With
`BUILD_TESTING=OFF` and `PRELOADER_L_BUILD_SOURCE_API_DLL=OFF`, reference targets
are not configured. The owner-selected pugixml archive replaces TinyXML2; the other three vendor
pins are unchanged. `xml/reader.cpp`, `xml/reader_policy.cpp`,
`xml/reader_values.cpp` and `xml/memory_printer.cpp` are production inputs.
Production and test targets link `preloader_l_pugixml`; the MSVC development
DLL links `preloader_l_source_pugixml`. No legacy XML vendor target aliases
are provided.
The evaluated production graph rejects `third_party/tinyxml2/` sources.
The external TinyXML2 control exists only in the standalone comparison.

## Evidence and limits

The matrices and measurements below describe the historical first partition of
74 sources. The current additional four-source review, split test coverage and
fresh results are in [build-cleanup/VALIDATION.json](../metadata/evidence/build-cleanup/VALIDATION.json).
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

The check runs after linking and as an incremental `ALL` target. Unsupported or
empty maps fail. Both LLVM-MinGW and MSVC map syntax have negative fixtures;
actual MSVC linker execution is still pending. Raw compiler DLLs and maps stay
in the build directory; no dist promotion or publication is performed here.

The original callback mixed tests remain intact. Additional `production_callback_contracts`
and `production_network_hook_contracts` compile their native callback/connect
fixtures with only the production core; reference-only adapter cases stay in the
original mixed targets. This prevents reference libraries from masking a missing
runtime binding in these tests.

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
`BUILD_TESTING=OFF` and `PRELOADER_L_BUILD_SOURCE_API_DLL=OFF`. Do not reuse a
testing build as the sole proof that production can build independently.

To regenerate the symbol matrix from a built Ninja cross-build:

```text
python -B scripts/audit_translation_units.py --build-dir <build-directory> --config Release --nm <llvm-nm> --objdump <llvm-objdump> --readobj <llvm-readobj> --output <build-directory>/usage.json --csv <build-directory>/usage.csv
```

The audit currently supports CMake's Ninja object layout and LLVM inspection
tools; it fails for unsupported layouts rather than silently omitting files.
Source QA includes 18 boundary regressions, separately from native tests.

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
