# Independent static review — Content, shared context and prepared fixture

Result: **no blocking source finding** in the final inspected scope. No compiler, test, linker, executable, DLL, ZIP or REA operation ran. The reviewer authored Information's three files and does **not** label those as independently reviewed here.

| Independently reviewed source | Final SHA256 |
| --- | --- |
| application-io-source/call.hpp (root author) | `2d2fd2f4bfc1b4abaad932a88b93dd1ad9f03240ce3cc5561b252d733ab096d4` |
| content-effects-source/effects.hpp | `5904d4c70e8910b63f10be9215076680300ab0089990e8cccf1f028690f24a91` |
| content-effects-source/effects.cpp | `58b482063f9fa7d4263de0f6f21aa33cc5d5983b38ac2f96dc4b287cbe4b1812` |
| content-effects-source/effects_tests.cpp | `a7e1a97e13540d85de7798e97bbfbb102c88899e2169fc8b1633d3e018da521a` |
| worker-handoff-source/handoff_tests.cpp (SDK author) | `0488c4e8410cca91a55dffcd83d184b92df1cf2719faf8a5412ba3f569472e8b` |

The captured before files and absent-new-header record are in the root's stage snapshot. Draft header41c8 and draft Content2ae90 were not final recorded proofs; this review uses the final header and Content hashes above.

## Shared context

The immutable context captures an actual Parent, copied JobPermit, exact thread/fiber and borrowed checkpoint. Validation checks actor first, then exact permit/Parent admission and permit again, without the borrowed callback. A copied context grants no new permission or Operation lifetime. The final header locally restores LastError around **admission-checkpoint metadata**, matching existing provider metadata behavior. Actual clock/file/SDK leaf bodies remain outside that restorer. This resolves the earlier draft distinction between Information and Content checkpoint wrappers; it does not restore a whole source call's incoming SDK error or claim native ABI equivalence.

## Content production

The new narrow CallView uses the same actual configuration/Runtime/Paths/Online and identical lower provider targets. Its copied Options contain shared owners of those targets, not business-state copies. Each registered view has a private GuardedFiles, persistence Engine and leaf adapters, isolating active/checking/file ownership. Creator-side root behavior remains on the legacy execution.

The wrapper strongly retains both root and state. Registry-owned state refers back to the root nonowningly, avoiding a root-registry-state strong cycle while preserving backing until normal close. Root destruction rejects open/pending/failed rows. Closed handles can later destroy empty metadata without touching expired raw checkpoint data or moving provider retirement onto an unadmitted child.

Factory stack admission precedes allocations/callbacks. Stable preparing rows remain published through wrapper allocation and context checks. Short root-gate transactions exclude same-actor reentry, including another already-created idle view and legacy with_job, while allowing other actors and healthy original calls. New error construction occurs after releasing the metadata gate; no gate spans SDK, allocation, checkpoint, stream, dialog, filesystem or file effect. Shared sticky root poison prevents a failed private view from leaving legacy/future views falsely healthy.

Explicit close requires the same live context, no active call and no pending raw file owner. It retires private Options outside the gate while registered/nonreentrant, revalidates, marks closed and unlinks by splice. The root still owns the same lower targets during that retirement. GuardedFiles later destruction examines only its empty retained-owner metadata; its lower reference is not used after close. Failed/unclosed rows remain registered and retain actual raw prefix ownership; dropping a wrapper does not synthesize close, retry or raw destruction.

All Effects/AvatarRequest overrides route through this view's Engine/adapters. The unsupported legacy one-path avatar boundary and old HOST behavior remain. Shared log locking still ends before the Content dialog, and actual shared event/achievement lock identities are not cloned by this provider stage.

## Content authored tests

The real prepared-child fixture adds two idle same-context views, actual XML/count-preserving file writes, UTC, wrong-actor preparation denial, closed-handle/sibling behavior, explicit close and expired context rejection. Existing child/legacy cases and terminals171–174 remain. New isolated175/176 verify adopted raw-file retention after initial-state/open-return admission failure;177 verifies that an idle second handle cannot bypass same-actor active execution. These are authored source assertions, not observed passing results.

## SDK prepared integration fixture

The fixture reuses actual Domain/NativeThreads/PTD/FLS, PreparedMemory/JobMemory and explicit concurrent Parent operations. One actual Content owner and Information owner share the same controlled clocks/formatter/stream and exact borrowed business/Online roots. No foreign Online load/query is selected; the code-lease callback traps unexpected use.

A remains inside an actual adopted raw Output.initial_state callback. B, on a real independent child, completes a different actual guarded file through another Content view, uses actual Content/Information UTC and logging, and keeps its healthy views open at an explicit barrier. The original worker then calls legacy Content/Information methods. Clock/stream trace accesses have explicit event ordering; A's held file callback uses neither shared clock nor stream. B closes its own views, returns JobMemory and is physically joined/reaped **outside** the original Operation before A is released. A's raw bytes, counted embedded NUL, one-time close/destruction and later closed-handle metadata retirement are checked. Foreign/stale contexts deny before their borrowed checkpoint.

The isolated704 variant throws an exact custom failure from A's raw initial-state callback only after B is physically reaped. While A's context is still valid, normal close and replacement factory deny shared poison; raw initial=1/write=close=destructor=0 is retained. After A's real child exit/reap, the original Content route still denies, the failed raw owner remains retained, and the unrelated Parent stays healthy. It exits the isolated process without unwinding or fabricating cleanup of the retained owner. Existing normal scenarios and retained child modes remain registered.

## Scope limits

This verifies source composition and prefix reasoning only. It does not enable actual production Original concurrency or exercise full Graph/Profile/NativeJobs/kernel networking, modal UI, arbitrary shared-provider safety, licensing mutable roots, native MSVC class/string/EH ABI or OS unload success. The per-call provider foundation and its authored scenarios are concrete prerequisites, not a claim that the full DLL is complete.
