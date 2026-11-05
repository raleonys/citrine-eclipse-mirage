# Independent static review: NativeJobs per-row admission

Status: no blocking source finding remains in the frozen production change below. Review used saved original-binary evidence, current source, and its direct caller/lower contracts. No compile, project test, link, DLL load/execution, or ZIP update was performed. The SDK fixture has a separate review; this report does not treat authored assertions as executed results.

## Exact reviewed bytes

| File under outputs/ | SHA-256 |
| --- | --- |
| anadius-owned-jobs-source/jobs.hpp | f7f4194daea473156c401f3f88da0d61cc04d8a0faaf335bcb4b67cd351ab67d |
| anadius-owned-jobs-source/jobs.cpp | 79888e8d3797ce94c2305a5391ddf35d7700cacd666e6d1efd0273b573a4c437 |
| anadius-session-memory-source/memory.hpp | 73bb13f7c083b0546b6e45e52e67fa5fbdd8aadbc761217fa5af55f7bfbb5392 |
| anadius-session-memory-source/memory.cpp | 2ca6df7d1ed04b7e32cf0db904824729fdfac20c735d1f07624b76f5073ae73a |
| anadius-worker-handoff-source/handoff_tests.cpp | de2dbd557873984bdae5dbd5a2704ca19f87d6400e5974ab5f4d4e6e907be2b0 |

The Parent-specific independent report is `parent-reap-independent-review.md`, SHA-256 `aa7b37775e716996d9c15e299b91d6cc2768648cd2fc023aea9f516c8b6a2d55`. Native author report `native-jobs-implementation-v2.md` describes the same frozen Native2 source. The older initial author report is not the authoritative source freeze.

## Addressed evidence

All four saved REA replies bind to original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`, Ghidra 12.1.4:

- Complete `18004a458..18004a472`, Evidence `ev_e96e4d0d1bc911491e6be0de8637182614908744734c58f437235ab504993447`, calls IAT `18007f0a8` at `18004a45f` and maps BOOL to 0/4. `symbol-18007f0a8.json` identifies CloseHandle with Evidence `ev_885b6131300f5bad3d151d683ba682cb8693c29c446cab5e83c82c534c2b7632`. The complete helper contains no detach event signal or wait.
- Complete `18005e8e0..18005e9b9`, Evidence `ev_4ce98b79108cf3f40e3dbf8e1c26b83fef80d05cc6dc6d65ac7b44ff33df84d6`, forwards flags at `18005e93e/18005e948`, calls IAT `18007f468` at `18005e94c`, then copies the native id and returns the handle on success. `symbol-18007f468.json` identifies CreateThread with Evidence `ev_79e2a53e3272243d2f11625d73cdf898abc6efd6bddeae2acbf3d75c926e3e02`. This launch wrapper does not wait for detach.

Previously retained caller assembly has zero creation flags at `180025543`, native launch at `180025556`, and later detach at `18002557f`. Retained child assembly calls the captured function at `1800217aa` before cleanup at `1800217ac`. The bounded evidence supports removing the scheduler's invented event/body exclusion. It is not proof that every lower shared provider is safe for arbitrary concurrent use.

## Source review findings and resolutions

The native scheduler no longer contains `source_gate`, a row detach-event handle, or calls through `api.create_event`/`api.set_event`. The compatibility API fields remain optional. The exact prepared handoff, token, unique receipt, captured completion port, and work obligation are published before zero-flags native launch. Stable list nodes keep child `Row*` valid during registry growth. Child entry takes the metadata gate only for capture/reservation updates, then performs actual acquire/process/return outside that gate. Parent's existing selected policy supplies its actual operation exclusion; no production caller is silently switched to Concurrent.

The original worker records known returned original handle/id and duplicate output before health postchecks. Its later launch completion preserves an already Returned/Consumed stage. Exact detach accepts a normal Launched or early Returned row, consumes only the original native handle, and records the successful close before admission checks can fail. No cancellation or speculative resource rollback was added.

A child marks Returned only after its full process/return prefix. Raw post may awaken the original receipt consumer before that API returns. The receipt path therefore does not require `notified` or absence of `executing`; it still requires one exact returned, detached, live-work row and delegates its one source completion before handing the single work decrement to the actual dispatcher. The post path changes only its separate posting/notified fields and never overwrites Consumed. Physical maintenance skips an executing/posting row, so this receipt schedule cannot reset a handoff still in use. Unknown post after a delivered receipt remains a retained poisoned physical prefix rather than a fabricated successful exit.

Physical maintenance has no live-sibling/FLS eligibility condition. It delegates original-worker reaping to the exact Parent, then reserves only an eligible consumed row using its stable generation. A zero-time handle poll and normal native exit, together with completed post and no retained exception, precede handle close and handoff retirement. Successful joined close clears the handle before any later seal can throw. Handoff destruction is outside metadata. Unknown effects retain the currently reserved row without retrying known-consumed effects.

Child failure uses a separate helper from original lifecycle failure: it retains the first exception by empty-slot swap and cannot clear the original worker's active reservation while that worker is still in start/detach/maintenance. An unused exception target is destroyed after releasing metadata. This avoids both lifecycle reentry and replacement exception destruction inside the gate.

Independent draft review found allocating exception construction under Native metadata locks. The source author resolved it before freeze with Native-only `unique_lock` and an unlock-before-construction rejection helper, including original affinity and locked Application identity checks. Scalar publication, list splice, and passive snapshot work remain under the metadata gate; source callbacks, SDK imports, node allocation, callable retirement, and handoff destruction remain outside it. No blocking finding remains from that draft issue.

## Integration boundaries

Parent busy-aware Serialized reaping is necessary after removing the scheduler-wide gate. The separately reviewed Root3 change validates original Borrow before returning healthy busy zero, rejects own actual/pending Operation, tries the existing Serialized gate without waiting, and retains Concurrent foreign-actor progress. It adds no all-thread-exit barrier.

Older actual NativeGraph post-failure fixtures still select default Serialized Parent and hold their real original Operation through start and detach. That selected policy orders their child body/post after normal source start/detach; it explains their deterministic 1/1 counter prefix without a NativeJobs event. Explicit Concurrent fixtures must separately cover early child execution and receipt-before-post-return; their authored implementation and execution status are reviewed separately.

The normal reconstructed route still processes child message work then performs its response/021770 continuation on the original worker. Full original028d30 child-tail ordering remains open, as do generic tracked Environment work, broader mutable provider/Online/token state, actual OwnedAccept concurrency, and runtime equivalence. The change closes a bounded scheduler source gap, not the entire native-message progress blocker or DLL reconstruction goal. No new native-function coverage or total completion percentage is claimed.
