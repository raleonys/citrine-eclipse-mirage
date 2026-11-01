# Reconstruction archive 11: intake and development tracker

This workstream tracks the owner's eight-part `dll-reverse-engineering(11).zip`
delivery against accepted baseline `d9c860d` (main after PR #23). The owner
requested PR #82; GitHub assigns the actual number when the PR is created.
This document is the review entrypoint for subsequent focused test and source
transfers. Intake is complete; implementation adoption and subsystem review are
open. Statements inside the delivery are reference claims, not project policy.

## Source identity and preservation

- [Provenance](RE_ARCHIVE11_PROVENANCE.json) pins all eight parts, the merged ZIP,
  key supplied records and independently checked counts.
- [File inventory](RE_ARCHIVE11_FILES.json) records the path, size and SHA256 of
  every one of the 4,266 regular files, including `package-manifest.json` itself.
- [Module ledger](RE_ARCHIVE11_MODULES.json) accounts for 199 top-level directories
  and one root-file group. It records review depth, snapshot membership and test
  file counts; `not_reviewed` must not be interpreted as approved or rejected.

Numeric concatenation of parts 001 through 008 produces 209,091,682 bytes with
SHA256 `db394434e38c7edad33607a846ffdc36b04a1e5978d2fbe3d74da24bda5179da`.
The ZIP contains 4,542 entries: 4,266 files and 276 directories, totaling
758,168,656 uncompressed bytes. Every file was read to EOF, checking ZIP CRC and
SHA256. No duplicate or case-colliding entries, traversal paths or symlinks were
found. All 4,265 supplied package-manifest rows matched size and digest. The only
unlisted file is the manifest itself, explicitly excluded by its own contract.
All 1,017 current source-snapshot digests and its build-record digest matched.
Another 656 supplied build-artifact records matched their archived bytes:
296 objects, 355 executables, three libraries, one hook fixture executable and
one synthetic fixture DLL. These checks establish internal consistency, not
compiler authenticity, test execution or behavioral equivalence.

Original parts and the combined archive are retained outside the checkout in
the task's attachment workspace. The repository stores an index, not the source
payload. Preserve the original uploads or a private archive with the pinned
digest for future tasks; Git alone cannot recover the indexed source bodies.
Generated `.exe`, `.dll`, `.o` and `.a` files were inspected as data and never
executed or loaded. Original input binaries and a generated historical DLL
omitted by the supplier are not prerequisites for reviewing this delivery.

## What the supplied results establish

| Supplied claim | Assessment and limit |
| --- | --- |
| Fresh common build: 296 objects, 356 normal test groups, 19 terminal scenarios | Recorded in `source-workspace/build-record.json` and `full-build-verification.json`; all pinned bytes match. Tests have not been independently rerun here. There are 355 normal executables; test groups and executable counts are different units. |
| Native adapter testing | The record names Windows-hosted LLVM-MinGW clang 23.1.3 and owned fixtures. It is not MSVC acceptance of this repository or real Consumer/game execution. |
| Complete DLL readiness | Explicitly **BLOCKED** in `dll-link-gate-source/current-full-link.json`: link exit 1, no production DLL created or loaded. `PASS_EXPECTED_BLOCKER` means the diagnostic identified the expected failure. |
| Loader integration | Two unresolved contracts remain: `require_loader_attach_runtime` and `require_loader_detach_quiescence` in the donor DLL link gate. They are donor gaps, not demonstrated defects in the current application. Do not fill them with dummy successful definitions. |
| Broad reconstruction coverage | Address/body counts describe the donor's reconstruction model. They do not measure current product completion or prove original ABI, CRT, loader, hook or network equivalence. |
| B49 accept cancellation | Tests use real local overlapped pipe IO and IOCP at an injected socket boundary. This is useful cancellation/lifetime evidence; it is not a real AcceptEx interoperability test. |
| B51 application admission | Tests exercise a selected donor host policy. The donor itself leaves external foreign/hook admission, ABA and full image unload outside that proof. |
| Historical B54 totals | Historical snapshots coexist with the current 296/356/19 record. Do not add their totals or describe the older counts as the current build. |

The delivery contains valuable failure and concurrency scenarios. Their benefit
to this project depends on reproducing them against the current implementation.
The archive does not substantiate a working replacement production DLL or a
quantified product-completion percentage.

## Development ledger

Each row is a stable work-item ID. Future commits update its status and add the
actual changed paths, commit, test command and result. A reference path below is
relative to the external ZIP, not an active build input. Pending items are not
promises to replace a subsystem.

| ID | Priority / status | Reference and current counterpart | Next concrete acceptance step |
| --- | --- | --- | --- |
| A11-00 | P0 / intake complete | Package manifest, source snapshot and fresh build records; full file/module indexes linked above | Retain exact input identity for every later transfer. Recheck any replacement delivery; keep supplied results separate from fresh execution. |
| A11-01 | P0 / donor blocker confirmed | `dll-link-gate-source/current-full-link.json`; current `src/platform/dll_entry.cpp` and unload recovery contracts | Map the missing donor attach/detach contracts to current loader-safe startup and explicit shutdown ownership. Record gaps before considering any implementation transfer; require genuine Windows lifecycle tests for a behavior change. |
| A11-02 | P1 / initial source comparison | `anadius-startup-publication-source/publication.cpp`, `publication_tests.cpp`; current `src/runtime/application.cpp`, `tests/native/startup_publication_tests.cpp` | Inventory tests for preseeded flags, duplicate publications, foreign issuers, exceptions before publication and visibility of payloads from separate workers. Current native coverage already pauses extra completion until factory publication. Port only distinct scenarios against the current execution object. |
| A11-03 | P2 / substantial existing coverage | `anadius-config-defaults-source/defaults.cpp`, defaults tests; current `src/runtime/application_defaults.cpp`, `tests/unit/config_defaults/` | Compare each assertion with existing constructor, allocation-failure and config-to-XML tests. Preserve the accepted SURGE name, current defaults and standard CRT. Donor constructor values are not authorization to overwrite product policy. |
| A11-04 | P1 / document and test review begun | `anadius-application-admission-source/admission_tests.cpp`, `admission_integration_tests.cpp`; current application/DLL worker and callback lifetime guards | Construct a controlled start-versus-shutdown race. Prove new entries are refused after stopping, an accepted in-flight entry retains ownership, and retry occurs only after it returns. First map existing serialization; do not assume the donor Binding API exists here. |
| A11-05 | P1 / document and test review begun | `anadius-accept-stop-source/stop_tests.cpp`, `stop_integration_tests.cpp`; current `src/net/asio_service.cpp`, socket retirement tests | Compare stale/duplicate completion, cancellation-success races, ERROR_NOT_FOUND and late acknowledgements with current Asio-owned operations. Add missing real Winsock scenarios without replacing the accepted Asio transport with donor IOCP composition. |
| A11-06 | P1 / pending | Donor aggregate teardown and configuration/services factory modules; current application stop, atomic-file recovery and native shutdown failure tests | Map every resource to its existing owner; distinguish already-drained resources from failed cleanup retained for retry. A test must detect premature release or a repeated release of successfully retired ownership. |
| A11-07 | P2 / pending | `hook-engine-source/`; current pinned MinHook build-copy fix and eleven-hook contracts | Compare negative cases with existing single-disable IP relocation, rollback and unload tests. Preserve vendor hashes and the reviewed build-copy patch; do not copy an older hook implementation over them. |
| A11-08 | Reference only / implementation excluded | Private CRT reconstruction modules; current standard compiler CRT and four-vendor contract | Use documented failure scenarios only when they can be expressed against accepted APIs. No custom CRT, allocator/container ABI or donor CRT object imports. |
| A11-09 | P2 / pending | Remaining modules in the module ledger | Triage by subsystem and link a concrete current source/test path. Deduplicate before proposing code changes; retain unreviewed status until an actual comparison is recorded. |
| A11-10 | P0 / native execution pending | Any later runtime/test transfer | Bind fresh MSVC Debug/Release and applicable Native/Golden evidence to that exact commit. Record portable, native fixture, production DLL and real Consumer/game outcomes separately. |

The first test-transfer candidate is A11-04, followed by A11-05. Both concern
ownership at shutdown and need a comparison with the current design before code
changes. A11-02 offers additional publication test ideas; A11-03 already has a
substantial product-specific implementation and test suite.

## Reproduce intake verification

Run from the repository root with the merged ZIP retained outside the checkout.
Join parts in numeric suffix order first. This command verifies the exact merged
bytes and every indexed member without extracting or running archive content:

```sh
python -B - /absolute/path/to/dll-reverse-engineering-11.zip <<'PY'
import hashlib, json, stat, sys, zipfile
from pathlib import Path
from scripts.project_files import safe_member

archive = Path(sys.argv[1])
provenance = json.loads(Path('docs/RE_ARCHIVE11_PROVENANCE.json').read_text())
expected = json.loads(Path('docs/RE_ARCHIVE11_FILES.json').read_text())['files']
pin = provenance['combined_archive']
with archive.open('rb') as stream:
    digest = hashlib.sha256()
    for block in iter(lambda: stream.read(1024 * 1024), b''):
        digest.update(block)
if archive.stat().st_size != pin['bytes'] or digest.hexdigest() != pin['sha256']:
    raise SystemExit('FAIL: combined archive identity')
with zipfile.ZipFile(archive) as bundle:
    entries = bundle.infolist()
    names = [entry.filename for entry in entries]
    if len(entries) != 4542 or len(set(n.casefold() for n in names)) != len(names):
        raise SystemExit('FAIL: ZIP member count or duplicate paths')
    actual = {}
    for entry in entries:
        safe_member(entry.filename[:-1] if entry.is_dir() else entry.filename)
        if stat.S_ISLNK(entry.external_attr >> 16):
            raise SystemExit('FAIL: symlink')
        if entry.is_dir():
            continue
        digest = hashlib.sha256()
        with bundle.open(entry) as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b''):
                digest.update(block)
        actual[entry.filename] = (entry.file_size, digest.hexdigest())
if actual != {row['path']: (row['bytes'], row['sha256']) for row in expected}:
    raise SystemExit('FAIL: indexed file bytes')
print('PASS: archive identity and all 4266 indexed files; no execution')
PY
```

## Local validation and next review boundary

Intake verification above is independent source/data validation. The archive's
Windows tests have not been rerun in this Linux environment. No MSVC, Golden,
production DLL or real Consumer/game PASS is claimed by this tracking change.
Repository source QA and portable CTest results are recorded in the PR body.

This PR can establish the tracking baseline without claiming completion of its
open development ledger. Each implementation transfer must identify its donor
input hash, explain its product-specific adaptation and demonstrate that the new
test detects the intended failure. Keep generated receipts under ignored
`build/` or hosted artifacts; update the existing ledger rather than accumulating
unreviewed evidence dumps in the source tree.
