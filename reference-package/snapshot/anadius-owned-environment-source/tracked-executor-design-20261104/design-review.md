# Tracked executor integration assessment

Independent bounded design/source/native assessment, 2026-11-04. No production/test edits, new REA queries or sessions, compilation, tests, linking, DLL execution, or packaging.

## Decision

Defer the tracked-family implementation until a concrete selected caller supplies or creates a tracked environment. Keep it explicitly SOURCE-unmapped. There is enough saved evidence to specify its normal operations, but adding them to the current default provider would chiefly add an externally callable extension and a new cross-thread stop/error interface. It would not repair an observed remaining branch in the selected OriginalDefault request/response flow.

This is not a claim that these native bodies are unimportant, already implemented, or globally unreachable in the original DLL. The known selected flow below does not create a tracked input. A future newly selected factory58 or tracked-source caller is the precise trigger to revisit this decision.

## Reachability through actual selected callers

- The initial accepted-socket family comes from native0211b0/023000/023d20: operations084928, schedule084908, executor084860, flags0. The fresh 20-record evidence and exact EIDs are retained in `../native-environment-targets-20261104/native-findings.md`.
- OriginalDefault now makes the existing023dd0 equality branch produce a sentinel/state-zero IO environment. That branch never queries source operations and never calls factory58. Consequently selected send/receive completion uses its direct handler body; the nonempty receive prepare40 path is bypassed. See `anadius-socket-source/environment_constructor.cpp:36-62`, `io_completion.cpp:90-144` and the current classifier stage review.
- Native initial copy0228c0 does not acquire a work retain. Source message_dispatch028d30 copies the real root twice but does not thereby produce a tracked family.
- Fresh response audit shows initial factory20=023400 and factory88=0234c0 preserve the initial family through020c80, transforming flags `(0|1)&~2` to1. They do not call factory58, emit tracked tables, or add a tracked environment work pin. The selected force-post1 adapter therefore remains matched. See `../response-executor-native-20261104/native-findings.md`, especially `ev_ff9feb0b13a5de34c4e5512a0b292c6ce9e2d675c177a7c349f45cb133bf73b4` and `ev_4542e19dbc5aa876c5f3ec9316e7ab904c9363a20ddafccedf029f5894b106b0`.
- Actual Profile constructor source bindings resolve owner+0x78 to initial Slot roots. Temporary HOST response/factory rows are not inserted into that source registry. Prepared native Jobs returns its response to the original worker before the response path; the existing clone helper now accepts the empty constructor result.
- Arbitrary callers can obtain descriptors from `OwnedEnvironment::configuration()` and explicitly invoke its HOST factory. That public projection is not a proof that the production path selected a native tracked factory. The contract explicitly bounds OriginalDefault to the initial-source classifier and keeps native tracked families separate.

## What remains genuinely unmapped

The current HOST `Row` (`environment.cpp:57`) tracks address/cap/state/origin/job epoch, not native family or DWORD flags. Its refs are ledger references, not scheduler work pins. Current prepare/factory/response20/response88 all use one `construct()` implementation (`:220-224`), copy uses make (`:225-228`), move transfers a row address, and destruction removes a HOST row (`:204-210`). Those callbacks are therefore not a reconstruction of arbitrary tracked native input behavior.

Saved native requirements, if a selected caller is introduced:

| Operation | Initial family | Tracked family |
|---|---|---|
| copy |0228c0 copies context/flags/self; no work retain |0228a0 copies context/flags, retains work for nonnull context, sets self |
| move |0228c0 leaf copy, with source-reset behavior in outer023cb0 |0231b0 transfers context/flags, clears source context; no new retain |
| destroy |initial guard/no retained work |0229a0 drops work for nonnull context; old count1 invokes02a190 |
| prepare40 |023520 clears bit0, emits initial via020c80 |023310 clears bit0, retains work, emits tracked via020c10 |
| factory58 |023580 preserves flags, retains work, emits tracked |023360 preserves flags, retains work, emits tracked |
| response20 |023400 sets bit0, preserves initial |023220 sets bit0, retains work, preserves tracked |
| response88 |0234c0 clears bit1, preserves initial |0232c0 clears bit1, retains work, preserves tracked |

Tracked schedule type query023880 returnsA2690, distinct from initial023890 returningA25C0. It must not inherit the initial empty classifier. The raw leaf records are actual saved-byte decoding, not fabricated Ghidra pseudocode. No native EH cleanup or arbitrary concurrent lifetime completeness follows from these normal-body observations.

## Minimal storage design if selected later

Integrate within the existing provider, scoped to OriginalDefault; preserve Host callbacks and defaults. Do not add a detached unused helper module.

1. Add a separate native-family enum and uint32 flags to Row. Keep HOST `state_18`, capability, and Origin independent: a real proof root remains state1 even when its native source flags are0. A tracked pin is distinct from ledger refs, owner strong refs, IO initiation pins, and posted028ca0 pins.
2. Supply distinct initial/tracked schedule identities and type-query bindings. The family-specific executor callbacks implement the table above. Both schedule identities must still resolve the same Control safely; neither is a native address converted into a callable object.
3. Extend every row validation path, growth source proof, protected job-source proof, publish/copy/move prefix, and source_matches comparison to include family/flags. Current `row/fresh/source_matches` assume one table/token/executor (`environment.cpp:125-151`); adding a second identity only to configuration would be incomplete. Message copy's prepublished operations/schedule prefix must remain valid for the exact source family.
4. Transfer a tracked row/pin on move; retain exactly once on tracked copy/factory output; drop exactly once on tracked destruction. All allocations for HOST metadata need preparation and post-allocation source/authority revalidation before the no-throw commit. Do not call stop, error categories, allocation, or callbacks while Control's nonrecursive gate is held. If stop throws after a work drop, preserve the native changed-counter/flags prefix; do not fabricate a rollback pin.
5. Preserve context-null behavior explicitly. The selected typed context maps to the bound scheduler; do not decode a numeric native pointer or assume every future family shares this scheduler without validation.

## Existing stop body and missing binding

Reuse `stop_worker_02a190` in `anadius-socket-source/worker_run.cpp:5-12`; do not reimplement its sequence. It performs stop exchange, wake exchange, PostQueuedCompletionStatus-equivalent call, immediate last-error capture, unconditional system-category lookup on false post, then raises for nonzero error using084530. Both changed flags remain changed on post failure. The consumer needs a real `WorkerRunApi`, `Api` errors, and `WorkerRunEffects` exchange/raise projection.

Current interfaces are not a ready child-safe bundle:

- `server_runtime::Ports` (`runtime.hpp:21-26`) provides WorkerRunApi and DispatcherApi, but no error Api or WorkerRunEffects/WorkerControl.
- Ports.worker is Runtime::WorkerApi, not the raw SDK provider (`runtime.cpp:33-39,159`). Its post_stop routes through Runtime Completion and the selected admitted dispatcher. During serving that dispatcher requires the original worker/fiber and its whole Parent Operation (`owned-server-source/server.cpp:22-35,59`). A child JobPermit does not confer that original dispatcher admission.
- The runtime's existing complete WorkerRunEffects object is local to run (`runtime.cpp:321-333`). Exporting its address would create a lifetime/scope error. `Bindings.worker` is an existing stable WorkerControl, but is obtained from the Application after creation, and its selected Profile implementation delegates through `v.effect` to Policy (`profile.cpp:153-158`).
- Policy exchange/raise already validate scheduler identity, but Policy::enter enforces original worker/fiber and nonreentry (`policy.cpp:35-50,148-149,195`). Calling it from an Environment JobScope would reject; a raw atomic decrement alone does not make the last-work stop path valid.
- Environment::perform deliberately admits some child operations using the selected JobState/Parent permit (`environment.cpp:97-123`); JobScope close requires zero child-epoch temporaries (`:312-323`). Thus a future tracked copy/destroy on that child can reach last-work release while holding valid HOST row authority but lacking original-worker stop authority. This mismatch must be resolved explicitly, not hidden by rejecting only the rare oldcount1 case after the decrement.

For an **original-worker-only** selected tracked caller, a small bind-time optional stop projection could reuse existing errors/WorkerControl and a local adapter to the existing WorkerRunEffects API. Its unused dispatch/cache/thread methods would reject; only exchanges/raise would be used by02a190. Profile would supply stable errors and WorkerControl after its owned application is formed. This is feasible without changing the source algorithm, but is not sufficient for unrestricted JobScope tracked callbacks.

For **child** selected tracked callers, reuse requires an explicit child-authorized posting/stop capability tied to the exact Parent scheduler and JobPermit, plus a matching error/raise route. Reusing the raw SDK dispatcher to bypass existing admission, using an expired runtime-local Worker, or deferring the native immediate stop to a later receipt would each change the contract. No current selected caller justifies that interface expansion. Do not broaden admission or add a new lifecycle framework speculatively.

## Revisit criteria and proposed validation

Revisit when native/source caller evidence shows a selected transition into factory58/tracked input, or when tracked-family external API support is expressly selected as a reconstruction objective. Then pin that caller and whether it executes on original or child worker before choosing the stop projection.

Required source cases would distinguish initial versus tracked families, bit0/bit1 transforms, nonnull/null context, copy-retain/move-transfer/destroy-drop, exact oldcount1 stop invocation and failed-post prefixes, stale source during allocation, and both original/child permissions. Host legacy tests must retain their current meaning. They are proposed cases only; none was built or run here.

The highest-value current result is therefore to retain the exact recovered bodies and document this remaining boundary, then continue toward another selected business-function gap rather than presenting unused tracked adapters as progress on the default production path.
