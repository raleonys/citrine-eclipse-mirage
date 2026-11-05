# Remaining child tracked-release validation boundary

Read-only source triage, 2026-11-04. No new fixture, source edit, build/test/link, DLL/process execution, ZIP or native query. No concrete valid-caller production defect was established.

The selected NativeJobs path cannot make child tracked-row destruction the scheduler's last old1 drop while its body is still valid. `jobs.cpp:193` acquires an independent receipt work pin before native launch. Its entry processes/returns JobHandoff memory before marking Returned and posting the receipt (`jobs.cpp:154–162`). Only the original's returned_prepared acknowledgement clears row.work (`jobs.cpp:213–220`), then `completion_dispatcher.cpp:125–136` consumes the receipt's scheduler pin. That pin therefore remains during any valid child environment-row destruction. A fixture that removes it early to force old1 would not represent this caller.

Current `JobHandoff::clone_environment` (`profile.cpp:638–668`) uses exact child Parent/permit/JobScope and actual023dd0. OriginalDefault's protected session source is Initial and classifies to the empty family for this constructor; it does not manufacture a tracked executor. The broader public exact-JobScope/factory API permits other supported projections, but this audit does not claim every such composition is impossible or native-complete.

`Environment::Control::destroy` (`environment.cpp:232–272`) records ReleaseEntered, snapshots the exact protected source/epoch, passes the real permit and borrowed proof to Runtime.finish_job, and retires metadata only after exact consumption. Failure records release_consumed and retains/poisons the entered row; it does not re-add or retry the pin. Runtime's child private zero-stop wrapper (`runtime.cpp:96–126`) captures a failed raw post's status before later proof callbacks and returns the latch. Policy (`policy.cpp:210–275`) uses a separate child work reservation, validates actual source/permit, marks consumed immediately after the atomic drop and retains completed effects on throw. These reviewed paths expose no established defect at the proposed composition boundary.

Validation remains bounded and unexecuted: controlled Environment row failures310/311 exercise the original-cap before/after-consumption prefix; their WorkStop.finish_job intentionally rejects. The prepared real-child success source exercises the actual Runtime/Policy child capability on a separately fixture-owned final pin, not an OwnedEnvironment row. The original-serving failure source exercises actual latch/diagnostic behavior under real admitted original authority. No single authored or executed test combines a child tracked-row destructor with an actual failed last post. This is a remaining composition/negative-validation limit, not evidence of a source bug or a reason to invent an unreachable selected-NativeJobs fixture.

| Reviewed current source | SHA256 |
| --- | --- |
| owned-environment/environment.cpp | `3695a4f05d63f935af4ec2d968c85e2645c8ae16c5654f52a0dd3e18c88df239` |
| server-runtime/runtime.cpp | `c2cd34da990d9c2b2ba5463181ece96f985f72d9ef22dfd06c9ca9693a1857c2` |
| session-policy/policy.cpp | `361fab5cb99698408eafb8513a5e73d27ee88898edcd839ebf0605844f600521` |
| owned-jobs/jobs.cpp | `c9d625565e89fd7605de34374f5548733d0b9a5603912a0366c2c9405c70a7cd` |
| session-profile/profile.cpp | `f0744429c82f91a86509e17e9b530dc2bcce8626c55eae943dc06518e737f298` |
| worker-handoff/handoff_integration_tests.cpp | `d2eb5c962e85876bc0551974a684797546c1a901e30ec689c1cb8c9f02ab4f5f` |
