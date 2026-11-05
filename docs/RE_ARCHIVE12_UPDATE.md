# Archive 12 package update

The owner's nine-part `dll-reverse-engineering(12).zip` delivery replaces the
registered research snapshot on `research/reconstruction-package`. All 6,514
expanded files (860,673,543 bytes) retain their supplied relative paths and exact
bytes under `reference-package/snapshot/`, including supplied artifacts and
historical records. The combined ZIP and split parts remain outside Git.

The combined archive is 232,446,047 bytes with SHA256
`539fc80596bfa058afdfe74b01cee95c1c1b807be3e983127510e4dc170acb6d`.
[Provenance](RE_ARCHIVE12_PROVENANCE.json) records all nine part hashes, checked
receipt identities and the scope of independent verification.

Compared with project commit `e10561b256a5793f857ec4bed4b5a9fb9fcfbadd`:

| Change | Files |
| --- | ---: |
| Added | 2,294 |
| Changed | 898 |
| Removed from the new delivery | 46 |
| Unchanged | 3,322 |

The complete path delta is in the [update journal](RE_PACKAGE_UPDATES.json).
Removed files remain available in Git history. Existing Archive 11 intake and
comparison records remain historical records tied to their original hashes.

## Integrity and review state

The existing staged importer verified member paths, types, case uniqueness,
CRC and SHA256 before replacing the snapshot. Independent checks matched all
6,513 supplied manifest rows; the manifest itself is the only unlisted file.
The 1,017 canonical sources and three additional dependency headers match the
source snapshot. Current build-artifact and linked-DLL/archive hash records
match the supplied bodies, and the current status/build/link records identify
the same source snapshot and build record.

Unchanged files keep their review state. Changed and new files are pending.
All three source rows for `RPK-0001` changed; their earlier partial comparisons
remain in `prior_reviews`, and the historical finding stays open for rechecking.
No detailed file review or product parity conclusion is established by intake.
Attached instructions remain reference content and do not alter project policy.

## Supplied progress and its limits

The delivery reports a clean Windows-hosted LLVM-MinGW clang 23.1.4 rebuild:
296 objects, 356 normal test groups in 355 executables, and 19 framework terminal
cases. Its current donor DLL report now records link exit 0, a created
`anadius64.reconstruction.dll`, and no undefined symbols. This supersedes the
Archive 11 missing-loader-symbol result for these new donor bytes only.

The current status is explicitly `LINKED_NOT_RUNTIME_VALIDATED`: the supplied
production DLL was not loaded or executed, and runtime/loader equivalence is
not established. These are preserved delivery claims with matching bytes;
their tests and compiler were not independently rerun here. No supplied code
or binary was executed during import. Windows/MSVC, Golden, original DLL and
real Consumer/game acceptance remain separate evidence classes.

This update adopts no donor implementation into the active application.
Subsequent product changes require their own source comparison, focused PR and
actual validation. The continuing research project remains open.
