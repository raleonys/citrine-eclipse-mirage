# FALCION Golden Cleaning

## 3.2.0 repository-layout addendum

The current compact root contract is enforced by Repository Structure Contract. Historical FALCION policy and change records follow unchanged; where historical paths differ, current ownership is authoritative for execution while the historical text remains documentary evidence.

<!-- DOC_PRESERVE_BEGIN:HF22_V5_FALCION -->
# FALCION Golden Cleaning

Status: **active release-cleaning and package-boundary authority**

FALCION Golden Cleaning governs release minimality, stale/historical material exclusion, active-document truthfulness, package-root shape, and release-asset boundaries. It is not a PE mutator and does not replace any binary Golden authority.

## Clean release boundary

The official release package must contain the active canonical source, include/export/resource/third-party inputs required by the project, active tests/QA tools, current build/native-acceptance scripts, current release/provenance documentation, and current Golden authority metadata.

The official release package must exclude:

- per-version historical PACKAGE_STATUS/QA/FEATURE_COMPLETENESS/QUALITY_PROMOTIONS/PHASE material;
- embedded evidence/research archives or obsolete historical-bundle release payloads; active release source/package trees contain no nested archive payloads (`.zip`, `.7z`, `.rar`, `.tar`, `.tgz`, `.tar.gz`, `.gz`) unless a future governance revision explicitly authorizes one;
- superseded checker history;
- `surge.cfg` and `surge.png`;
- PDB/RSDS/REPRO payloads and project release identity embedded in the shipping DLL.

## Flat packaging

Flat packaging is a FALCION responsibility, not part of the protocol name. The final release ZIP must open directly onto the release root files and directories (`BUILD_ID.md`, `src/`, `include/`, `scripts/`, `metadata/`, and peers). An extra wrapper directory named after the ZIP/package is forbidden.

Historical evidence is not a separate current release class and must never be folded into the official release payload merely to recreate obsolete FULL HISTORY/FULL SOURCE packaging models.

When a flat package is validated, its embedded `metadata/ACTIVE_SOURCE_MANIFEST.json` must be byte-identical to the manifest of the verified source root. A package may not legitimize drift by regenerating a private self-consistent manifest.

## Naming

The only canonical protocol name is **FALCION Golden Cleaning**. Functional terms such as flat packaging, stale-file rejection, forbidden-asset filtering, and clean staging describe responsibilities only; they are not alternate protocol names.

Canonical checker: `scripts/check_falcion_golden_cleaning.py`.

## Generated-artifact hygiene

Official release trees and flat release ZIPs fail closed on `__pycache__`, `.pyc`, or `.pyo` artifacts. Python bytecode caches are generated workspace material, never release source.

## 3.1.9 Data Virtual Footprint integration

The production launcher appends the Data Virtual Footprint audit directly to `dist/05_FALCION_GOLDEN_CLEANING.log`. The final FALCION result is PASS only when both cleaning/package checks and the footprint audit pass. 3.1.9 no longer emits a separate `DATA_VIRTUAL_FOOTPRINT.log`.
<!-- DOC_PRESERVE_END:HF22_V5_FALCION -->
