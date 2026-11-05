# Cooperative pre-unload controller

Current source checkpoint: **unbuilt/unexecuted Domain provenance binding**.
Compilation, tests and linking require an explicit user rebuild instruction.
The older evidence.json and full-build ZIP apply to their prior source snapshot.

This implements a callable HOST shutdown path outside DllMain, using the
existing Runtime and Domain closure algorithms. It does not move startup,
define either require_loader_* hook, change original ordinals or publish a new
production DLL export. Future image integration must supply a stable context
and expose/retain the callback deliberately.

```cpp
// Runtime and its exact Domain were constructed on this controller/fiber.
auto shutdown = anadius::pre_unload::Controller::take(runtime, domain);
auto entry = anadius::pre_unload::Controller::entry(); // BOOL WINAPI(void*) noexcept
if (!entry(shutdown->context())) {
    // ERROR_IO_PENDING: retain shutdown and retire outstanding providers,
    // complete actual source/IO drain or explicit known cleanup repair; retry.
    // Poisoned: retain owners; do not retry unknown native outcomes.
}
```

take() validates nonnull Runtime/Ready Domain and exact certified thread-provider
root identity, then allocates bookkeeping and its shared control block before
consuming donors. It transfers without starting, publishing or closing anything.
DomainStorage::make_threads alone sets the private NativeThreads root. Runtime
captures that owner before its Resources sink is consumed and retains it through
all provider and registry retirement. A custom/legacy thread provider with no
certified root, or a different Ready Domain, rejects before donor consumption.
This does not attest that every other Resources provider uses that same Domain;
existing assembly identity checks and caller obligations still apply. Public
Runtime construction with custom thread providers remains supported, but that
Runtime cannot be adopted by this checked cooperative controller.
All Runtime/Domain-affine operations must share their original controller and
fiber. Both roots and caller controls/code must remain valid during invocation.

Shutdown proceeds in three real stages:

1. Call Runtime.close. False retains Runtime and all its remaining providers.
2. On success, destroy Runtime while Domain remains owned. This retires source
   provider owners; a successful close alone does not destroy them.
3. Close Domain. False retains it and retries only that stage. Other host controls
   or memory/threads/callback provider leases must be retired by their owners.

Successful completion is idempotent and preserves incoming LastError. Pending
closure returns FALSE/ERROR_IO_PENDING. Wrong thread/fiber is refused before
source work. Exceptions or swallowed managed reentry set sticky Poisoned and
retain ownership; the callback ABI never propagates a C++ exception. Destroying
a live/poisoned controller fails stop. Closed metadata remains owned until its
caller releases it. Final release uses Retiring before Domain-owner destruction.

closed() and snapshot() use lock-free atomic metadata. closed() is only a receipt
for these managed roots. It does not prove exclusion of arbitrary cached native
entries, external aliases, independently retained code owners, CRT thunks or
unregistered SDK borrowers, and it is not permission to FreeLibrary. The caller
must separately satisfy those contracts, retain mapped code and live context
for all invocations, and release the final image reference from outside that
image. No final-reference self-unload is implemented.

This API must not be called under loader lock, from DllMain, natural CRT exit
or process-termination callbacks. Existing Runtime.close performs source stop,
joins and source exit cleanup; it cannot be moved into DLL_PROCESS_DETACH.
Process termination/failed attach need separate policies, not a guessed call to
this controller. Natural CRT callbacks remain registered after logical finish.

Pending validation separates two scopes. The updated unit source uses actual
Runtime with controlled b4 lower providers and the real Domain native thread
provider, publishes the Runtime without invoking application attach, and starts
no workers. It includes missing/unknown Domain rejection, source false/retry,
retirement, provider-blocked Domain close, wrong controller, idempotency and
three isolated unknown/reentrant/live-destruction prefixes. The full Resources
test source supplies its actual same Domain/Runtime after Runtime's earlier
successful close, then follows provider retirement and Domain closure through
this callback. It explicitly releases Assembly's new stopped_sources receipt
alongside its callback/milestone aliases before requiring zero Domain leases.
Neither updated test was compiled or run. No live full Resources stop prefix
through the new controller is claimed.

Different-Ready-root rejection is currently inspected in source only. The
native FLS Binding reserves one image-wide callback owner at construction, so
two real native Domains cannot coexist for that scenario. No synthetic pointer,
extra live root or callback-binding bypass was introduced to claim coverage.

NativeThreads and Runtime HOST object layouts gained shared owner fields;
rebuild every allocating/passing/embedding consumer together when authorized.
No virtual slot, export, shutdown callback ABI or loader hook was added. The
certified root is an ownership/identity record, not kernel/foreign finality.

See evidence.json for hashes and retained compiler receipts. The accompanying
../dll-critical-path/loader-context-map.json records the remaining synchronous
attach, failed-attach, explicit-unload, process-exit and CRT integration gaps.
