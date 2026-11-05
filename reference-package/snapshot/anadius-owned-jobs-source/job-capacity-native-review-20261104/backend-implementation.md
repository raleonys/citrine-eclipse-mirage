# Native job rows and prepared offers — source implementation

This subagent changed only `anadius-owned-jobs-source/jobs.cpp`, `jobs.hpp`, `anadius-worker-handoff-source/handoff.cpp` and `handoff.hpp`. Exact before copies are retained in this directory. No compilation, project tests, linking, DLL execution or packaging occurred. Root owns fresh REA/Ghidra acquisition and coordinated Profile/test changes.

## Fresh native basis

Read the seven complete saved static replies in this directory and their bundle. The examined025480 frame-completion path allocates a new0x30-byte closure at `0x1800254ed`/`0x1800254f2`, starts05e8e0 at `0x180025556`, detaches04a458 at `0x18002557f`, and restarts0280d0 at `0x180025607`. The05e8e0 routine calls the CreateThread import through `0x18007f468` at `0x18005e94c` (named by saved pseudocode). The021770 cleanup frees the per-job0x30 closure at `0x1800217fa`/`0x180021802`. No fixed64-row admission table is established by these examined bodies. This does not prove arbitrary thread availability, native heap/CRT/exception ABI or exact runtime equivalence.

## NativeJobs stable physical rows

Physical rows now reside in a list. Every existing Row address, handoff, handle, token, receipt and owner pointer remains stable when more rows are added. The actual native entry argument still points at that exact Row; no thread closure or trampoline signature changed.

`reserve_row` first scans for an Empty row under metadata gate. If none exists, it allocates one unpublished list node outside the gate, assigns its Impl owner and runs existing source admission checks. It then rechecks the active/binding/checking/phase seal under the gate and splices the node without allocation. Container topology changes use the same gate as snapshot iteration. Existing rows are never moved or erased while threads may reference them. A list capacity overflow remains a natural storage limit.

The selected row is written back through the caller's Row pointer before reserving the stage/token/receipt. Thus any later exception, including receipt exhaustion after publication, reaches the existing `failed(row)` path with the exact retained row. Allocation failure before publication leaves the caller's pointer null and retires only an unpublished Empty node outside the gate. No extra post-reservation checkpoint was inserted on the normal preallocated-row path.

NativeJobs constructor capacity is now an initial allocation rather than being rejected above PreparedMemory's former64 constant. The existing begin/admit/poison logic, detach barrier, scheduler work pin, exact ready receipt, source_gate, Parent transfer and physical thread/FLS reaping sequence remain. Consumed rows are not recycled before the existing physical proof. No list storage allocation/deallocation was introduced inside metadata gate. Existing InlineJobs is intentionally unchanged in this stage; the actual selected production path uses NativeJobs. The separate Profile-layer author changes Profile job/closure storage only.

## PreparedMemory offers

The public maximum_job_borrows64 constant and prepare_job's hard64 rejection are removed. References were inventoried first; root separately adapted the old constant-based test. JobState was already shared heap storage in a list, so existing offers and JobScope identities require no relocation scheme.

prepare_job now uses the existing job_control reservation across bookkeeping allocation. It validates original worker/borrow state under gate, sets that reservation, then constructs the unpublished shared JobState and list node outside gate. It rechecks worker/fiber/PTD/phase and the reservation before a nonallocating list splice. Only after publication does it construct the noexcept move-only JobMemory result. The reservation clears on all exits.

An allocation failure before publication creates no capability requiring guessed cancel/return; local nodes retire outside gate. Reentrant worker lifecycle calls see the existing job_control rejection/poison behavior, and a swallowed allocation-callback violation is detected by the worker postcheck. Existing child acquire, scope pins, logical return, cancellation, real self-handle retention and physical reaping are unchanged. Per root instruction, the older reaper/close implementations were not broadened in this stage.

## Final source SHA-256 before independent review

| Source | SHA-256 |
| --- | --- |
| anadius-owned-jobs-source/jobs.cpp | 47f1098982d46ffc3cc2951bcce433f80d718cc0e14c98425aa85f5304cdedc7 |
| anadius-owned-jobs-source/jobs.hpp | a2a1f920550b3d9ef9112aa0e312733c8430dc1617141082595e2bfbf944743d |
| anadius-worker-handoff-source/handoff.cpp | c082860b0727fbeb9cf506bda213f3c13778353c99e6ef1de73f2e6bcf09170b |
| anadius-worker-handoff-source/handoff.hpp | 37078e266fad3e124c07a62192ad7f73963e437feff70939274de12e9460bbec |

These identify uncompiled source bytes only. Full selected behavior also requires the coordinated Profile job/closure routing growth; neither this document nor the removed cap establishes runtime validation or an unlimited resource guarantee. No new unload/CRT framework was added.
