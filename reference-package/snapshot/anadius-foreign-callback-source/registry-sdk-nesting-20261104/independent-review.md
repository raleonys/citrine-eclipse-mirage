# Independent integration review — 2026-11-04

Reviewer: separate subagent `registry_sdk_integration_review`, read-only production/test review. Static review PASS for the bounded source design; this is not compiler, test-run, loader, CRT ABI or runtime-equivalence evidence. No REA session, compilation, project tests, linking, DLL execution or packaging was performed by this reviewer.

## Production integration

- `OriginalEffects::original_call` resolves the typed original before the new SDK lease. With a checkpoint, the marker exists only inside `Boundary::invoke`'s lower-call lambda, after the precheck and before the postcheck. Normal and exceptional returns retire the marker. Resolver and checkpoint callbacks do not inherit the new current-body SDK interval.
- `native_context::Owner` binds its exact Admission once before publication. Admission is declared before Effects, so reverse member destruction retains it through Effects retirement. Standalone Effects have no policy by default; declined/default/cooperative SDK and nested operations preserve the existing ordinary route.
- `AdmissionLease` retains existing argument-only pure and connect selection precedence. A selected general child releases its exact frame before the native Context lease on success and exceptions. Automatic frame types cannot be copied or implicitly moved.
- Domain reserves each child in `Checking` before opaque hook/PTD probes. An ancestor SDK marker is insufficient: it must bind the exact current body identity. Same-slot recursion is therefore distinct from child CRT/log/proof reentry. At depth, `SDK.parent == parent_body.sdk` preserves the call chain.
- Root `active_` and the existing CallbackThreads scope remain held. Children validate and borrow the root PTD; they never call CallbackThreads enter/leave. Foreign general overlap remains rejected. Nested leave restores the exact parent callback while retaining Full mode; only root leave retires PTD ownership.
- SDK begin publishes its frame only after throwing scope checks have completed. Nested proof failure restores its parent reservation; unknown proof/PTD outcomes poison through existing policy. Exact frame cleanup remains possible under closing/poison. Domain metadata helpers preserve LastError. Pure-forward and connect SDK parents are excluded from the new general nesting route.

## Test source review

The new unit source covers exact same-slot and deeper body identity, checking-phase denial, ancestor-marker exclusion during a child body, known missing receipt rollback, another-thread overlap rejection, metadata LastError and pure-forward exclusion. These are source scenarios, not passing test results.

The integration source exercises an actual `native_context::Owner` and concrete callback Policy with a Publication proxy and explicit Domain scopes. It does not exercise the outer `foreign_callback::make_factory` wrapper or actual Windows hook installation. Its typed synthetic chain is openA -> openA -> queryA -> queryW -> real close, checking raw arguments/output/status bits, per-entry native Context leases and one retained root PTD. Provider resolution and child encoding callbacks probe denial outside the current child's SDK interval. A direct Effects call checks SDK-throw unwinding without crossing the noexcept native wrapper's terminal error path. Positive typed OpenW dispatch, arbitrary shared-set concurrency and actual Windows reentry are not established by these scenarios.

No concrete blocking defect was found in the inspected combined patch. Compilation and execution remain unverified; a consistent rebuild is required before using the changed Admission vtable/Effects layout.

## Reviewed source hashes

| File | SHA-256 |
| --- | --- |
| native_callbacks.hpp | 3091e29479502b304339ac2e6aa5aeff3d1bbbef23aa1ffec09cc007706174bf |
| native_callbacks.cpp | 2d37a2dba5a1f7cbcb2c47f0791edcb79153e4fd6abc6d5040168733e9b3d85f |
| native-context/context.cpp | e1fab8143fc37d33dbe1e8db3cb6a90ef52d645a1cd4ce1ba3262bad388a0266 |
| foreign.hpp | 70da46e1d7fd4bbd74d8d32dc8a03d80d4bb733a3fa4dcd7ad695c65c95b08a5 |
| foreign.cpp | bd28299de32cce8cfe9628ab4880a1eb86c38101cf0bcfa64ab481a550ef3732 |
| foreign_tests.cpp | 4e637b28b56f3fa012728a46376e7b769c76b038fd9484433af2e21153708f26 |
| foreign_integration_tests.cpp | dbde68ae4021e68881b2601b0d5234a3c49f1f3a362715cb4f416e513192e761 |
