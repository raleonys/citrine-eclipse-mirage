# Independent socket helper and Policy test source review

Result: STATIC PASS for the seven root-authored files below. No blocking finding in the reviewed diffs. This reviewer authored Runtime/Profile/Policy production bridge files; those five files are deliberately excluded from this independent review and are being reviewed separately.

Read-only comparison against this stage's `before/` copies, 2026-11-04. No compiler, project tests, linker, DLL execution, package update, or new REA session was used. Test assertions describe proposed source cases, not passing executions.

| Exact reviewed file | SHA256 |
|---|---|
| outputs/anadius-socket-source/worker_run.hpp | b5914a1df6b05be60a133b3104bd7129267b00c8ebd8cbddb927c0640ed71de7 |
| outputs/anadius-socket-source/worker_run.cpp | 75d617268edcb4b8ad610dd4509571a5420d1918e92e60b485d66d0d564fedf6 |
| outputs/anadius-socket-source/handler_dispatch.hpp | f7354ada6b7de627dbb5db6ac910f86548d2e7b483703500fe2034e6d1b3b83d |
| outputs/anadius-socket-source/posted_operation.hpp | 29d0ee6439b2d7ce82bd6ef852cbd303d13ea8734b272c95aecb448377222bb6 |
| outputs/anadius-socket-source/posted_operation.cpp | 6f7f17ea26c8e08abdd6fb5a892f3337162743de76ffa93c658c6397f20ca215 |
| outputs/anadius-session-policy-source/policy_tests.cpp | eebf32b0b958224f01d90e510cffca1d597c4b45f6076ed480a260c582649670 |
| outputs/anadius-socket-source/posted_operation_tests.cpp | 405926aca64514681fb9bb8e83c07b7a622c55833fad55fcc2677aeabb40180f |

## Stop interface extraction

WorkerStopApi exposes only post_stop/get_last_error. WorkerStopEffects exposes only stop/wake exchanges and raise. Existing WorkerRunApi/Effects inherit those exact signatures and keep their TLS/dispatch/CRT operations. The02a190 definition changes only its parameter types: exchange-stop, exchange-wake, zero-packet post, immediate status read, unconditional failure-category read and nonzero-status raise remain byte-for-byte the same source body.

That ordering agrees with fresh saved `instructions-18002a190.json` and this stage's native-findings.md, including old stop/wake short circuits and false/status0 category lookup. No work count is moved into stop_worker itself; the tracked release bridge owns its prior decrement. No new receipt, flag rollback, retry, TLS grant or unload check is introduced by these two files.

## Real typed handler slot and envelope ownership

OwnedHandlerTask now derives from PostedFunctor with the same call_slot_00 flag signature. Existing receive/send/accept task subclasses remain typed projections; their inherited base conversion is a real C++ conversion, not a reinterpret cast or native-layout claim. Destruction still happens through each selected callback, not a delete through PostedFunctor's protected nonvirtual destructor.

The posted dispatcher shares its existing implementation through a template over the caller's pointer type. Both public overloads pass the actual reference to that template. On the inline path it converts the current pointer to a local PostedFunctor*, nulls the real caller's slot before invoking, then never writes the caller's slot again. A callback replacement therefore remains visible. On the posting path allocation/projection happen before transfer; the actual source pointer is assigned to operation.functor_38 and the real caller slot is null before publication. No temporary base-pointer reference delays the typed slot update.

The common body preserves the live scheduler reload, flags/hint extraction, existing cache allocation, posted pin and fallback queue behavior. Successful publication may end the operation's lifetime; following cleanup uses the existing null frame, not the operation. The header dependency is valid: handler_dispatch.hpp includes io_completion.hpp, which includes posted_operation.hpp and defines PostedFunctor before the new inheritance; include guards prevent the implementation include from duplicating definitions.

## Substantive source cases

The new typed-handler cases invoke the real public typed overload and actual complete_posted027120. They check:

- real slot null before inline callback, callback replacement preserved, no queued allocation/pin;
- allocation before transfer retains the source; OS publication sees it null and one posted pin;
- actual posted envelope stores the correctly converted base pointer, completion frees the envelope then invokes once;
- pre-allocation throw retains source/no pin, while post-publication throw retains source-null/one pin prefix.

These cases exercise the mutable-slot alias problem, rather than comparing a helper's return value with its own implementation. Existing generic posted tests, immediate-deletion cases, cache/queue checks and main entry calls remain present.

The new concrete Policy cases call finish_environment_work with the fixture's actual bound scheduler, whole Parent Operation, exact policy errors and a controlled narrow stop API. The Ready case is repeated after host_return using the cleanup checkpoint, so it checks the Returned admission branch without the serving-only stored checkpoint. It verifies nonfinal decrement, oldcount1 stop/wake matrix, low32/high32 behavior, post success and false/status0, and restoration of the fixture's saved scheduler state after each normal matrix.

Four retained terminal cases cover nonzero failed-post error mapping, unknown post exception, pre-decrement checkpoint failure and foreign scheduler rejection. consumed distinguishes before/after-drop failure; native changed counter/flags are asserted without a guessed rollback. Existing isolated cases remain and the printed terminal count changes from23 to27 consistently.

## Limits

Policy tests use an explicitly controlled WorkerStopApi; they do not exercise Runtime's private admitted/raw target selection or its failure-error latch. They also do not create a child JobPermit or establish child last-release behavior. Those source cases must be identified separately, and cannot be inferred from this review. False/status0 reaches the unchanged actual02a190 body, but this test fixture does not separately count the category getter; the unconditional category ordering is established here by direct source/native evidence, not a claimed test observation.

No result here establishes original CRT/virtual ABI equivalence, general reentrant callback support, or compiler/runtime success. The final production bridge and Environment cross-layer review remain separate work.
