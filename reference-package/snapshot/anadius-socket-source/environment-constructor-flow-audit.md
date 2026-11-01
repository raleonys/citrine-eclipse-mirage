# Environment constructor independent retained-evidence audit

Independent reviewer: `environment_constructor_audit`, 2026-10-23. Complete retained assembly and pseudocode for `0x180023dd0` and the context functions `023cb0`, `021fd0`, `027a20` were read. This is static inspection; original DLL/game/network execution: none. Real user configuration, tokens and achievement files were not read. CRT production remains unchanged.

Target `anadius64.dll` SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

| Function | Complete pseudocode Evidence | Complete assembly Evidence | Counting |
|---|---|---|---|
| `0x180023dd0` | `ev_d162fd99180d0294652d44b39fe2cceaf384a9996b3bae0f40e08fd0e2ac7f82` | `ev_747feac84362fbb739d9c68cee8e8fbc51d55c081309ddaa794f2c19a13f7fac` | One new normal body |
| `0x180023cb0` | `ev_104fb6d87cffb0a02e61c0173403243f305305ffcf18a5fdb95e886f36fb27d1` | `ev_5393cd5463f3a87b8f294c63f701e30396bb840e2ae1f35dd015b1a5c38fa588` | Existing context only |
| `0x180021fd0` | `ev_cb8feee93f61ab1f79de415f7b58cf399d0febb87fbce65a9c6f749925ed1724` | `ev_6326ebb30e495078e0ed47d7346de01edd7f346608c3d5c91890ce2c1637533e` | Existing context only |
| `0x180027a20` | `ev_ac8acca2b33bd760e4ba5f3d9ec5784ef473a66bd1f78d4aeb47033d2bcbc4d9` | `ev_c7fd45e2d65c3e1f508706bdf6dc9e7561b9d896d56095bbebac7e82853f4463` | Existing context only |

## Type selection and empty branch

`023df2/023df5` cache the source and destination addresses. `023df8/023dfc` load source native `+20`, then call its slot `+0` without explicitly replacing RCX with source. This is a no-executor-this type-information query. `023dfe` adds eight to the returned type address; `023e02/023e09` call existing CRT type comparison `04d2d0` against literal `0x1800a25c8`. Only comparison result zero selects the empty branch. The comparator body and actual RTTI bytes are not newly recovered here.

`023e12`–`023e37` initialize a temporary's operations, state, schedule and executor to `0846c8`, zero, `0846e8`, `084770`. The temporary payload prefix at `+0/+8` remains unspecified. `023e42` calls actual existing `023cb0` to move this temporary into destination. Afterwards `023e48`–`023e59` reload the temporary's operations and its slot `+0`, comparing the live destroy identity to `017a30`. Only a different identity invokes the live callback at `023e5f`. No destruction of the destination's previous contents is evidenced: this is a constructor and requires fresh exclusively writable destination storage.

## Nonempty branch and first move

`023e67/023e6b` read source's live executor at native `+28` and **cache the actual function pointer at executor slot `+58`** before the query callback. `023e6f/023e73/023e77/023e7a` then read source's live operations at `+10`, call its query slot `+18` with source as RCX, and preserve the returned query value. `023e7c`–`023e87` call the cached prepare function with temporary destination, query result and uninitialized scratch-address token. There is no executor-this parameter. Changes to source executor during the query must not select a replacement function. Slot `+58` is distinct from the existing completion path's slot `+40`.

`023e8a`–`023e96` copy first temporary operations and schedule into a second temporary. `023e9a`–`023eac` reset the first temporary operations and schedule to the sentinels **before** invoking the copied operations' move slot `+10` at `023ebc`, with second destination and first source. `023ebe` clears first temporary state **after** that callback. `023ec6/023eca` then copy the first temporary's **live executor** into the second; `023ece/023ed5` reset first executor to its sentinel. The existing `move_io_environment_023cb0` API has exactly this defined-field ordering, and can represent this first inline move without counting a second original body. The move callback owns payload and destination-state transfer; no extra payload copy or zero is inferred.

## Two destruction points and second move

`023ed9`–`023eea` reload first temporary operations and slot `+0` **after** the move callback. The guard comparison uses that live function identity, and invokes it at `023ef0` only when different from `017a30`. A move callback may replace first temporary operations, so caching the sentinel table for this guard is incorrect.

`023ef3`–`023efa` call existing `023cb0` again, moving the second temporary into final destination. `023f00/023f04/023f0b` then reload second temporary operations and its destroy slot and invoke it **unconditionally**, including the `017a30` identity. This reload occurs after the second move callback, so callbacks replacing second temporary operations affect the final destroy. Neither destructor is replaced by automatic RAII cleanup; native unwind behavior is outside this normal-body reconstruction. `023f0e` returns the original destination address.

## Existing caller and completion context

Existing async-send initiation `021fd0` calls this constructor at `0220d5`–`0220e7`, destination operation native `+a0`, literal zero/zero unused arguments, selected environment member address. Handler owned pair is cleared at `0220cd/0220d1` before construction; weak-temporary decrement and caller-buffer reread occur afterwards at `0220ec`–`022129`. Source composition must preserve this existing caller sequence.

Existing send completion `027a20` moves operation environment into a local through `023cb0` at `027a62/027a6d`, cleans operation storage, dispatches direct or scheduled using the moved local state (`027b34`–`027b5b`), then unconditionally destroys its local through live operations at `027b98`–`027ba3`. This is context, not a newly recovered completion body. No IoHandlerEnvironment, IoHandlerSentinels or existing move API changes are needed.

## Boundaries

Exactly one new normal original procedure is established. Required type query, CRT RTTI comparison, prepare slot58 and native operations/move/query/destroy implementations remain caller-supplied effects. Typed logical environments and opaque address identities are not native memory, vtable, allocator, DLL or calling-convention ABI overlays. Scratch and native uninitialized bytes are not reconstructed as meaningful zeros. Native exception/SEH/unwind, lifetime/concurrency, receive/completion application helpers and full DLL equivalence remain unproven. No original target was loaded, executed or changed.
