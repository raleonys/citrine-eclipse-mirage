This document records the earlier delta review. The subsequent wider
audit and corrections are in
[the whole-source audit](BROAD_SOURCE_AUDIT.md).

# Source delta, corrections, and test contributions

Four findings from the new owner-provided RE package review were
corrected. An additional comparative test exposed a fifth adapter
defect: the second document/configuration entry lost the Config2
root. That defect was also corrected. This document does not
claim source/ABI equivalence with the entire original DLL.

## Reviewed scope

The 33 CPP/HPP files in twelve new modules and eleven changed
CPP/HPP files from existing modules were classified individually
against the preceding archive. For 24 new test CPP files,
adoption/correspondence decisions were recorded. The main
pseudocode/assembly dumps remained byte-identical to the prior
package; progress in source composition must not be counted
as new native function bodies.

- [Crosswalk and decisions for 44 source/header files](../metadata/evidence/SOURCE_DELTA_REVIEW.csv)
- [Adoption decisions for 24 new test files](../metadata/evidence/SOURCE_TEST_ADOPTIONS.csv)
- [Current policy](../metadata/CONFIG_PLATFORM_POLICY.json)
- [Executed validation](../metadata/evidence/SOURCE_REVIEW_VALIDATION.json)

Function names in these tables assist source navigation. This
is a text inventory containing call names, not a compiler AST
or executed line coverage. The relevant call families from the
application owner to actual Windows adapters were also reviewed.
Unrelated private CRT/domain paths were not brought into the
product scope.

## Changes implemented

| Area | Before | After / new regression |
|---|---|---|
| UTF-8 CFG paths | `resolve_path` selected the filesystem ANSI/OEM codepage. | The original 0192d0 -> 0145d0 path uses UTF-8/65001. Test ASCII, Latin/non-English/CJK/non-BMP, absolute/UNC/relative, counted NUL, malformed input, and two actual CFG loading paths. Separate filesystem-codepage operations remain intact. |
| Initial avatar open error | A false result from the binary writer generated an additional `runtime_error`. | Ignore only the initial open/state failure; propagate downstream write/close exceptions. Test an actually unopenable path and continuation of configuration. Reject an invalid image enum before file access and verify an existing sentinel file is unchanged. |
| Directory side-effect ordering | Filtering/deletion happened after the entire listing was collected. | Shared `cleanup_cached_avatars` applies the exact `_AVATAR_` filter within a synchronous visitor. Concrete Environment uses `std::directory_iterator`; do not increment before callback completion. Vector API remains an optional fallback for older providers. Test late enumeration and filename/narrow/remove errors, preservation of previous effects, and stopping subsequent effects. |
| Thread errors | Generic `runtime_error` contained only an index. | Seven binary table codes 16/22/3/12/1/36/11 plus generic category and context; actual Windows Execution and Events use the same standard-compiler helper. Invalid indices are rejected safely. |
| Document root | `Environment.load_document` treated the selected Config2 subtree as the full document, while its caller looked for `child(Config2)` again. | Return Config2 under its own root from the document interface. Keep the active hash entry's subtree contract unchanged. Main/override files produce the expected Unicode paths through both adapters. |

Additional defense: validate UTF-8 input length before the SDK
integer limit; do not treat unknown avatar IDs as a fallback.
These changes do not require a private CRT or different filesystem
backend.

## Test contributions and limitations

The new `configuration_platform_contracts` target links only
the Production core. It exercises actual file/parser/hash/adapter
paths and NativeFactory. Configuration unit fixtures inject
controlled stream/callback failures into the real
`initialize_configuration` implementation. Actual Windows API
fault injection for a late FindNext failure is not claimed:
normal deletion, callback exceptions, reentry/nested cursors,
and compatibility collections are separately tested with
real files through the current Win32/standard iterator.

The native callback fixture now changes the original slot, map,
and selected port **after** constructing Effects/Context/Binding.
Because callback results come from the same owner storage,
equal-valued snapshot copies cannot pass this test. Busy
close/drain/retry and LastError tests already existed and
remain. Entire new callback/domain packages were not copied.

The binary output test now checks that there is exactly one
destructor during a write exception and no invented explicit
close. Existing close-exception/open-failure tests remain.
Failures are not normalized as successful results.

Among the compared test families, literal stream flushing,
private PTD/FLS, and single-controller admission models are
not contracts of the current standard-CRT product. Copying
them would mandate incorrect behavior. ByteWide staging with
arbitrary/unaligned inputs was not added to Production in
this pass; no new crash was demonstrated under the existing
typed SDK pointer contract. Debug log format and product
UI title are existing approved policies. The standard
filesystem create/remove implementation remains; error
prefixes across all compiler versions are not declared
equivalent to the original.

An optional virtual visitor was added to the C++ Dependencies
interface; development C++ consumers must be rebuilt with
all the sources. The three Production C exports/ordinals,
hook 11/static CRYPT32, metadata/PNG policy, four vendor
pins, 64 Production/74 reference translation-unit split,
and previously approved transport/persistence/lifetime
decisions remain unchanged.

## Acceptance boundary

Source review, controlled regression, Wine cross-build,
and native Windows/game acceptance are separate classes.
The owner-provided Windows/Golden/game PASS of the earlier
build remains historical evidence; native MSVC/game tests
must be repeated for the changed source. Five corrections
and expanded tests are not proof that no other defects
remain. Unexecuted platform/domain conditions are left
open in the validation record.
