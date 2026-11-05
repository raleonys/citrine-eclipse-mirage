# Response executor audit — NO CHANGE

The current owned response's `PostedExecutor(..., flags=1)` and actual `dispatch_posted_022e40` match the selected initial native response path. This candidate is closed **NO CHANGE**. No production or test source was edited.

Static-only fresh REA 6.8.0/Ghidra 12.1.4 investigation, 2026-11-04. `session.json` verifies aligned configured server identity and available Ghidra provider; `open.json` admits original `C:/Users/Skysect/Downloads/anadius64.dll`. SHA256 at open and after close is `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. No target execution, build, tests, linking, package update, version.dll access, network or original writes. The agent exclusively owned this connected target lifecycle and closed it successfully.

## Selected initial family

The previous `native-environment-targets-20261104` fresh constructor/table evidence establishes initial flags DWORD0 from `0211b0`, operations `084928`, schedule `084908`, executor `084860`; executor slot+20 (`084880`) targets `023400`, and slot+88 (`0848e8`) targets `0234c0`. This audit freshly decompiles both bodies and the actual caller `021420`.

1. `021420` moves the incoming response task, caches the source executor's slot+20, queries operations slot+18, then calls that cached factory with literal `0x180084425`.
2. `023400` constructs flags `source_flags | 1`, preserves context, calls `020c80`, and writes executor `084860`. The previously retained `020c80` body emits the same initial operations/schedule/context/self-pointer family with no native work retain.
3. After the returned environment move/conditional destruction, `021420` uses the live first executor and caches slot+88 before querying it. `0234c0` constructs flags `source_flags & 0xfffffffd` (clears bit1), preserving context and the same family via `020c80`.
4. Therefore the selected initial flags0 become `(0 | 1) & ~2 = 1`. These factories do not transition this initial response into the separately work-tracked family, nor add native work pins.

The transformations are concrete bit operations. `+20` sets the force-post bit0; `+88` clears the bit1 hint, conventionally described as choosing fork rather than continuation. The native observation is the bit transformation and downstream argument below; no additional semantic priority/fairness/concurrency guarantee is inferred from those conventional names.

## Exact schedule and enqueue path

The fresh complete table read at `084908` covers both selected schedule tables. Initial schedule `084908` and tracked schedule `0849f8` each contain owned slot+10=`022f90` and borrowed slot+18=`0`. Consequently the fresh `021420` branch tests a null borrowed callback, creates the owned response through `020cb0`, then invokes the owned slot. It does not choose its alternative borrowed `022240` descriptor route for these tables.

`022f90` is a tiny mapped stub not represented as a Ghidra Function: the pseudo lookup error is retained, and the bytes `48 8b 49 18 e9 a7 fe ff ff` derive `MOV RCX,[RCX+0x18]; JMP 0x180022e40`. This is analyst decoding of retained exact bytes, not fabricated pseudocode. Its self-pointer load selects the prepared concrete executor payload before calling the actual posted dispatch body.

Fresh `022e40` instructions give the decisive branches:

- `022e5a` tests flags bit0; `022e5e` jumps directly to the allocation/posting path when set. Only clear bit0 allows the matching worker-TLS inline-consumption path. Selected flags1 therefore force posting even on that worker.
- `022f1d` calls actual cache allocator `021820` for size0x48; the source functor is transferred into the resulting operation and the original slot is cleared.
- `022f54–022f5b` form `R8B=(flags>>1)&1`, and `022f66` calls `028ca0` using the scheduler read through the live context. Selected flags1 pass hint0.
- Fresh complete `028ca0` instructions do not read the incoming hint before overwriting R8D with0 for the OS call. `028cb5` LOCK increments scheduler work_count+30, `028cc1` sets queued1, and `028ccd` calls `PostQueuedCompletionStatus(port,0,0,operation)`. Thus exactly one source posted-work pin is added on this normal enqueue path. A post failure retains that same operation in the scheduler fallback FIFO and sets dispatch_required; it does not drop/re-add the work pin.

This agrees with current source `OwnedEnvironment::response`: select flags1, call recovered022e40, and let recovered028ca0 own its one enqueue work increment/failure-prefix behavior. Existing source intentionally ignores the third hint in enqueue, matching the observed unused incoming argument here. No patch is needed for this selected default route.

## Separately tracked input family

The previous exact table bytes establish tracked executor `084950`: slot+20 (`084970`) is `023220`, slot+88 (`0849d8`) is `0232c0`. Both bodies are freshly recovered here. They apply the same respective `|1` and `&~2` transformations, but additionally LOCK increment `*(context+0x10)+0x30` when context is nonnull, then call previously recovered `020c10` to emit the **tracked** family. They do not convert it to the initial family.

Each factory's produced tracked environment owns its own work retain; the earlier move/destroy evidence describes transfers and corresponding releases. These retains are distinct from `028ca0`'s posted-operation work pin. Native tracked-family DWORD accounting, overflow, exceptional unwinding and every caller lifetime were not newly audited here. A host adapter with a fixed default response policy is not thereby proven equivalent to arbitrary tracked-family inputs or arbitrary nondefault flags.

## Retention, limits and identity

The bundle contains 11 fresh Evidence records, 60742 bytes. Ten records cover the initial focused caller/factories/table/schedule/dispatch audit; the eleventh is complete028ca0 instructions to answer the concrete question whether incoming bit1 hint is consumed. One full failed pseudo lookup for the mapped tiny022f90 stub is retained separately. All successful replies retain full inline Evidence and limitations. The export's zero recorded Unknown objects does not remove the explicit limits above. `close.json` reports successful close (`result:null`); original hash remained unchanged afterwards.

| Reply | Evidence ID |
|---|---|
| pseudo-180021420.json | ev_d02f865246dbb1ce2e13b0bf9275f3cb4e3818a21b5fcd7610440c42ec47a4dd |
| pseudo-180023400.json | ev_ff9feb0b13a5de34c4e5512a0b292c6ce9e2d675c177a7c349f45cb133bf73b4 |
| pseudo-1800234c0.json | ev_4542e19dbc5aa876c5f3ec9316e7ab904c9363a20ddafccedf029f5894b106b0 |
| pseudo-180023220.json | ev_4367ac20cf5544100b1dd0810115b7e254219f0ec640e28cae9c8047352ef9ee |
| pseudo-1800232c0.json | ev_6f651162d956ba7796bb91a99e9a45b96ebe88e6e71ea24671eb771820745aba |
| bytes-180084908.json | ev_b2a75dec2224cc9360f98d117c6f36dc4a6f3e5b0dfcc32a5746f2d35ee71245 |
| bytes-180022f90.json | ev_4070fad86001feeebb1e83a4323241fd14693910c07e96a6e21d4ded4767dda9 |
| pseudo-180022e40.json | ev_beafe42932833e480345dc2e6152c82ff8ae79f641545de67d582d2ca903a851 |
| instructions-180022e40.json | ev_e5c61f5c9c230ffab846c7006cb3525c9ba3867d38379fa38eb5c757ac8150c8 |
| pseudo-180028ca0.json | ev_cd97b322e9f185059bb73f6f7615904034c6db6d75bc66be4f51d267a6525343 |
| instructions-180028ca0.json | ev_0f42f83d0480ca43dff44ab8645ba9393daf2d1d0f85245aafc18ac70fe7d1a5 |
