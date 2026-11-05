# Independent static review — actual Graph call-view route

Result: **no blocking source finding** in SDK-authored `outputs/anadius-application-graph-source/graph_integration_tests.cpp`, SHA256 `13094a04164a2890cafbfa9637ed38e1b801c2b7c15daf3a4326af211d0a99d3`. Compared the exact diff with the root-captured stage before file and manually traced the selected production interfaces. The reviewer authored Graph production, not this test. No compiler, project test, linker, executable, DLL, ZIP or REA operation ran.

The new isolated `--prepared-native-call-views` mode reuses the actual existing prepared Graph→Profile→NativeJobs→private np::JobLogic route, not a fabricated public testing API or synthetic Effects substitute. It keeps the actual runtime Parent policy explicitly **Serialized** and retains the same seventy real native starts/receipts, source continuations, private/barrier handle closure and independently observed child/FLS exit/reaping path. It tests the new call-view foundation, not actual simultaneous Graph message admission.

## Sentinel discrimination

The three actual concrete providers are created and authentication is bound on their original worker/fiber. Only this mode then prebinds all root CheckedCalls checkpoints to a sentinel which requires that exact original actor before incrementing its counter. They remain bound through all seventy child routes. A legacy child with_job/EffectScope fallback would either encounter already-bound root checkpoint state or invoke the original-only callback on the wrong actor. A successful authored route therefore depends on private per-message CallViews using their own immutable contexts, not silently falling back to those root scopes.

The sentinel data is creator-owned; wrong-actor validation occurs before its non-atomic counter increment. Root sentinel counts are checked on the original worker. The fixture's general require counter is atomic. Explicit clear_content_checkpoint calls precede the existing creator-side provider teardown; the callback's backing outlives that clear. No custom foreign SDK object/module is introduced by this mode.

## Real Content writer, lock and queue path

For this mode only, the existing achievement's total is changed from1000 to1. The first actual GrantAchievement completes it; subsequent grants do not repeat its mutation/persistence branch. The expected final progress1 versus legacy14 is conditional on the new mode, preserving default existing assertions.

The concrete Content provider's Paths object was already borrowed by its persistence Engine. The mode updates that same `providers->paths.xml_path` cell before Graph installation and makes Graph StateView.achievement_file point to that exact cell. Thus actual Router/GrantAchievement creates the recovered document writer, XML serializer, event and memory-file output through the real private Content view; it does not compare unrelated copies or manufacture a handler result. The empty INI path keeps its existing no-file branch.

After the third actual completed response (the first grant in the established five-request cycle), the original observes initialized queue, no held logical event lock and exactly DOWN+achievement count2. The plaintext message path does not consume the encrypted-event queue. Final memory-file assertions require exactly one output/write at the same actual Paths cell, no reads, expected Achievement/id and controlled UTC timestamp, plus Information's actual profile log. The existing binary mode/share, writer, count-preserving file and source lock interfaces remain selected.

## Preservation and bounds

The diff leaves existing default modes, delayed-exit/growth, source-drain/cancel and retained failure groups present. New flags default false, function arguments default false and mode-only path/progress/file assertions branch explicitly. Existing response FIFO ids, seventy exact processed messages, source counts, strong/control/IO/work drain, provider creator retirement and actual native/FLS closure assertions remain. There is no new project build-script or production policy selection.

This is authored-source review, not an observed passing integration result. The Parent/Profile/NativeJobs whole-body admission remains serialized; the fixture makes no same-session progress or arbitrary concurrent business/provider/mutex-safety claim. Separate lower fixtures/native evidence establish the motivation and prerequisites. General string/map/scalar mutations and the pre-SRW native mutex owner field still require the documented work before actual concurrency is enabled.
