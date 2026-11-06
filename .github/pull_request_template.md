## Change and reason

Describe the concrete problem and resulting behavior.

## Scope

List affected contracts and any intentional behavior changes. Link the owner's
request when it changes an existing product decision.

## Branch policy

- [ ] This PR is ready for review, or a specific owner authorization for
      research Draft status is linked below (existing PR #81 is authorized).
      Draft does not waive validation, unresolved blockers or owner review.
- [ ] For a bugfix, routine audit or other single-PR task, use a fresh
      descriptive branch based on current `main`; do not reuse the retired
      shared `bugfix` branch or a closed task branch.
- [ ] For a continuing research, Reverse Engineering or reconstruction
      project, identify its dedicated project branch and documented scope.
      Existing reverse-engineering work such as `research/reconstruction-package`
      may span multiple PRs; ordinary audit findings still remain temporary.
- [ ] **Only** for an explicitly owner-designated long-running audit campaign:
      record the designation, continuing scope and closure criteria. Ordinary
      `audit/*` branches remain temporary.
- [ ] No direct changes or pushes to `main` or shared-history rewrites are
      part of this PR. Only the owner merges after review.
- [ ] Overlapping contributions and dependencies have been checked.
- [ ] For a temporary PR head (including routine audits and bugfixes), record
      its tip, preserve required work and delete it when this PR closes.

GitHub's repository-wide **Automatically delete head branches** may also
delete retained research or designated long-running audit heads. Prefer manual
cleanup of temporary PR branches unless verified protections exclude all
retained project heads. See [branch cleanup](https://github.com/raleonys/citrine-eclipse-mirage/blob/main/docs/GITHUB_WORKFLOW.md#branch-completion-and-cleanup).

## Changelog coverage

- [ ] `CHANGELOG.md` records this PR's changes, or the owner reviewed the `changelog-exempt` label and a concrete justification appears in the PR description.

## Validation

Record executed commands/results and relevant CI or artifact links. Distinguish
source QA, portable tests, native MSVC/Golden checks and real game acceptance.

- [ ] New/edited/deleted source and test files have reviewed SOURCE_FILES entries;
      the separate ACTIVE_SOURCE_MANIFEST was refreshed only for approved changes.
- [ ] Full local source QA passed on the committed source being pushed; include
      the commit SHA and result. A targeted test or manifest-only PASS is insufficient.
- [ ] The local pre-push hook is installed and was not bypassed; inspect CI for
      this commit before requesting review.
- [ ] PR creation needs only hosted source integrity. After synchronization,
      inspect the automatically scheduled full validation; do not duplicate it.
      Never prepare or label full validation merely at PR creation. On a later
      synchronization request with no push, complete `operation=synchronize-pr`
      for this PR/head/current main before adding `run-full-validation`, only
      when no successful or active exact-head PR run exists. Missing full
      checks remain required before merge; focused diagnostics are manual.

## Remaining work

Record failures, checks not run and unresolved risks. The owner reviews and
merges this PR; neither automated approval nor automatic merge is requested.
