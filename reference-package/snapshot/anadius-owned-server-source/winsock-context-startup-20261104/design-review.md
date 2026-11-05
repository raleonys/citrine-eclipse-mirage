# Minimal shared Winsock/context design review

Planning only, 2026-11-04/05. No implementation, build/test/link, DLL/process execution, packaging or native query by this reviewer. The completed19-record native-findings report has now been read; this note is a proposal, not approval to mutate the59-source baseline.

## Concrete ownership/API proposal

Keep all common primitives in existing `anadius-owned-server-source/server.hpp/.cpp`; no new common build unit or provider registry is necessary. Provisional names:

```cpp
enum class ContextProfile { Host, Original };
struct WinsockImports { /* immutable typed WSAStartup/WSACleanup pointers */ };
class WinsockModuleState; // passive construction with explicit imports for tests
WinsockModuleState& native_winsock_module() noexcept; // stable, passive default
void acquire_winsock(WinsockModuleState&, bool& held);
std::int32_t winsock_status(WinsockModuleState&); // checked023f30 atomic read
void release_winsock(WinsockModuleState&, bool& held);
```

One actual module state represents A3900's reference count and A3904's cached status, using host-owned storage rather than original addresses. Imports are fixed at construction. Its default instance has static/module lifetime, passive zero initialization and no SDK-performing common TU constructor. Native count/cache ordering must follow the fresh report; do not add a busy rejection, retry loop, timeout, TLS selector or arbitrary new cleanup framework.

The per-owner held bit is committed immediately when the native increment occurs, before startup can fail; nonzero cached status does not undo the increment. A checked context explicitly reads the cached status and can throw its native system `winsock` diagnostic while retaining the exact acquired prefix. The unchecked001310 path only acquires and must not gain an invented cached-failure check. A release consumes its owner's claim once; the last decrement invokes cleanup and ignores its return, in native order. The bit prevents caller-side replay of a completed release without inventing native EH cleanup. Unknown injected C++ exceptions do not authorize guessed rollback or retry.

Append `ContextProfile context_profile=Host` and a borrowed `WinsockModuleState*` slot to existing Dependencies. HOST requires a null native-state slot and continues its current per-Core2.2 KernelImports behavior. Original requires a nonnull explicit state reference. **Do not make Original+nullptr silently choose the default global**: that would hide an omitted lifetime/selection, mix fake KernelImports with real global SDK state and accidentally share unrelated HOST fixtures. The production selector supplies `&native_winsock_module()` explicitly. A test may supply a separate immutable-import state whose owner is retained in Dependencies.state_lifetime, or keep a direct primitive-test state alive until all claims end. Never retain a pointer to a temporary fixture state through a live/failed Core.

## Actual original image first owner

`dll-link-gate-source/entry.cpp` is compiled separately for the DLL target, while common server.cpp is also linked into many HOST test executables. Put the actual001310 initial claim **only in that DLL-only entry TU** using normal compiler-CRT initialization. A dedicated dynamic initializer should register its release callback through the compiler atexit mechanism **before** acquiring the shared claim, matching the newly observed001310 order. The callback and its held flag bind the exact same default module state used by contexts. Initial unchecked cached failure is retained; later checked contexts report it. The full report confirms unchecked acquisition here; registration-result failure handling beyond the observed call remains bounded.

The full report confirms registration-before-acquire and unchecked001310 behavior, plus its membership at7f668. It explicitly does not establish complete original CRT dispatch timing. A DLL-only compiler-CRT initializer is the selected source integration, not proof of exact native chronological ordering against every other initializer. Do not infer that chronology from0241b0's local gate-before-acquire sequence.

Do not rely on a C++ global object's compiler-generated destructor-registration timing if it registers only after the constructor returns: the observed registration-before-startup order should be explicit inside the dedicated initializer. Do not invoke recovered outer CRT, manually call original initializer addresses, duplicate the claim in Image.start, add a DllMain detach join, or tie native release to cooperative graph quiescence. Existing application reason0 behavior remains separate from compiler-CRT exit callbacks.

The alternative of acquiring in Image.start is later than real CRT001310. It avoids common-library side effects but does not reproduce the proven initializer timing, so it is not the preferred native selection. An SDK-performing global in common server.cpp is unsuitable: linking any owned-server HOST fixture would acquire WSA before main and alter its import counts, default lifecycle and failure prefix even when it never selected Original.

## Files and context sequencing

- `server.hpp/.cpp`: explicit profile/state slot and passive typed primitives; one Native Core claim flag; actual registry-gate storage/live flag. Native initialization must prepare that registry gate before shared023f30 acquire/check, then proceed to scheduler/service initialization. Existing Gates.initialize currently recognizes only scheduler/service sections and must recognize the exact new registry section. Native gate failure precedes WSA acquisition. Failure after acquisition keeps the increment and completed gate prefix; no constructor/dtor rollback is invented.
- `server.cpp` retirement: native shared release must precede registry teardown and ignore cleanup's result. Existing HOST retirement checks per-Core cleanup after port/gates; preserve that branch. The precise registry/service/scheduler deletion order must use the full native report rather than assume the new gate is interchangeable with the existing two gates.
- `anadius-resources-factory-source/factory.cpp`: configure_native_worker commits Original plus the explicit default module reference passively, alongside its current native adapters. Its fresh-input checks must include the new slots so it cannot overwrite a caller's prior context selection. Selecting only native IO or only OriginalApplication lifecycle must not implicitly select this context profile: existing HOST tests deliberately use those compositions too.
- `entry.cpp`: dedicated compiler-CRT first owner and release registration as above. Graph and ApplicationWorker already pass server Dependencies through; they need no inferred lifecycle-to-WSA rule. Existing build registration changes are unnecessary if these primitives stay in server.cpp and the image initializer stays in entry.cpp.

## Bounded validation and remaining evidence

Future primitive source tests can inject a static typed SDK pair to observe WORD2, only0→1 startup, cached-success/failure sharing, failure with retained count, last-only cleanup with ignored nonzero return, and separate image/context held flags. Constructors/getters/configure selection remain passive. Actual Native backend tests should cover registry-gate-before-startup and release-before-registry cleanup with the chosen explicit state. Existing HOST2.2/two-gate tests retain their meaning; they must not become native tests by accidental module initialization. No fixture should count compiler-CRT startup in an executable that never links the DLL-only image initializer.

The completed report confirms023f30's increment/first-only WORD2/startup-result exchange and checked atomic cached read, the `winsock` literal, normal029ea0 socket-destruction→shared-release→registry-teardown ordering, and the registered07df30 last-release cleanup tail jump. Complete CRT dispatch timing, registration-failure treatment beyond the observed call, native EH and concurrent-first-initialization completion behavior remain bounded. Do not add a wait/retry merely because a concurrent checked reader can precede first-owner status publication in the measured body. No new unload condition or whole-DLL equivalence claim follows.

Reviewed unchanged planning inputs: server.cpp `7aa4b341b8ef7e96e62833f320f5d30ad43386aa869155a4275e28a7b5bd2343`; server.hpp `b847284e3498e43e34213ec84125197111bd27aff3d3aa9ffcaf16db540e98f1`; resources factory.cpp `96b302618650373492e80c3a47d1cba2e645c24f11e5f4802fffe477992c4919`; DLL entry.cpp `66e79d07847ca62bbf577bb9595e106baf3fa8ea60ef3e15a1bf3140f48f8299`.
