## Change and reason

Describe the concrete problem and resulting behavior.

## Scope

List affected contracts and any intentional behavior changes. Link the owner's
request when it changes an existing product decision.

## Branch policy

- [ ] This PR is ready for review; draft mode is not used to track remaining
      work or pending validation. Any merge blockers are recorded below.
- [ ] Bugfix PR: this uses a fresh descriptive branch from current `main`;
      the retired shared `bugfix` branch is not reused. If this is not a
      bugfix, mark the item not applicable and identify its scoped branch.
- [ ] Audit/research/other sustained work: its dedicated project branch and
      scope are recorded. A related PR closing does not mandate branch deletion.
- [ ] No direct changes or pushes to `main` or shared-history rewrites are
      part of this PR. Only the owner will merge it.
- [ ] Overlapping contributions were checked; unrelated active work is excluded.
- [ ] For a bugfix PR only: its source branch will be manually deleted after
      this PR closes (merged or unmerged), once needed work is preserved.

Do **not** rely on GitHub's repository-wide **Automatically delete head
branches** toggle for bugfix cleanup: it may also delete retained audit/research
heads. Follow [branch cleanup](https://github.com/raleonys/citrine-eclipse-mirage/blob/main/docs/GITHUB_WORKFLOW.md#branch-completion-and-cleanup)
and record any bugfix preservation work or owner-approved research retirement.

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

## Remaining work

Record failures, checks not run and unresolved risks. The owner reviews and
merges this PR; neither automated approval nor automatic merge is requested.
