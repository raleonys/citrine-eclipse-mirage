# Owning initial-locale Domain construction

`make_initial_domain(InitialInputs&)` composes the existing DomainStorage with
an owned opaque PTD identity and optional actual SDK/handler backing:

```cpp
thread_domain::InitialInputs inputs({cookie, new_mode, new_handler,
                                    invalid_handler, thread_policy});
inputs.backing = supplied_provider_owner; // when dependencies need ownership
auto domain = thread_domain::make_initial_domain(inputs);
// domain is Constructed; caller explicitly initializes and retains it for retry.
if (!domain->initialize()) { /* retain root and inspect existing phase */ }
```

No policy values are inferred from the original binary. Default SDK tables use
existing native adapters; custom tables can be supplied before construction.
All 21 heap/FLS/lock/runtime slots must be nonnull before inputs are consumed.
Successful validation consumes inputs once. A later allocation or construction
failure does not make those inputs reusable. A preflight rejection permits
correction. Initialization, native SDK calls, worker creation and publication
are not performed by this factory.

Ownership contains a distinct opaque identity object, not table storage. The
current restricted PTD source only stores its address at PTD+0 and compares it
at destruction. It never reads or dispatches an EH/signal table through that
address. This factory does not add exception dispatch or recreate original EH
data. A separate equivalent native exception-table/ABI implementation remains
outside this selected initial-locale profile.

The identity and supplied backing are retained through Seed, the first member
of DomainStorage::Impl; all other Domain members retire before that Seed.
Neither failed initialize nor successful close releases them while a root
alias remains. LastError is preserved during factory bookkeeping and final
backing retirement. A supplied token alone cannot prove API/handler executable
provenance or prevent explicit unload; omitted backing remains the caller's
lifetime obligation. Factory/Domain/SDK/handler code must stay mapped.

Existing Domain constraints remain: one FLS binding can exist at a time even
before native slot initialization; simultaneous construction is rejected by
the existing source. Controller operations belong to one serialized thread
outside loader lock. Close seals the Domain into Stopping before checking
provider leases. Retain it through provider retirement, native FLS release
retry or unknown-outcome poison; do not destroy live/poisoned roots or invent
rollback. Full CRT/EH/loader safety is not established.

The actual Resources test now selects this factory for its root instead of
fabricating a caller-owned byte-array identity. `factory_tests.cpp` covers
passive construction, all SDK preflight slots, single-use/second-root rejection,
retained SDK backing, actual controller PTD and memory allocator, known failed
initialization retry, provider-blocked close, failed native FLS release retry,
final PTD/heap/lock retirement and shared root aliases. Existing actual
three-worker Resources and concurrent PTD integration suites pass separately.

Two failed test receipts are retained: V1 assumed simultaneous Domain support;
V2 recorded fail-fast termination; the calloc-only test boundary reused for
malloc is a code-supported diagnosis, not a captured diagnostic stack. V3
corrected the own test boundary to accept the real malloc flag0 and passed
with the same production objects. See `factory.evidence.json` for exact hashes,
source evidence, qualifications and remaining production work.
