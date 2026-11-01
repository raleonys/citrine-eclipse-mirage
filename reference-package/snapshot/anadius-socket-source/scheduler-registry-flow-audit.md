# Scheduler service registry retained static flow audit

Scope: original normal procedures `0x180022fd0` and `0x180023b30`. Reviewer: `scheduler_registry_audit`. Complete retained pseudocode and assembly SHA-256 values independently verified. Original binaries, user configuration, tokens, achievements and real networking were neither executed nor read.

## Fixed key and first protected scan

`022fd5/022fe6` stores opaque scheduler RTTI descriptor identity `0x1800a2540` into key type; `022fe9` stores ID zero. The variadic argument spills do not add behavior to the selected typed key operation. These are identities, not callable host pointers.

`023b48` clears both local key qwords, then `023b55` invokes `022fd0`. Crucially, `023b5a` caches registry native `+0x28` owner-context identity **before** first `EnterCriticalSection` at `023b66`. The fixed type/zero ID locals are captured into RSI/RDI after entering at `023b75/023b7a`. A gate callback changing the registry owner must not change the cached constructor argument. This differs from the prior socket-service registry resolver's context handling.

`023b71` reads the live registry head at native `+0x30`. A candidate's nonzero ID and requested nonzero ID can match at `023b84..023b95`; requested ID is always zero in this procedure, making that branch unreachable on the fixed-key normal flow. For each nonnull entry type, `023ba9/023bad` passes requested descriptor+8 and entry descriptor+8 to required CRT comparator `04d2d0`, entry on the left. Only EAX zero matches (`023bb6/023bb8`); no nonzero ordering result matches. Missing type skips comparison. `023bbe` reloads entry next **after** the comparator, permitting callback-visible live link changes. First match preserves current RBX, leaves the gate at `023c9c`, and returns that cached entry at `023ca2`.

## Miss allocation, construction and second protected scan

First miss calls Leave at `023bca`, then requests exactly `0xb0` bytes from mandatory allocator `04b0bc` at `023bd5/023bda`, outside the gate. A nonnull allocation invokes required `024990` at `023bf8` with allocation, cached pre-enter owner, `0xffffffff` and byte one. **The constructor's return RAX**, copied to R14 at `023bfd`, supplies the candidate; it must not be replaced with the original allocation identity. A null allocation skips construction (`023be7/023c02`) and flows to a native null write at key copy. A selected source guard before second Enter may expose that unsupported fault domain without inventing a valid null return or a successful native allocation-failure behavior. A null constructor return has the same boundary.

`023c05/023c0a` copies the original two-qword fixed key into candidate native `+0x08/+0x10` **before** second Enter at `023c12`. No next-pointer initialization is established before this Enter. The second pass starts at the new live head (`023c18`), uses the same zero-only RTTI comparison (`023c42..023c51`), and reloads live next after comparison (`023c53`). Thus a competitor created during unlocked construction can win.

## Publication, duplicate deletion and cached return

On second miss, `023c5c` reloads live head again, `023c60` writes candidate next to that head, and `023c64` publishes candidate as registry head. Using an earlier pre-scan head snapshot would lose comparator callback updates. Leave is called at `023c6b`; the candidate cached in R14 is returned even if that callback changes registry head.

On second match, `023c87..023c95` invokes candidate vtable first slot with flags exactly one **while the registry gate is held**. Winner RBX remains cached across the deletion effect and the subsequent Leave. The deleted candidate is not returned, and winner selection must not be repeated after callbacks. Scalar deletion is an explicit provider effect; host virtual dispatch does not prove native deleting-destructor implementation or ownership ABI.

## Typed source composition and proof limits

A selected shared `ServiceRegistry` can hold socket and scheduler entries without a second owned native graph. A scheduler construction effect may expose allocation and `024990` as required callbacks. An explicit registry-identity binding and entry-to-scheduler-identity table can connect the real resolver body to the existing socket constructor. Those adapters recover no extra original procedure and must not eagerly allocate, own borrowed entries or manufacture native pointer/ABI equality. Native allocator/deallocator, scheduler constructor `024990`, CRT RTTI comparator `04d2d0`, deleting destructor, registry teardown, lock exception/SEH/unwind cleanup, dispatcher/executor and DLL entry/export ABI remain unproven. Original DLL equivalence is not established. CRT implementation remains deferred.

## Retained evidence

| Procedure | Pseudocode Evidence | Assembly Evidence |
| --- | --- | --- |
| `0x180022fd0` | `ev_1ff0395466a0d3686d30b1880c706f664cb34e113230aabead84f67e5dfc1ac5` | `ev_c66e0df033cecbf820b73605aa1591d589004974c15b6c62ae45785d57699bb8` |
| `0x180023b30` | `ev_e19c4187c6fd93eed1f7799ea815833381b3dbe02527496c06be2e73f9fd1811` | `ev_5bd06ce10f718331325ac29108e8afe82981dcdd81da85e580033f641b8dae7c` |
