# Independent static review: per-call Policy work release

Status: no blocking finding remains in the exact three-file change below. Review used source and saved REA/Ghidra evidence. No compiler, project test, link, reconstructed DLL execution, or ZIP operation was performed. The authored tests are component assertions, not execution results or a complete Environment concurrency proof.

## Frozen reviewed files

| File under outputs/ | SHA-256 |
| --- | --- |
| anadius-session-policy-source/policy.hpp | 178f03f81fd2f8b1a2a7133b416ae122d3de671283e53aaf20587829fb9e1527 |
| anadius-session-policy-source/policy.cpp | c0649435800d5297f664f03c5c99a7f2c07c7de1803525e9560494790a7ab777 |
| anadius-session-policy-source/policy_tests.cpp | 831ddf43348a478bf922e5e338a8c7f56ed253833b3e504c048c5fc119450b76 |

Source author report: `policy-work-implementation.md`, SHA-256 `3bd259723555d60d0e37ea256b7b24d5d3ac56684477d0e6584b4ba9c8b2e1d4`.

Test author report: `policy-work-test-implementation.md`, SHA-256 `b3d2ee778041c4f3763f3d612e09750337b3a3a2aefb7eabc846cb6af945c8e1`.

## Native constraints reviewed

Saved REA replies bind original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` and Ghidra 12.1.4:

- Complete `1800229a0..1800229c9`, 42 bytes, Evidence `ev_ba745032bc8bbc9c8e63affa14037cda6d0a2342f65052f4c22be7a6e4046dee`: load the underlying scheduler, lock xadd minus one at `1800229b5`, compare the old count with one at `1800229ba`, and synchronously call `18002a190` at `1800229bf` only for that value.
- Exact instruction `1800229b5`, bytes `f00fc14130`, Evidence `ev_59546899db41e6dca910681edb4cbfe96b78b48b7bddf4e77affbdd4e283eb13`, independently corroborates the selected counter instruction representation.
- Complete analyzed `18002a190..18002a1f6`, 103 bytes, Evidence `ev_f7b8c7e0fa1119d3457d42a801700ff189916e678cdcd3db644e8a7c42e95a79`: stop exchange `18002a19b`, conditional wake exchange `18002a1a2`, zero-argument post `18002a1b3`, cached failed-post status before category retrieval, then conditional `180028240` raise. Ghidra reports a terminal no-return call at `18002a1f2`; that provider flow limitation is not independent native exception proof.

The separately saved request for `180022940` resolved to constructor `1800228e0`. It is retained for provenance but is not work-release evidence. No fresh function-coverage credit is inferred from that misselection.

## Admission, effects, and retention

The singleton `environment_work_active` is removed. Exact thread/fiber WorkAdmission is linked before node allocation, so an allocator/new-handler callback cannot see an unreserved actor. WorkCall is a stable list node published before copying the opaque checkpoint target. The record owns the source lease, an optional copied child JobPermit, exact Parent and scheduler, actor identity, a private checkpoint, and its checking/entered/consumed/failed state. The durable errors backlink names Impl's owned NativeApi. Caller StopApi, errors argument, consumed bool, and stop Effects are only synchronous borrows on the executing stack; none is stored by the durable record for replay.

Each call validates its registered identity and actual Parent authority around source/checkpoint callbacks. An original release requires its bound actor and no own ordinary-call reservation. A child uses its explicit copied live permit, not an ambient current-job selection. Concurrent mode does not borrow another actor's work/cleanup activity as authority or exclusion: healthy sibling work and cleanup plus original ordinary calls can progress. Serialized retains its previous exclusion, including the existing child release case while Runtime's original active reservation is outside Parent at physical wait. No new all-sibling eligibility gate was introduced.

Pending and registered cleanup/work records jointly reject same-actor reentry. Independent draft review identified an ordering inconsistency: the Serialized cleanup preflight could reject for foreign busy work before reaching the same-actor sticky check. The final source checks exact actor reservation first, so a swallowed same-actor cross-registry attempt poisons in both policies while preserving the separate healthy foreign busy refusal. Original ordinary calls likewise reject an actor already reserved by scoped work. Callback-bearing error construction in changed validation paths occurs after unlocking metadata.

The actual scheduler drop remains seq_cst fetch_sub once. Caller consumed and the record's durable atomic consumed are published immediately after that effect, before any callback or old-count-one stop invocation. Existing `stop_worker_02a190` remains synchronous and preserves stop/wake ordering, zero post, failed status before category, and conditional raise. There is no re-add, guessed cancellation, zero/saturation guard, deferred completion receipt, or alternate stop channel. The existing atomic high34 and shared Policy-gated low32 wake effects are unchanged.

On success, the copied checkpoint target is destroyed outside metadata while its exact WorkCall is still registered. Source and actual permit are revalidated afterward without invoking the retired target; only then is the exact node unlinked and its lease/node destroyed outside metadata. A target constructor, invocation, destructor, native post, permission, or later validation failure preserves the actual admitted/consumed prefix and shared sticky health. Child failure does not clear an overlapping original ordinary-call reservation. Unknown records retain owned authority rather than dangling stack adapters; the caller must continue retaining the poisoned Policy graph under its existing unknown-owner contract.

Snapshot.active now aggregates pending, live, and retained failed scoped records. It is observation, not admission authority. Close and external quiescence return false while either scoped registry has membership, including poisoned failed records, before lifecycle closing or opaque proof callbacks. Final membership checks and closing publication share metadata, so a concurrent admitted record cannot be missed during retirement. Metadata is not held across target copying/destruction, callbacks, SDK imports, node allocation, or lease retirement.

## Test-source review

The existing original-actor component matrix still covers nonfinal work, old-count-one stop/wake order, preserved upper wake bits, suppression by prior stop/wake, zero packet, false status zero, and error-read counts. It uses controlled leaf counters and restores its fixture baseline; it is not presented as real Runtime accounting.

The new copied-target observer distinguishes the caller's original target from the admitted std::function copy. Normal admitted target destruction observes Bound/active and healthy close=false while the call remains registered; final success becomes inactive, with exactly one copied-target retirement and preserved work/flags/post/error/LastError. Hooks are cleared before their local captures leave scope.

The `workretire` isolated branch swallows an ordinary same-actor reentry rejection from a noexcept target destructor. Final validation still fails, preserving one completed nonfinal decrement without stop/post, retaining failed aggregate activity, and blocking close/external proof. The `workcopy` isolated branch throws during opaque target copy after observing the published call; it retains pre-decrement membership without an invocation, drop, post, or invented completed-target destruction. A failed constructor's unconstructed target is not counted as a target retirement.

Existing admitted work error/unknown-post/pre-drop-checkpoint failures now expect retained aggregate activity and false close/external proof. A fresh caller consumed=false cannot replay the poisoned operation or alter its exact diagnostic, checkpoint, counter, flag, post or error prefix. Foreign scheduler preflight remains Bound/inactive with no admission/effect. Thirty-one terminal branches are declared by the source driver; none was run in this stage. Terminal fixtures intentionally retain unknown owners and exit through their existing isolated-process model rather than destroying or guessing cleanup for poisoned graphs.

## Remaining limits

This is a Policy prerequisite. OwnedEnvironment's separate scope, executor-family/vector growth, per-row release exclusions, source lineage, and original-affine raw callbacks remain open. Explicit CallContext adapters, actual tracked-family sibling overlap, the real Runtime old-count-one path under that composition, complete native child response/021770 ordering, broader mutable providers, native EH/ABI behavior, compilation and runtime equivalence are not proved by these leaf tests or this static review. Production Parent remains Serialized. The broader native-message blocker stays open; no overall completion percentage is claimed.
