# GitHub development and review

The repository starts from the owner's exact source archive, pinned externally
by SHA256 `ef7813322f0eec73785e9838d657b8b5a65f1cdfa9d579800065622fa96b9ab4`.
All 4113 archived source files, vendor payloads, avatar bytes and public ABI were
imported unchanged. The initial README commit only initialized the empty GitHub
repository; the following source import commit establishes the full baseline.
Previous ZIP deliveries remain independent backups, not recreated Git history.

## Change and merge policy

The mandatory rules at the start of [AGENTS.md](../AGENTS.md) apply to all agents
and contributors. Branch choice follows the workstream, not the number of tasks.

| Branch | Purpose | Lifetime |
| --- | --- | --- |
| `main` | Accepted integration; updated only by the owner's reviewed PR merges | Permanent |
| `bugfix` | Routine fixes, dependency updates, documentation, tests and small maintenance | Permanent; reused across PR batches |
| Descriptive audit/project branch | Explicitly scoped, sustained investigation or substantial project | Retained throughout the workstream; retired only after completion review |
| Exceptional temporary branch | Work explicitly requested by the owner to be isolated temporarily | Deleted after the completion and preservation checks below |

Never edit, commit, push, reset, delete or force-update main directly, locally or
remotely. File-content API writes targeting main are also prohibited. Reading,
fetching and testing it in a clean checkout is allowed. Agents prepare changes
on the appropriate working branch and open PRs; only the owner merges into main.

Before writing, inspect the working tree and current branch, fetch remote refs,
and inspect open PRs for overlapping work. Read the current AGENTS.md and
governance/CURRENT_SCOPE.md from fetched origin/main as well as the working branch;
an older audit checkout does not exempt an agent from current owner rules.
Synchronize bugfix with current main
before the next batch: fast-forward when possible, otherwise merge main normally.
Preserve others' uncommitted work. Never rebase published shared history or use
force-push, including force-with-lease. Coordinate overlapping edits and do not
combine unrelated active contributions merely because they share bugfix.

Open a focused PR for each coherent reviewable batch, stating the problem, scope,
tests and remaining work. Retain bugfix after merge. Prefer merge commits for
these PRs to make later synchronization simple. A dependency update, documentation
change, new task or new PR does not by itself justify a new branch.

Dedicated audit/project branches need a descriptive name and an explicit scope
recorded in the PR. Reuse each branch through the sustained workstream rather than
opening another branch for each finding. Do not create a dedicated branch merely
to submit one or two small PRs. Existing active audit branches remain protected
from cleanup. Exceptional temporary branches require an explicit owner instruction.

## Branch completion and cleanup

Keep automatic head-branch deletion disabled. A merged PR alone does not retire
a shared or ongoing workstream branch. Never delete main or bugfix.

For a completed dedicated or exceptional temporary branch:

1. Obtain the owner's confirmation that the workstream is complete. Check all its
   open PRs and outstanding work; do not delete a branch still needed by either.
2. Verify the tip is contained in current main. For squash/cherry-pick histories
   where ancestry cannot prove preservation, review the actual changes and record
   their preserved destination; do not infer safety from a closed PR or its title.
   Unmerged work requires explicit owner instructions for preservation/disposal.
3. Record the branch name and full tip SHA in the cleanup record before deleting
   the remote branch. Do not remove local worktrees with uncommitted changes.
4. Recheck the remote tip immediately before deletion. If it changed, or ownership
   or preservation is uncertain, preserve the branch and report the issue.

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

The owner explicitly chooses to keep the repository private on the current plan
and continue with manual PR review/merge. Rulesets are not enforced on this
private repository under the current plan. PR/CI checks remain visible, but
GitHub does not technically block direct pushes or merging failed checks. The
assistant must still follow the owner-only merge policy. No plan upgrade or
organization transfer is part of this task.

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
It rejects direct main pushes, bugfix deletion, dirty checkouts and a pushed commit
other than the checked-out HEAD. If HEAD/files change during QA, it rejects the push.
Pure branch deletion does not run source QA; the owner-confirmed cleanup rules
above still apply. Do not bypass the gate with --no-verify or disable it.

Hooks do not install themselves in other clones and cannot prevent API/browser
writes or a deliberate bypass. Every agent must configure its clone. Keep CI as
the independent remote check; these local safeguards do not replace branch
protection or establish native Windows/game acceptance.

## CI and acceptance evidence

The workflow runs on PRs, main pushes and manual dispatch. Official checkout,
Python and artifact actions are pinned to verified commit SHAs. To reduce
hosted-runner usage in the private repository, automatic runs perform only
source QA and portable tests. Both Windows jobs in this CI workflow require
manual dispatch. The separate owner-authorized weekly draft workflow is described
in [weekly draft releases](WEEKLY_RELEASE.md).

- Linux executes source QA and portable CMake/CTest checks.
- On manual dispatch, Windows executes the existing Native Acceptance Release profile, including
  source API and native tests, without changing its assertions or budgets.
- A separate manually dispatched Windows job executes the existing Production/Golden Release
  profile, including its six gates and real final-name DLL lifecycle checks.
- Evidence artifacts are retained for 14 days, including toolchain provenance,
  receipts, CTest results and Golden reports. The local Golden DLL, checksum
  and receipt are downloadable only after the production profile succeeds.

To request Windows validation, open Actions, select **Source and Windows checks**,
choose **Run workflow**, and select the branch to validate. The manual run first
executes source QA/portable tests and then both Windows profiles. There is no
schedule in this CI workflow, or automatic Windows build on PR updates or main pushes. The existing
local Windows build and acceptance scripts remain available unchanged.

A skipped Windows job is not a passed Windows test. On an automatic run, a green
workflow badge records source/portable success only. Review native MSVC/Golden
evidence for the exact source commit separately before treating it as accepted;
do not inherit acceptance from a parent commit or from Wine tests. Local Windows
receipts or an explicitly requested manual CI run can provide that evidence.

This CI workflow does not create releases, publish, upload to Library or merge code. A local
Golden candidate remains NOT_RELEASE. A green CI run is not real game acceptance.
Debug/Compile/CRT-specific operator profiles and game checks remain available
separately; this initial workflow does not claim to run every profile.

Hosted runner images/toolchain patches can change. Use captured toolchain and
binary hashes to identify each result; do not equate a runner build with the
owner's compiler or accepted game binary. Newly wired Windows jobs are unverified
until their real GitHub Actions runs complete successfully.
