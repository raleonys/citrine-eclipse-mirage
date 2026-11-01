# Universal Shipping Policy — QA-only integration V1

## Role separation

The release-independent policy **engine** is `scripts/check_universal_shipping_policy.py`.
It never contains a release codename, ordinal, fixed TU count or canonical source SHA.
`metadata/SHIPPING_POLICY_CONTRACT.json` is an **unapproved proposal** describing
one concrete source topology, its exact production TU byte digests, CMake target,
required safety policies, bootstrap and hook lifecycle roles. The same engine
can validate another approved graph after receiving another external binding;
it must never auto-trust a contract from the source ZIP itself.

The 64-character SHA-256 passed in `--trusted-contract-sha` is a source-claim
pin for standalone *QA-only* replay. For an **authoritative** release this pin
must originate in independently approved governance, not a self-generated ZIP.
If the pin is absent or wrong, the engine must fail closed. A policy PASS does
not establish G0, native runtime, game acceptance, 8/8 release QA or publication.

## Covered invariants

- Exact CMake shipping graph and hashes of **every** shipping source, no fixed
  TU count in the engine, no test/debug TU leakage, required core/addon owners
- Static CRT, release link flags, pinned vendored MinHook source
- No MinHook mutation under DllMain loader lock
- PTD → release service → committed config → freeze → addon selection →
  transaction; one begin, one stage-core, one stage-addon, one commit
- Staging failure abort, rollback/quiescence ownership before config unfreeze
- Fail-closed external SHA contract binding; no claim of runtime verification

## QA-only run

`python -B scripts/test_universal_shipping_qa.py --trusted-contract-sha <sha-of-proposed-contract> --report result.json`

`python -B scripts/check_universal_shipping_policy.py` with no pin must fail closed.

## Open gates

The historical release QA orchestrator still lists legacy release-specific
regression scripts. Its G0 is pinned to historical release identity and cannot
be reused as present release authority. A separate governance-master approval
and role-based orchestrator migration are required before **authoritative**
source 8/8 and clean-extract 8/8 can be claimed. Security-sensitive historical
source regressions remain active; this first integration does not delete or
weaken them. Native transaction fault injection, Consumer/game and publication
are not authorized by these results.

The original Candidate01 source ZIP, compiled DLLs, official parent and
`CURRENT_ARTIFACT` are immutable. This ZIP is IDS / NOT_RELEASE / QA-only.
