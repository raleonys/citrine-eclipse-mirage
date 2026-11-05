# Independent test-source review

Result: no blocking source finding. All assertions remain uncompiled and unexecuted.

Reviewed server_tests.cpp SHA256 **0905773c98d744cd47dd3b47a740d14038b82e5e2de0b97399cba5c421e9dcfb** and server_integration_tests.cpp SHA256 **73679f010f505b6da45412f52121c84a7484fd6cb2dd5878635fcbcfbc590268** against the stage/before copies, explicit-category constructor and unchanged source algorithms.

The direct overload test destroys caller-local operation/source arguments, then destroys the original exception while retaining typed, system_error and runtime_error value copies. It checks owned operation/what/source metadata and the module-lifetime category reference. The earlier unknown-address, context-free, non-system, misc and IOCP cases remain in place. It does not claim survival of module teardown or a native exception object layout.

Six future own-child modes exercise actual backend source paths with narrowly injected SDK faults: second service-gate initialization, socket creation, IOCP attachment, reuse option, bind and listen. The existing first scheduler-gate and port-error cases remain separate. The second gate fails after the real own IOCP and first physical gate have been created; it checks two category lookups, context0 mutex and no socket construction or synthesized rollback.

The five acceptor modes preserve exact captured raw values and source operations. Recorded fault suffixes distinguish WSA category-before-error FCE from attachment's Win32 error-before-category/close FGCX. Trace metadata preserves the last-error value around the category observation. The source close helper skips invalid socket on open failure and naturally closes200 only on failed attachment; later failures retain their existing ownership. Assertions match those concrete paths, category count3, bound-buffer prefixes and poisoned backend. No cleanup, quiescence or worker run is fabricated after failure.

Expected complete diagnostics use one separately computed default-native SDK oracle per case. The constructor's SDK calls are not intercepted/count-measured, and these sources do not establish a once-only runtime formatting observation. Copies compare stored values. Existing native/provider/compiler records establish the bounded semantic design.

The default test main lists all six hidden modes through the existing bounded own-process harness. No child, SDK fixture, project test, build, linker or DLL was executed during this review. Native EH/allocator/RTTI/ABI, arbitrary zero/unknown cases, full application lifecycle and whole DLL equivalence remain open.
