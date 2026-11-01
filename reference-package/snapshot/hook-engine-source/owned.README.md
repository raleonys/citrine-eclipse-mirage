# Selected owning engine and cleanup proof

`OwnedEngine` owns one selected MinHook-compatible Api and its actual supplied
code backing. The lower Api is a unique owner, so it is destroyed before the
code owner. Use the consuming factory for allocation-failure ordering:

```cpp
std::unique_ptr<recovered_hooks::Api> lower = /* selected engine */;
std::shared_ptr<void> code = /* actual mapped-code/backing owner */;
auto engine = recovered_hooks::make_owned_engine(lower, code);
// On valid consumption both donors are empty, including on allocation failure.
host.external_quiescence = /* independent hook/source exclusion proof */;
anadius::host_platform::select_owned_engine(host, engine);
```

The lower engine must be fresh/uninitialized and exclusively used through this
wrapper, including the same per-image upstream global state. Allocating a new
NativeApi object does not reset MinHook globals. For actual NativeApi each DLL
must link its own compatibility copy, as required by the existing Api contract.
These tests use controlled lower engines only; no real hooks are installed.

Construction/selection is passive. Controller operations call the exact lower
method with unchanged arguments/results/output-on-error. No metadata mutex
spans lower calls or owner deleters. Operations mark state as needing cleanup
before opaque lower entry. Only normal uninitialize MH_OK0 or
MH_ERROR_NOT_INITIALIZED2 establishes upstream logical empty state. All other
normal statuses remain visible and retain the cleanup obligation. Successful
disable of individual targets alone does not establish emptiness.

The vendored MH_Uninitialize implementation checks g_hHeap and disables all
hooks before clearing logical state. It ignores native HeapFree/HeapDestroy
BOOL returns, so this wrapper does not claim successful physical heap release.
This is the selected upstream compatibility behavior, not original native
engine equivalence. Unknown lower exceptions or swallowed managed reentry
poison retention, with no guessed rollback. Live/poisoned owner destruction
fails stop rather than silently uninitializing an engine. Clean retirement
keeps metadata available in Retiring while lower/code owners are destroyed.

Ordered local ownership protects constructor-body failures. The borrowing
factory additionally protects facade, Impl and shared-control-block allocation
failures; direct forwarding callers must retain code through their own outer
allocation/argument prefixes. No automatic cleanup operation is invented.

`select_owned_engine` publishes the same Engine owner as AssemblyInputs.engine
and code_lifetime. It composes the existing external predicate with actual
engine-empty status, checking again after the opaque predicate returns. False
engine cleanup cannot become safe merely because the external bool is true.
The predicate still must independently prove excluded/drained sources.

All trampoline/detour/original users must be drained BEFORE uninitialize.
This later engine-empty/external check cannot retroactively establish that
earlier drain. A code owner retains only what it actually owns; neither an
arbitrary token nor ownership of NativeApi itself pins image/vtables or stops
explicit engine/global calls through borrowed aliases. Image lifetime, no
self-unload, external drain and loader integration remain caller obligations.

Actual Resources integration now exercises failed uninitialize status10 with
external proof true: native Context remains bound, retry close does not replay
source cleanup, an explicit controller uninitialize retry after source drain
establishes empty state, and external false still prevents Context closure.
Known cleanup repair is explicit; source library/online/hook ordering is kept.

See `owned.evidence.json` for exact receipts. V1 test compilation failed on
Windows/Winsock include ordering; WIN32_LEAN_AND_MEAN corrected the new header.
V2/V3 recorded successful normal-path suites but did not cover constructor
allocation failures. Independent review identified that ownership flaw; V4
adds the ordered factory, Retiring gate and three deterministic allocation
failure cases. Historical results are not overwritten or promoted beyond
their tested scope.
