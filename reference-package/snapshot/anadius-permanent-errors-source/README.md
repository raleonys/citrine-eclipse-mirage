# Explicit permanent error policy

The native SDK callback and Bootstrap bindings retain `last_errors` pointers
after their Context/Bootstrap owner closes. A policy scoped to Runtime cannot
meet the contract for a later cached entry. `compiler_abort_policy()` returns
stable typed references to one concrete implementation of both error interfaces:

```cpp
const auto errors = anadius::permanent_errors::compiler_abort_policy();
host.permanent_errors = &errors.host;
auto runtime = anadius::initial_cells::make_fresh_runtime(
    resources, errors.bootstrap, host_api);
```

Select explicitly before publication and outside loader lock. Nothing replaces
caller policies implicitly. This is a new HOST choice, not recovery of the
original error/exception ABI or behavior. Every lifecycle/callback error calls
compiler `std::abort`; there is no successful SDK result, exception rethrow,
cleanup, provider/stream access or retry. Terminal policy storage/construction
has no heap owner. No claim is made about allocation or internal behavior in
the compiler CRT's abort/signal implementation.

One-time placement construction uses aligned trivial static byte storage.
The object is never automatically destroyed, deleted or reset; both interface
references use normal adjusted base conversions. It captures no Runtime,
Context, configuration, stream, Domain or dynamic provider state. Storage
outlives those owners, but code/vtables/CRT remain in the provider image:
the caller must keep that image mapped while any entry can still be attempted.
This does not prove loader/process/CRT teardown safety or foreign quiescence.
Ordinary pre-binding calls retain the existing wrapper fallback behavior.

`errors_tests.cpp` checks typed identities across threads and after actual
Context/RuntimeOwner destruction. Twelve isolated own child routes cover both
interface methods, actual duplicate-binding failures, caught callback failures,
cached post-close entries and cached entries from ordinary `atexit` callbacks
registered before policy selection. The terminal handler checks that both
native/bootstrap active-call counters have been released; these are controlled
source tests, not original process execution or loader/CRT teardown proof.

Actual Resources integration explicitly chooses both permanent policies for
the existing three-worker/eight-request Runtime path. Source provider policy
selection, Domain seed/relocated exception identity, real foreign IO drain,
host/engine code lifetime and loader attach/detach remain separate obligations.
See `evidence.json` and `work/permanent-errors-v1-results.json` for exact scope.
