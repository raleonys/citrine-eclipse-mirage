# Next Graph fixture: real sequential call assembly (read-only plan)

No source, frozen proof, policy, native session, build, test or ZIP changed/ran.
Entry: `graph_integration_tests.cpp::prepared_native_profile` (580) → real controller NativeThreads/PreparedMemory → `run_prepared_native` (498) → actual Graph install/prepared server start, Profile, NativeJobs and source completions.
Actual Profile calls the selected `np::JobLogic::process_native_job` at profile.cpp 739; private Graph::Logic delegates to Graph's child assembly. Graph must invoke new `np::Logic::process_native_call` with owned exact-call dependencies there.
No public test-only Graph seam, manual Parent/work/ready state or early ConcurrentActors selection is necessary.
The current default Parent comes from owned-server constructors 152/157; NativeJobs holds source_gate across child acquire/body/return (jobs.cpp 154), and Profile's child admission/slot.processing still serialize bodies (profile.cpp 724–740).
Therefore this stage can validate real per-call wiring and retirement sequentially, never same-session parallel execution.

## Minimal separate mode
Add `--prepared-native-call-views` to the EXISTING native prepared fixture; retain all ordinary 70-message/delayed-FLS/growth/negative modes unchanged.
Reuse `ConcreteProviders` (422), `NativeHooks` (438), actual XML/Router, actual initialized achievement mutex, native child wrappers, receipts, exit waits and real IOCP completion fixture.
Use an optional mode flag through existing State/run helpers; preserve the same actual 70 native_body requests and FIFO response IDs, receipt/job/thread counts and physical handle/FLS retirement assertions.
The five existing bodies at 386 already select Information logging/time, Content atoi/UTC/save and Licensing bound auth; all must use the same existing business/configuration/Online/authentication roots.

## Fail-on-default providers without debug APIs
In this mode only, AFTER root authentication binds and BEFORE Graph install, use existing CheckedCalls bind_content_checkpoint on the actual concrete Information, Content and Handlers owners.
Retain one borrowed, nonmutating probe through teardown; it accepts only the real original worker thread/fiber and rejects a child. This is a checkpoint observation, never fabricated child authority.
Current CallView factories copy/share the immutable lower roots but give executions their own explicit context; they do not inherit the root legacy checkpoint field (Content/License copy constructors and Information State).
Legacy with_job explicitly rejects an already-bound checkpoint; accidental default/EffectScope/ambient provider fallback therefore cannot silently pass as correct private CallView wiring.
Allow genuine original-worker root checkpoints during binding/retirement; clear all three through their normal public APIs on the original worker before ConcreteProviders destruction.
The test should assert real output/state/callback effects, not introduce an internal provider-count getter; source review separately traces the new process_native_call dependency identities.

## Writer and lock bookkeeping coverage
Only in this mode, set the existing own achievement's total to 1 BEFORE Graph install, preserving natural zero progress/readiness/work values.
The first actual GrantAchievement then reaches real Content DocumentWriter, Graph Writer/Xml, achievement lock, semantic event lock and DOWN+achievement insertion; subsequent requests observe the same completed root achievement.
Change only this mode's final mutation expectation to exact progress=1; keep ordinary modes' exact progress=14. At the established original after-completion point, assert initialized queue count=2 and no held event lock; normal Graph retirement must free the retained events/writers/lease.
For actual guarded persistence too, seed only this mode's `providers.paths.xml_path=u"MEM\\call-views.xml"` and bind `options.state.achievement_file=&providers.paths.xml_path` (the SAME actual Content Paths cell); leave ini_path empty. Assert exactly one existing MemoryFiles XML write/output with the completed achievement instead of this mode's old empty-files expectation. No CreateDirectory/real filesystem or encrypted event dequeue is selected.

## Bounds
Existing lower handoff CallView fixtures separately cover healthy actual-child overlap and unknown raw ownership; they do not substitute for this actual Graph assembly path.
The owned Online/EXE slot30 unit fixture remains separate; this Graph mode's offline bound-auth path does not claim foreign query/code-lease coverage.
Actual Graph concurrency stays pending coherent Profile/NativeJobs/Parent factory changes. A later held-A/B-before-release network/session case must be authored only after that complete route permits progress; never relax serialization checks or release A to manufacture success.
