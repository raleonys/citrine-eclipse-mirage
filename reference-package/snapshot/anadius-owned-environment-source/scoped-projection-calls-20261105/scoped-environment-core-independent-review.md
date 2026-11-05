# Independent static review: scoped Environment core and component tests

Status: no blocking finding remains in the bounded frozen change below. This is source inspection only. No compiler, project test, linker, reconstructed DLL execution, or ZIP operation was used. Actual two-Scope Runtime overlap and child old-count-one integration remain open requirements, not results of this review.

## Exact reviewed source

| File under outputs/ | SHA-256 |
| --- | --- |
| anadius-owned-environment-source/environment.hpp | a8d070d3b8dd5f0ed500fdc70984778136403b33748c21cb21dc3d0bec58416b |
| anadius-owned-environment-source/environment.cpp | d081adfdee04dec54352b4f3b0b702baa39e4a7958d7ebd31d61bffb8a25e448 |
| anadius-owned-environment-source/environment_tests.cpp | 02b5350e6359e276391e76a895f52cac9fe47eaffcaa270a53c9452c92e3bc7f |

Authoritative source report: `scoped-environment-implementation.md`, SHA-256 `b3c400181d6346d8ced9608cacd067f51444cd4acbe2b89b61c244239802efae`.

Authoritative test report: `scoped-environment-test-implementation-v2.md`, SHA-256 `22d23fad0702ed7deb944bc6695de2a3b17c87a1eb2e63b678d7dad9e1df96b3`. The earlier test report remains historical and does not identify the final raw-copy regression source.

The separate seven-file socket/Profile seam review is `root-seam-independent-review.md`, SHA-256 `0ae105402d66d0ce93b43dd928957580c773a567199e07ddc0146be738f4513c`. Those files have their own frozen hashes and are not counted again as this core author's three files.

## Native constraints

Saved complete REA/Ghidra 12.1.4 replies bind original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`:

- `180023cb0..180023d19`, 106 bytes, Evidence `ev_cd85a47c1ecc95b2c78a406e8478fe137fdc65a60d22d94b04b2f4dfa8d0ec65`: copy ops/schedule, reset source ops/schedule, invoke slot at `180023cee`, then reset source state at `180023cf1`, reload the LIVE source executor at `180023cf9`, copy it and finally store its source sentinel at `180023d08`.
- `180023dd0..180023f2f`, 352 bytes, Evidence `ev_a796bdbc87106dd39b97f8933a91e478c3113ea2ac400041f0e295bcba20e562`: raw type query before compare; empty branch avoids a factory; nonempty branch caches factory58 at `180023e6b` before query18 at `180023e7a`, invokes that function at `180023e87`, performs its first inlined move/guarded cleanup, then helper move at `180023efa` and unconditional intermediate destroy at `180023f0b`.

These constrain the source field and callback order. They do not prove native C++ ABI/EH equivalence, execution overlap, or new recovered-function coverage.

## Core review

Stable list rows replace relocating vector publication. Pending effect/scope admissions are nonallocating and precede node/target allocations. Registered effects retain exact address claims and durable failure/release state. The fixed eight-address budget belongs to one measured constructor algorithm, which uses source, result and at most two temporaries; it does not limit rows, peers or scopes. There is no ambient weak current-job, actor/TLS selector, global growth/release body gate, or active==depth authority.

A public JobScope pins Control. Its registered JobState has a raw Control backlink, copied actual Parent permit, provider lease, exact protected source capability/origin, owned checkpoint, and immutable CallContext with a raw thunk into that same stable state. Control owns registered states/effects, which do not own Control back through a public Scope. The canonical Profile child checkpoint was also inspected: it captures JobHandoffState, permit and epoch; its DispatchState holds a raw Application tombstone target rather than a shared Profile/Environment owner. No structural healthy ownership cycle was introduced by that path. Arbitrary caller callback contents still have their ordinary ownership obligations.

Actual bad/expired/foreign permits and mismatched SessionView are checked before scope admission publication, followed by source/capability revalidation before linking. Known wrong actors, other-scope lineage, protected-source writes, scoped-empty ownership and foreign busy claims reject before a checkpoint/effect/native write without poisoning healthy siblings. Same-actor checkpoint/admission reentry remains sticky; genuine registered field corruption and admitted unknown effects retain shared sticky health. The final source separates these cases rather than treating every refused caller as an unknown native result.

Concurrent scopes can share a Root source for reading. Their nonempty temporaries carry exact scope epochs; exclusive address claims cover mutation and helper prefix/tail windows. Factory entry by capability resolves and preclaims the exact source plus destination before effect publication and checkpoint invocation. Owning Environment.release checks scoped rows, empty ownership, protected source and in-flight address claims before reading native fields. Raw exported nonempty callbacks remain original-worker operations; explicit scoped adapters carry child authority.

Whole Scope construction prevalidates the selected canonical type/context/factory bindings, then claims full source and destination before the raw type query. It makes a private constructor-context copy and uses a local EffectOperations adapter bound to the exact outer registered effect. Internal factory, query, move and destroy calls reuse that explicit effect instead of reentering a public facade or inferring authority from the actor. The Initial empty-type path does not read or require an unused factory binding. After the constructor returns, ended internal stack temporaries are checked only through address-ownership metadata before any further payload validation.

Owning Root and scoped moves use the same measured helper with a private admitted slot bridge. Claims precede the first header write and remain through state reset and live executor tail. Move destination's prior payload, state and executor are not interpreted. Registered source fields are checked as full before the helper and as the known partial prefix only at the slot. Scoped copy owns its complete prefix and final live executor load.

Raw Root slot08 retains its existing narrower ABI: payload/state are written in the callback, while the actual caller reloads the executor after it returns. The final code neither writes that executor early nor validates it as already defined at a post-copy checkpoint. Its exact copy-output prefix exception does not suppress full checks on the source. Direct legacy slot10 remains a partial callback boundary too. These trusted raw caller windows are not represented as whole Root copy concurrency proofs; an owning caller guard/commit seam and arbitrary deallocator reentry into an unfinished raw destination remain separate concerns.

Sentinel-empty Scope results retain only an address in that Scope's metadata, without a fake IoEnvironment row, token, work retain or reference count. Scoped destroy removes the address; empty move transfers it; close requires it drained. Explicit owning APIs reject sibling results. The global raw Empty table stays a trusted stateless low-level projection and grants no Scope permission. This is a C++ projection contract, not a memory sandbox against callers directly mutating another actor's raw storage.

The shared family/flags/work accounting remains one source registry and scheduler. Tracked publication retains once; move transfers; tracked destroy marks only its row ReleaseEntered and stores the expected snapshot and consumed flag in its registered Effect. The Policy proof owns no caller-stack expected/depth/selected/consumed reference: it captures a raw pointer to that stable retained Effect. The existing EnvironmentWorkStop/Policy path retains its synchronous decrement and old-count-one stop behavior. Unknown outcomes preserve the entered/consumed prefix without replay, re-add or speculative cleanup.

Node allocation and callbacks occur outside metadata. Exact source fields, Parent control ownership, strong count and embedded owner/control identity are rechecked before publishing a new row after allocation callbacks. A prepared row node is committed even if a sibling freed another row meanwhile, avoiding an unused-node deallocator in a copy's published-payload/executor-tail window. Global query_owner scans metadata with separate registry and Control lock lifetimes; it invokes no callbacks and drops no final shared target under those locks.

Normal Scope close retains its membership and protected source through checkpoint-target destruction, then checks actual source graph, Context/permit and origin without invoking the retired target. Only then are state, lease and permit unlinked/retired outside metadata. A failed destructor attempt retains failed state instead of terminating before Profile can handle the unknown outcome. Unknown records block later native field access and teardown. Whole Environment close treats healthy foreign scoped work as retained ownership rather than a new failure; own lifecycle reentry remains sticky.

Original handler/response scheduling retains its explicit ownership handoff: family/flags are captured and the row-use claim ends before a callback may consume/delete the task and release the environment. No task/environment is reread afterward. This preserves the existing self-deleting task contract and does not authorize child scheduling.

## Findings resolved during review

The source author corrected the independently raised preflight poison distinction, source-capability claim before factory checkpoints, late canonical-context checks, reference-stripping generic return, final source validation after Scope target retirement, wrong-permit admission ordering, first native-field read in owning release, per-Scope empty-result ownership, unused prepared-node retirement window, and public scoped FullOrEmpty/sentinel preflight. Root separately identified the raw slot08 early executor write; the final code and new component test preserve the actual caller-owned tail. These draft issues are resolved in the reviewed hashes above.

## Component test review and limits

The valid Root whole-move test observes full source and untouched destination on the first checkpoint, including deliberately undefined destination payload/state/executor values. It requires one effect, exact capability transfer, native moved-from sentinels, unchanged retain/accounting except one move, preserved LastError and normal cleanup. The move-probe terminal swallows same-actor query reentry, then requires the outer call to fail before any prefix and retain one failed effect with no replay.

The new raw-copy test calls the actual existing copy helper. After payload/copy-count publication, every checkpoint must still observe the destination's placeholder executor. Only the real caller's after-return load installs the source executor; exact source fields, independent copy capability and counts are checked afterward. It deliberately does not query an incomplete destination.

Existing unknown admission, tracked pre/post-drop and handler failures now truthfully report one retained failed effect instead of transient zero activity. Genuine corrupted-source preflight remains distinct with no admitted effect. The existing 4100-projection growth, family transformations, OriginalDefault paths and self-deleting task source are retained. These are controlled sole-worker FLS/WorkStop component assertions; no child permit or concurrent Runtime success is fabricated. None was executed here.

Meaningful actual PreparedMemory/NativeThreads two-Scope tracked-family overlap, B native/FLS/Parent retirement while A remains held after a real decrement, and naturally established child old-count-one release after original Runtime return remain OPEN. Child prepare40/response factory/scheduling authority, whole raw Root copy caller protection, original full 028d30 response/021770 child-tail ordering, broader shared providers, compilation, native exception behavior and runtime equivalence also remain open. Production remains Serialized. No overall completion percentage or DLL completeness claim is made.
