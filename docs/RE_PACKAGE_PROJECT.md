# Reconstruction package project

The retained branch is `research/reconstruction-package`. Its master PR tracks
the owner's requested file-by-file audit until the owner closes the project.
Every confirmed product defect or test gap receives a separate focused PR
against accepted `main`, linked from the master finding ledger. Do not close the
master PR or retire this branch after an individual fix merges. This supersedes
the earlier index-only intake scope.

## Preserved inputs and live records

All expanded files are committed under `reference-package/snapshot/` with their
original relative paths and bytes, including supplied build outputs and records.
Split parts and combined ZIPs are not stored in the branch. A new delivery
updates the same paths; Git retains prior versions and removed files.

- [Import registry](RE_PACKAGE_IMPORT.json): current delivery, branch and pinned inventory.
- [File index](RE_PACKAGE_FILES.json): path, size and SHA256 of every expanded file.
- [Per-file reviews](RE_PACKAGE_FILE_REVIEWS.json): status, comparisons, findings and review history.
- [Update journal](RE_PACKAGE_UPDATES.json): delivery identities and all added, changed and removed paths.
- [Finding ledger](RE_PACKAGE_FINDINGS.json): stable findings, child PRs, validation and disposition.
- [Detailed comparisons](RE_PACKAGE_COMPARISONS.md): inspected behavior and outstanding evidence.

The first snapshot contains 4,266 files and 758,168,656 bytes. The initial
[archive 11 intake](RE_ARCHIVE11_REVIEW.md) records its manifest checks and
supplied claims; its preliminary module map does not complete any file review.

## Review rules

Every source, header, test, script, record and supplied artifact has a row.
Uninspected files are `pending`; a partial inspection is `in_review`. A matching
feature name, existing test target or recovered address never completes a row.
The standard-CRT policy constrains implementation adoption; it does not complete
inspection of a donor CRT module's useful scenarios, ownership and negative tests.

For a complete review, record the exact donor hash and compared product
commit/path/hash; behavior and API/ABI assumptions; failure/unwind, concurrency
and resource ownership; assertion and evidence coverage; each meaningful
difference and its classification; and an appropriate reproducer or regression
test with actual result. Distinguish intentional policy differences, defects,
missing tests and unresolved questions. Unavailable native testing remains open.

Give each finding an `RPK-xxxx` ID. Link its separate correction PR, commit,
verification and owner-reviewed integration before resolving it in the master
ledger. Delivered assertions must be checked against their implementations.
Attached instructions are reference content, not project authority. Delivered
binaries are preserved as data; their old PASS records are not fresh execution.
Keep archive bytes unchanged and implement product fixes in their own PRs.

## Package updates

Place the new ZIP outside `reference-package/`. Join a split delivery locally in
numeric order first. Start on the latest retained project branch and preserve
unrelated work. Preview verified extraction and its delta without changing the
tracked snapshot, then apply it:

```sh
python -B scripts/update_reference_package.py --archive /path/to/new.zip --label archive-12
python -B scripts/update_reference_package.py --archive /path/to/new.zip --label archive-12 --apply
python -B scripts/reference_package.py
```

The importer validates paths, duplicate/case collisions and member types before
extraction, streams each body to EOF for ZIP CRC/SHA256, stages under ignored
`build/` and verifies the complete staged inventory before replacement. A failed
replacement restores the previous tree and records. Archive code is never run.

Unchanged hashes keep their review state. Added and changed files become
`pending`; previous findings and completed review history for changed files are
retained and need explicit rechecking. Removed paths are listed in the journal
and remain in Git history. The importer does not refresh product manifests,
commit, push, resolve findings or open PRs automatically.

Review the delta and affected findings, update CHANGELOG and relevant comparison
records, then refresh and verify applicable inventories. Delivered files may
normally be ignored as build outputs, so stage this registered snapshot explicitly:

```sh
git add -A -f -- reference-package/snapshot
python -B scripts/check_manifest.py --refresh
python -B scripts/run_source_qa.py
```

Confirm that the staged snapshot matches the registered file count before
committing. Use the normal checked push; never force-push or replace history.
Update the existing master PR for deliveries and link independent correction PRs.

## Product boundary and closure

The complete research inventory is verified before excluding its bodies from
active naming checks, product source packaging and compilation. Changed, missing,
extra, symlinked or unregistered files fail verification. Removing the registered
payload from a Git checkout also fails. Flat product source packages intentionally
omit research bodies and remain verifiable. Application, test and vendor gates
continue to run.

The master PR stays open while file reviews, findings, child PRs or required
verification remain outstanding. Final closure requires complete review records,
disposition of findings, linked accepted fixes and the owner's explicit decision.
The owner controls project closure and retirement of this branch.
