> **Historical checkpoint.** Use [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

# SURGE product name consistency

Owner instruction (translated): "These must be corrected consistently."

`include/citrine_eclipse_mirage/core/product_identity.hpp` defines configuration,
override, data-directory, avatar, and online-DLL names. Configuration projections
and the concrete runtime use the same names. The configuration search location
remains the EXE directory, with base -> override precedence, and the default
data root remains `%LOCALAPPDATA%/SURGE`. Missing configuration is optional;
no automatic fallback to old filenames or file migration is added.

The earlier two-component data root in the lower configuration projection is
aligned with the existing runtime SURGE root. The online module requests
`citrine_eclipse_mirage_online.dll` directly; the application adapter no longer has a
special case that translates the former name to the new one.

Four configuration error messages identify `surge.cfg`. The shutdown source
type is `DllShutdownStatus`, with `DLL_SHUTDOWN_*` constants. Values 0..4 and
`DllMain@1` / `SURGE@2` / `DllShutdown@3` remain unchanged. Hosts and existing
configuration/provider/diagnostic tests are updated accordingly. No new
Production export is introduced.

Two provenance comments pointing to original source files, historical reports,
comparison tables, licensing/source attributions, and `yellowfield.*` JSON
schema identities are retained. These are not filenames or visible product
identities used by the application. The test host continues to verify that the
removed `YellowfieldShutdown` export is absent. Earlier ZIP packages and
vendored payloads remain unchanged.

Validation results are in
`metadata/evidence/PRODUCT_IDENTITY_FIX.json`. Cross-build and Wine results do
not replace actual Windows/MSVC Golden or game acceptance.
