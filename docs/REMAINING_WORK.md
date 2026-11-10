# Current outstanding work

This list is reconciled against accepted
`main@44967de499bf3d00ccb6d3ac76302a267aedee2a`.
Current build/CI policy is in [GITHUB_WORKFLOW.md](GITHUB_WORKFLOW.md), and
runtime contracts are described in [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md).
The dated evidence below records earlier source and DLL checkpoints; former
compiler failures and their old test totals are not current acceptance status.

| Outstanding item | Next action | Evidence boundary |
|---|---|---|
| Validation of a changed head | Inspect all eleven required source/portable/Windows/reference checks for that exact head after eligible synchronization. | PR creation runs integrity only; old receipts and skipped jobs are not fresh acceptance. |
| Owner game acceptance for a release candidate | Exercise the named/versioned host, startup, LSX flow and shutdown using the identified Production DLL. | Earlier owner-reported game acceptance belongs to its recorded DLL, not all later source changes. |
| Actual external online provider | Pin the external provider hash/version and exercise factory, getter, cleanup and callback lifetime behavior. | Controlled fixture DLLs do not establish arbitrary external provider compatibility. |
| Native size attribution | Collect comparable DLL/MAP/OBJ evidence with identical MSVC/SDK/settings using `collect_size_evidence.py`. | Earlier DLL/log sizes without matching same-build native objects do not prove current attribution. |
| Exceptional Asio teardown | Extend fault coverage for exceptional stop/resource ownership and unresolved destructor paths without discarding retained owners. | Receive/send/accept schedules, active-provider drain, partial provider startup and receive-to-send timer faults have source-linked coverage; see SOURCE_DIFFERENTIAL_B49_B54.md. They do not prove every partial-construction failure or arbitrary provider/handler liveness. |
| Original-source equivalence and retirement | Resolve source-provenance, kernel identity, Registry origin and foreign-callback questions with the exact inputs and meaningful source-linked tests. | The donor review remains open; controlled packet schedules and successful builds do not authorize donor retirement. |

[PR #352's full-validation run](https://github.com/raleonys/citrine-eclipse-mirage/actions/runs/42889357574)
passed all eleven required contexts, including Native Debug/Release and
Production/Golden, for `7f592289d64494793a45290a854fa3830f63cb21` before its
integration into this baseline. This is exact-head hosted evidence, not fresh
execution for this documentation PR or owner game acceptance. Older failures
below are historical, not live universal blockers.

Current source already contains the five retained-cleanup MinHook entry guards,
real-peer/live-call/churn fixtures, installed Registry A/W fixture paths and
bounded provider/startup/packet-delay regressions. Do not propose those same
scenarios as absent features. Registry provenance (Issue #109), aggregate memory
budgets, arbitrary foreign activity and complete donor semantic equivalence
remain unresolved. Invalid-LSX behavior also remains an explicit difference:
Production closes the rejected peer; the donor's modal diagnostic and same-session
continuation are not silently restored. Proposed reference work in open PR #350
is not an accepted Production policy. The [documentation index](INDEX.md) maps
each current guide and historical comparison.

An absent AST reference is not proof that a file can be removed. Virtual,
callback, lifetime and development API roots remain in scope. Historical
results retain their original limits and are not reclassified as current PASS.

## Historical acceptance notes

2026-10-29 checkpoint (`YF-BLUEFIELD-ALIGN-20261029`): the Bluefield eleventh
hook/lifecycle and all declared Brownfield metadata targets are connected
at source level. Checkpoint verification is recorded in
[HOOK_ALIGNMENT_STATUS.json](../metadata/evidence/HOOK_ALIGNMENT_STATUS.json).
Actual Windows/MSVC Native/Production and game acceptance on the new source
remain required. The earlier acceptance details below are historical.

2026-10-29 checkpoint (`YF-GOLDEN-AVATAR-20261029`): all four Golden product targets
are now defined, resolving the former missing-target `BLOCKED` status.
The remaining task is actual Windows Native/Golden acceptance on new source.
See [current record](GOLDEN_AVATAR_FIX.md). The list below preserves
historical acceptance details.

### Outstanding acceptance tasks at that checkpoint

This list distinguishes missing application code from acceptance tests
that were not executed. The checkpoint source, its C interface, and Wine
results are in [PUBLIC_ABI_STATUS.json](../metadata/evidence/PUBLIC_ABI_STATUS.json).

| Item | Checkpoint next action | Limit at checkpoint |
|---|---|---|
| Windows/MSVC | Rerun the Native profile with the namespace-collision fix and retain full CTest results. | Latest owner logs show Compile/Debug runtime and CRT 6/6 PASS. The complete Native test suite did not run because of C2668 in an asynchronous receive test. |
| Windows command interface | Exercise error codes, final result, and console pause behavior for Compile/Debug/Native/CRT entrypoints using actual PowerShell/CMD. | Source and negative contract checks passed; the Windows console behavior was not exercised. |
| Actual online provider | Exercise version 2/factory/virtual-slot and error/cleanup paths using a `citrine_eclipse_mirage_online.dll` with a known hash and version. | Current tests use a controlled fixture DLL; an original external provider has not been supplied or accepted. |
| Named/versioned Consumer/game | Pin the exact host and how it loads the branded DLL; observe actual startup, LSX requests, hook conditions, and shutdown. | Static imports in the supplied original EXE reference `anadius64!anadius`. This source package does not modify the EXE or claim automatic compatibility under other names. |
| Complete Golden pipeline | Prove remaining section/header/import/CRT targets after actual MSVC linking, then run all six stages against the real DLL. | Resource profile and five C exports are pinned. All product targets are now defined; actual MSVC six-gate acceptance and final-name runtime checks remain required for new source. Fixture results are not an MSVC Golden substitute. |

The 548-byte legacy ABI was removed from the proven outstanding-work list;
the supplied RE sources had no authoritative interface definition of that
size. The SURGE marker and actual DllShutdown were implemented; see
[PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md). Library synchronization, publication,
and official release were outside that work's scope.
