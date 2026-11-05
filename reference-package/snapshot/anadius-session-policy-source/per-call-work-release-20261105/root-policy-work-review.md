# Root independent Policy work-release review

Frozen exact sources reviewed:

* `outputs/anadius-session-policy-source/policy.hpp`: `178f03f81fd2f8b1a2a7133b416ae122d3de671283e53aaf20587829fb9e1527`
* `outputs/anadius-session-policy-source/policy.cpp`: `c0649435800d5297f664f03c5c99a7f2c07c7de1803525e9560494790a7ab777`
* `outputs/anadius-session-policy-source/policy_tests.cpp`: `831ddf43348a478bf922e5e338a8c7f56ed253833b3e504c048c5fc119450b76`

No blocking finding after reading the actual WorkCall/pending admission, ordinary/cleanup membership and lifecycle changes, exact synchronous effect order, owned callback copy/retirement and corresponding component assertions. This is static source review only; no compiler, project tests, process scenarios, linker or DLL was run.

The stack admission precedes allocation and the durable call is spliced before copying the caller's opaque checkpoint. Exact Source/Parent/scheduler/native-provider identity, original actor or copied child JobPermit and published membership validate authority. The durable errors pointer is Impl-owned NativeApi identity; the caller StopApi/errors arguments/external consumed reference stay synchronous stack borrows. The retained call has no method that replays the old Effects or uses an expired caller stack API. Source lease and copied permit remain owned for an unknown failure.

The final source preserves the measured seq_cst unconditional work drop: external consumed and durable atomic consumed publish immediately afterward, then only old1 invokes the existing synchronous stop algorithm. Shared high34 uses atomic_ref; shared low32 wake updates use the same Policy gate as ordinary exchange_wake and preserve the high32 bits. Callback-capable validation, post/error/category/raise and callable destruction are outside metadata. Native02a190's false-post error latch/category/conditional raise order remains in its existing implementation.

Concurrent mode permits foreign WorkCall/CleanupCall and original ordinary progress while exact same-actor pending/registered calls reject across both registries. Serialized retains its previous selected exclusions and the prior explicit child work allowance while the Runtime original reservation may be outside Parent. A child WorkCall failure does not clear original v.active. Snapshot.active truthfully includes retained failed work; it is not a permission count.

Copied target destruction occurs while the exact record is still registered. Final validation follows destruction, and only then are membership, lease and node retired outside metadata. Both early close/external false returns and final closing/attested checks account for cleanup AND work membership. Failed work is never treated as empty/healthy drain or decremented again. The independent reviewer also identified the Serial cleanup busy-preflight order: the final source tests same-actor membership first so swallowing a cross-registry reentry cannot evade sticky failure.

Tests retain the existing exact leaf counter/stop/wake/error/category checks and clearly label the synthetic component fixture. Added success copied-target retirement checks active/Bound/closefalse through destruction and normal inactivity only after unpublication. Pre-drop checkpoint-copy failure, swallowed target-destructor reentry, pre-drop admission failure and post-consumed post/error failures retain exact prefixes and reject a retry even with a fresh consumed flag. Foreign scheduler preflight remains Bound/inactive and causes no checkpoint or decrement. No assertion was executed and synthetic initialized/restored counters are not actual Runtime/Environment pin ownership evidence.

Fresh native claims use complete0229a0, complete02a190 and exact229b5, with provider terminal-call limitations preserved. The unrelated constructor returned by exploratory022940 is separately retained, not release evidence. Production still selects Serialized. Actual two-scope OwnedEnvironment authority, per-row tracked family effects, naturally established child last-work stop, full028d30 child response tail and shared/accept/packet integration remain open.
