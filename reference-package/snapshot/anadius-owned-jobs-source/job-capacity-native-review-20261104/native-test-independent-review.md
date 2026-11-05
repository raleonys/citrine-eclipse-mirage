# Independent static review — NativeJobs physical-row growth test

Result: STATIC PASS for the new `graph_integration_tests.cpp` diff. No compilation, project tests, linking or DLL execution occurred. The scenario is NOT RUN; this review does not claim an observed passing result.

Reviewed final source SHA-256: `f0a459b4418a56c5b10abc42186d69c689806764b866d90c61b89e67fe010695` (independently rehashed). The retained before copy is `147d3d14e28ee7a70fb07048ce15b69c0642eac1cb346112c67bffc165e3562c`. Test author notes `test-native-growth.md` agree with the inspected diff.

The separate `--prepared-native-growth` mode enables both the existing first-exit gate and the new growth flag. NativeJobs still starts with one physical row. The first ready receipt is consumed while the first child remains held before ExitThread/FLS; source completion alone does not free that physical row. Only this mode feeds request2 before the first row becomes physically reusable. The assertions require the exact retained first owner and its unsignaled independent proof handle.

After the second child is launched and detached, `join_native_children` waits its independent proof handle while excluding the still-held first proof. This happens in the existing lower Dispatcher.get path outside Parent. Inspected `anadius-owned-server-source/server.cpp` confirms that AdmittedDispatcherApi releases its Operation before the lower get call, runs physical maintenance after that call, and only then reacquires Parent. The second wait is not performed in the admitted lifecycle hook or under Parent/source gate.

Before releasing the first exit gate, the new scenario requires two distinct actual native actors, both original handles detached, both private handles/barriers still retained, two ready publications, pending2, started2, detached2, completed1, exactly one consumed notice, processed2, and zero premature private/barrier closes. The first proof must still return WAIT_TIMEOUT/STILL_ACTIVE. These checks force actual production physical-row growth; the test does not expose or fabricate a Row pointer.

Only after those assertions does the fixture release the first exit gate. Later waits join the retained proofs, and the existing post-wait maintenance gets the physical-exit evidence before source continuation. The remaining70-message response/receipt/FLS/cleanup/work/handle assertions and final exact ownership drain are preserved. This is two overlapping physical rows followed by the existing sequential workload, not70 simultaneously running children or a proof of unlimited native threads.

The new flag defaults false. Existing ordinary delayed-exit behavior still releases the first child before feeding request2, preserving the one-row sequential reuse scenario. Existing prepared/native, stop/deferred-drain, timeout, denied post, duplicate receipt, wait, close and Worker scenarios remain selected through their previous command-line modes. The new mode adds an isolated invocation and a final observation flag assertion without weakening their expected outcomes.

No blocking static defect was found. Exact Windows scheduling, failure-path execution, original native allocator/CRT/EH ABI and actual runtime success remain unverified until an explicitly authorized rebuild/test run.
