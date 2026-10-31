## Change and reason

Describe the concrete problem and resulting behavior.

## Scope

List affected contracts and any intentional behavior changes. Link the owner's
request when it changes an existing product decision.

## Branch policy

- [ ] This PR is ready for review; draft mode is not used to track remaining
      work or pending validation. Any merge blockers are recorded below.
- [ ] Routine work reuses `bugfix`; a dedicated audit/project branch has a
      documented sustained scope, or a temporary exception has an explicit owner instruction.
- [ ] No direct changes or pushes to `main`, shared-history rewrites, or branch
      deletion are part of this PR. Only the owner will merge it.
- [ ] Overlapping contributions were checked; unrelated active work is excluded.

For a dedicated branch, describe its continuing scope and completion criteria.
Keep `bugfix` after merge; follow [branch cleanup](https://github.com/raleonys/citrine-eclipse-mirage/blob/main/docs/GITHUB_WORKFLOW.md#branch-completion-and-cleanup)
before retiring another branch.

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
