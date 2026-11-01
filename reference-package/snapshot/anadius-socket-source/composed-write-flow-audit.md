# Composed write independent retained-evidence audit

Independent reviewer: `composed_write_audit`, 2026-10-23. All five complete normal assembly and pseudocode bodies below were read. Whole retained listing SHA-256 digests match the prior baseline. This is static evidence: no original DLL/game was executed and no real user configuration, tokens or achievement files were read. Existing CRT production remains unchanged.

Target `anadius64.dll`: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

| Function | Complete pseudocode Evidence | Complete assembly Evidence |
|---|---|---|
| `0x180021fd0` | `ev_cb8feee93f61ab1f79de415f7b58cf399d0febb87fbce65a9c6f749925ed1724` | `ev_6326ebb30e495078e0ed47d7346de01edd7f346608c3d5c91890ce2c1637533e` |
| `0x1800261d0` | `ev_45b2f55e8f0ef15562ee5d5978c239a8bd7dbe150a53979f20633e3f28880e61` | `ev_5500451b02e60551ba95436e9c19673671b10b219573052f2e88f4d15d4a4e75` |
| `0x1800269b0` | `ev_0c7504dc089c22b188406a74817295196cc8a5422d0c58100e40ce078253ba9c` | `ev_586096def6fb76b35ed73eb2537b5179396be2688df8b2b1f3eae25366596e73` |
| `0x180028b50` | `ev_4cd499c90aa2c873056ab0577de3c976776a6225dba31287ff7bb0c5e26fd9c3` | `ev_74dbc4b50b84117c1dbbb54892dc26facd77da66434072f8d19653fe28600196` |
| `0x180028c00` | `ev_68da7f3569738e82ce651786d7b8273c01ff51932b8530ff9e502a18021c2bf6` | `ev_4b5ff1e2f1d04186333f6a462ab01c20a6a558a918c5f97ad7456fba5c427db1` |

## `0261d0`: composed write continuation

`0261d4` stores incoming flags to the handler dword at native `+28` before reading the error value. Incoming flags exactly `1` skip progress accumulation and all finish decisions (`0261e0`–`0261e4`, `02622b`); they always initiate another send, including error or already-complete cases. The error chooses cap `65536` or zero, so the initial error case sends zero bytes.

For every other flags value, `0261e6`–`0261ed` first adds incoming bytes modulo 2^64 to the existing progress and stores it. Only then does the code finish if `(error == 0 && incoming_bytes == 0)`, updated progress is at least total length, or error is nonzero. Finish snapshots the defined 16-byte error carrier at `026210`, calls existing provider `025a70` on handler `+30`, and returns; `025a70` is not counted as recovered here.

Send preparation reads live total length/stream/progress at `026239`–`026241`, clamps the offset to `min(progress,total)`, wraps data-address addition natively, and uses `min(total-offset,cap)` (`026245`–`026275`). It calls actual `021fd0` with service `*stream+28`, socket state `stream+8`, flags zero, the same mutable handler, and environment `stream+68` (`02627a`–`026285`). The handler projection corresponds to `SendHandler.words60_78 = {stream,data,length,progress}` and `dword80 = flags`; these are not added native ABI guarantees.

## `0269b0`: current TLS cache allocation wrapper

The wrapper saves requested bytes in RBX, reads the live global TLS index at `0269c7`, calls TLS exactly once, uses only current context `+8` state or null (`0269d3`–`0269de`), and tail-calls existing `021820` with requested bytes and alignment 1 (`0269e5`–`0269f3`). The allocator tag read from native stack `RSP+20` at `0269e0` is unspecified; `021820` does not consume the tag. It is not recovered initialized state and should not be synthesized as meaningful caller input. The second original argument saved at entry is not the allocation state.

## `021fd0`: asynchronous send initiation

`02200a`–`022013` requests `0xd0` bytes through actual `0269b0`; its ignored allocator argument is handler `+30`. It reads socket weak control `+18` and pointer `+10`, increments low32 weak count `+0xc` for a temporary (`02201b`–`022031`), initializes operation linkage/callback/SDK overlapped/queued/cancellation weak pair (`022035`–`02205a`), and makes a second weak increment for operation ownership (`02205e`–`02206b`). Callback identity is `027a20`.

The original buffer pair is copied to operation `+48` at `02206f`–`022074`. Defined handler payload is copied to `+60..+98`; its owned pair at source `+38/+40` is cleared at `0220cd/0220d1` BEFORE calling required environment-constructor provider `023dd0` on operation `+a0` with literal zero/zero and environment argument (`0220d5`–`0220e7`). The constructor body is not recovered or counted. Temporary weak is then decremented low32; only returned old count 1 calls weak control vtable `+8` (`0220ec`–`022106`). No strong count is used in this sequence.

Only after construction and the weak-release callback does `022109`–`022129` reread the caller's buffer pair. This may differ from the pair previously copied into the operation. Native WSABUF length takes only low32 of the live length (`02211f`), while the socket flag `0x10` zero-length shortcut tests the entire 64-bit length (`02212e`–`022138`). Data pointer is preserved as a 64-bit token.

In both shortcut and ordinary paths, scheduler work low32 is incremented before publication/socket validity checks (`02213a`–`02214f`, `02215b`–`02216a`), without overflow rejection or rollback. Shortcut publishes success/zero through `028b50`. Invalid socket `-1` publishes error `10009`, bytes zero (`02216c`–`02217c`) without WSASend/WSAGetLastError.

Valid socket calls WSASend with one WSABUF, a zero-initialized bytes output, original flags, operation OVERLAPPED and null completion routine (`022185`–`0221b4`). WSAGetLastError is called immediately and unconditionally at `0221bc`, even when WSASend succeeded. Error `1234` remaps to `10061` (`0221c2`–`0221cc`). Return zero OR normalized error `997` calls `028c00` (`0221cf`–`0221fa`); every other return/error calls `028b50` with the bytes output. There is no extra GetLastError, retry, category lookup in the socket imports, or work rollback.

All publication paths may end operation lifetime. There is no operation dereference afterward; final normal `0294c0` receives a cleanup frame with object/allocation both zero (`0221ff`–`022214`). This is no-op normal cleanup, not recovered native exception/unwind semantics.

## `028b50`: known completion packet metadata

Queued dword is set to 1 BEFORE category provider `028620` (`028b72`–`028b7e`). The returned category is written to native operation `+0`; status/bytes are written to `+10/+14` (`028b83`, `028b8a`, `028b91`). These are completion packet metadata distinct from cancellation reason at native `+38` and distinct from the typed SDK OVERLAPPED carrier.

PostQueuedCompletionStatus receives live scheduler port, transferred bytes 0, key 2 and operation OVERLAPPED (`028b95`–`028b9b`). Success returns without further operation reads. Failure enters the existing completion gate, clears operation next, reads the live queue tail, appends/replaces head, publishes tail, exchanges dispatch-required to 1, then leaves (`028ba5`–`028bde`). No GetLastError, wake, retry or work increment occurs here.

## `028c00`: early completion handshake

Native locked CMPXCHG compares queued dword with 0 and attempts to store 1 (`028c15`–`028c1c`). Old 0 becomes 1 and returns; old 1 remains 1 and posts key2 (`028c21`–`028c32`); every other old value is unchanged and returns. There is no loop. Failure uses the same completion gate/queue append/dispatch exchange as above (`028c3c`–`028c7a`). No category lookup or packet rewrite occurs. Success may end operation lifetime and is followed by no operation reads.

## Existing `027c10` context: fallback packet metadata

This context was independently checked against the complete retained `027c10` assembly/pseudocode and the existing `completion-dispatcher-evidence.json` / `completion-dispatcher-evidence.asm.txt`. Pseudocode Evidence: `ev_f152e2905264f9dc18af84544b646d77dac85539841a6cd365e7e2309d3f7065`; assembly Evidence: `ev_d48cd3bea9d0dcba074c541aa1bcfa2e381f3b7fc10b43089ec0486b70fb14bf`. This existing function is context only and does not increase the five newly recovered procedures.

When `028c00` sees an early completion (old queued1) and its key2 post fails, the operation joins the fallback queue. Existing dispatcher fallback processing clears next and sets queued1 at `027d17/027d1b`, then posts the operation with SDK bytes0 and key0 (`027d22/027d25/027d27/027d2b`), preserving no key2 packet marker in that retry. On dequeue, the key2 comparison at `027f0a` selects the cached status/category/bytes only for key2 (`027f11`–`027f2d`). Every other key replaces packet category/status/bytes from current SDK GetQueuedCompletionStatus/GetLastError outputs (`027f32`–`027f3e`) before the queued handshake and callback.

Consequently, in the controlled integration fixture with early completion plus one failed key2 post, the successful fallback retry is key0/bytes0, the fixture GetQueuedCompletionStatus/GetLastError return success/zero, and the resulting callback finishes the composed write after one handled operation with progress0, finish count1 and WSASend count1. The directly delivered key2 fixture preserves cached bytes and completes all three chunks. This is a narrow inference from the retained control flow plus the explicitly configured synthetic SDK outputs; it does not prove that every real fallback, socket provider or race produces zero progress. The reviewer inspected the controlled fixture source and native retained evidence, did not execute the original binary or network, and does not claim an original-runtime differential test.

## Boundaries and limitations

Only these five normal caller bodies are newly recovered. Required providers remain `023dd0` environment construction, `025a70` completion application, existing `04b0bc/04b788` allocator/free, typed/raw storage projection and lifetime bindings, actual weak-control deletion, native SDK imports, GS/SEH/unwind/concurrency and full DLL ABI/integration. Bounded raw allocation and typed SDK/handler objects remain distinct models. Unspecified allocator tag, payload padding and uninitialized native locals do not become recovered source data. Injected synthetic checks cannot establish native race behavior, ABI identity, original compiler equivalence or full DLL behavior.
