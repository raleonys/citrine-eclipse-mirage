# Weekly draft candidates

The owner authorizes preparing a draft every Friday at **16:30 Europe/Istanbul**
(13:30 UTC). The schedule lives in `.github/workflows/weekly-release.yml` and
becomes active after that workflow is merged into the default branch. GitHub
can delay scheduled runs; the time is a trigger, not a completion guarantee.

Only the default branch is eligible. Each run pins one full commit SHA across
all jobs. A completed candidate for the same commit prevents another build.
If no managed candidate exists yet, scheduled runs report `BOOTSTRAP_REQUIRED`
and do not invent a release version or start Windows builds.

## First candidate and later series changes

Open **Actions > Weekly draft release > Run workflow** on the default branch.
For the first candidate, enter the owner-selected external tag in `start_tag`.
The format is `<series>-R<NN>` with a filename-safe series and revision R01-R99.
Leave `start_tag` empty for normal retries or to continue the current series.

To select a different version/codename later, run the same workflow with the
new external series and its starting revision in `start_tag`. The next successful
candidate establishes that series for subsequent weekly runs. This does not
require editing the workflow or application code. Even an unchanged commit may
be packaged under an explicitly requested new series. Existing releases and Git
tags cannot be reused for another source. R99 stops automatic revision allocation
until the owner explicitly selects the next series; the software version never
advances by inference from a calendar, dependency or compiler update.

External tag/package names do not change DLL VERSIONINFO, the fixed PE profile,
SDK identity, runtime configuration, or source ZIP contents. Those are separate
product contracts and require separate owner instructions. The first tag is
supplied through the dispatch input, not hard-coded in the repository or stored
in Actions variables. No additional variable-management permission is needed.

## Validation and assets

For a new candidate, the workflow executes:

1. Source QA, vendor/source integrity and portable CMake/CTest checks.
2. The existing Windows Native Acceptance Release profile, with all CTest tests.
3. The existing Windows Production/Golden Release profile, including all six
   Golden gates and final-name DLL lifecycle validation.
4. Source/DLL/evidence binding and package verification on the exact source SHA.
5. Creation or completion of a **draft**, with generated release notes.

Failures or skipped tests stop candidate assembly. The normal PR/main CI stays
lightweight; the scheduled workflow is the explicit exception for Windows
validation. Its Windows jobs consume the private repository's Actions allowance.

Each finished draft contains:

- `<tag>_source.zip` and its adjacent `.sha256`, using the existing flat-root
  source packager and preserving all pinned source/vendor bytes.
- `<tag>_windows_x64.zip` and its adjacent `.sha256`, containing the validated
  `preloader_l.dll`, DLL checksum, local Golden receipt, license notices and
  test/build evidence.
- `RELEASE_RECEIPT.json` and its `.sha256`, binding the candidate to its source
  commit, source manifest, DLL hash and executed tests.

The original compiler DLL stays separate from the finalized runtime DLL.
No signing, game execution, Library upload or release publication is performed.
Receipts retain `DRAFT_NOT_RELEASED`/`NOT_RELEASE` and `game_acceptance: NOT_RUN`.
Green CI alone does not establish game acceptance.

## Retry and owner publication

The workflow serializes its own runs. A draft initially carries an incomplete
asset notice; it becomes complete only after every expected asset is present
with matching size and, when returned by GitHub, matching SHA256 digest. An
interrupted upload leaves the draft incomplete and reuses its tag on a retry
for the same commit. Failed uploads do not consume a new revision on that retry.
Published or unrelated releases are never selected for replacement.

Review the completed draft's source SHA, receipts, notes and game acceptance
before selecting **Publish release** yourself. Keep the hidden `surge-weekly`
source marker in the release body when editing notes; it identifies the source
and series for subsequent automatic runs. Do not publish an incomplete draft
while its workflow is uploading assets.

Workflow artifacts and failure reports are retained for 14 days. Draft release
assets form the candidate archive until the owner publishes or deletes them.
The source archive is not rewritten merely to mark a draft as an official release.

[Implementation validation](../metadata/evidence/weekly-release/VALIDATION.json) distinguishes
local regressions from the first real scheduled/manual Windows candidate run.
