# Independent static review of root's three test edits

Reviewed the three final files against this stage's saved before copies, plus the relevant fixture/backend definitions. No test source was edited by this review. No compile, tests, link, DLL execution, packaging or REA query was performed. No blocking defect was found in these changes by static inspection.

## Coverage and limits

`profile_tests.cpp`, new `deferred_job_growth()`: uses the actual Profile Frame API with a deferred fake Jobs backend, initial allocation64 and70 simultaneous pending detached jobs. It checks distinct slot/generation identities, old closure addresses and contents throughout growth, exact live-job/strong-pin counts, completion of the first job while69 remain, reuse of that retired slot with a later generation, owner survival after release of the original strong reference, all71 normal cleanup generations, and final actual session destruction. The fake backend's entries are reserved through71 to avoid its own storage relocation during the first runner/reuse sequence. The remaining loop executes entries1 through70, exactly the69 older jobs plus the new generation. This exercises the Profile row/closure growth and normal lifetime chain; it does not execute native Windows job workers, force allocation/new-handler reentry, or cover deque-block boundaries beyond this selected input.

Only obsolete Profile `jobsfull`/ExitProcess219 behavior and its main invocation were removed. The other ten negative cases remain unchanged: duplicate212, rerun213, unknown214, outside215, detach216, worker217, reap218, forged220, detachreentry221, heldroot222. The new positive regression is called from main. The summary count correctly changes eleven to ten. A bounded search found no current test/build script invoking `jobsfull`; `outputs/anadius-session-profile-source/architecture-review.json:93` retains the earlier historical mode description and was not rewritten as a new validation proof.

`handoff_tests.cpp`, modified `job_memory_handoff()`: holds70 distinct unacquired offers under the actual original-worker Borrow, checks outstanding count and LastError, confirms they prevent logical retirement, cancels every offer, and checks exactly70 reaped rows with no remaining offers. This replaces the old64-bound expected failure. Existing later actual sibling acquisition, exit, reaping, transfer and authority checks are retained. Seventy held offers do not launch70 OS threads; that is outside this fixture's new assertion.

`jobs_tests.cpp`, modified `passive_native_selection()`: initial NativeJobs capacity65 is now accepted and checked to remain Fresh with no pending, started or active jobs. The prior zero-capacity, incomplete API and legacy-callable rejection checks remain. This tests passive initial allocation above64 by source intent; it is not a dynamic NativeJobs growth, SDK launch, or full70-job integration test.

The three edits therefore cover distinct layers, not an end-to-end native70-worker run. None alone establishes runtime PASS, successful compilation, failure-injection coverage or ABI equivalence.

## Reviewed final SHA-256

| File | SHA-256 |
| --- | --- |
| outputs/anadius-session-profile-source/profile_tests.cpp | af2fe8a0d7bfee786c8882c4bad3578bcf3bd05873fd951b3d285b3169490491 |
| outputs/anadius-worker-handoff-source/handoff_tests.cpp | 159845a23c142a7444504a0f9a03d87cf220180550d4fdd5d100504f0254db0e |
| outputs/anadius-owned-jobs-source/jobs_tests.cpp | 976ba611904a257e38c7b6f41607fc4ee4d014d8eeee4673430e7c07df4aeba3 |
