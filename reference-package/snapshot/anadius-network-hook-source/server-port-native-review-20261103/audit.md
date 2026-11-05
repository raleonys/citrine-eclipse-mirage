# Independent live server-port audit — 2026-11-03

The proposed live relaxed uint16 scalar model is supported by the native access pattern. It can remove the Original profile's cold80/443 snapshot fallback without adding an all-ready proof, provided every live conflicting scalar access and the before-publish identity binding change together. This audit made no source edits and performed no build, project test, linking, target execution, binary mutation, or ZIP change.

Target: `C:/Users/Skysect/Downloads/anadius64.dll`; SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`; Ghidra12.1.4. Fresh Evidence was collected in this auditor's own isolated immutable session, then that session was closed. Full structured replies are adjacent JSON files; the evidence index is `findings.json`. Source hashes of21 reviewed production files are in `source-hashes-pre-change.json`.

## Native observations

| Native address | Access/meaning |
| --- | --- |
| 0x18000fd73 | CMP requested port against current word at0x1800a3c90 |
| 0x18000fda9 | Separate MOVZX of current word for redirect log |
| 0x18000fdd3 | Separate MOVZX of current word for htons/final caller port rewrite |
| 0x18001a516 | Configuration writer, within0x180019ad0 |
| 0x18001a52a | Configuration writer, within0x180019ad0 |
| 0x180024676 | MOV word ptr [0x1800a3c90],AX: actual local endpoint host-order port |
| 0x180029eca | MOVZX of shared word into constructor argument after context initialization |

The static initialized word is `0000`. The exact xref list has these seven entries, corroborated by typed READ/WRITE incoming-reference metadata. This bounded search does not exclude unknown aliases, computed accesses, overlapping wider writes, or external runtime changes.

In00fd00,80/443 branch straight to the shared-port comparison without wait. Other ports call036880(-1) before the comparison. Requested3216 branches directly to the redirect body after that wait, bypassing the comparison. Once selected, the redirect body rereads the global for its log and again after logging/counter/terminal handling for its final store. A changed scalar does not undo the earlier route decision.

In024480, the bound-port store024676 precedes the EALsxPort export call0246e4 and finish036820 call02472c. Consequently there is a native interval where the live scalar is meaningful before the server-completion observation used by the source snapshot. This is static ordering, not an observed scheduling trace.

Configuration ordering is freshly anchored:01c870 calls019ad0 at01c8d1 and01c8d8;038970 calls01c870 at0389d4, launches server worker029ea0 through05e8e0 at038a1d, and only then invokes host initialization00e310 at038a61. Thus ordinary startup configuration port assignments precede server-worker concurrency and hook publication.

Important Evidence IDs:

- 00fd00: `ev_839077f99c02612bc8d409358a72a00b4078cf9a201c931361fd8e1c4381c349`.
- 024480: `ev_4e8773ec4bcaae1dbadc4f0ef3ba6465ed4df416f35cdf6cc0f9d1a5444a8d2f`.
- Xrefs: `ev_428c49259e2143faad6bdac9a8767e4ae061b71dc5bc4a316344461061e300fe`.
- Typed refs: `ev_afc5c75bd9f7deae42be2664dc9dd14d0f680948b60c4987de25a7b9c7a02b5e`.
- Initial word: `ev_2578c8fd7c805fe4009ff79f50c16dd3b7bf2fade20e4a1e72574d08c01d0806`.
- 019ad0: `ev_abaf2c82c268413aba6598278f889fc8fde09a62e88dd0256ce12cfa7137e3f0`.
- 01c870: `ev_b4cc6a9568013d8519443e1f3ab818ea43db24c82864aa427be8333402c84ef4`.
- 038970: `ev_3d7de1704f039245efc370ccfeeac36afcf6d505a03851d803acb8f63f2404a7`.

## Production source access inventory

| Source site at review | Treatment |
| --- | --- |
| config-application/application.cpp:70–72 | Startup parser writes. May stay plain under before-workers/hooks contract. |
| config/configuration.cpp:161–163 | Alternate recovered parser's equivalent writes. Same startup-only restriction. |
| config/configuration.hpp:44 | uint16 cell declaration. Preserve ordinary scalar identity; add access contract, do not silently copy/move while live. |
| server/server.cpp:16 | Shared input read into endpoint. Convert to relaxed load. Native input capture is at029eca before constructor's address parsing. |
| server/server.cpp:33 | Shared bound-port assignment. Convert to relaxed store. |
| server/server.cpp:35–36 | Current rereads for environment/publication. Prefer cached local bound_port after store, matching nativeAX use and avoiding plain shared rereads. |
| network-hook/connect.cpp:56–57 | Shared classification comparisons. Convert to relaxed loads at the original short-circuit points. |
| network-hook/connect.cpp:26 | Log's live word read. Convert to relaxed load. |
| network-hook/connect.cpp:46 | Final post-effects word reread. Convert to a separate relaxed load. |
| native/native_callbacks.cpp:89–91 | Original lane currently copies published value to local port and passes local reference. Pass exact Context live port reference for Original body. |
| foreign-callback/foreign.cpp:208 onward | Original lane currently reads redirect_port_proof snapshot for web decision and after waits. Use bound exact live-cell reader for Original profile. |
| foreign-callback/foreign.cpp:267 onward | Original completion currently rechecks snapshot equality and route predicate. Retain actor/hook/lifetime checks, remove scalar-equality dependency for already-chosen Original body. |
| host-platform/assembly.cpp:51,57–58 | Original constructor currently receives milestone snapshot proof. Bind/use live scalar for Original; Cooperative snapshot profile can remain independent. |
| native-context/context.cpp:31 | References exact configuration cell. Reference binding itself is not a scalar load/store and needs no atomic conversion. |
| bootstrap-backend/backend.cpp:191–192; application-worker/worker.cpp:253,328 | Reference forwarding/address comparison only; no scalar-value access here. Preserve exact identity. |
| execution/execution.cpp:116–119 | Takes port by value; no shared scalar access. Preserve payload issuer/flag rules. |
| startup-publication/publication.cpp | Separate atomic milestone snapshot, not an alias of shared scalar. Retain its cooperative/event semantics. |

The search covered production C++ headers/sources under outputs; tests, fixtures, work snapshots, and toolchain copies were excluded from the active source inventory. The public State and RuntimeOwner::configuration() accessor can expose the cell to downstream callers; source ownership documentation must require atomic helpers or quiescence for live reads/copies/writes.

## Minimal coherent implementation plan

1. Add a shared helper for transient atomic_ref<uint16_t> relaxed load/store, with is_always_lock_free and required_alignment<=alignof(uint16_t) assertions for supported source targets. The static initialized/native address is naturally aligned; the source assertion must establish source alignment independently. The installed libc++ implementation derives minimum alignment from sizeof(T) and lock freedom from its compiler builtin; no probe was compiled.
2. Convert live connect classification/log/final reads and server input/bound assignment together. Keep a local bound_port for environment and milestone publication. Plain startup parser writes remain only before workers/hooks and without an active atomic_ref object.
3. Give Original Domain a once-bound pointer/reference to exact CallbackStateView.configuration.server_port. Bind it in the passive attachment factory, before Owner publication and any hook receipt can admit calls. Native Context already stores a reference to the same cell. Capture a pointer, not a persistent atomic_ref: binding happens before configuration parsing in the owning startup path. Require same-cell identity, stable owner lifetime, and fail closed for absent/mismatched binding.
4. Original80/443 compares an immediate relaxed live value; Original non-web waits first, then compares a live value unless requested3216 takes the unconditional body route. Retain successful-create receipt, actor/fiber, poison, stop, and code-owner checks. Scalar availability does not grant private CRT, registry, or general callback-body permission.
5. Pass the live Context reference into Original redirect_body. Its two shared reads remain distinct. After body completion, preserve the prior route choice and hook/actor checks; do not fail because the scalar differs from its comparison-time value. Cooperative callers may continue to pass their immutable local snapshot.
6. Keep lifetime/quiescence restrictions for public field access and shutdown. No ready flag, milestone event, or port snapshot should stand in for binding the exact scalar. This stage establishes only a source model; emitted instructions, target ABI closure, full callback concurrency, and runtime scheduling remain unvalidated.

There is no newly discovered native-analysis blocker. The concrete source blockers are the remaining plain live accesses, snapshot-only Original binding, local snapshot passed into Original body, and completion equality check. Address them as one source change; converting only one side of the read/write pair leaves a C++ data race.
