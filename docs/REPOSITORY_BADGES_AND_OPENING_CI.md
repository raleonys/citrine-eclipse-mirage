# Repository badges and opening-PR validation

The README uses repository-owned SVGs under `resources/badges/`. It makes no
requests to Shields or GitHub's generated workflow-badge endpoint. This removes
the external badge-generation hop that produced intermittent Camo 504 responses.
GitHub still hosts and caches the image responses; this does not promise that
GitHub itself cannot have an outage.

## Badge ownership and updates

`scripts/repository_badges.py` publishes eleven badges using the Python standard
library. The six static SVGs are downloaded snapshots of the original Shields
responses, including the embedded C++, Python and CMake logos. The Windows and
license responses retain their original styling. Changelog uses the same style.
The publisher copies these static bytes unchanged; no logo or font is loaded
from a remote service by the reader or publisher.

Dynamic release badges retain Shields typography, gradients, clipping and text
shadows. The CI renderer retains the original GitHub workflow badge template,
including its embedded Octocat path, workflow label and gradients, while changing
the actual status and its color. No network image generator or font package is
needed during updates. The source tree contains reviewable CI/release snapshots.

The static responses were retrieved from the image URLs previously used by the
README; Changelog uses `https://img.shields.io/badge/Changelog-view-blue.svg`.
Shields is published under CC0; embedded Simple Icons and product marks retain
their original attribution and brand identities. The GitHub workflow template
was retrieved from this repository's original `ci.yml/badge.svg?branch=main`
endpoint. These cached assets are retained rather than represented as newly
drawn project logos.

The live CI, Latest release, Release integrity, Tests and Checksums images are read from the same repository's
long-lived `badges` branch, at `resources/badges/ci.svg` and
`resources/badges/latest-release.svg`, plus `resources/badges/release-integrity.svg`,
`resources/badges/tests.svg` and `resources/badges/checksums.svg`.
All eleven generated SVGs live there, and
only those eleven paths are permitted. That branch is generated image data, not
an application/source branch or a PR head. Keep it after ordinary PR cleanup.

The small Update repository badges workflow runs after a completed main-push
Source integrity run, or a published/edited/deleted nondraft release. Manual
refresh is allowed from the default branch. Initial PRs and feature-branch
pushes do not allocate a badge-update job. The workflow always checks out the
trusted default branch; it never runs code or downloads artifacts from the
workflow-run head. API responses are read again at publication time so stale
events cannot restore an older release label.

CI means the last completed Source integrity push run on the default branch,
matching the scope of the previous GitHub-hosted CI badge. It is not a combined
Windows/Golden or game-acceptance verdict. Latest release means GitHub's current
published stable latest release. Browser/GitHub caches can delay image refresh.
Release integrity displays that release's actual GitHub `immutable` flag: green
immutable, orange mutable, or an explicit unknown/no-release value. It makes no
claim about game acceptance or independent signature verification. Changelog
links directly to the project change history. Both reuse existing data or static
text and add no workflow or API request.

Updates are serialized, avoid commits when the SVG bytes match, use normal
fast-forward reference updates and refuse an unexpected existing branch tree.
The publisher never writes main, application sources, tags or release assets.
The generated-only branch is maintained through the GitHub Git-data API;
application source inventories and pre-push QA apply to the source PR, not to
this eleven-image data tree. Initialize or refresh it from reviewed source with:

```sh
python -B scripts/repository_badges.py --publish
```

The workflow supplies its scoped GH_TOKEN; local execution uses existing GitHub
CLI authentication without extracting or storing credentials. Initial SVG
snapshots can be rendered locally with `--output build/badges-preview`.

## Test count and checksum badges

Tests (native) displays passed/total CTest cases from Windows Native Release,
without adding repeated Debug/Release/portable executions together. The existing
native job reads its actual JUnit report and records the numeric counts in a
completed step name. This adds no job, compilation or test execution. Failures,
errors and skipped cases are excluded from passed and remain in total.

The publisher reads GitHub job metadata, not logs or downloaded PR artifacts.
It considers only this repository's full-validation PR runs whose tested Git
tree exactly equals current main. A merge commit may have a different SHA but
the same tested tree. An ancestor, an unrelated PR or a standalone diagnostic
cannot supply the count. Missing, incomplete or older reports without a count
step display not available; zero tests cannot produce a green 0/0. A failing
native job never receives a green count. The scan is bounded to the latest 30
PR validation runs. The badge describes this native CTest inventory, not test
coverage, all platform executions, CodeQL or game acceptance.

Checksums displays SHA-256 when the latest published stable release has both
its correctly named binary ZIP and the matching .zip.sha256 sidecar uploaded
with nonzero sizes. Otherwise it reports missing, unknown or no release. It
checks availability, not a fresh cryptographic verification of the download.
Both badges use the existing small main/release publisher; neither runs a
workflow when a PR opens. The bounded data publisher permits a one-time
upgrade from exactly the original nine SVG paths to exactly eleven; missing
or extra files outside those two recognized layouts remain an error.

## One basic check when a PR opens

`ci.yml` has exactly one job, Source integrity. It uses the runner's existing
Python interpreter to verify source hashes, the whole-source inventory and
locked vendor inputs. There is no compiler, CTest, complete Source QA,
CodeQL matrix or interpreter installation in this job. It is also useful after
updates and accepted-main pushes. Manual Windows operator-output fixtures have
their own `operator-output.yml` workflow, so no skipped Windows placeholder is
attached to ordinary opening checks.

Full validation still requires the existing verified later synchronization:
current main must newly enter the PR head, or a later synchronization preparation
must authorize the exact PR/head/main before the full-validation label is added.
The existing eleven required status names and strict current-main merge
protection remain unchanged. Missing full checks still prevent merging.

CodeQL for Actions, C/C++ and Python now sits behind that same readiness gate.
It analyzes the verified source head and uploads to that head's PR reference.
The default query suite and C/C++ build-mode none are retained. Ineligible
events skip the job before its language matrix expands.

## Required GitHub Default Setup migration

GitHub's autogenerated `dynamic/github-code-scanning/codeql` workflow is separate
from repository YAML. It previously ran all three analyses when a PR opened;
the observed C/C++ analysis alone took about 26 minutes. Editing `ci.yml` cannot
disable that external trigger.

The owner/admin must switch CodeQL from **Default Setup** to the reviewed
**Advanced Setup** workflow in this PR. In repository Settings, open Code
security (Code security and analysis on older layouts), find CodeQL analysis,
and choose **Disable CodeQL** from its menu to disable Default Setup. Then use
the existing advanced workflow `.github/workflows/msvc-reference-graph.yml`.
This disables the default scheduler, not uploads from the advanced workflow.
Do not create a second generated
CodeQL workflow with an opened/push/scheduled trigger. The analyzed languages
remain Actions, C/C++ and Python; only their scheduling changes.

The connected write account cannot read or change the Default Setup setting:
its code-scanning configuration endpoint returns HTTP 403, Resource not
accessible by integration. Until the owner performs the migration, GitHub can
still start its autogenerated opening analyses, and advanced SARIF uploads may
be rejected because Default Setup is still enabled. The one-check opening
guarantee therefore requires both the repository change and that setting
migration. This PR does not silently claim or alter an inaccessible setting.
GitHub documents this migration in
[Default setup is enabled](https://docs.github.com/en/code-security/code-scanning/troubleshooting-sarif-uploads/default-setup-enabled).

Once the migration is complete, verify a newly opened PR has only Source
integrity, then use a later authorized synchronization to verify all eleven
existing validation jobs plus the three gated CodeQL languages. A skipped,
cancelled or old run is not a new PASS. Keep initial-PR validation deferred
while this PR is reviewed; do not add a validation label merely to open it.
