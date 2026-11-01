# Queued write independent retained-evidence audit

Independent reviewer: `queued_write_audit`, 2026-10-23. Complete retained pseudocode and assembly for `0x18002a500`, `0x180025a70`, `0x180021fd0`, `0x1800261d0`, and `0x180027a20` were inspected. Exactly `02a500` is newly recovered in this stage. No original DLL/game/network execution, original modification, or real user configuration/token/achievement content reads occurred. CRT production remains paused and unchanged.

Target `anadius64.dll` SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

| Function | Complete pseudocode evidence | Complete assembly evidence | Counting |
|---|---|---|---|
| `0x18002a500` | `ev_74e713f6d63b5db2c8594c89965baf991c280377ff6f5607819938f99daa863f` | `ev_7b4a354660d536700feb53ad52ca0a2f961ca2042dbeb3a635143135bd1fe188` | One new normal body |
| `0x180025a70` | `ev_f7448c3646f8693c49a161e2449f559db548111bdf4c3a9cacfa5295f5e963b8` | `ev_55d30b85e7b6ed50c63ac8a363324c343cf0045dfef90f8fc377ed5e9e01b4d7` | Existing completion caller |
| `0x180021fd0` | `ev_cb8feee93f61ab1f79de415f7b58cf399d0febb87fbce65a9c6f749925ed1724` | `ev_6326ebb30e495078e0ed47d7346de01edd7f346608c3d5c91890ce2c1637533e` | Existing direct initiation callee |
| `0x1800261d0` | `ev_45b2f55e8f0ef15562ee5d5978c239a8bd7dbe150a53979f20633e3f28880e61` | `ev_5500451b02e60551ba95436e9c19673671b10b219573052f2e88f4d15d4a4e75` | Existing continuation context |
| `0x180027a20` | `ev_ac8acca2b33bd760e4ba5f3d9ec5784ef473a66bd1f78d4aeb47033d2bcbc4d9` | `ev_c7fd45e2d65c3e1f508706bdf6dc9e7561b9d896d56095bbebac7e82853f4463` | Existing eventual completion context |

## Empty queue and strong acquisition

`02a513` compares only owner count at `+e8`, and `02a51a` branches to return when zero. This branch must not resolve the control, queue map, stream, allocation environment or send operation, and must not call a provider. Stack/register initialization has no externally observable effect.

For a nonempty queue, `02a52c` caches owner control pointer `+8` in RDX. A null pointer (`02a530`–`02a533`) or zero strong low32 (`02a539`–`02a53e`, also the retry `02a54e`–`02a550`) reaches `02a67d`'s nonreturning `026860` call followed by INT3. This stage retains an explicit required terminal provider; it does not implement `026860` or manufacture a native C++ exception implementation.

`02a544`–`02a556` use a locked low32 compare-exchange loop on the **cached** control `+8`. Desired value is expected+1 modulo32, so nonzero `0xffffffff` can wrap to zero. A failed compare-exchange updates EAX to the actual current low32; its zero test and next desired calculation reuse that failed expected value. There is no saturation, high32 mutation, owner-control re-resolution, or silent success on an expired control. Actual race scheduling and native lifetime synchronization are not established by static inspection or deterministic synthetic tests.

Only **after** acquisition succeeds, `02a558` and `02a55b` reload owner object `+0` and owner control `+8` into RBX/RDI. These are the pair later copied to the handler, and can differ from the cached acquisition pointer in a controlled field-mutation model. An implementation that uses the cached acquisition pointer as the handler's control conflates two different native reads.

## Queue selection and string view

`02a573`–`02a58b` calculate unsigned64 `(owner.map_size_d8-1) & owner.offset_e0`, then select the string pointer from owner map `+d0`. No modulo by count, automatic deque advancement, or count recheck occurs here. The normal body requires coherent accessible projected map storage.

`02a58f` reads full64 string size `+10`. A zero size selects null data at `02a5a4`, without reading or depending on capacity/heap contents. Otherwise `02a598`–`02a59f` select the string's own first-byte address for capacity<=15 or the stored heap address for capacity>15. No allocation, string copying, string destruction or terminator read occurs.

## Handler fields and direct first initiation

`02a5a7`–`02a5da` construct only the defined native handler payload. In the shared logical `SendHandler` projection: words60..78 are stream identity `owner+10`, data token, full64 original string length, and progress0; dword80 is initial flag1; word88 is owner identity; owned object90/control98 receive the post-CAS reloaded pair. Native handler padding, vptr-like first word and undefined local bytes are not recovered or declared initialized.

`02a5e2`–`02a601` produce first buffer `(selected_data,min(full64_size,65536))` with zero progress; full length remains in the handler. `02a606`–`02a615` derive service from the stream's member0 plus28, implementation at stream+8 and allocator member **address** stream+68 (equivalently owner+78). `02a624` clears flags, and `02a62c` directly invokes actual existing `021fd0`. It does **not** call `0261d0`, even though that routine implements subsequent composed-write continuation. Calling the latter as a shortcut would add different entry effects and bypass this exact call boundary.

Native owner/member addresses are opaque identities in the reconstruction; typed stream/owner/string/reference providers must maintain one coherent projection. This is not a native object layout or vtable ABI claim.

## Ownership transfer and residual local release

Existing `021fd0` copies owned object/control into the operation at `0220b7`–`0220c6`, then clears the source handler's owned pair at `0220cd`–`0220d1` **before** `023dd0` at `0220e7`. Thus an ordinary completed initiation leaves the local owned control null; it must not be decremented again by `02a500`. The call can publish completion and end owner lifetime. There are **no owner reads after** `02a62c`: native `02a632` reads only the local handler control.

If the local residual control is nonnull, `02a63f`–`02a64e` atomically fetch-sub1 on its strong low32. Only old1 invokes its disposal slot0 at `02a656`; then `02a658` decrements the **same saved control**'s weak low32, and only weak old1 invokes its live slot8 at `02a668`. A disposal callback can change fields/vtable, so saving the delete callback before disposal is incorrect; selecting the control again from owner/source after disposal is also incorrect. Strong old0 wraps to `0xffffffff` and skips disposal; weak old0 wraps likewise and skips deletion. No host shared_ptr semantics, additional reference reset, guessed cleanup or error recovery is substituted.

Required effects that throw are modeled only through their completed normal prefix. This stage adds no native unwind/SEH/exception cleanup. Valid providers must keep the saved control alive through its last native access, even if disposal ends the owner's lifetime.

## Existing closed source chain and outstanding boundary

Existing `025a70` calls `02a500` at `025b22` or `025b36` after successful string cleanup and queue advancement, reloading its owner pointer just before that call. The new bridge can close that prior next-write provider with the actual recovered normal body. Existing error/logging/deallocation helpers and category/table projections remain required where previously required. Eventual completion `027a20` moves the operation's owned pair into its local handler (`027af5`–`027b14`) before invoking dispatch and applies its existing post-dispatch reference release (`027b62`–`027b94`); none of these context bodies is newly counted.

Exactly one new normal original body is counted. `026860`, receive `025c40`, native factories/tables/ABI, original allocation storage, thread races, exception/unwind/SEH and full DLL equivalence remain outside this claim. CRT production is unchanged. No original file was loaded, executed or changed; real network and user data were not used.

## Retained listing integrity

Pseudocode SHA-256: `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`. Assembly SHA-256: `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`. These refer to the complete retained artifacts, not new live original execution. The JSON companion records their fresh hash comparison and this report's digest.
