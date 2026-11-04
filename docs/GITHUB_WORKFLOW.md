# GitHub development and review

The repository starts from the owner's exact source archive, pinned externally
by SHA256 `ef7813322f0eec73785e9838d657b8b5a65f1cdfa9d579800065622fa96b9ab4`.
All 4113 archived source files, vendor payloads, avatar bytes and public ABI were
imported unchanged. The initial README commit only initialized the empty GitHub
repository; the following source import commit establishes the full baseline.
Previous ZIP deliveries remain independent backups, not recreated Git history.

## Change and merge policy

The mandatory rules at the start of [AGENTS.md](../AGENTS.md) apply to all agents
and contributors. Temporary single-PR branches are the **default**, including
routine audit findings; dedicated research workstreams and specifically
owner-designated long-running audits are the limited exceptions. The old shared
`bugfix` branch must not be revived.

| Branch | Purpose | Lifetime |
| --- | --- | --- |
| `main` | Accepted integration; updated only by owner-reviewed PR merges | Permanent integration branch |
| Fresh `fix/<task>` branch | One bugfix PR | Temporary; delete after closure and preservation |
| Fresh `audit/<finding>` branch | One bounded audit finding/review PR | Temporary by default; delete after closure and preservation |
| Dedicated `research/<project>` branch | Continuing scoped research project | May span PRs; owner decides final retirement |
| Dedicated Reverse Engineering workstream (for example, `research/reconstruction-package`) | Reverse engineering, source reconstruction and donor-parity investigation | May span PRs; owner decides final retirement; the branch name alone does not grant an exemption |
| Explicitly designated long-running audit branch | Owner-approved multi-PR audit campaign with documented scope and closure conditions | May span PRs; **not** inferred from the `audit/` prefix |
| Other single-PR task branch | Documentation, policy, test or feature contribution | Temporary unless separately designated a continuing project |

Never edit, commit, push, reset, delete or force-update main directly, locally or
remotely. File-content API writes targeting main are also prohibited. Reading,
fetching and testing it in a clean checkout is allowed. Only the owner merges
reviewed PRs into main.

Before writing, inspect the working tree, current refs and open PRs, and fetch
current `main`. Read `AGENTS.md` and `GOVERNANCE.md` from both fetched main and
the working checkout. For bugfixes, routine audits and other individual PRs,
create a fresh descriptive branch from fetched `main`; never reuse the retired
shared `bugfix` branch. For an established continuing research project or an
owner-designated long-running audit, inspect and continue its dedicated project
branch if the scope matches. Preserve provenance, uncommitted work and unrelated
active contributions.

While a PR is open, update its source branch and synchronize accepted main by
fast-forward or normal merge when needed. Never rebase published shared history
or use force-push, including force-with-lease. Continuing research and explicitly
owner-designated long-running audit campaigns may span successive PRs without
retiring the workstream branch at every PR closure. Routine audit PRs do not have
this exception; preserve any further work before normal closure and cleanup.

Open a focused PR for each coherent reviewable batch, stating the problem, scope,
validation results and remaining work. Ordinary PRs are ready-for-review, not
drafts; do not create a new draft or convert a PR to draft without an explicit
owner decision. **Existing Draft PR #81** is the specifically owner-authorized,
long-running four-field research/source-provenance project; preserve its GitHub
Draft state until the owner requests readiness. Its `research/field-retirement-review`
branch and four evidence branches are continuing research, not routine audit heads.
This narrow exception does not waive change validation, merge blockers or owner
approval and does not grant draft status to ordinary audit/bugfix PRs.
Complete required local checks before requesting normal review and record pending
CI/native/Golden/game checks. Ready-for-review is not merge authorization.
Owner-authorized weekly draft releases remain a separate flow.

## Branch completion and cleanup

Temporary PR heads, **including ordinary audit PRs and bugfixes**, must be
deleted after PR closure (merged or unmerged). The standing cleanup authority
does not allow deletion of a continuing research project or a specifically
owner-designated long-running audit campaign solely because one PR closed.

1. Check every open PR, branch user and dependency. If a routine audit reveals
   further work, preserve that work before closing and deleting its temporary
   head; do not silently turn a finding branch into a permanent project.
2. Record the exact final branch tip SHA and name. Verify a merged PR's changes
   are represented in accepted main, including squash/cherry-pick integration.
3. For unmerged closures, preserve needed commits/evidence in a verified bundle,
   archival tag or successor workstream, with recorded exact source identity.
4. Recheck the remote tip immediately before manual deletion. If it changed,
   dependencies remain, or preservation is uncertain, resolve the discrepancy.
   Never force-delete a dirty worktree or delete `main`.

Research, reverse-engineering and reconstruction branches may remain while their defined projects are
active. A long-running audit branch needs **explicit owner designation** and a
documented continuing audit scope and closure criteria; neither its `audit/`
prefix nor an open finding suffices. Retire either class only after owner
decision, preservation and dependency review. Do not bulk-delete older audit
branches before evaluating whether any are still active long-running projects.

### GitHub setting

**Automatically delete head branches** applies across the repository. When
enabled it may remove a retained research branch or owner-designated long-running
audit branch after a merged PR. Unless protections are independently verified to
exclude all such retained heads, leave the setting disabled and manually delete
closed temporary PR heads (including routine audits and bugfixes).

For a closed unmerged temporary PR, preserve any needed work first, then use
**Delete branch** or `git push origin --delete <branch>`. The owner manages the
repository setting; this policy does not assert or change its current state.

### Existing branch migration

Do not recreate the retired shared `bugfix` branch. Evaluate existing
`fix/*` and ordinary `audit/*` branches for cleanup after their PRs close,
with source SHA, dependency and evidence preservation. Do not assume every
existing `audit/*` branch must be retained, nor bulk-delete them: determine
whether a specific audit is an owner-designated continuing campaign.
Existing research, reverse-engineering and reconstruction project branches remain available while their
workstreams are active. Retirement requires the owner's decision and verified
source/evidence preservation; an individual research PR closing does not itself
end that workstream.

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
HEAD. Temporary branch deletion, including routine audit heads, follows the closure
rules above; the hook enforces neither branch lifetime nor long-running project
designation. If HEAD/files change during QA, the hook rejects the push. Pure branch deletion does not run
source QA; the preservation and dependency checks above still apply. Do not
bypass the gate with --no-verify or disable it.

Hooks do not install themselves in other clones and cannot prevent API/browser
writes or a deliberate bypass. Every agent must configure its clone. Keep CI as
the independent remote check; these local safeguards do not replace branch
protection or establish native Windows/game acceptance.

## CI and acceptance evidence

The lightweight [source integrity workflow](../.github/workflows/ci.yml) runs on
PR creation and updates, accepted-main pushes and manual dispatch. It checks
source sizes/hashes, the whole-source manifest and exact vendor inputs, without
compiling or running the full regression suite.

The [full validation workflow](../.github/workflows/msvc-reference-graph.yml)
retains its existing file path so it can be dispatched before this policy PR
is merged. Its displayed name is **Full validation after main synchronization**.
It accepts a PR `synchronize` event or explicit manual dispatch, and checks that
the selected, still-current head includes the latest default-branch commit.
Only then does it schedule:

- Source QA and full Linux portable CTest.
- Windows MSVC Native Release and Debug, Production/Golden Release, CRT Release,
  Debug and Compile Only profiles, using the existing operator scripts.
- Full Windows portable CTest in Debug and Release.
- Windows reference objects and the existing socket/source-host/session checks
  in Debug and Release, within the same full-validation workflow.

Official actions remain pinned. The eleven required status names are preserved;
the ruleset is not edited. On initial PR creation, missing full-validation
checks remain pending and prevent merge. On an ineligible update, only the
readiness job runs; full jobs are skipped before their matrices expand or any
build runner is allocated. Waiting is not a failed product test. Matrix
placeholders marked skipped are not the required expanded Windows contexts:
those real contexts remain absent and keep the PR blocked until testing runs.
A readiness API error fails the readiness job itself, rather than fabricating
eleven failed test jobs. A skipped entry is never evidence of native PASS.

GitHub's `synchronize` event means any new PR head, not specifically a merge of
`main`. Automatic full validation requires an actual transition: current `main`
was absent from the event's previous head and is present in the new head.
An ordinary edit on an already synchronized branch waits for explicit manual
validation, as does a fix after a failed test run. A stale branch must first
integrate `main`. Tests check out the exact approved head SHA, not a
moving branch name. A subsequent head update cancels an older in-progress full
run for that branch. Strict current-main branch protection still applies if
`main` advances during validation.

After normal synchronization and push, inspect the automatically created run;
do not immediately dispatch a duplicate. When synchronization makes no commit,
or a retry is explicitly needed, choose the branch in Actions → **Full
validation after main synchronization** → **Run workflow**, or use:

```sh
gh workflow run msvc-reference-graph.yml --ref <pr-branch>
```

Manual full validation also requires a current branch containing current
`main`; selecting a tag, stale head or stale branch is rejected. Before merging,
inspect the exact-head results rather than the lightweight integrity badge.
Do not manufacture an empty commit merely to trigger testing.

Manual runs on a branch differing from main also require changelog coverage.
The only exemption is the existing owner-reviewed `changelog-exempt` label on
an open PR with that exact head and integration base. A manual request does
not waive changelog policy or borrow an exemption from a different commit.

Focused diagnostic workflows remain available through their own **Run workflow**
button and branch selection. They no longer create automatic parallel runs for
ordinary PR changes. Weekly release draft validation is unchanged and remains
independent of unmerged PRs.

Inspect actual conclusions for the exact proposed commit. A skipped Windows job
is not a PASS, a configured job is not proof of execution, and earlier-head
results do not establish acceptance after a relevant change. Evidence artifacts
retain toolchain provenance, receipts, CTest results and Golden reports for
14 days. The Golden DLL, checksum and receipt are downloadable after
the production profile succeeds.

These CI jobs do not merge PRs, publish releases or establish actual
Consumer/game acceptance. Weekly release drafts remain separately governed by
[WEEKLY_RELEASE.md](WEEKLY_RELEASE.md). Hosted runner images and toolchain patches
can change; use recorded source, toolchain and binary identities when assessing
results rather than treating runner execution as the owner's game session.
