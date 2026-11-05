# Response executor and publication flow — source audit

Read-only audit of current selected production source. No code edits, REA queries, original-binary access, compilation, tests, linking, DLL execution or packaging. The native agent owns the fresh factory20/88/schedule investigation. Native selectors identify existing reconstructed algorithms; no new target-body or equivalence claim follows from source names alone.

## Actual message source and factory sequence

The OriginalDefault constructor-classifier correction affects023dd0's IO environment optimization, not the context-bearing message source. Profile still creates Slot.environment state1, binds it as the owner's environment_78, and retains its capability/origin/job authority (`anadius-session-profile-source/profile.cpp:464-475`). Inline message processing reaches `dispatch_message_028d30`; selected prepared native Jobs processes its body under a child permit, then normal completion routes the prepared response back through `dispatch_prepared_response` (`profile.cpp:353,717,729`; `anadius-owned-jobs-source/jobs.cpp:217`). The response scheduling policy below therefore still matters on the actual OriginalApplication path.

`anadius-socket-source/message_dispatch.cpp:47-89` creates a MessageWriteTask with function028270 and an independently acquired owner strong reference. It transfers the response string, performs two environment copies through the supplied copy-slot+8 binding, and reloads the executor after each copy. It calls021420 with the second environment and destroys both local environment copies afterward. Source Slot.environment remains live.

`dispatch_response_021420` (`response_dispatch.cpp:65-107`) already models this precise algorithm:

1. Move the incoming response/task ownership into a local; the input string/owner/control are cleared by transfer.
2. Cache source.executor's factory20 function before invoking the source query. Call it with query and literal084425, move its result with023cb0, and conditionally destroy the moved-from temporary.
3. Resolve the first result's executor live after cleanup and cache factory88 before invoking its query. Construct the next result, move twice with023cb0 and preserve conditional versus unconditional destruction ordering.
4. Resolve the **final** schedule and choose borrowed slot18 when nonnull; otherwise create an owned response functor via020cb0 and publish through schedule.owned10. The final and first environments are destroyed after publication. Residual local control/string ownership is released last.

The recovered021420 wrapper is present; concrete native factory/flags/schedule implementations are the pending comparison, not a missing dispatch algorithm.

## What selected OwnedEnvironment supplies

`Environment::configuration` (`anadius-owned-environment-source/environment.cpp:253-256`) installs the host Control executor with response20/response88, copy and schedule bindings. response20 validates literal084425; both response functions route to Control::construct -> make (`:220-227`). They preserve the source's host state/Origin and create a ledger capability. They do not model a native DWORD flags transformation or native tracked work retain.

The host response schedule has owned slot10 and null borrowed slot18 (`:241`). `Control::response` (`:234-239`) validates the exact environment row, increments its response statistic, and constructs:

`PostedExecutor{ports.view.identity.value, &ports.view.scheduler, 1}`

It then calls the recovered022e40 using current Runtime storage/worker/completion/coherent memory. Thus the selected host policy is explicitly force-post bit0=1, bit1=0 regardless of the source's actual native flags. Ledger refs/caps and the responses statistic are host bookkeeping, not scheduler work-count pins. OriginalDefault leaves these response functions unchanged.

## Existing flags, TLS, OS publication and work-count models

| Existing body | Actual current source behavior |
| --- | --- |
| dispatch_posted022e40 (`posted_operation.cpp:89-123`) | If flags bit0 is clear, inspect the TLS linked stack for the exact address of scheduler.completion.port. A first matching context with nonnull state permits immediate functor consumption without a posted operation; a matching null-state context breaks search and falls through. Bit0 set bypasses this inline attempt. |
| allocate_cached021820 (`posted_operation.cpp:20-45`) | Reuse one of two worker raw-cache slots when marker size/alignment fits; otherwise free one cached block as selected and allocate rounded requested size plus marker. For a posted operation request0x48 this is a real raw/cache path, not an environment ledger row. |
|022e40 post path | Project PostedOperation, zero its defined OVERLAPPED/queue fields, transfer source functor into operation.functor_38 and null the caller's slot. Reload the scheduler **after** allocation/projection effects. Forward `(flags>>1)&1` as028ca0 hint. |
| enqueue_posted028ca0 (`posted_operation.cpp:75-87`) | Explicitly ignores the supplied hint in current source. Atomically increments scheduler work_count once, marks queued1 and attempts OS completion post with bytes0/key0/exact typed OVERLAPPED. False post appends to scheduler's intrusive fallback queue under its gate and sets dispatch_required. It does not decrement the work pin or invoke the functor as failure recovery. |
| dispatch_completion027c10 (`completion_dispatcher.cpp:29 onward`) | Collect/repost fallback operations under the existing dispatch_required path. On an actual operation packet, resolve current binding, copy packet state before callbacks, complete it and then decrement one work unit; old count1 calls02a190. It does not inspect the old operation after completion may free it. |
| stop_worker02a190 / run_worker0298b0 (`worker_run.cpp:5-46`) | Existing stop/wake exchanges and stop packet path; worker installs a linked TLS context keyed by its actual port field, runs dispatcher and frees terminal raw caches after return. |

Here “inline” is the022e40 matching-TLS direct-consume path. The false-OS-post fallback queue is a separate scheduler-owned retry queue; it is not a successful inline completion. The current WorkerThreadState has raw caches, pending exception state and context fields, not an added per-thread posted-operation queue. Bit1 is forwarded then ignored by028ca0; fresh native evidence is needed before calling that a mismatch or inventing a local continuation queue.

For Host flags1, response always reaches the posted-operation allocation and one work increment, even when called on the matching worker. If fresh native factory20/88 ultimately sets bit0, this may match its publication route; if the final native flags permit inline execution or alter continuation behavior, the difference is concrete and should be fixed at that selected transformation/publisher boundary. Source alone does not choose between these outcomes.

## Functor, control and residual ownership

`make_owned_response_020cb0` (`owned_response.cpp:42-57`) allocates/project a separate0x48 response task through the handler cache path, transfers response and object/control into its payload, clears the source and publishes callback022280. It does not increment an already transferred owner strong.

The posted envelope and owned response task are distinct allocations/projections. `dispatch_posted022e40` nulls its caller's functor slot on both immediate consume and queued transfer. `consume_functor021ac0` clears its own local before invoking flag1. After each publisher returns,021420 discards only a still-nonnull residual slot with flag0; it cannot guess that a transferred functor remains caller-owned.

`complete_posted027120` (`posted_operation.cpp:125-133`) first saves and nulls operation.functor_38, then cleans/caches/frees the envelope before invoking its local functor. owner!=0 invokes flag1; otherwise the saved functor is discarded with flag0. Posted cleanup028fd0 only discards residual functors still present on its operation and applies the two raw cache slots or free, preserving exact lifetime order.

`call_owned_response022280` (`owned_response.cpp:67-89`) transfers payload into local MessageWriteTask, clears the task, and releases/caches its allocation using the distinct handler-cache slots4/5 before any response callback. It then never touches task/storage again. For flag1 it calls0219d0, which copies the response through009eb0 before function028270 consumes that independent copy. On return, the local owner strong and response storage are released. Flag0 skips invocation but still releases the transferred ownership exactly once. Residual message-dispatch task/control and response strings are likewise cleaned only if still owned after transfer.

These session/control strong refs keep response data and session alive; they are distinct from environment ledger refs and scheduler work_count. A future native tracked environment implementation must preserve all three ownership domains rather than substituting one for another. The current code models normal defined-field paths and fail-closed retained prefixes; it does not add native EH rollback or arbitrary concurrent callback guarantees.

## Concrete comparison questions for native evidence

- Starting from the initial084928/084908/084860 environment copied by028d30, what exact tables/context/flags do executor20 then the result's executor88 emit? Are temporary factories untracked or tracked, and which actual retain/release effects survive their moves/destructions?
- What function is final schedule.owned10, and is borrowed18 actually null for this selected family? Do final bit0/bit1 values match host PostedExecutor flags1?
- Does original028ca0 ignore the continuation hint as current source does, and are post failure, fallback queue and work-start effects ordered the same? The recovered wrapper already exists; fresh exact-target evidence should constrain any change.
- If native scheduling differs, repair only actual selected family/flags at the source binding boundary. Do not add unused generic tracked helpers, globally clear roots, or change OriginalDefault's separately proved023dd0 optimization.

## Audited source hashes

| File | SHA-256 |
| --- | --- |
| owned-environment-source/environment.cpp | 7306d849d5ca78c59dd7aaa3b92825773bd236145e68d095df130e31ee7ea7d3 |
| socket-source/response_dispatch.cpp | 9b8659bdbaa95afc66e3759152168ad1add30bf5e856210e159f5e9f553401d0 |
| socket-source/posted_operation.cpp | 6389460fc5978b609f4752581170eb8565cd1a8d959bd036692690760ede312a |
| socket-source/owned_response.cpp | a9125c30bf6d551a3eb32435ab088c8266a60f421e545e5e211c4fd34a7c821c |
| socket-source/message_dispatch.cpp | 47efc10e32430961cd3f78f5604537cec1aa46a7848b48c1cf4b41dab259ca23 |
