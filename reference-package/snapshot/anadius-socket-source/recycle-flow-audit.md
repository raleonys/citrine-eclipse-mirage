# Independent recycler flow audit

Scope: retained static evidence for `anadius64.dll`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. No original binary execution, live network calls, or user configuration/achievement contents were involved. This is a normal-flow reconstruction contract for initialized borrowed data, not a native ASIO memory-layout or exception-ABI claim.

The full pseudocode and assembly listing hashes were independently checked: `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb` and `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

## `0x1800283c0`: bucket removal and recycle-list transfer

Evidence: pseudocode `ev_9f53a74fdc3280f360d1ce5770b00ecf0d27721d1f88cc316eeb303a6ea950b8`; assembly `ev_1c51eada2fac2491b445eaad1ca3641ce117107dbab6a3a71dd6a7392dc453bc`.

The native map has map size at +0, active-list sentinel pointer at +8, active-list size at +0x10, free-list sentinel pointer at +0x18, free-list size at +0x20, bucket array at +0x28, and bucket count at +0x30. These are evidence offsets, not permission to overlay a typed C++ object onto the native structure. A node has next +0, previous +8, and 64-bit socket key +0x10.

- `0283cc..0283e4`: divide the unsigned 64-bit node key by the map bucket count; select the two-pointer bucket at remainder*16. A zero bucket count is outside the initialized-input domain; unlike the caller, this function contains no zero-count guard.
- `0283e8..028407`: if node equals both first and last, set bucket.last to the active sentinel, then set bucket.first from bucket.last. The reload/order is explicit in assembly.
- `028409..02840f`: if node is first but not last, set bucket.first to the old first node's next.
- `028411..02841a`: if node is last but not first, set bucket.last to the old last node's previous. For an interior node, no bucket pointer is changed.
- `02841e`: clear the node key to zero **before** any free-list length failure is checked. The preceding bucket mutations also survive that failure.
- `028426..028434`: load freeSentinel, capture freeSentinel->next as freeFirst, and capture node->next as after, **before** the failure check. These captures must not be moved after an effectful provider call.
- `028437`: compare the addresses of inline fields map+0x18 and map+8. These are distinct within every valid map object, so the self-splice branch `02843c..028446` is unreachable. Comparing the sentinel pointer values instead and skipping counts when they are equal would invent a reachable branch.
- `028448..02845d`: unsigned `(0x0666666666666666 - freeSize) < 1` is true **only for equality**, even when freeSize is greater than the constant because subtraction wraps modulo 2^64. It must not become `freeSize >= max`.
- `028495..02849c`: on that exact equality call nonreturning `047fc4` with token `0x1800846b8`. A required length-error effect may represent this call; silently returning would invent behavior. No counter or list-link mutation has occurred yet, but the key and bucket changes have occurred.
- `02845f..028467`: increment free size modulo 2^64, then decrement active size modulo 2^64.

The splice at `02846b..028488` has six stores whose order and intervening snapshots matter. With `after` and `freeFirst` already captured, execute:

1. Load `previous = node.previous`; store `previous.next = after`.
2. Load `afterPrevious = after.previous`; store `afterPrevious.next = freeFirst`.
3. Load `freePrevious = freeFirst.previous`; store `freePrevious.next = node`.
4. Store `freeFirst.previous = afterPrevious`.
5. Store `after.previous = previous`.
6. Store `node.previous = freePrevious`.

This transfers the node to the head of the free circular list, immediately before the captured old freeFirst. In valid disjoint circular lists, afterPrevious is the removed node and freePrevious is the free sentinel: stores 2 and 3 set node.next=oldFreeFirst and freeSentinel.next=node. Combined with host reuse that removes freeSentinel.next, recycling/reuse is LIFO, not FIFO; the cancellation operation FIFO is a separate structure. The explicit order also defines the routine's behavior for aliases that stay within live initialized link storage; replacing it with a generic detach/append helper can alter reads after earlier writes.

`02848c` decrements map size modulo 2^64 **last**, after the link stores. The routine does not allocate or free node storage, zero an operation FIFO, invoke the scheduler, post completions, or wake a socket. A host-owned container can supply storage and insertion/reuse separately, but those helpers are reconstruction design and add no recovered original procedures.

## Caller and remaining boundaries

At `026b50..026b9c`, caller `026aa0` drains the matching node's pending operations into its aggregate FIFO. At `026b9e..026ba4` it calls this recycler, including when the matched FIFO was empty. Scheduler delivery happens later. Caller evidence: pseudocode `ev_d4f71cf8b783578dcbaae1a45c68bba3e631d5381f8102b4c3d6ec82db60b419`, assembly `ev_5263c2b7208ef28e3ec671a729ee180d773d5dcd6ba45d16972e7ebef04cba21`.

Exactly **one** original procedure is newly in scope: `0x1800283c0`. Original hash-map construction/insertion/rehash/allocator, native ownership/layout, CRT length-error construction/unwind, concurrency/lifetime, DLL entry/export wiring, and live IOCP behavior remain unproven. Typed borrowed state and explicit host-owned storage do not establish those properties.
