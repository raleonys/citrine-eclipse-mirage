# Socket service constructor frozen source review

Result: **PASS**, no material findings in the four frozen production files under their documented initialized, exclusively writable object and stable borrowed lifetime domain. Reviewer: `service_constructor_audit`. This was an independent static review against the retained complete pseudocode and assembly. The reviewer executed no tests, original DLLs, game executables, SDK calls, sockets, IOCP or user files.

## Reviewed files

| Source | SHA-256 |
| --- | --- |
| `outputs/anadius-socket-source/service_constructor.hpp` | `3e03015e00752ec4f3020bdaa6559eb89e3c311b25877198a296361ef4a19365` |
| `outputs/anadius-socket-source/service_constructor.cpp` | `e4125fb99d7cbf61c29fd82ac5b7027f5face09a92e77890c66da96a8ec74413` |
| `outputs/anadius-socket-source/service_constructor_native.hpp` | `4dfe0aebd535f22fd9cf264c13508055b79bcbaab62aaf9c8bcec2b6bdb24702` |
| `outputs/anadius-socket-source/service_constructor_native.cpp` | `9bc4cac66954bcb9583fea697cfa1e9ebae2daac3284a5ac8cd33781495a19aa` |

## Original normal source behavior

`construct_service_0228e0` copies the context identity before calling the required allocator with literal request `0x80`. Its null result returns immediately with no later field writes or callbacks. The host allocator's already-live dynamic object contract is explicit; this does not allocate or reinterpret a native 0x80-byte object.

For a live result, the source writes key type zero, key ID zero, owner context, next null, base table identity `0x180084590` and implementation context in the assembly order. It then reads the live context registry identity for required `resolve_scheduler_023b30`, stores the result, and only afterward zeroes the three words corresponding to native offsets +0x38/+0x40/+0x48. Partial construction observed by scheduler callbacks therefore retains the required store order. The opaque table fields never become executable host pointers. The caller-bound `completion_port` projection is left unchanged; it is not invented as an original constructor field.

`initialize_gate_027be0` passes the borrowed critical section and exact spin value `0x80000000` to the required initialization effect. Any nonzero int32 result returns zero without the last-error effect. A zero result delegates last-error and returns its uint32 value unchanged, including zero. The constructor calls the existing category provider unconditionally after this helper, retaining a local const uint32 status across that callback.

On nonzero status, the required nonreturning raising effect receives the exact 32-bit error bits through `std::bit_cast<int32_t>` and the returned category, together with the selected literal context `mutex` corresponding to native identity `0x180084428`. The head and final table stores are after this branch and do not occur when the raising effect respects its required nonreturning contract. Native error-object internals and exception/unwind cleanup are not claimed. On zero status, head null precedes final table identity `0x1800845b0`, then the cached allocated entry returns. This includes the original unusual BOOL-zero/GetLastError-zero success case.

Only `0x1800228e0` and `0x180027be0` are counted as new original normal procedures. Model definitions, registry composition and SDK adapters add no original procedure count. There is no new RAII cleanup, successful failure fallback, allocator ownership, mutex deletion or native destructor implementation.

## Selected host composition and SDK wrapper

`SocketServiceEntry` is an abstract `ServiceEntry` subclass containing an existing `SocketService` projection and a borrowed SDK critical section. The allocator must supply a real live subclass implementing the still-required scalar deleting effect. This keeps native vtable identities separate from C++ dispatch. Stable section, service gate, entry and context lifetimes, a fresh unlinked allocation and serialized mutation remain caller obligations.

`ConstructorRegistryEffects` validates the configured context identity and delegates the prior registry's creation effect to the actual reconstructed constructor body. The CRT type comparator stays abstract and required. A null allocator result is faithfully returned by this constructor; the preexisting registry's explicit UnsupportedInput boundary for null candidate key assignment remains distinct from a native successful null lookup. The adapter creates no eager service and introduces no ownership or teardown. Existing registry/lifecycle/closing/cancellation/recycler source composition remains possible through stable entry/service bindings.

`ServiceGateImports` uses SDK-derived function pointer types for the two imports. `NativeServiceGateApi` merely stores those pointers on construction; it makes no SDK call until an override is invoked. `initialize` forwards the exact section address and uint32 spin value; `get_last_error` forwards and preserves the DWORD. Controlled injectable imports can exercise these wrappers without any real SDK initialization. Default imports are callable integration boundaries, not evidence that they were executed by the reviewer or that native object ownership was recovered.

## Remaining proof boundaries

Required native allocation `04b0bc`, scheduler `023b30`, category `028620`, raising internals `028240`, service scalar deletion, CRT RTTI comparison `04d2d0`, native constructor exception/SEH/unwind and cleanup, registry teardown, allocator/deallocator, dispatcher/executor and entry/export/DLL ABI are unproven. The host's logical object storage is not an ABI overlay. SDK adapters and source composition do not establish complete original DLL equivalence. CRT implementation remains deferred.

The supporting independent retained flow audit is `work/service-constructor-flow-audit.md` and `.json`. The reviewer edited no production, tests, build or project documentation files.
