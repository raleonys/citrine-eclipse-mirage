# Independent frozen worker run source review

Result: **PASS**. No material production finding remains within the defined normal-flow and typed-projection scope. Four original normal bodies are newly recovered; helpers, adapters and existing CRT providers add zero original procedure.

Reviewer: worker_run_audit. The reviewer independently read complete retained assembly and pseudocode for `026320`, `029890`, `0298b0` and `02a190`, verified both listing SHA-256 identities, and compared all four frozen production files below. No original target or real SDK event/thread/TLS/IOCP operation was executed by this review. Tests were not executed by the reviewer.

## Frozen source identities

- `outputs/anadius-socket-source/worker_run.hpp`: `191ac2548f32bd8828d5d3d12b9ce41f99716e1f9028ae554e2d483fab06df0f`
- `outputs/anadius-socket-source/worker_run.cpp`: `1d5610735a7959e422675181268d30f16e6ea4e772a9fde65986f3744f4d30a4`
- `outputs/anadius-socket-source/worker_run_native.hpp`: `0abeed67dba180b8506005ecaade1093b1e966e847953a094cb8855e13c76d93`
- `outputs/anadius-socket-source/worker_run_native.cpp`: `df0d743263b53d4845456d78766e86daa47c9bf4358f51b91ff25f8dcda3ec38`

## Reviewed behavior

- `stop_worker_02a190` requires exact low32 atomic exchange effects, follows the two old-value branches, reads the live port only afterward, passes zero bytes/key/overlapped, caches last error before the unconditional category callback, raises only for nonzero cached error with opaque message token084530, and preserves both changed flags on failure.
- `run_worker_0298b0` uses the existing uint32 sequentially consistent fetch-add of zero. Its zero-work path calls real recovered stop before setting meaningful success fields from the supplied logical mapping of constant category objectA10F8. It does not claim native status padding is zero.
- The active path sets count/pair fields, calls required04a240 before zeroing cache slots, installs a live typed TLS frame with the address of the port projection, reloads the live TLS index for each SDK call, and routes every iteration to required dispatcher027c10 with INFINITE. Only nonzero dispatcher returns increment the uint64 count, using saturation. No invented active-path error write occurs.
- Restoration reads the LIVE context.previous field after dispatch, with a fresh index. Ten slots are then read individually in ascending order and passed to exact04b788 when nonnull. Earlier free callbacks can alter later slots. Slots are not reset. Required04a2cc runs last. Throwing effects do not trigger an invented RAII restoration or cleanup.
- `run_worker_function_029890` reads scheduler18 and calls the recovered loop with local ignored logical result. Null scheduler is explicitly rejected before the native loop dereference; no successful native null behavior is asserted.
- `delete_worker_function_026320` stores opaque0844b8 before testing bit0; calls exact04b0f8(function,32) only for odd flags; returns the primitive cached original address identity without accessing the function after deletion.
- `RecoveredWorkerFunctionEffects` joins real worker startup, old recovered05e8e0, worker entry and both recovered virtual slots. Required allocation, SDK/CRT effects, dispatcher, deallocation and lifetime remain explicit.
- `NativeWorkerRunApi` forwards exact SDK TlsGetValue, TlsSetValue, PostQueuedCompletionStatus and GetLastError signatures through injectable imports. Adapter construction makes no SDK call. No original-image token is called as code.

## Explicit limits

This review establishes conformance of selected defined normal-flow logical effects to retained static evidence. WorkerRunResult optional logical Error does not expose native undefined status bytes. WorkerThreadState and WorkerTlsContext are live typed projections, not native stack or ASIO ABI overlays. Category mapping is supplied explicitly; A10F8 is not incorrectly inserted into the existing name-function identity field. Atomic stop/wake operations, native state storage and actual concurrency remain required effect contracts. Required027c10 is not recovered in this stage. CRT04a240/04a2cc/049be0 remain providers with old CRT production unchanged. Native allocation/deallocation, ownership, exception/SEH/unwind, executable vtable binding and complete DLL ABI remain unproven. This is not complete original DLL equivalence.

The flow report and its JSON contain the canonical evidence IDs and exact native instruction addresses. Review hashes below bind that final independent audit.
