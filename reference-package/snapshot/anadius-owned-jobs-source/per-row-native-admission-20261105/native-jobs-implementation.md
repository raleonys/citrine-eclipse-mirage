# NativeJobs per-row native admission implementation

Status: source implementation frozen for independent static review. No compiler, project tests, linker, reconstructed-DLL execution, or ZIP update was run. This is not runtime validation or a full native-order equivalence claim.

## Frozen source

- `outputs/anadius-owned-jobs-source/jobs.hpp` SHA-256 `f7f4194daea473156c401f3f88da0d61cc04d8a0faaf335bcb4b67cd351ab67d`.
- `outputs/anadius-owned-jobs-source/jobs.cpp` SHA-256 `05c95fea1d10f1a3107ea1462b6ed64af98b86a8af89b951a481af474ee46b3f`.

Only these two production files are owned by this implementation. InlineJobs source behavior is unchanged. Existing native API create_event/set_event fields remain source compatible, but are unused and not constructor requirements. Production Parent policy remains Serialized; explicit Concurrent selection belongs to existing composition, not to this scheduler.

## Binary constraints

The complete saved REA/Ghidra replies in this directory bind to original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`:

- `instructions-18004a458.json`: native detach `18004a458..18004a472`, Evidence `ev_e96e4d0d1bc911491e6be0de8637182614908744734c58f437235ab504993447`. `18004a45f` calls IAT `18007f0a8`; only CloseHandle result is mapped to 0/4. No event publication or wait is present in this helper.
- `symbol-18007f0a8.json`: CloseHandle IAT identity, Evidence `ev_885b6131300f5bad3d151d683ba682cb8693c29c446cab5e83c82c534c2b7632`.
- `instructions-18005e8e0.json`: complete native launch body `18005e8e0..18005e9b9`, Evidence `ev_4ce98b79108cf3f40e3dbf8e1c26b83fef80d05cc6dc6d65ac7b44ff33df84d6`. `18005e93e/18005e948` forward caller flags; `18005e94c` invokes the native creation IAT. Normal handle/id return does not wait for detach.
- `symbol-18007f468.json`: CreateThread IAT identity, Evidence `ev_79e2a53e3272243d2f11625d73cdf898abc6efd6bddeae2acbf3d75c926e3e02`.

Previously retained caller/child evidence establishes zero creation flags before native025556 and later detach02557f, with captured function child0217aa followed by native cleanup0217ac. These addresses justify removing the reconstructed scheduler's invented detach event and global body/reap exclusion; they do not prove arbitrary shared lower providers race free.

## Implemented behavior

1. Stable list nodes now carry a monotonically increasing per-row generation, prepublished exact JobToken/receipt, prepared same-Parent JobHandoff, work obligation, and immutable completion-port capture before native launch. Row publication precedes the zero-flags begin_thread call. Acquired JobMemory and Parent OperationPolicy supply source authority; the scheduler does not wait for start return or detach.
2. Separate launch-entered/returned, executing, post-entered, notified, physical and maintaining fields preserve real effects without converting lifecycle booleans into child-body barriers. Child entry holds no scheduler-wide lock across acquire, message processing, memory return or SDK post. Removing source_gate preserves Parent's own selected Serialized/Concurrent policy.
3. Original returned handle/id and DuplicateHandle output are adopted before postchecks. An early Returned/Consumed state is never overwritten by a later Launched publication. Child failure preserves the first exception_ptr by empty-slot swap, does not destroy a replacement exception under metadata, and does not clear the original worker's active lifecycle reservation.
4. Detach accepts exact normally launched or early Returned rows, closes only the original handle, and records known close/detached/counter effects before any health query. It does not signal an event. Unknown outcomes retain the real handle/offer/work prefix without speculative cancellation, rollback, retry, or handle close.
5. Raw IOCP post can deliver a receipt before its native call returns. Receipt consumption requires the exact returned, detached work row, but not notified/executing completion; app.complete_job and the Runtime dispatcher preserve one acknowledgement and one dispatcher-owned scheduler decrement. Child post result is recorded independently, without rewriting an already Consumed result. Unknown post after delivery still poisons/retains the ambiguous row rather than inventing successful native finality.
6. Outside-Parent maintenance uses Parent's existing reap policy, skips live/unconsumed/executing/posting rows, and reserves only one exact consumed row for zero-time native handle probing. Actual signaling plus normal exit, completed post, and no retained failure authorize physical retirement. A held sibling never becomes another row's eligibility prerequisite. Known joined-handle close is cleared before postcheck; JobHandoff destruction occurs outside metadata; generation and reservation are checked before reuse.
7. Native-only metadata validation uses unique_lock plus an unlock-before-exception helper, including original-affinity and Application matching. Callback/new-handler-capable exception construction, SDK calls, source callbacks, list-node allocation, callable retirement and handoff destruction run outside metadata. Stable-node splice and scalar state updates stay under metadata.

## Static review scope and limits

Reviewed normal early-child return, post-delivery-before-return, child failure overlapping original start/detach, incremental node growth, source and joined handle close prefixes, healthy physical timeout, per-row generation reuse, and one held sibling with another consumed row. The existing InlineJobs prefix was not changed. No new native-function coverage percentage is claimed.

The Root-owned Parent Serialized busy-aware reap change is a separate prerequisite in this stage; actual SDK scenario authoring/review is separately owned. The normal NativeJobs route still uses child process_message/return_memory plus original-worker response/021770 continuation; complete original028d30 child-tail order, generic tracked Environment work, arbitrary shared-token/Online/provider mutation, actual OwnedAccept concurrency and runtime equivalence remain open. No production caller is silently switched to Concurrent and no new admission/unload barrier is introduced.
