# Registry language log temporary lifetime — 2026-11-04

The shared A/W query implementation now destroys its narrow language logging string before entering the replacement value writer. Previously the temporary remained alive through that writer. This is a bounded source ownership-order correction; it does not recover a new normal registry algorithm or establish original CRT/SSO/allocator/LastError equivalence.

## Fresh binary evidence

REA 6.8.0, Ghidra 12.1.4; original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

| Path | Finish log | Conditional heap free | Replacement writer |
| --- | --- | --- | --- |
| A, helper 0x180009520 | 0x18000973d -> 0x1800088e0 | 0x180009779 -> 0x18004b0f8 | 0x1800097ab -> 0x1800091a0 |
| W, helper 0x180009850 | 0x180009a6d -> 0x1800088e0 | 0x180009aa9 -> 0x18004b0f8 | 0x180009adb -> 0x1800093a0 |

The capacity check branches skip the free for an inline native string; both paths reach the writer only after completing temporary cleanup. Fresh pseudocode and complete instruction replies are retained beside this report, plus a six-record REA Evidence bundle. Content-addressed IDs may equal earlier replies; these calls were made in this stage. Ghidra's reported terminal invalid-parameter calls are provider assumptions, not independent proof that those callees never return.

`current_document` is unsupported by this Ghidra provider; its complete error is retained. Successful function Evidence identifies the actual binary, provider and analysis profile. No unsupported operation was retried. The native session was closed after Evidence export.

## Source and review

`outputs/anadius-registry-source/registry.cpp` uses a nested block around language resolve/log/finish only. The writer and its subsequent type-field handling retain their existing order. The exact pre-edit source is retained as `registry.cpp.before.txt`. One subagent independently located the mismatch using retained Evidence; root confirmed it with fresh instructions before editing. Final independent static review is in `review.md`.

Other compared paths agree on the original-capacity snapshot, client-before-language selection, writer-before-type-zero update and fabricated-key fallback. This is a bounded comparison, not a whole-registry equivalence proof. General close/query/shared-set callback concurrency remains unresolved; no argument-only query shortcut or new unload requirement was introduced.

## Validation and delivery

Static inspection only. No compilation, project tests, linking, reconstructed DLL execution or ZIP update was performed in this stage. The existing linked DLL and delivered ZIP remain the prior verified delivery and do not include this edit. One canonical production source now differs from that build. No numerical whole-DLL completion increase is claimed.

Next priority: review early general registry callback admission during per-hook publication against synchronous native startup; make changes only when a concrete native/source discrepancy is established. Do not repeat a full build without an explicit rebuild request.
