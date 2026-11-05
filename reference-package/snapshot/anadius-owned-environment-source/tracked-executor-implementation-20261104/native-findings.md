# Tracked executor: last release, stop, factory and move evidence

Static fresh REA 6.8.0/Ghidra 12.1.4 investigation, 2026-11-04. `session.json` verifies aligned configured server identity/provider availability; `open.json` admits the original immutable input. Original `C:/Users/Skysect/Downloads/anadius64.dll` SHA256 at open and after close is `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. No source edits, builds, tests, links, target/DLL execution, version.dll access, network requests or original writes. This agent exclusively owned this MCP target from open through export/close.

## Tracked destructor `0229a0`

Fresh complete instructions cover `0x1800229a0–0x1800229c9`:

1. `0229a4` reads context QWORD from environment+0. `0229a7/0229aa` return immediately if that context is null: no scheduler dereference, atomic work change or stop call.
2. `0229ac` loads scheduler from context+0x10. No independent null-scheduler check is observed for a nonnull context.
3. `0229b0` loads EAX=-1; `0229b5` performs `LOCK XADD DWORD [scheduler+0x30], EAX`. The update is committed and EAX contains the **old** work count.
4. `0229ba/0229bd` compare that old count to1. Only old1 calls `02a190` immediately at `0229bf`; all other values return after the decrement.

The destructor does not clear the source context before or after this call. These are finite DWORD atomics, with no saturation/underflow validation visible in this body. This report does not infer a safe native wrapping state or any concurrent lifetime authority from that absence.

## Stop `02a190`: exact ordered normal/error prefixes

Fresh complete instructions cover `0x18002a190–0x18002a1f6`:

- `02a19b`: atomic memory `XCHG DWORD [scheduler+0x34],1`. If the old stop value is nonzero (`02a19e/02a1a0`), return without touching wake or posting.
- Only if old stop==0, `02a1a2` atomically `XCHG DWORD [scheduler+0x38],1`. If the old wake value is nonzero (`02a1a5/02a1a7`), return without posting. Stop remains committed.
- Only if both old values were0, load port from scheduler+0x28 and call `PostQueuedCompletionStatus(port,0,0,nullptr)` at `02a1b3`. Nonzero return exits normally; both flags remain1.
- On post failure only, `02a1c2` calls `GetLastError` immediately. `02a1c8/02a1ca` cache the DWORD in EBX and in the error pair.
- `02a1ce` calls system-category provider `028620` **unconditionally on the post-failure path, even when the cached error code is0**. `02a1d3` tests the cached EBX value; `02a1d5` stores the category returned in RAX into the pair.
- Cached error0 returns after the category call. Cached nonzero selects `02a1e6–02a1f2`, passing the error pair and literal `0x180084530` to `028240`.

No flag restoration, post retry, delayed stop, extra quiescence callback, or reconstruction of a different error from category-provider LastError effects appears in this body. The work decrement occurred before stop entry. These prefixes remain committed if a supplied source effect has an unknown outcome; native exception/unwind behavior is not reconstructed here.

Ghidra marks the call to028240 terminal/hasNoReturn and excludes fallthrough. The retained instruction record explicitly says this is Ghidra's function-flow flag, not an independent proof that the callee cannot return. Its body and native EH were not expanded in this task.

## Factory `023580` and tracked factory `023360`

Fresh `023580` pseudocode reads source flags DWORD+0x0c and source context QWORD. When context is nonnull it atomically increments DWORD `*(context+0x10)+0x30` before publishing the destination fields. It then emits context, flags, self pointer destination+0x18, operations table `084a18`, schedule `0849f8`, executor `084950`. Null context skips the increment but still emits that tracked table family with null context; no later work is implied solely by the table identity.

Fresh `023360` similarly snapshots flags/context, retains one work unit only for nonnull context, then invokes `020c10` with the local context/flags pair. The previous `native-environment-targets-20261104` exact020c10 body is supplementary evidence for emitting the same tracked family and moving/clearing its local context. No flags transformation occurs in these two factory58 bodies; other prepare/response property factories have distinct transformations already retained in the prior stages.

The two factory records are normal source-operation order, not a claim about original C++ exception cleanup or a whole concurrent atomic transaction spanning metadata publication. The body itself exposes no overflow guard.

## Move `0231b0`

Fresh complete instructions cover `0x1800231b0–0x1800231f0`:

- `0231b4/0231b7` copy source context QWORD to destination.
- `0231ba/0231bd` copy source flags DWORD+0x0c to destination.
- `0231c0` clears the source context QWORD, then `0231c7` writes destination's self pointer at+0x18.
- `0231cb` reloads the **cleared source** context, tests it, and skips cleanup when null. The retained normal distinct-object/nonmutated sequence therefore transfers the existing retain with no extra increment/decrement.
- The actual body also contains a post-clear nonnull path: scheduler load at `0231d3`, LOCK XADD -1 at `0231dc`, old1 test, and immediate02a190 call at `0231e6`. It is retained as observed; no fabricated destination release or extra normal-path decrement replaces that source reload.

There is no opaque callback between the clear and reload in this observed sequence. Aliased/mutated source storage, races, exceptions and native constructor preconditions are outside this bounded normal-transfer claim. Native source flags are not explicitly zeroed by this leaf.

## Integration boundary for source implementers

The semantic requirements evidenced here are a separate tracked work retain for each created/copy reference with nonnull context, transfer on move, one atomic decrement on destruction, and **immediate** stop when that decrement observes old1. Stop's two ordered atomic flags, immediate post, cached failure error, unconditional failure-path category call and conditional raise must preserve their observed order. Passive capability ledger ownership is a separate host bookkeeping concept and must not substitute for this native work accounting.

This does **not** establish that standard Runtime Ports' WorkerRunApi/Api/WorkerRunEffects already authorize execution from a child. A source implementation must use its actual existing Parent/JobPermit-authorized route for that immediate stop effect, with source-layer gates/admission matching the selected actor. No new controller-side stop, arbitrary late retry, safe-unload framework, native EH equivalence, or child authority is proved by these native records.

## Retention

`rea-evidence-bundle.json` contains exactly8 fresh records (40363 bytes); all full replies are retained below. Prior20-record target mapping and11-record response audit are supplementary, not relabeled as new work. Export reported zero recorded Unknown objects; the explicit virtual/lifetime/EH/provider limits still apply. `close.json` confirms successful close (`result:null`); no active target was left open, and the original hash remained unchanged after close.

| Reply | Evidence ID |
|---|---|
| pseudo-1800229a0.json | ev_b75f1b612ffb367e02b17c6f3bdcc696484bc4d5d3b21df0735f575dc04f2063 |
| instructions-1800229a0.json | ev_ba745032bc8bbc9c8e63affa14037cda6d0a2342f65052f4c22be7a6e4046dee |
| pseudo-18002a190.json | ev_f19334cb5522dc71425b904ab65e9e84ee4d5c32a56b35a69e10f8e7d5493df7 |
| instructions-18002a190.json | ev_f7b8c7e0fa1119d3457d42a801700ff189916e678cdcd3db644e8a7c42e95a79 |
| pseudo-180023580.json | ev_53d6ef0e2c9f71bff9e0f9cf81dd87e2c07090b15e7f168b9cb3017938219bd1 |
| pseudo-180023360.json | ev_3fba83066e41fdf32b2b1ede478859073eebe9c0a114ca5898861e05386e3af1 |
| pseudo-1800231b0.json | ev_f6a9bbba8bdfcf2e101361e7713fc75e3f1734605caaf97365262dcb5ec1898d |
| instructions-1800231b0.json | ev_4a7398d5c0c802a7f3988d21de772ffc6cb0a8b141d8b454beb21583f23b9942 |
