# Native mutex owner projection — read-only concurrency audit

No production source, frozen report/proof, policy, native session, build, test or ZIP was edited/run. This is a new unreferenced prerequisite note while Graph's production actor policy remains Serialized.

## Concrete source race

`execution.cpp::lock_04a6ac` reads plain `MutexState.owner_48` BEFORE `Platform::acquire_srw_exclusive` at lines 55/62. A different owning actor writes that same plain field at 57/67 or clears it at 78. Under actual concurrent achievement callers, the SRW lock does not protect the waiting actor's pre-lock read: the read itself can overlap the current owner's store. This is a source data-race candidate even though the subsequent actual SRW acquisition correctly serializes the body.

The pre-lock check is part of selected recursion behavior and must remain before the actual native SRW call. It cannot be replaced by unconditional AcquireSRWLockExclusive, a new body mutex, waiting on a separate source gate or a deferred owner commit. Type 1 and recursion-flag paths, error 3 count rollback, unsigned count wrapping, error 5/6 and existing unknown SDK prefixes must remain as selected.

## Minimal projection

Keep the raw `uint32_t owner_48` field and actual layout. Use short-lived lock-free `std::atomic_ref<uint32_t>` relaxed load/store projections for the two live pre-lock reads and ALL live owner writes in lock/unlock. For example, an internal helper can return `std::atomic_ref<std::uint32_t>(mutex.owner_48)`; each use performs a relaxed load or store. Include `<atomic>` and assert required alignment and target lock-free support; do not silently substitute a library body lock on an unsupported target.

The unlock order remains count decrement → owner store of 0xffffffff → actual ReleaseSRWLockExclusive. Successful acquisition publishes the same actual thread id at the same point as current source. Relaxed owner operations convey only the ownership/recursion projection; actual Acquire/ReleaseSRW remains the synchronization for protected application data. Same-thread recursion retains its already-held native lock. No CAS, retry/wait or new ownership protocol is needed for this candidate.

Do not mix atomic pre-lock readers with plain owner writes during live use, even when those writes occur under SRW: waiting readers are outside that lock. Plain initialization, pre-publication binding validation, isolated test seeds and post-join observations remain quiescent operations. Borrowers of the public raw MutexState must honor that lifetime rule; adding an atomic_ref helper does not make arbitrary raw field access safe.

## Count and reference escapes

Current `count_4c` accesses in lock occur only AFTER actual SRW acquisition, or on the same actor's recursion while it already owns SRW. Unlock and Execution's overflow guard access it before actual release. Graph's achievement `count_4c()` returns the corresponding signed `int32_t&` to the SAME raw unsigned cell only after its own achievement lock succeeds; content_handlers.cpp 11–16 performs its max check/correction under that lock. Graph unlock's count==1 check is also under ownership. These selected accesses do not establish a second pre-lock count race.

Consequently owner-only atomic_ref is smaller and preserves the existing borrowed-reference interface. Converting count to an atomic wrapper would break or complicate that raw reference escape; introducing atomic_ref count access while retaining live plain-reference users would require a separate complete access audit. Do not broaden the change without an actual out-of-lock count consumer. Snapshot/inspection of a live count by a future foreign actor would need an explicit compatible projection or the actual lock.

`type` is fixed before publication; storage_08 is initialized once. `srw_10` remains the same address-bearing raw uintptr_t passed to the platform's real SRW API; it is not an owner/count substitute and must not be atomically rewritten by this fix.

## Actual Runtime and other users

`execution.hpp` registers MutexState layout explicitly: size 0x50, SRW at 0x10, owner at 0x48, count at 0x4c, alignas(8). An owner-only atomic_ref projection preserves these field types, aggregate/copy behavior and signed count alias. This source layout contract is not complete original Runtime/MSVC ABI equivalence.

Backend RuntimeSeed contains the execution mutex (`backend.hpp:14`); SourceBackend::Impl separately owns the achievement mutex (`backend.cpp:111`), initializes it with type 2 at 132, and binds its exact address/platform to Worker at 252. Worker forwards the actual mutex to Graph through MutexBinding; Worker/Graph idle owner/count reads occur before publication and require initialized idle state. The selected source has no `NativeRuntimePrefix` object casting these cells; the +428/native comments describe this semantic backing, not evidence that the whole host Runtime is a native byte layout.

The Execution wrapper already has one SourceGuard around its own phase/ready operations and documents exclusive wrapper access to its refs. That existing guard masks this pre-read issue for callers restricted to the wrapper; it is not authority to put another body gate around direct shared-achievement lock calls.

services_completion::WaitTarget owns a DIFFERENT private MutexState and native SRW. It writes owner/count only after acquiring its own actual lock, checks a separate atomic owner_/registration_ before admission, and calls only the recovered unlock helper via a release-only platform. No selected pre-lock `mutex_.owner_48` reader exists there; its live plain assignment does not establish this same race. Migrating that assignment for uniform style is optional scope, not a demonstrated prerequisite. Its existing count checks are owning/registered-thread checks. No exit-wait framework change is needed.

## Validation paths

Preserve execution_tests.cpp mutex_semantics (type2 relock error3; type1/0x102 recursion; count/overflow/error prefixes) and startup-publication tests. They use controlled single-actor platforms and do not prove physical contention. Graph `shared_achievement_mutex()` at graph_tests.cpp 195 exercises the exact Runtime-style root/SRW and actual QueryAchievements, reserved bytes, original binding and normal retirement. The new prepared-native-call-views mode exercises real child achievement lock/write/event routing sequentially; neither is a concurrent mutex proof.

For actual contention later, reuse existing real NativeThreads/PreparedMemory actor helpers with one source MutexState and an actual Win32 SRW-forwarding platform: A owns the real lock while B reaches the real acquire callback, B cannot enter until A releases, then B observes its own owner/count and both leave the same raw root idle. Keep own callback counters atomic or ordered by events and release all waits on terminal failure. This adds no production body lock/policy enable. Existing graph_tests AchievementPlatform assumes owner==-1/count==0 BEFORE acquire; it is intentionally single-actor and must not be reused unchanged for contention (its pre-acquire plain count inspection would be unsafe and its idle assumption is false while another actor owns SRW).

Observed frozen sources: execution.hpp `f044c252ab791b02d7b00fadf5ac4c484dac2804ec6b64c2e543cbc72e371fcc`; execution.cpp `03946d42e546072f0e875c4a45327b63a24419303e9b4b23325f79b55f75b705`; backend.cpp `d327f045085f20a968ea1781fd38a6c233a3ff970f9c8a9d208661b4d16ed019`; exit_wait.cpp `814af5ec8e30a72a9e76dedfb0ab5a1adb950df735efe7a89f865fa426524eaf`. Graph line references above refer to inspected source/before-stage projection while its separate author works. No new native evidence or executed race/test result is claimed.
