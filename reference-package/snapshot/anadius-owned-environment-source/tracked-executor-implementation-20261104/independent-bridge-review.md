# Independent Runtime/Profile/Policy work-stop bridge review

Static review of the five final bridge files below against their saved before copies in `tracked-executor-design-20261104/before`, plus the Environment call boundary and selected admitted dispatcher/stop helpers. No code was edited by this review. No compiler, project tests, linker, REA call, original/reconstructed-DLL execution or packaging was used. **No blocking defect found in the reviewed final bridge source.** This is not runtime PASS or an executable native-equivalence claim.

## Exact-bound channel and lifetime

Runtime owns a stable EnvironmentWork capability and publishes its address in the trailing Ports.environment_work_stop field. matches checks exact bound Parent/scheduler without requiring application bindings, which correctly supports Profile's environment-bind-before-bindings construction order. Runtime validate later requires complete bindings, eligible Runtime/backend phase, unchanged TLS selector, exact scheduler, and either original thread/fiber plus held Parent admission or the explicit JobPermit checked against that same Parent. IO/Accept phase is rechecked. The capability object is part of Runtime; existing Graph/Profile/provider lifetime and active Parent/source ownership keep it alive through Environment rows/effects.

Serving original calls use the current admitted dispatcher target. Child and returned/quiescing original cleanup use only the private retained lower target for an exact zero stop packet. No general get/wait/TLS/dispatcher capability is exported to children. StopApi validates port, bytes0/key0/null OVERLAPPED and checks borrowed source admission immediately before and after each effect. Runtime's unchanged TLS selector is validated on this path too.

On false post, StopApi captures target.get_last_error immediately, before any subsequent checkpoint can change SDK error state. Its private latch is consumed once by the actual02a190 get-error step. Existing AdmittedDispatcherApi preserves physical-wait status only until dispatcher consumes it immediately after get; the serving callback route does not reuse that stale status for a stop failure. The child/raw route avoids original-worker admission entirely. Unknown SDK/checkpoint effects propagate and poison, without post replay or invented flags restoration.

## Work ordering and source authority

Profile::Impl::Worker explicitly forwards the new operation only to the selected concrete Policy. It deliberately does not route child work through the ordinary original-worker-only v.effect wrapper. Policy performs independent bound-phase/Parent/scheduler/native-error-provider/actor checks; a child must be a distinct actor with a current JobPermit. It reserves environment_work_active separately from ordinary Policy.active. Existing ordinary Policy calls/lifecycle/exit cleanup deny reentry through that reservation. A paused original call's active flag is not confused with a child's Parent authority.

Policy admission validates the live source and borrowed Environment checkpoint before the atomic operation. work_count.fetch_sub occurs once, followed immediately by consumed=true. Only oldcount1 calls the existing actual stop_worker02a190. Its stop34, wake38, post, cached-error/category and raise ordering remain in the existing helper; the bridge does not duplicate or approximate the body. Stop uses exact-scheduler effects; low32 wake state is updated through the same selected Policy serialization while preserving high bits. No saturation, underflow-recovery, delayed stop or counter compensation is added.

The capability rechecks source around stop effects and after return. A throw before decrement leaves consumed=false; a throw after decrement leaves consumed=true, including later stop exchange/post/get-error/raise/checkpoint failure. Policy clears only its work reservation and poisons; Runtime also poisons the effected call. It never increments work back or authorizes replay. Error0 on a failed post still reaches the helper's unconditional category call before normal return; nonzero error raises through the selected category mapping and literal084530.

## Environment cross-layer prefix

Environment validates matching capability at OriginalDefault bind/admission. A tracked row enters ReleaseEntered before calling it, with a retained source/protected-source/family/flags/cap proof and exact selected JobState/active depth. The callback and borrowed std::function lifetime are outside the nonrecursive Control.gate, with no retained Row reference across opaque effects. New perform entry while releasing poisons and rejects after unlocking. The borrowed checkpoint admits the same captured root/job actor and revalidates actual current/protected environment fields.

On normal consumed return the exact row retires once. On any invalid/throwing prefix it remains ReleaseEntered with its consumed marker and phase Poisoned, so neither a pre-decrement failure nor a post-decrement failure can be retried, re-added or reported as successful cleanup. Protected child source destruction remains prohibited. No source JobScope close, original image unload, generalized lifecycle permission or external quiescence is manufactured by this work channel.

## Validation limits

Reviewed source implements both finish_original and finish_job; there is no deliberate child-last-release omission. Actual production Runtime child last-release execution, injected failures across each stop prefix and complete composed integration remain separate source-test/execution coverage. A fixture capability that refuses children does not prove production child behavior. All tests remain unexecuted in this stage by user instruction. The native finite DWORD behavior, raw ABI/EH, nonadmitted null contexts and other executor families retain their documented limits.

## Final independently rehashed bridge source

| File | SHA-256 |
| --- | --- |
| outputs/anadius-server-runtime-source/runtime.cpp | c2cd34da990d9c2b2ba5463181ece96f985f72d9ef22dfd06c9ca9693a1857c2 |
| outputs/anadius-server-runtime-source/runtime.hpp | b558cbfb48936b021c86d038a501fd83d9229c4a53fd2165de6121d270a3f656 |
| outputs/anadius-session-profile-source/profile.cpp | f0744429c82f91a86509e17e9b530dc2bcce8626c55eae943dc06518e737f298 |
| outputs/anadius-session-policy-source/policy.cpp | b00747912def3dd0c14dd5be59aa8b42fa1459d8981330f96e56f51378e7fefe |
| outputs/anadius-session-policy-source/policy.hpp | 77b073c58846de51e57a035462add6db66c6b94e8849dbf60703fd59285318cd |

Environment cross-layer source reviewed at cpp3695a4f05d63f935af4ec2d968c85e2645c8ae16c5654f52a0dd3e18c88df239 and hpp255097bda918a238e4e8c50901de3bc7285c7734784074e5f7b4b209750cc660. Root separately reviews that implementation and socket helper/test changes.
