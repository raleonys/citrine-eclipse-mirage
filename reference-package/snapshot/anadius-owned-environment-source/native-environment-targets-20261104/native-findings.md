# Concrete native environment targets

Static-only fresh comparison, 2026-11-04. No production/test source edits, compilation, tests, linking, target execution, network requests, version.dll access or original writes. This agent exclusively owned the connected REA target from `binary_session` through close. Session identity is aligned REA 6.8.0 at the configured project path, Ghidra 12.1.4. Original `C:/Users/Skysect/Downloads/anadius64.dll` SHA256 at open and after close: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

## Result and coordinates

Concrete function targets are now established for the initial accepted-socket environment and its work-tracked factory output. They were selected from fresh constructor writes and exact table bytes, not nearby function-name guesses.

Fresh `023000` allocates 0x1c8 bytes at control base C and returns session owner O=C+0x10. It constructs the accepted socket at C+0x20=O+0x10 via `023d20`, after first copying the supplied socket to a local through that same function. Fresh `023d20` copies socket operations at socket+0x78 and schedule at socket+0x88, calls operations slot+8 with destination/source socket+0x68, then reloads/copies executor at socket+0x90. It subsequently calls `028950` for the socket implementation transfer; that separate callee is not newly recovered here.

Thus the environment base E=socket+0x68 is exactly owner+0x78. E+0x10 is socket+0x78 (operations); E+0x20 is socket+0x88 (schedule); E+0x28 is socket+0x90 (executor). These match the previous stage's actual `028d30` owner+0x78 copy caller and `023dd0` source-environment layout. Socket executor coordinates must not be confused with session-owner coordinates.

Fresh `0211b0` writes:

- socket+0x68: supplied context pointer; socket+0x74: DWORD zero.
- socket+0x78: operations table `0x180084928`.
- socket+0x80: self pointer socket+0x68.
- socket+0x88: schedule table `0x180084908`.
- socket+0x90: executor table `0x180084860`.

This is a context-bearing untracked environment, not an arbitrary nonempty heap-backed target. The native DWORD at E+0x0c and self pointer at E+0x18 are evidenced separately; no meaningful initialization of unspecified bytes is invented.

## Exact selected slots and bodies

All table values below are decoded little-endian QWORDs from the retained complete 256-byte read at `084860` or 240-byte read at `084950`.

| Initial table / slot address | Target | Body evidence / behavior |
|---|---|---|
| ops `084928` +0 (`084928`) | `017a30` | Guard/no-op identity from table; its body was not newly queried. |
| ops +8 (`084930`) | `0228c0` | Same target as move. Context QWORD and flags DWORD copy; destination E+0x18 becomes destination E. |
| ops +0x10 (`084938`) | `0228c0` | Same exact short leaf, no native work/reference increment and no source clear in this leaf. Outer move wrappers separately replace source table/state/executor. |
| ops +0x18 (`084940`) | `006180` | Fresh pseudocode returns its first argument unchanged: query yields E, not a host row capability. |
| schedule +0 (`084908`) | `023890` | Complete mapped bytes are RIP-relative LEA and RET; derived returned type-info address `0x1800a25c0`. |
| executor +0x40 (`0848a0`) | `023520` | Copies context, clears flags bit0, calls `020c80`, writes executor `084860`. `020c80` emits initial ops/schedule and self pointer; no work increment. |
| executor +0x58 (`0848b8`) | `023580` | Copies context/flags; when context is nonnull, LOCK increments DWORD at `*(context+0x10)+0x30`; emits tracked family below. |

The short copy/move leaf `0228c0` has a decoded Ghidra instruction at its entry with incoming data references from **both** `084930` and `084938`, but Ghidra has no Function object for it. `resolve-1800228c0.json` records that absence. Accordingly its pseudo lookup failed rather than producing an invented body. The exact 17 bytes, `48 8b 02 48 89 01 8b 42 0c 89 41 0c 48 89 49 18 c3`, derive the following complete straight-line snippet:

```
MOV RAX,[RDX]; MOV [RCX],RAX
MOV EAX,[RDX+0x0c]; MOV [RCX+0x0c],EAX
MOV [RCX+0x18],RCX; RET
```

This snippet interpretation is analyst decoding of retained raw bytes, supported by the provider's decoded first instruction; it is not Ghidra pseudocode or a newly created function boundary.

## Initial constructor fast branch is now bounded

`023890` bytes `48 8d 05 29 ed 07 00 c3` derive `LEA RAX,[RIP+0x7ed29]; RET`, yielding `0x1800a25c0`. The previous fresh `023dd0` record calls type-query slot0 then passes returned+8 and literal `0x1800a25c8` to the type comparator. For this concrete initial table those comparison operands are the **same address**. This establishes the selected initial type route to the constructor's empty/sentinel branch under the established comparator semantics, provided no intervening executor/schedule rewrite changes the environment family. It does not prove every runtime environment has this type, or that other virtual calls can be ignored.

The type remains context-bearing in message-copy callers: `028d30` copies it with `0228c0`. Its type's constructor optimization is distinct from the borrowed context/self-pointer representation copied by those operations. No broad claim that every environment is stateless or zero is made.

## Actual work-tracked family emitted by factory58

`023580` writes ops `084a18`, schedule `0849f8`, executor `084950`, context QWORD, flags DWORD and destination self pointer. The narrowly expanded investigation follows those exact newly observed tables; it is not an unrelated inventory.

| Tracked table / slot address | Target | Fresh body behavior |
|---|---|---|
| ops `084a18` +0 (`084a18`) | `0229a0` | If context nonnull, LOCK decrements DWORD at `*(context+0x10)+0x30`; calls `02a190` on old count1. |
| ops +8 (`084a20`) | `0228a0` | Raw complete leaf copies context/flags, increments that work count if context nonnull, writes destination self pointer, returns. |
| ops +0x10 (`084a28`) | `0231b0` | Transfers context/flags, zeros source context, writes destination self pointer. Decompiled post-clear source-check remains retained; no extra guessed decrement is introduced. |
| ops +0x18 (`084a30`) | `006180` | Same identity-query body. |
| schedule +0 (`0849f8`) | `023880` | Raw LEA/RET derives type-info address `0x1800a2690`, distinct from the initial family's address. Type-name string comparison for this family was not newly analyzed. |
| executor +0x40 (`084990`) | `023310` | Clears flags bit0, retains work if context nonnull, calls `020c10`. |
| executor +0x58 (`0849a8`) | `023360` | Preserves flags, retains work if context nonnull, calls `020c10`. |

`020c10` moves the already-retained context into the same tracked family and clears its local source context. Its unreachable-looking post-clear cleanup test is retained as provider output, not reinterpreted as an additional retain/release. The callee `02a190` was not expanded; a complete stop/wake or concurrency/unwind claim is outside this comparison.

`0228a0` likewise has no Ghidra Function object admitted by the pseudocode selector. Its full 30-byte leaf is retained in `bytes-1800228a0.json`: context load/store, flags load/store, null test/jump, scheduler dereference, `LOCK INC [RAX+0x30]`, destination self pointer and RET. This is raw-byte body evidence with analyst decoding, not fabricated pseudocode.

## Limits and source implication

The native query returns the actual projected environment pointer. Host `Control::rows`, globally unique capability tokens and their retained reference ledger are source policy/bookkeeping, not a recovered native ABI. No 4096-row ledger appears in these observed concrete leaf/factory bodies. Conversely native work accounting uses a finite DWORD with actual atomic increments/decrements; this report makes no unlimited native capacity or absence-of-overflow claim.

Current host Profile's separate Slot environment and selected inline-handler/force-post response policy are not automatically equivalent to these native families merely because the generic native constructor/copy wrappers are reconstructed. A future concrete adapter should distinguish borrowed/untracked context copies, work-tracked retain/release, actual pointer query, bit0 transformations, empty-constructor optimization, and source-clear semantics. This evidence task does not change that production policy or authorize an ABI switch.

Not expanded: executor slot20/slot88 response factories, native RTTI string bodies, all other table slots, nonselected runtime rewrites, stop/wake body `02a190`, exceptions/unwind or concurrent native behavior. The exact table reads retain other slot values as data without claiming their semantics.

## Retention and cleanup

`rea-evidence-bundle.json` contains 20 fresh Evidence records, 92961 bytes. The additional queries beyond the initial narrow set only followed the exact tracked-family table returned by `023580`, or inspected tiny leaves lacking a Ghidra Function. Three full pseudo lookup error replies (`0228c0`, `0228a0`, `023890`) are retained separately and are not successful Evidence. An initial duplicate `0228c0` lookup was made after a result-handling error; it produced the same static invalid-input result and no mutation. No further unchanged retry was performed.

`close.json` records successful close (`result:null`). Original hash was rechecked after close and remained identical. Zero recorded Unknown objects in the export is not a statement that the limitations above are resolved. Full inline Evidence retains provider limitations; pseudocode is reconstruction, not original C++ or observed execution.

| Fresh reply | Evidence ID |
|---|---|
| pseudo-180023000.json | ev_118a0cfef34aca9cd2f70a9f902647bc2e8014332037d147b693b6afa2c3fa5a |
| pseudo-180023d20.json | ev_7b2eb7e1eff80d28c8357649ce44845d3f1f2ae103f9f9bc728f878f799acb0e |
| pseudo-1800211b0.json | ev_4bfe7f01a2866644c84442648a9a9a6b05c04a4e72bacc315bb3393fdb8f17eb |
| bytes-180084860.json | ev_5285ed8a9d48a3c5d65888b6c87483362e35c5d9c7605d6e0e3d09352fa63562 |
| resolve-1800228c0.json | ev_156b7caaa66278fdd4e6440712fcde81ce505f22a68370154cecec6633ea9d8e |
| pseudo-180006180.json | ev_0bb88867d8cd13e47aad0a5150715e9ed600f7a07abd9b72666ab02ee323378f |
| pseudo-180023520.json | ev_79b335ce03113d93d67c6e8fc484230ca1c0f7431c65304cb7fb643a852a9f98 |
| pseudo-180023580.json | ev_53d6ef0e2c9f71bff9e0f9cf81dd87e2c07090b15e7f168b9cb3017938219bd1 |
| bytes-1800228c0.json | ev_79550a46046c957e71260f8fa4d9b7fd4df33f9ececfd07dca26ee75bae21135 |
| bytes-180023890.json | ev_1111199304b117e12a6173706805b0dc91d27124a93de4813b26e339f32541d6 |
| bytes-180084950.json | ev_625b9bf703d6f70961e7cc1779172e21079d561fbb10ba2d871f957cd9a784c5 |
| pseudo-180020c80.json | ev_56f9208492af3c48225266594b699db46ece5a5e23a622a9a2f20b7a65da5540 |
| instruction-1800228c0.json | ev_8fa91787ff7452ab0fbd332f6a0b82cd6461de209320499ca81e7f1310c4cb8f |
| pseudo-1800231b0.json | ev_f6a9bbba8bdfcf2e101361e7713fc75e3f1734605caaf97365262dcb5ec1898d |
| pseudo-1800229a0.json | ev_b75f1b612ffb367e02b17c6f3bdcc696484bc4d5d3b21df0735f575dc04f2063 |
| pseudo-180023310.json | ev_4c080eef901ce8bd03591b227ae17edc0be60db8395970d77fcee0d3fa15f25f |
| pseudo-180023360.json | ev_3fba83066e41fdf32b2b1ede478859073eebe9c0a114ca5898861e05386e3af1 |
| bytes-1800228a0.json | ev_d68748bb23d49f9d034a81cb58f0dab071c1d51a819a761b38be1102f45f908f |
| bytes-180023880.json | ev_8daf0429b5eb6b0229da4d9d300f2d2be8ac43491bc1ebba32f6f12078297bfd |
| pseudo-180020c10.json | ev_df8cd3dc3aa578e3d38bcc14cdb22e9ad65860b5186abafc29d640b922997060 |
