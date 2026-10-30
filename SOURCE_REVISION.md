# Yellowfield source revision

Local non-release layout correction, 28 October 2026.

Application functions originate from the owner-supplied anadius64 RE modules.
Paths, include ownership, first-party namespace and build ownership were adapted.
The source selection excludes the foreign preloader/version projects and private
CRT reconstruction. The authorized hook-11 selector remains as a bounded feature.

Source history: initial Yellowfield and hook-11 ZIPs remain separate. Current
file mapping, excluded references and exact hashes are in metadata/.

YF-INFRA-20261028 adds the authorized Brownfield operational infrastructure.
Application src/include bytes remain the same as the preceding layout-correction
package. Selected infrastructure input/output identities are recorded separately;
the new flat source package is not an official release.

YF-VENDOR-20261028 replaces the previous RE dependency copies with all five exact
owner-uploaded vendor archives. No application .cpp implementation changes;
the XML reader header describes the new source/default and its regression uses
the supplied unchanged depth 500. No 100-depth compatibility patch is applied.
Vendoring, toolchain/API fixtures and final active production usage are distinct.

YF-BLUEFIELD-ALIGN-20261029 explicitly adopts Bluefield hook11 and required native
lifecycle bodies/adapters, plus the complete supplied Brownfield declared metadata
profile. Earlier comparison-only and preserve-compiler choices are superseded only
within this scope. See metadata/evidence/HOOK_ALIGNMENT_STATUS.json and
docs/REFERENCE_METADATA_AUDIT.md. All original archives and earlier ZIPs remain
unchanged; this package is source-only, NOT_RELEASE.

YF-STARTUP-BUILD-20261029 binds the missing supplied Bluefield FIX19 registry
startup-wait Guard/Pause dependency after an observed failing regression, fixes
NOMINMAX for the independent test target and restores the five build titles/quiet
cache diagnostics. Optional config and vendor bytes remain unchanged. The owner
reports parent game no-start; actual game acceptance is pending the new Windows run.

YF-WINDOWS-IO-20261029 restores the supplied Windows standard filesystem/fstream
backend, removes the now-unused Gulrak vendor by explicit owner selection, fixes
Brownfield Windows API target macros, and restores Brownfield three-export
ordinals. Final-copy import descriptors are normalized to the owner-requested
Anadius module order with unchanged import names/thunks/IAT data and recomputed
checksum. Release/Debug log behavior is explicitly checked. Parent game startup
was owner-confirmed; this source requires an updated native MSVC/game run.
