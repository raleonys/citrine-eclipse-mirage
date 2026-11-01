# Cooperative pre-unload controller

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

take() validates nonnull Runtime/Ready Domain, allocates bookkeeping and its
shared control block before consuming donors, and transfers without starting,
publishing or closing anything. The supplied Domain must actually be the one
used by Runtime: Ready validation alone does not establish that identity.
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

Validation separates two scopes. The unit uses actual Runtime source with
controlled b4 lower providers and a real initial-locale Domain; it covers live
Runtime false/retry, retirement, provider-blocked Domain close, wrong controller,
idempotency and three isolated unknown/reentrant/live-destruction prefixes.
The full Resources test supplies its actual same Domain/Runtime after the
Runtime's earlier successful close, then verifies provider retirement and real
Domain closure through this callback. It does not rerun the whole live Resources
stop prefix through the new controller.

See evidence.json for hashes and retained compiler receipts. The accompanying
../dll-critical-path/loader-context-map.json records the remaining synchronous
attach, failed-attach, explicit-unload, process-exit and CRT integration gaps.
