# NativeProfile per-message invocation foundation

Source-only implementation; no compiler, project test, linker, DLL execution or packaging was used. Actual Original Profile/Graph/NativeJobs composition remains unchanged and does not yet enable concurrent message admission.

## Exact source authority

| File | Before SHA256 | Final SHA256 |
| --- | --- | --- |
| `outputs/anadius-native-profile-source/native.hpp` | `fbf8c0e31f5df3479ce8a15e550194443750416f52ee30843a57a096f11958fa` | `ca43abd9f9fe72250f9ad60dc435603c5df2b79c3ad4a7a8971d62e53e425239` |
| `outputs/anadius-native-profile-source/native.cpp` | `750f958bdc0d8209bafbc98f00325b8081892eb10ae0fe2ed63d8c3ccb933758` | `ae98f0527ddf2f50d61223d182834e0a4ea2e591f0447963cb808be7f60d2535` |

The root-captured before files are in this stage's `before` directory. Root owns the eight fresh instruction records and native evidence index; this author did not open or alter a REA session.

## Implemented contracts

`process_native_call(parent, strings, output, request, const Dependencies&, checkpoint)` supplies immutable per-call providers. The legacy typed method forwards constructor defaults without changing them. Every XML, random, temporary cipher, effect, event and version access uses the invocation-owned dependency copy. The native context's show-LSX field is a **reference** to that copy's scalar, so the deferred copy updates the actual selected flag. Session cipher access continues to reference the actual shared Session owner; arbitrary concurrent cipher/provider mutation is not certified.

A stack, nonallocating Admission is published before invocation/list/dependency metadata allocation. It reserves the exact thread/fiber and external string prefixes, catches same-actor allocator-handler reentry and rejects prefix aliasing. Explicit Parent admission and exact live Session/control ownership are required before the first Parent/policy binding. Serialized remains the default and preserves single-invocation rejection; the explicit Concurrent Parent can admit independent actors on this same Logic.

Stable list-owned Invocation records separate healthy Preparing/Running/Retiring calls from Failed retained prefixes. `retained_prefix()` preserves its historical broad HOST meaning; `failed_prefix()` and `active_calls()` expose the distinction. No registry mutex spans native processing, allocators, checkpoints, provider callbacks or target destruction. New rejection exceptions are constructed after unlocking. Added registry metadata operations locally preserve LastError, without wrapping source SDK effects.

Each Invocation's private composite byte resolver covers only output/request inline arrays and its own response/temporary arrays. All other requested windows go to the exact admitted Parent resolver. Identical physical Parent/private windows deduplicate; overlapping/conflicting private mappings reject. No whole Parent catalog or event-slot collection is copied/scanned, and unrelated payloads are not pinned by refresh. Existing recovered 009eb0 directly copies an SSO event array; heap event copies already use the exact memory view.

Opaque results are retained before proxy allocation/postchecks. Normal object retirement, checkpoint-target retirement and per-call dependency destruction happen while the call is still published, outside the metadata gate, with subsequent exact actor/registry validation. First unknown failure retains that call's actual partial prefix; no guessed rollback or second cleanup is added. Logic's final default-provider destruction retains a retirement seal and exposes no active invocation, preserving its historical destructor callback contract.

Output is native return storage: the caller supplies a fresh/reset prefix or releases its previous heap. Recovered 033d70 overwrites it; this implementation does not invent prior-output cleanup.

## Validation and remaining integration

Manually traced the original typed 033d70/proxy/control paths, all chosen-provider accesses, short-lock/reentry paths, private resolver overlap logic, and normal/failed retirement. Root authors core test sources and the SDK agent authors the real PreparedMemory two-child fixture; execution is not authorized. No runtime passing claim is made.

This is a disabled foundation. Coupled Profile/Graph/provider invocation state and NativeJobs body admission still need integration before the actual DLL can claim the measured same-session or distinct-session progress behavior. It adds no native DOM/string/EH ABI completion claim, no ambient TLS authority, no readiness barrier, no shared-provider thread-safety guarantee and no new unload condition.
