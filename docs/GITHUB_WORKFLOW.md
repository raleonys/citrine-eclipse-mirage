# GitHub development and review

The repository starts from the owner's exact source archive, pinned externally
by SHA256 `ef7813322f0eec73785e9838d657b8b5a65f1cdfa9d579800065622fa96b9ab4`.
All 4113 archived source files, vendor payloads, avatar bytes and public ABI were
imported unchanged. The initial README commit only initialized the empty GitHub
repository; the following source import commit establishes the full baseline.
Previous ZIP deliveries remain independent backups, not recreated Git history.

## Change and merge policy

The mandatory rules at the start of [AGENTS.md](../AGENTS.md) apply to all agents
and contributors. The owner replaced the former permanent `bugfix` model:
`main` is now the only permanent branch.

| Branch | Purpose | Lifetime |
| --- | --- | --- |
| `main` | Accepted integration; updated only by the owner's reviewed PR merges | Permanent |
| Descriptive PR branch, such as `fix/registry-close` or `chore/logo-and-pr-branches` | One coherent fix, feature, documentation change or audit PR | Temporary; delete when the PR closes, merged or unmerged |

Never edit, commit, push, reset, delete or force-update main directly, locally or
remotely. File-content API writes targeting main are also prohibited. Reading,
fetching and testing it in a clean checkout is allowed. Only the owner merges
reviewed PRs into main.

Before writing, inspect the working tree, current refs and open PRs, and fetch
current `main`. Read `AGENTS.md` and `GOVERNANCE.md` from both fetched main and
the working checkout. Create a fresh descriptive branch from fetched main for
each new PR. Do not recreate a shared `bugfix` branch, reuse a closed PR's branch
or keep permanent audit/research branches. Preserve others' uncommitted work and
keep unrelated active contributions separate.

While a PR is open, update its own branch and synchronize accepted main by
fast-forward or normal merge when needed. Never rebase published shared history
or use force-push, including force-with-lease. An audit can continue across
successive PRs, but each new PR gets a new temporary branch. Move any remaining
work to a successor PR before closing the current one; no continuing project
creates an exemption from source-branch cleanup.

Open a focused PR for each coherent reviewable batch, stating the problem, scope,
validation results and remaining work. Never create a draft PR, convert a PR to
draft or use draft mode for pending validation. Complete required local checks
before opening a ready-for-review PR and record pending CI/native/Golden/game
checks and merge blockers explicitly. Ready-for-review is not approval or merge
authorization. Owner-authorized weekly draft releases remain a separate flow.

## Branch completion and cleanup

Source-branch deletion is mandatory when a PR closes, whether merged or closed
without merging. Closure is not administratively complete until its temporary
branch has been removed. This standing owner rule authorizes routine cleanup;
no additional per-branch approval is needed after the checks below. It never
authorizes deleting `main` or losing needed unmerged work.

1. Before closure, check every open PR and dependency using the branch. Move
   continuing work to a fresh successor PR first; do not delete an active PR's
   head or a branch still needed by another open PR.
2. Record the branch name and full final tip SHA in the PR's closure record.
   For a merged PR, verify the tip is contained in current main. For squash or
   cherry-pick integration, verify the actual changes and their preserved
   destination rather than relying on ancestry alone.
3. Before closing without merging, preserve needed unmerged commits in a verified
   Git bundle, an archival tag or a successor PR. Record the preservation
   location and exact SHA before removing the source branch. Deletion is cleanup,
   not permission to discard work.
4. Recheck the remote tip immediately before deletion. If it changed or
   preservation is uncertain, resolve that discrepancy before completing
   cleanup. Delete the remote branch and remove local branches/worktrees only
   after checking for uncommitted work; never force-delete a dirty worktree.

### GitHub setting

The repository owner should enable **Settings > General > Pull Requests >
Automatically delete head branches**. This deletes eligible source branches
after a PR is merged. Branch protections/rulesets can prevent deletion; retain
protection for `main` and do not apply a permanent deletion prohibition to
temporary PR branches. Do not weaken review, CI or main protections.

The setting does **not** delete the head of a PR closed without merging. After
preserving unmerged work, manually use the closed PR's **Delete branch** button
or `git push origin --delete <branch>`. Local branch cleanup is also manual.
The setting is an owner-managed repository option, not a YAML configuration or
an assertion that automation has already been enabled.

### Existing branch migration

The old `bugfix` branch and every existing audit/research branch are subject to
this lifecycle. Check their exact tips and open PR associations before cleanup.
Remove closed-PR branches after verified preservation; keep open-PR branches
only until their own closure. Move branches serving solely as historical source
archives into verified bundles or archival tags, record those destinations and
remove the branches. Do not bulk-delete active branches or infer that an old
branch is fully preserved from its name, age or PR title.

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
HEAD. Deleting `bugfix` is allowed under the closure rules above. If HEAD/files
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
