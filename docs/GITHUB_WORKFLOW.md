# GitHub development and review

The repository starts from the owner's exact source archive, pinned externally
by SHA256 `ef7813322f0eec73785e9838d657b8b5a65f1cdfa9d579800065622fa96b9ab4`.
All 4113 archived source files, vendor payloads, avatar bytes and public ABI were
imported unchanged. The initial README commit only initialized the empty GitHub
repository; the following source import commit establishes the full baseline.
Previous ZIP deliveries remain independent backups, not recreated Git history.

## Change and merge policy

The mandatory rules at the start of [AGENTS.md](../AGENTS.md) apply to all agents
and contributors. The owner applies temporary per-PR branch cleanup **only to
bugfixes**. The retired shared `bugfix` branch must not be revived. Dedicated
audit, research, and other sustained project branches may remain long-lived.

| Branch | Purpose | Lifetime |
| --- | --- | --- |
| `main` | Accepted integration; updated only by the owner's reviewed PR merges | Permanent integration branch |
| Fresh `fix/<task>` branch | One coherent bugfix PR based on current `main` | Temporary; remove after PR closure and preservation checks |
| Dedicated `audit/<scope>`, `research/<scope>` or similar project branch | Continuous, scoped audit/research/reconstruction over one or more PRs | May persist after related PRs close; retire only with owner decision |
| Other scoped work branch | Feature, documentation, policy or sustained work outside bugfixes | Scope-based; no automatic bugfix-style deletion mandate |

Never edit, commit, push, reset, delete or force-update main directly, locally or
remotely. File-content API writes targeting main are also prohibited. Reading,
fetching and testing it in a clean checkout is allowed. Only the owner merges
reviewed PRs into main.

Before writing, inspect the working tree, current refs and open PRs, and fetch
current `main`. Read `AGENTS.md` and `GOVERNANCE.md` from both fetched main and
the working checkout. For a bugfix PR, create a fresh descriptive branch from
fetched `main`, never reusing a closed bugfix branch or the shared `bugfix`
branch. For dedicated audit/research work, inspect and continue its established
project branch where appropriate; retain provenance and avoid mixing unrelated
work. Preserve others' uncommitted work and active contributions.

While a PR is open, update its source branch and synchronize accepted main by
fast-forward or normal merge when needed. Never rebase published shared history
or use force-push, including force-with-lease. Audits and research may span
successive PRs on their dedicated branches. Closing one of those PRs does not
require branch deletion or moving unfinished research to a successor branch.

Open a focused PR for each coherent reviewable batch, stating the problem, scope,
validation results and remaining work. Never create a draft PR, convert a PR to
draft or use draft mode for pending validation. Complete required local checks
before opening a ready-for-review PR and record pending CI/native/Golden/game
checks and merge blockers explicitly. Ready-for-review is not approval or merge
authorization. Owner-authorized weekly draft releases remain a separate flow.

## Branch completion and cleanup

Only **bugfix PR** source branches have mandatory deletion on closure, whether
merged or closed without merging. No separate per-branch approval is needed
after the checks below. This standing cleanup rule does **not** apply to audit,
research, reverse-engineering or other dedicated sustained-work branches.

1. Before closing a bugfix PR, check every open PR and dependency using its
   branch. Preserve any needed continuing work before the branch is removed.
2. Record the full final tip SHA and branch name. For a merged PR, verify the
   integrated changes reached main, including squash/cherry-pick equivalents.
3. When closing without merging, preserve needed unmerged commits in a verified
   bundle, tag, or successor PR and record their exact identity.
4. Recheck the remote tip immediately before manual deletion. Do not remove a
   branch if its tip has changed, a dependency is unresolved, or needed work
   is not demonstrably preserved. Never force-delete a dirty worktree.

Dedicated audit/research branches may remain even after their PRs merge or
close. They may be retired only by an explicit owner decision after verifying
the branch tip, evidence/source preservation, and live dependencies; never
bulk-delete them as part of routine PR housekeeping.

### GitHub setting

**Automatically delete head branches** is a repository-wide GitHub setting:
when enabled it can delete any eligible merged PR head, including research and
audit branches. Do **not** enable it simply to clean up bugfix PR heads while
persistent project branches share this repository. Unless an independently
verified protection configuration excludes every retained head, leave the
setting disabled and manually delete eligible closed bugfix branches.

For bugfix PRs closed without merging, preserve unmerged work, then use the
closed PR's **Delete branch** button or
`git push origin --delete <branch>`. The owner controls repository settings;
these instructions do not assert or change the current toggle state.

### Existing branch migration

The retired shared `bugfix` branch must not be resurrected. Evaluate existing
`fix/*` PR branches for closure and cleanup using the bugfix steps above.
Existing audit/research/reconstruction branches are **retained**, not targets
for mandatory post-PR cleanup. Any voluntary retirement requires the owner's
explicit decision, exact SHA/provenance retention, and review of dependents.
Do not require moving long-running project source into tags merely because a
related PR has closed.

Written rules guide contributors but do not add technical server enforcement.
Do not change repository visibility, access or protections to bypass the process.

Existing owner decisions, AGENTS.md, current governance and product contracts
continue to apply. A move to GitHub does not authorize new runtime behavior.

The assistant may prepare commits and PRs but must not merge, enable auto-merge,
approve its own PR, bypass failing checks or publish a release. The owner reviews
the diff and test evidence and decides whether to merge. Additional fixes remain
on the PR branch and invalidate earlier conclusions when relevant.

Use English for future commit messages, PR titles/descriptions, and newly
authored repository-facing text. User conversations can remain in the user's
chosen language. Preview README changes locally and obtain explicit owner
approval before pushing them; this additional approval requirement is specific
to README changes and does not apply to all otherwise authorized work.

The current GitHub connection acts as the owner's account. GitHub cannot record
an independent approval from a PR's own author. The owner can still review and
manually merge; requiring an independent formal approval needs a separate bot
or reviewer account. No branch-protection rule or approval bypass is installed
by these workflow files. Repository settings remain a separate owner decision.

The repository is public by owner decision. Repository permissions, rulesets
and the merged-branch deletion option are owner-managed settings; this PR
workflow does not change them or bypass them. Only the owner merges reviewed
PRs, and failed required checks must be resolved before merge.

## Exact source and vendor identity

The root .gitattributes disables automatic text conversion. Checkouts must retain
manifest-pinned CRLF/LF, BOM and binary bytes. Both locked Asio aliases are normal
files containing their exact relative-target descriptors, as in source ZIPs.
Do not replace them with directories or regenerate vendor files.

Changes to tracked source require an explicit reviewed refresh of the whole-source
manifest. Never refresh merely to hide drift. Build/dist remain untracked. No
release version/revision is introduced into product code or CI job names.

## Evidence retention and repository hygiene

No new `docs/evidence/` files are to be committed. This includes build/test logs, JUnit/XML output, transient JSON/CSV reports, before/after captures, duplicate contract snapshots, and bulk audit dumps. Use ignored `build/` for local outputs and CI run artifacts for temporary test evidence. Preserve non-reproducible provenance or owner-native acceptance evidence externally with a checksum and concise traceable reference; keep current contracts in `metadata/` and human-readable conclusions in `docs/`. The root `.gitignore` discourages accidental additions, but reviewers must also reject forced additions unless specifically authorized by the owner.

Historical `docs/evidence/` files were reviewed: 119 byte-exact records were relocated to locked `metadata/evidence/`, while 492 redundant/reproducible outputs were retired from HEAD and remain available in the original Git history. The immutable archive is checked against `metadata/EVIDENCE_RETENTION_POLICY.json`. Do not add to it or restore legacy output folders without an owner-approved change. Current documentation and metadata refer to the relocated evidence; historical path strings within archived snapshots still describe their original context.

## Local validation before a push

There are two independent source records. SOURCE_FILES.json lists files under
src/, include/, tests/ and third_party/ with their byte lengths and SHA256 values.
ACTIVE_SOURCE_MANIFEST.json records the entire deliverable. Updating the latter
does not update SOURCE_FILES.json. New tests need entries, and edited tests need
new hashes just as application files do; deleted files need their entries removed.

Use this order for an authorized change:

1. Review the actual code/test diff. Explicitly update only the approved first-party
   SOURCE_FILES entries using raw bytes, retaining other entry/provenance fields.
   Keep vendor pins immutable; a failed verification is not permission to bless drift.
2. Run `python -B scripts/verify_sources.py`. Resolve every missing/unrecorded/hash
   error, then run `python -B scripts/check_manifest.py --refresh` explicitly.
3. Inspect both JSON diffs and run `python -B scripts/run_source_qa.py`. All groups
   must pass; targeted tests alone do not cover the inventories.
4. Commit the reviewed change and both applicable records. Push that exact commit
   from a clean working tree, then inspect its CI result before requesting review.

Install the repository's local gate once in each independent clone (Git Bash on
Windows, or a POSIX shell):

```sh
git config --local core.hooksPath "$(git rev-parse --show-toplevel)/.github/hooks"
git config --get core.hooksPath
```

The absolute path also keeps the hook available to older worktrees of that clone.
Python must be available as `python`, as for the existing source QA command. The
hook executes local source QA on the committed checkout before transferring refs;
it performs no Windows build, automatic hash recording or hosted Actions run.
It rejects direct main pushes, updates that recreate/reuse the retired shared
`bugfix` branch, dirty checkouts and a pushed commit other than the checked-out
HEAD. Bugfix branch deletion follows the closure rules above; the hook does not mandate deleting audit/research branches. If HEAD/files
change during QA, the hook rejects the push. Pure branch deletion does not run
source QA; the preservation and dependency checks above still apply. Do not
bypass the gate with --no-verify or disable it.

Hooks do not install themselves in other clones and cannot prevent API/browser
writes or a deliberate bypass. Every agent must configure its clone. Keep CI as
the independent remote check; these local safeguards do not replace branch
protection or establish native Windows/game acceptance.

## CI and acceptance evidence

The current [CI workflow](../.github/workflows/ci.yml) runs on pull requests,
main pushes and manual dispatch. Official checkout, Python and artifact actions
are pinned to verified commit SHAs. Every PR schedules:

- Source QA and full Linux portable CTest.
- Windows MSVC Native Release and Debug, Production/Golden Release, CRT Release,
  Debug and Compile Only profiles, using the existing operator scripts.
- Full Windows portable CTest in Debug and Release.
- The separate [reference graph workflow](../.github/workflows/msvc-reference-graph.yml).
  Focused diagnostic workflows also run for their configured changed paths.

Inspect actual conclusions for the exact proposed commit. A skipped Windows job
is not a PASS, a configured job is not proof of execution, and earlier-head
results do not establish acceptance after a relevant change. Evidence artifacts
retain toolchain provenance, receipts, CTest results and Golden reports for
14 days. The local Golden DLL, checksum and receipt are downloadable after
the production profile succeeds.

These CI jobs do not merge PRs, publish releases or establish actual
Consumer/game acceptance. Weekly release drafts remain separately governed by
[WEEKLY_RELEASE.md](WEEKLY_RELEASE.md). Hosted runner images and toolchain patches
can change; use recorded source, toolchain and binary identities when assessing
results rather than treating runner execution as the owner's game session.
