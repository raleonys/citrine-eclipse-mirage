# Original Online startup/query test source

Edited only `anadius-online-query-source/query_tests.cpp`, `anadius-online-factory-source/factory_tests.cpp` and this note. **Authored/static source validation; no compilation or execution.** No project tests, child process, network/game/SDK/DLL execution, build/link or ZIP was run. Production's five files belong to the separate implementation agent. The fresh12-record native bundle, native-findings, source-hash snapshot and closed-session/original-identity proofs remain frozen; they are not test PASS evidence.

## Binding query admission/ownership

The existing Fixture now accepts the public immutable QueryPolicy while defaulting to Serialized. Its old overlap test still expects six rejections and retains its original meaning. Shared observation fields (`calls`, `incoming`, `args`, `claim`) are updated under the existing fixture mutex. Only Username10 blocks, and that mutex is released during a bounded condition-variable wait, so new Concurrent siblings can enter safely. Incoming LastError is captured before test metadata and outgoing fake SDK errors retain their existing values.

`concurrent_original_queries()` selects Concurrent and holds a real typed Username10 body. A second Persona18 owned copy completes, followed by a third status query. After each sibling, Snapshot.active must still account for the first query. An otherwise-valid exact destroy remains rejected, as do module free and lower SDK retirement. The existing factory-code lease remains retained. Only after releasing and joining the first query may explicit destroy/free succeed. The username and persona result strings remain unchanged after their respective provider buffers are mutated; no extra public counter or mutation hook is used. Main-body exceptions are captured so release/join occurs before propagation.

New own-child `--concurrent-null-slot` (expected193) starts that same first held query before zeroing only the sibling Persona18 slot in its own allocated page. The failed sibling must poison/mark unknown but leave Snapshot.active true for the first query and retain code/object without destroy. The page is then made unreadable; another query must reject before rereading its vptr. The first callback already resolved its function and returns a State-owned string, so it can safely finish without touching the protected page. After release/join, active must become false while poison/unknown/code remain and no cleanup is fabricated. Terminal193 is emitted only for that exact predicate. Existing invalid-layout children191/192 are preserved, and all three are registered in main.

## Actual OriginalCrt Online startup window

The factory regression uses the existing concrete Online fixture and actual `worker_035b60`. Options select OriginalCrt with the fixture's typed own-executable code/object leases; Cooperative fixtures stay unchanged. The first username getter signals a new mutex/CV only after the actual startup has authenticated, adopted/published the object and set online1. It waits before returning its string, so the original username/persona/userId fields have not yet been assigned and OnlineReady remains0. ServerReady is the fixture's existing1 input, not a flag modified to bypass startup.

During this synchronized pre-write window, the main thread calls the actual Online.avatar28 route with a valid cache path, owner `remote` and size2, followed by an owned UserId query. It checks the facade stays Open, the first query remains active after siblings end, both existing code leases remain live and controller close stays nonfinal without a foreign proof callback. The old unconditional active-query rejection would fail/poison this sequence before invoking the avatar target.

The worker is always released and joined before final assertions. A failed main/worker outcome exits after joining with the retained facade prefix instead of inventing cleanup of an unknown state. On the successful authored path, profile strings and OnlineReady complete normally, all reservations drain and exactly six typed queries occurred: startup status/username, sibling avatar/UserId, startup persona/UserId. Actual source cleanup then destroys the object and retires its two leases once under OriginalCrt.

Trace query counters/validity are mutex-protected. Startup lifecycle/export/trace writes occur before the username signal; profile writes occur only after release. Sibling avatar argument fields are written/read on the observing thread. The test does not introduce races in the shared trace or depend on unsynchronized polling of the plain online byte. It does not claim to solve every arbitrary shared-Configuration publication or foreign-buffer mutation problem.

## Ingress/native scope

The factory test invokes typed avatar28 directly, corresponding to the measured eligible QueryImage branch; it does not add a full LSX/client/network harness. Native/reachability reports supply that caller mapping. Ordinary recognized hooked3216/nonweb connects wait(-1) and may mask this startup window. The bounded source correction addresses other eligible ingress, such as a direct/unhooked listener client, without replacing native caller behavior with a new all-ready barrier. External SDK internal synchronization, full C++ ABI/RTTI/EH and real game/runtime behavior remain unverified. Controlled own-executable functions are not the external SDK implementation.

## Hashes and independent review

| File | SHA256 |
| --- | --- |
| before query_tests.cpp | `da02c93d51c2489f449b53683f1b0e03bb8cf5f8ec9c8122a7f76fc4a499f47b` |
| before online-factory/factory_tests.cpp | `3dbfd475501364f45d07e48a0118e05de651342b3f361f6712f727e6e5fc9c00` |
| before pending-source-status.json (64 sources) | `8aec2055b201801325c70e895ddd727fa646d89837b4e0f33b143c9fc82fdc55` |
| final query_tests.cpp | `84829906b463af588f95c3490e0e4be268ab6b0f827e7502cc4be844d69d9a0a` |
| final online-factory/factory_tests.cpp | `da2a4ef1e31161c96de5eec311f8ae3e005c80d9a22e6d83a627720503bfd44a` |

Static diff/whitespace checks found no defect. Independent exact-byte review found no blocking issue; `tests-independent-review.md` SHA256 `25085211a1e814bb77cce8c3ed3d1612b6378cca4b32f37fa1148105deb6a67b` records ownership, synchronization and coverage limits. Both test files are frozen. No runtime PASS or whole-DLL equivalence is claimed; the user's explicit rebuild/test instruction is required before execution validation.
