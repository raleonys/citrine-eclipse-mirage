# Socket service constructor retained static flow audit

Scope: original normal procedures `0x1800228e0` and `0x180027be0`. Reviewer: `service_constructor_audit`. Both complete retained listing SHA-256 values were independently verified. The original DLLs, game executables, user configuration, tokens, achievement stores, sockets and networking were neither executed nor read.

## Allocation and partial service state

`0228ef` caches the original context argument in RBX before the allocator callback. `0228f2/0228f7` requests exactly `0x80` bytes from mandatory `04b0bc`. A null return follows `022906/022909` to `02296e`, returning null with no field initialization, scheduler lookup, mutex setup or category callback. Native allocator internals and throwing allocation paths remain unproven.

On a nonnull live allocation, the stores occur in this order: `02290b` clears native `+0x08`; `02290f` clears `+0x10`; `022913` stores the cached original context at `+0x18`; `022917` clears next at `+0x20`; `02291b/022922` sets base table identity `0x180084590`; `022925` stores the same cached context at `+0x28`. These identities are opaque values, not host executable addresses or native vtable overlays.

Only then `022929` reads **live context[0]** and `02292c` calls mandatory scheduler provider `023b30`. The context object's registry link can have changed during allocation, so it must not be captured before allocation. The original context identity itself remains cached. `022931` stores the returned scheduler at `+0x30`, then `022935/022939/02293d` clears the three native fields at `+0x38/+0x40/+0x48`. A callback observing partial construction must therefore see the preexisting values of these latter fields until it returns.

## Gate helper and category ordering

`022941/022945` invokes `027be0` on native critical-section storage at allocation `+0x50`. The helper sets EDX to **`0x80000000`** at `027be4`, then calls imported `InitializeCriticalSectionAndSpinCount` through `0x18007f0f0` at `027be9`. Any nonzero BOOL, including a negative one, branches from `027bf1` to `027bfb` and returns zero **without GetLastError**. Only BOOL zero calls imported `GetLastError` through `0x18007f0b0` at `027bf3`; it returns the resulting DWORD unchanged, including zero. There is no synthesized nonzero failure code.

The constructor caches the helper's 32-bit result in EBX at `02294a`. **`028620` is called unconditionally at `02294c`, on both success and failure**, before the status test. The local error pair is formed from cached status at `022951` and returned category identity at `022955`. A category callback must not cause status to be reread from a mutable storage field.

Nonzero cached status follows `02295c` to `022984`, loads opaque `mutex` message identity `0x180084428`, and invokes required nonreturning `028240` at `022990`. No `+0x78` head initialization or final table write occurs along that path. The function listing establishes neither constructor unwind cleanup nor a native deleting destructor. Adding successful return or automatic rollback after the raising effect would overstate original behavior.

Zero cached status, including initialization BOOL zero followed by GetLastError zero, follows the success path: `02295e` clears the native socket-list head at `+0x78`; `022962/022969` installs final service table identity `0x1800845b0`; `02296c` returns the allocated service. The head is cleared **before** the final table store and after the category callback.

## Source composition and explicit proof boundaries

Typed host objects may expose the observed fields, a borrowed SDK critical section and a service projection while preserving callback order. They are not evidence of native 0x80-byte memory layout, native object lifetime, native vtables or allocation ABI. Required allocator, scheduler `023b30`, category `028620`, error raise `028240`, deleting-slot effect, and CRT RTTI comparison can remain explicit provider boundaries. A selected registry factory can invoke the real reconstructed `0228e0` body without claiming recovery of those provider internals.

The native initialization wrapper may bind imported SDK functions, but tests must supply controlled imports and must not execute original code or create real OS/network state. Only two original normal procedures are counted. Adapter methods, typed model constructors and host projection helpers add no original procedure count. Native allocator/deallocator, scheduler/dispatcher, exception/SEH/unwind, duplicate service destructor, registry teardown and entry/export/DLL ABI remain unproven. CRT implementation remains deferred.

## Retained evidence

| Procedure | Pseudocode Evidence | Assembly Evidence |
| --- | --- | --- |
| `0x1800228e0` | `ev_22c4bffe54fe0b986545326a4020802e4e98041a480e8cafc64b603ecd05d1ef` | `ev_cccd315f694dc43aeae87a61fc8912482989a03586a119141e1b2a2d8f2bdd7b` |
| `0x180027be0` | `ev_948db36a6b6838994310cbdd3ff325ea9d6c875b61a63f92c9c0d1ce03fdbe85` | `ev_ebd0f0654f2c3bac442e8d0023199cc155873bebe20cb19ded69b206149bb469` |
