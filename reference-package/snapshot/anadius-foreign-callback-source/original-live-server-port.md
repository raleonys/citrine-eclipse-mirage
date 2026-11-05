# Original live server-port source stage — 2026-11-04

Original connect admission and redirect effects now use the exact live uint16 configuration cell. They no longer require a Server-completion port snapshot or require the scalar to remain unchanged after route selection. This is a source reconstruction stage, not a compiled DLL or a runtime-equivalence claim.

## Native basis

The parent made a new read-only REA/Ghidra request for `0x18000fd00` in this stage. The active session verified REA6.8.0, Ghidra12.1.4 and original SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Complete session and instruction replies, plus close result, are in `live-port-native-20261104-root/`. The returned instruction Evidence ID is `ev_839077f99c02612bc8d409358a72a00b4078cf9a201c931361fd8e1c4381c349`; it is content-addressed and can match an earlier query. This does not claim a new auto-analysis run. The parent session was closed after evidence export.

The previously completed independent REA audit in `../anadius-network-hook-source/server-port-native-review-20261103/` supplies retained corroboration for the other functions and incoming references. It is reused evidence, not newly queried in this stage.

| Native site | Consequence for source |
| --- | --- |
| `0x18000fd73` | Compare requested port against current word at `0x1800a3c90`. Web80/443 reaches this without startup wait. |
| `0x18000fd6b` onward | Other ports wait first;3216 branches to redirect without the comparison. |
| `0x18000fda9` | Read shared word again for the redirect log. |
| `0x18000fdd3` | Read shared word again after effects/counter/terminal handling for final caller port rewrite. |
| `0x180029eca` | Capture configured input before constructor address parsing. |
| `0x180024676` | Store actual bound port before environment export and finish call at `0x18002472c`. |
| `0x18001a516`, `0x18001a52a` | Startup parser writes precede worker launch and hook setup in the retained startup ordering. |

These are static facts. Unknown aliases, computed or overlapping accesses, emitted reconstructed instructions, ABI equivalence and runtime scheduling remain unproved.

## Source implementation

- Existing network header supplies transient `atomic_ref<uint16_t>` relaxed load/store helpers, with lock-free and natural-alignment assertions. All identified live conflicting server/connect scalar accesses use them together. Startup-only parser writes stay plain; public State reads/copies require the documented quiescence or the owning mutable view's helper. No persistent atomic_ref overlaps startup parsing.
- Server captures its input before address parsing, stores the actual bound port with the helper and uses cached `bound_port` for environment export and startup publication.
- Original Domain passively binds the exact stable `CallbackStateView.configuration.server_port` address before native Owner construction/publication. Admission checks that Context supplies that same address. Missing, mismatched or repeated binding rejects; no value is read by binding.
- Untouched connect paths do not read the scalar. Web paths compare immediately, including the initial zero value before Server completion. Non-web paths use the actual wait first;3216 bypasses comparison.
- Native Original redirect passes the live Context reference into the real body instead of a local snapshot. The log and final rewrite remain independent live reads. A boolean phase query replaces the old copied-port API. Body completion rechecks actor/receipt ownership without rereading/requiring equality of the scalar.
- Cooperative snapshot paths retain their separate existing contract. Original construction rejects mixing those snapshot/staged proof providers into its selected profile. No extra safe-unload requirement was added.

One subagent implemented the network/server access conversion, another updated test sources, and an independent third reviewed production integration. Their exact-hash reports are retained alongside this stage. An initial model-capacity interruption of the test agent was resumed from its partial edits; it did not trigger a build or a change of model.

## Validation boundary

New source scenarios cover cold actual HTTP/HTTPS calls with zero startup milestones, exact-cell binding errors, decisions after the non-web wait and3216 with zero, and actual matching-web redirect with successive values80 (decision),4660 (log),6001 (caller rewrite),9000 (later receipt checkpoint). Existing Original concurrency, LastError, private-PTD exclusion, caller-tail and terminal scenarios remain in source. These test sources were **not compiled or executed**.

Thirteen source/header files changed in this stage: eleven production files and two test files. Seventy source/header files differ from the last real full build. This is a pending-difference count, not a completion percentage. No new normal native body recovery or native ABI closure is claimed. A defensible total percentage cannot be calculated from this stage.

No compile, project test, link, DLL execution, original mutation or ZIP update was performed. Frozen build/link proofs keep their historical meaning. There is no newly generated production DLL.

Next source target is the separately audited six argument-only original-forward callback routes and their counted nested actor frames. General close/query paths still access shared sets and require separate treatment; no Windows reentry behavior is inferred without evidence.
