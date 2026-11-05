# NativeJobs physical-row growth regression source

Changed only `outputs/anadius-application-graph-source/graph_integration_tests.cpp`. The exact pre-edit source is retained as `graph_integration_tests.cpp.before.txt`, SHA-256 `147d3d14e28ee7a70fb07048ce15b69c0642eac1cb346112c67bffc165e3562c`.

Added a separate `--prepared-native-growth` child scenario using the existing actual prepared Graph/Profile/NativeJobs fixture with initial NativeJobs capacity1. It preserves the70 mixed messages and all existing receipt, source cleanup, work counter, response FIFO, physical-thread/FLS and handle-close assertions. Only the first two physical rows overlap; this does not introduce a bulk concurrent-message framework.

The first child posts its ready receipt, its source handoff is consumed, and it remains blocked at the existing pre-ExitThread gate. Existing outside-Parent maintenance still checks exactly one work-pin decrement and retains its unsignaled private handle/barrier. In the new mode only, request2 is delivered after the first response while that consumed physical row remains occupied. The usual pending==0 prerequisite is retained for every other mode and later request.

At the existing Dispatcher outside-Parent wait point, the fixture joins the second child's independent proof handle while the first is still gated. It checks two distinct native actors/private owners, pending==2, started==detached==2, completed==1, two ready publications but only one consumed notice, and no premature private-handle/barrier close. The first proof must still report WAIT_TIMEOUT/STILL_ACTIVE. This forces a second NativeJobs row while its first physical row remains live. Private Row* identity is not exposed or fabricated by the test; the real start thunk uses production row storage.

Only then is the first exit gate released. Subsequent requests resume the original physical-drain prerequisite, and the unchanged final70-source/physical/FLS/receipt/free counts must complete. The normal delayed-exit mode still releases the first child before feeding request2 and retains its initial capacity1-reuse behavior. Existing stop/timeout/post/duplicate/wait/close negative modes remain unchanged and default the new flag off.

No wait for the second child is performed inside NativeHooks.after_completion or while its Parent Operation is held. Request2 delivery there only posts the injected receive completion; second-child physical observation/release happens in join_native_children outside Parent.

Fresh native Evidence supplied by root was read from this stage:025480 copies one framed message into a0x30 closure and begins/detaches021770 through05e8e0;021770 invokes the callback and retires the copied closure/current-thread state;05e8e0 creates a native thread wrapper for each request. These static observations motivate removing fixed row exhaustion; the synthetic fixture does not prove original scheduling or exact native allocator/EH ABI.

Final test source SHA-256 before independent review: `f0a459b4418a56c5b10abc42186d69c689806764b866d90c61b89e67fe010695`.

No compilation, project tests, linking, target/DLL execution, REA session or packaging was performed by this test-source author. The new scenario is NOT RUN and must not be counted as a passing result.
