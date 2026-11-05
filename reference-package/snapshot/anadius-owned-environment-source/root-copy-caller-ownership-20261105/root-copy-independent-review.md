# Independent static review: Root message-copy caller ownership

Status: no blocking finding in the exact six-file change below. This is source inspection and saved static binary evidence, not a compilation or runtime result. No project tests, compiler, linker, reconstructed DLL execution, or ZIP update was run.

## Reviewed source freeze

| File under outputs/ | SHA-256 |
| --- | --- |
| anadius-socket-source/io_completion.hpp | 337ab5ee731caa9c7f19680aa01beb3225876dd11c1c4eca7a1f3dc68cb01c5b |
| anadius-socket-source/message_dispatch.hpp | 409c366a32ece93070af656ccfd636b07ae625eece42551ec7309c7dac364536 |
| anadius-socket-source/message_dispatch.cpp | e7c259b62931308a813e0e2be1821bad2ab477f6dc9adaa3a1f16aacd904af94 |
| anadius-owned-environment-source/environment.cpp | d3b83e35e3cb1237b25398c35f14a29f3eab7131f893e478b5e1d8e1e02ba9cf |
| anadius-socket-source/message_dispatch_tests.cpp | edaaa9c3601fa6ee5f6e973ab959f8d3b9c73c6a46be6fb1c322d0751ce6a5f5 |
| anadius-owned-environment-source/environment_tests.cpp | 654fb74592b875290cd563e06bfea06aa45228ca20a64f95637a77aa8150d3f4 |

Environment author report `root-copy-environment-implementation.md`: SHA-256 `0222f475013fdec97d04aa041892f9cbdb3671cc71a92922ae178d0773bbfd99`.

Test author report `root-copy-test-implementation.md`: SHA-256 `4e08b05aa330dc164364e1221f1b915aac1919c814320f78edc3e39a79f1543d`.

These sources supersede their before-snapshot bytes in this new stage; prior scoped-projection reports remain historical and are not edited to identify the new helper/hook.

## Fresh addressed evidence

The complete inline `instructions-180028d30.json` and exported bundle identify original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`, Ghidra 12.1.4, Evidence `ev_c1e35ff0c5c57064a1ddf6f2ba08016bd1eecedf05e9c21ef99d15c95c75e2c7`.

The provider reports three analyzed body ranges: `180028d30..180028faf`, `180028fb1..180028fbb`, and `180028fbd..180028fc1`; 656 total bytes over a 658-byte span. This is not a claim of a contiguous 658-byte body. Ghidra marks three invalid-parameter calls at `180028fab`, `180028fb7` and `180028fbd` as terminal/no-return. Those flags and excluded fallthroughs are provider observations, not independent proof of callee termination or native EH behavior.

The two relevant copy windows are independently visible in that saved reply:

- First destination ops/schedule prefix at `180028e1f/180028e2a`, cached slot08 lookup at `180028e2e`, call at `180028e3a`, then LIVE source executor load at `180028e3d` and destination store at `180028e44`.
- Second destination prefix at `180028e4c/180028e54`, cached slot08 lookup at `180028e58`, call at `180028e64`, then LIVE first-copy executor load at `180028e67` and destination store at `180028e6b`.

Thus an ownership wrapper must surround the actual caller body. Writing the executor inside raw slot08 would move the load earlier than the measured native sequence.

## Production review

`copy_message_environment_028d30` factors those two existing inline segments into one typed helper. Its local Body retains stack references/span only for the synchronous call. Body writes destination ops/schedule first, looks up and caches the callback afterward, invokes either the explicit admitted slot or that cached callback, and only on normal return reloads the live source executor into the destination. Both real copies in `dispatch_prepared_response` use this helper; residual task/control/string cleanup order is unchanged.

The optional HOST whole-copy hook is attempted only when the source table is nonnull. A declined hook executes the ordinary body once. A handled hook executes the supplied body and returns true, so no fallback repeats it. Null source table and missing/null/duplicate bindings retain the old after-header lookup failure position instead of introducing an earlier lookup/throw. The convenience Context overload and narrow const-span overload select the same helper. The latter permits a real owning Environment component test without an unused fake Frame/MessageDispatchContext.

The actual owning Environment table reserves full source and an unowned destination through its existing one registered Root effect before Body writes any native prefix. Destination's previous payload/state/executor are not treated as meaningful admission data. The hook does not infer a child job, create another owner, or call an ambient copy router.

Its private stack Slot bridge is bound to that exact Control/effect/source/destination. It rejects repeated or foreign arguments and checks the function that Body actually looked up, after the measured header prefix. It then calls the existing `copy(..., prefix_ready=true)` on the SAME effect, which publishes payload/state and one real copied row but leaves the executor untouched. The slot postcheck deliberately runs while the exact output partial-prefix marker is active. It does not nest another Root run or dispatch the cached owner function through a second effect.

Body then performs the actual live executor tail. The owner requires the exact returned destination, one entered/returned slot, the captured new destination capability, and full source/destination fields before clearing `copy_output`. Full claim validation and final admission occur with both claims still held; effect/node retirement follows the completed tail. This closes the prior early-retirement/deallocator window for the selected actual message helper. No Body or Slot reference is stored by a durable failed effect.

Unknown lookup, callback, postcheck or body outcomes retain the real preceding header/payload/row effect and shared sticky health. No rollback, second callback, guessed success tail, re-add or speculative cleanup was introduced. Source/destination claims, durable row/work accounting and failure policy are the existing Environment machinery. The direct legacy exported slot08 callback remains its narrower partial-prefix contract for callers that do not select the whole helper; it is not silently redefined as a complete native copy.

The new aliases, Body/Slot interfaces and table hook are HOST projection mechanisms. They do not assert a native vtable/ABI change. Production policy remains Serialized; child scheduling and Scope authority are not widened by this Root hook.

## Authored test review

The message component adds seven cases through the existing Context overload: owner refusal before any prefix; default fallthrough; one handled body with a private bridge; missing, null and duplicate callback lookups after header commit; and a throwing slot that preserves actual header/payload/source mutation while skipping the executor tail. The slot sees a placeholder destination executor and changes the live source executor, so success must copy the changed value only after return. Trace/call counts distinguish one handled body from duplicate fallback. These are real helper calls with bounded observation providers, not production Environment permission substitutes.

The selected Environment test uses that owner's actual exported copy bindings through the span overload. Its first checkpoint observes the untouched destination; after payload/copy-count publication it observes the placeholder executor; a later checkpoint observes the real executor tail while the SAME single effect remains active. It does not query an incomplete destination. Normal completion checks distinct copied capability, unchanged source capability/fields, exactly one reference/copy, no extra create/move/destroy, inactive effect and normal projection drain.

The older direct raw slot08 placeholder test remains, preserving the distinction between a partial callback and complete caller. Existing message normal/prepared response, lifetime, failure, header, trampoline and authoritative-header assertions remain present, as do the Environment growth/family/OriginalDefault and known-input/unknown-prefix cases. No assertion here was executed. The Environment fixture is a controlled sole-worker FLS/WorkStop component, not an actual child Scope/Runtime proof.

## Remaining scope

The selected Root message caller ownership gap is addressed at source level. Arbitrary callers that invoke the direct legacy raw copy callback without the whole helper retain their trusted caller-tail obligation. Raw global Empty remains a trusted low-level projection. Actual positive Scope integration, two-Scope tracked-family progress with B native/FLS/Parent reaping while A is held after a real decrement, naturally established child old-count-one after original return, child prepare/response/scheduling authority, full original response/021770 child-tail placement, native ABI/EH behavior, compilation and runtime equivalence remain OPEN. No native function-coverage credit or overall completion percentage is claimed.
