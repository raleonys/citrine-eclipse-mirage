# Policy per-call tracked work release implementation

Status: production source2 frozen for independent static review. No compiler, project tests, linker, reconstructed-DLL execution or ZIP update was run. This is a bounded source prerequisite; it does not establish a complete concurrent OwnedEnvironment path or runtime equivalence.

## Frozen source

- `outputs/anadius-session-policy-source/policy.hpp` SHA-256 `178f03f81fd2f8b1a2a7133b416ae122d3de671283e53aaf20587829fb9e1527`.
- `outputs/anadius-session-policy-source/policy.cpp` SHA-256 `c0649435800d5297f664f03c5c99a7f2c07c7de1803525e9560494790a7ab777`.

Public finish_environment_work signature is unchanged. Only Policy2 production files were modified by this author. OwnedEnvironment, Runtime, Profile and production Parent policy are unchanged; Serialized remains the production default.

## Native constraints

Original binary SHA-256 is `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`; the complete saved REA replies identify Ghidra12.1.4 and retain the following evidence:

- `instructions-1800229a0.json`: `ev_ba745032bc8bbc9c8e63affa14037cda6d0a2342f65052f4c22be7a6e4046dee`.
- `instructions-18002a190.json`: `ev_f7b8c7e0fa1119d3457d42a801700ff189916e678cdcd3db644e8a7c42e95a79`.
- `instruction-1800229b5.json`: `ev_59546899db41e6dca910681edb4cbfe96b78b48b7bddf4e77affbdd4e283eb13`.

Complete native0229a0 body42bytes performs XADD.LOCK -1 at1800229b5, compares oldcount1 at1800229ba and calls02a190 synchronously at1800229bf only for that old value. Native02a190 body103bytes preserves stop exchange, conditional wake exchange, zero IOCP post, cached failed-post GetLastError before category retrieval, and conditional raise. The selected229b5 instruction corroborates the exact counter effect. The separate misselected022940 request resolving to0228e0 is construction evidence only and is not used as release proof. No new native-function coverage percentage is claimed.

## Implemented source contract

- Removed singleton environment_work_active. Before any node/target allocation, a nonallocating stack WorkAdmission publishes exact thread/fiber membership. A stable list-backed WorkCall is then allocated outside metadata and spliced before the caller checkpoint is copied. Reentrant new_handler/target copy cannot observe an unreserved actor.
- Each WorkCall owns source lease backing, an optional COPY of the exact child JobPermit, exact Parent/scheduler, the owned NativeApi identity, actor thread/fiber, private checkpoint, checking, entered, durable consumed and failed state. Source authority is the exact registered call plus actual Parent permit/original actor, never an aggregate count or ambient current-job lookup.
- SDK WorkerStopApi, errors argument, external consumed bool, and Effects remain synchronous stack borrows. No durable record stores these caller-stack references for replay. The durable errors backlink names the Impl-owned native provider. Unknown effects preserve registered failure/prefix state and shared sticky Policy health without clearing an overlapping original ordinary-call reservation.
- The actual native work counter is decremented exactly once with seq_cst fetch_sub. External consumed=true and the durable call's atomic consumed=true are set immediately, before any later callback or old1 synchronous02a190 invocation. There is no zero/saturation guard, re-add, deferred Root receipt, speculative retry, or replacement stop channel.
- The existing same shared scheduler high34 atomic_ref and low32 wake exchange under the same Policy metadata gate are preserved. Callback-bearing admission, post/error provider, system-category formatting and raise remain outside metadata and on the original ordered02a190 path.
- Concurrent Policy admits healthy foreign release/cleanup and original ordinary calls independently. Pending and registered cleanup/work registries jointly reject same-actor reentry, including callable construction/retirement. Serialized preserves prior exclusion and the existing explicit child exception when Runtime's original active reservation is outside Parent at physical wait; original work release still rejects its own active ordinary call.
- Normal release destroys its copied checkpoint target outside metadata while the exact call remains published, validates actual source/permit afterward without invoking the retired target, then unlinks its node and retires lease/node backing outside metadata. Unknown copied/invoked/retired-target outcomes remain registered and failed. Snapshot.active includes pending/live/failed scoped records and can remain true after failure; it is observation, not authority.
- Whole close/external quiescence return false while scoped pending/registered/failed records exist, before entering Closing or calling an opaque drain proof. Final close and attested-return membership checks cover BOTH registries, preventing a foreign call published during a callback from being missed. Final closing publication and absence checks share the metadata gate.
- New/changed validation paths unlock before exception construction. Record/list/callable allocation, callbacks, target destruction, SDK effects, lease and node retirement occur outside metadata. Short list splices, scalar flags, atomic prefix observation and shared wake-word publication remain inside metadata as appropriate.

## Static review and remaining work

Reviewed actor admission before allocation, registration before target copy, copied-permit lifetime, unknown prefix retention, immediate consumed-before-stop ordering, same-actor cross-registry checks, independent foreign Concurrent calls, existing Serialized child/root wait contract, target-destructor reentry, and both early/final lifecycle membership checks. Tests are independently authored in policy_tests.cpp and are NOT executed by this stage.

OwnedEnvironment still has its separate singleton scope, vector-growth/release exclusions and original-affine raw callbacks. This change does not select concurrent production scheduling or claim a full two-scope tracked-family/runtime fixture. The next actual Environment stage still needs explicit CallContext-bound adapters including type comparison and real move023cb0 ordering, per-row claims, protected-source lineage, and real graph/lease retirement proof. Full child response/021770 ordering, handler/response scheduling authority, arbitrary shared lower mutation and native EH/ABI equivalence remain open.
