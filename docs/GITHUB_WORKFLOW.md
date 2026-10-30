# GitHub development and review

The repository starts from the owner's exact source archive, pinned externally
by SHA256 `ef7813322f0eec73785e9838d657b8b5a65f1cdfa9d579800065622fa96b9ab4`.
All 4113 archived source files, vendor payloads, avatar bytes and public ABI were
imported unchanged. The initial README commit only initialized the empty GitHub
repository; the following source import commit establishes the full baseline.
Previous ZIP deliveries remain independent backups, not recreated Git history.

## Change and merge policy

Work starts from the current main branch in a separate branch/worktree. Every
change gets a focused PR stating the problem, scope, tests and remaining work.
Existing owner decisions, AGENTS.md, current governance and product contracts
continue to apply. A move to GitHub does not authorize new runtime behavior.

The assistant may prepare commits and PRs but must not merge, enable auto-merge,
approve its own PR, bypass failing checks or publish a release. The owner reviews
the diff and test evidence and decides whether to merge. Additional fixes remain
on the PR branch and invalidate earlier conclusions when relevant.

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

## CI and acceptance evidence

The proposed workflow runs on PRs, main pushes and manual dispatch. Official
checkout, Python and artifact actions are pinned to verified commit SHAs.

- Linux executes source QA and portable CMake/CTest checks.
- Windows executes the existing Native Acceptance Release profile, including
  source API and native tests, without changing its assertions or budgets.
- A separate Windows job executes the existing Production/Golden Release
  profile, including its six gates and real final-name DLL lifecycle checks.
- Evidence artifacts are retained for 14 days, including toolchain provenance,
  receipts, CTest results and Golden reports. The local Golden DLL, checksum
  and receipt are downloadable only after the production profile succeeds.

No workflow releases, publishes, uploads to Library or merges code. A local
Golden candidate remains NOT_RELEASE. A green CI run is not real game acceptance.
Debug/Compile/CRT-specific operator profiles and game checks remain available
separately; this initial workflow does not claim to run every profile.

Hosted runner images/toolchain patches can change. Use captured toolchain and
binary hashes to identify each result; do not equate a runner build with the
owner's compiler or accepted game binary. Newly wired Windows jobs are unverified
until their real GitHub Actions runs complete successfully.
