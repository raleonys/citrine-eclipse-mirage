# Repository badges and opening-PR validation

The README uses repository-owned SVGs under `resources/badges/`. It makes no
requests to Shields or GitHub's generated workflow-badge endpoint. This removes
the external badge-generation hop that produced intermittent Camo 504 responses.
GitHub still hosts and caches the image responses; this does not promise that
GitHub itself cannot have an outage.

## Badge ownership and updates

`scripts/repository_badges.py` publishes thirteen badges using the Python standard
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

The live CI, Latest release, Release integrity, Tests, Checksums, QA and CodeQL images are read from the same repository's
long-lived `badges` branch, at `resources/badges/ci.svg` and
`resources/badges/latest-release.svg`, plus `resources/badges/release-integrity.svg`,
`resources/badges/tests.svg`, `resources/badges/checksums.svg`, `resources/badges/qa.svg` and `resources/badges/codeql.svg`.
All thirteen generated SVGs live there, and
only those thirteen paths are permitted. That branch is generated image data, not
an application/source branch or a PR head. Keep it after ordinary PR cleanup.

The small Update repository badges workflow runs after a completed main-push
Source integrity or Full validation after main synchronization run, or a
published/edited/deleted nondraft release. The latter main-push event refreshes
the CodeQL result after all analyses finish. Manual
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
this thirteen-image data tree. Initialize or refresh it from reviewed source with:

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

Reports must contain actual test suites, with testcases directly owned by a
suite. Suites must be direct children of the root or another suite; nested
aggregate containers and unrelated wrappers cannot supply testcase ownership.
Every suite must declare a decimal `tests` count matching its descendant cases.
The root aggregate's `tests` count is optional but must match when present.

Optional `failures`, `errors` and `skipped` summaries on suites or aggregates
must contain unsigned ASCII decimal counts matching the corresponding testcase
outcomes. Contradictory or malformed declared totals reject the report before
test-count recording; omitted outcome summaries remain allowed.

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
upgrade from exactly the original nine, previous eleven or CodeQL twelve SVG paths to thirteen;
missing or extra files outside those recognized layouts remain an error.

## README rows and CodeQL status

The first README row ends with Changelog and groups the seven blue information
badges. An explicit line break starts the six status badges below it: Tests (native), Checksums, Release
integrity, Source integrity, QA and CodeQL, in that order. The normal
success state is green; pending, failed and unavailable results keep their honest
status colors. On narrow screens each group can wrap naturally without forcing
horizontal scrolling.

CodeQL is green only when the current accepted main SHA has a completed successful
main-push run and all three actual language jobs (Actions, C/C++ and Python) passed.
An eligible newer pending/failed run cannot fall back to an older green result.
Missing, duplicate or skipped analyses, a different commit, a feature branch or
another repository cannot produce green. Pending is yellow, failure is red and
unavailable/cancelled is gray. The source seed is unavailable; the generated branch
publishes the live result after owner integration. This image adds no PR analysis,
CodeQL runner, runtime dependency or new security permission.

## QA status

The compact QA badge reports the actual **Source QA and portable tests** job:
complete source-regression checks and Linux portable CTest. It is separate from
the lightweight Source integrity hash gate, Windows/native totals and CodeQL.
A Windows job failure does not replace this job's own result.

Only this repository's full-validation PR runs with a tested Git tree exactly
equal to current accepted main are eligible. The merge SHA may differ, but an
ancestor/different tree, fork or standalone diagnostic cannot supply green.
The newest matching run's latest attempt must contain exactly one completed,
successful QA job. Missing, duplicate, skipped, unrequested or truncated job
evidence is unavailable; newer pending/failed evidence cannot fall back to an
older green result. The run scan is bounded to thirty entries.

Passing is green, failing red, running yellow and cancelled/unavailable gray.
Unavailable uses the short n/a value. Updates use the existing completed-main,
release and manual publisher events; no PR build or publisher runner is added.
The source seed is unavailable. New generated paths appear after the publisher
runs from owner-integrated source; the source branch's live URL can be absent
before that first publication. Preview the source SVG for layout separately.
The closed migration accepts only the exact recognized 9/11/12/13-image trees.

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

CodeQL for Actions, C/C++ and Python runs only on accepted-main pushes.
The job has no readiness dependency and binds checkout/analysis to the exact
pushed SHA and `refs/heads/main`. PR opening, synchronization, validation labels
and manual diagnostics do not schedule CodeQL, even when full validation is
otherwise eligible. Source QA and all eleven required Windows/source/reference
contexts retain their existing readiness gates. On main pushes those build/test
jobs skip before runner allocation; only the three analyses run in this workflow.
Cancellation prevents new analysis jobs, and main scans cannot cancel PR
validation. The default query suite and C/C++ build-mode none are retained.
This policy reports security findings after integration rather than requiring
another analysis of each proposed PR head.

## GitHub Default Setup is managed separately

GitHub's autogenerated `dynamic/github-code-scanning/codeql` workflow is separate
from repository YAML. It previously ran all three analyses when a PR opened;
the observed C/C++ analysis alone took about 26 minutes. Editing the advanced
workflow does not disable an external Default Setup scheduler.

Use **Advanced Setup** with the existing
`.github/workflows/msvc-reference-graph.yml` for the main-only policy. If Default
Setup is still enabled, the owner/admin can disable it under repository Settings,
Code security, CodeQL analysis. This disables the default scheduler rather than
SARIF uploads from the advanced workflow. Do not generate a second workflow with
PR-opening, feature-push or scheduled triggers for the same analyses.

The connected write account cannot read or change the Default Setup setting:
its configuration endpoint returns HTTP 403, Resource not accessible by
integration. The setting is therefore not changed or represented as verified by
this repository edit. An external scheduler, if enabled, needs separate owner
administration. GitHub documents the distinction in
[Default setup is enabled](https://docs.github.com/en/code-security/code-scanning/troubleshooting-sarif-uploads/default-setup-enabled).

Verify that a new PR opens with Source integrity only. On a later authorized
synchronization, require the existing eleven validation contexts and no CodeQL
analysis runners. After owner integration, verify a main-push run executes all
three analyses and uploads to its exact main SHA. A skipped, cancelled or old run
is not a fresh PASS. Do not add an initial validation label or dispatch merely
to open the policy PR.
