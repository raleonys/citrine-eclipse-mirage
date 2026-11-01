# Selected online module reference

`make_code_lease()` supplies the existing `online_factory::Options::code_lease`
contract. The default SDK calls `GetModuleHandleExW(FROM_ADDRESS)` on the exact
loaded base, without PIN or UNCHANGED_REFCOUNT. Each successful lease acquires
one additional reference; shared aliases release it once on final retirement.
`FreeLibrary` releases only this additional reference. The normal source load
and source free remain separate operations with their own reported outcomes.

```cpp
#include "reference.hpp"
options.code_lease = anadius::module_reference::make_code_lease();
// options.foreign_quiescent remains an independently supplied obligation.
```

Both shared-record allocations precede acquisition. Known acquisition failure
throws `std::system_error` with the SDK error and leaves no reference to release.
Bookkeeping and reference retirement preserve the incoming LastError. Injected
SDK backing outlives the factory while any lease remains. Nonconforming success,
nonconforming failure, throwing SDK callbacks or failed final release fail-stop;
there is no speculative retry/unload or claim that a failed release succeeded.

Use only outside DllMain/loader lock, with an existing stable executable module
reference and acquisition serialized against its source free. The provider and
all return/deleter code must live independently of the selected target; this
does not implement last-reference self-unload. No in-module object/code may be
used after its lease retires. Callers still prove forwarded export/vtable code
provenance and drain all asynchronous/foreign users BEFORE object destruction
and source FreeLibrary. A late finality predicate cannot establish that earlier
drain. Host hook-engine lifetime and loader attach/detach remain separate gaps.

`reference_tests.cpp` loads ONLY the adjacent self-generated `fixture.cpp` DLL.
It observes actual Windows retention/unload both directly and through the
existing Binding's version/factory/object/query/destruction/free path. Controlled
SDK tests cover reference aliases, failures, error preservation, backing order,
unlocked Binding retirement and six isolated fail-stops. Resources integration
selects this provider with injected SDK boundaries; it does not execute original
target code or prove arbitrary real foreign code ownership.

Platform contracts: [GetModuleHandleExW](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandleexw),
[FreeLibrary](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-freelibrary).
This is new source ownership plumbing, not a newly decompiled native function.
See `evidence.json` and `work/module-reference-v1-results.json` for exact scope.
