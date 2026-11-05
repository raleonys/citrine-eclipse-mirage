# Live server-port source stage — 2026-11-04

Implemented the live uint16 scalar access model in five production files. Exact before/after SHA-256 values are recorded in `changed-file-hashes.json`.

- `anadius-network-hook-source/connect.hpp`: added inline `load_server_port_0a3c90(uint16_t&)` and `store_server_port_0a3c90(uint16_t&,uint16_t)`, each creating a transient atomic_ref and using memory_order_relaxed. Namespace assertions require lock freedom and sufficient natural uint16 alignment.
- `anadius-network-hook-source/connect.cpp`: classification, redirect log, and final caller rewrite use separate live helper loads. Web80/443 still skips waiting; non-web waits first; requested3216 still short-circuits the classification load. Counter behavior is unchanged.
- `anadius-server-source/server.cpp`: captures constructor input with a relaxed load before address parsing, stores the actual local endpoint port with the relaxed helper, and uses cached bound_port for environment export and startup publication.
- `anadius-server-source/server.hpp`: documents exact-cell lifetime and live-access/quiescence requirements.
- `anadius-config-source/configuration.hpp`: documents plain startup-only parser writes, helper access after publication, stable address, and quiescence for plain accesses/copies.

The requested owned-server paths contain backend endpoint-value handling rather than the shared scalar accesses. The actual server source paths above were corrected; owned-server files were not edited.

Evidence basis is the previously saved fresh static Ghidra review in `server-port-native-review-20261103`: three distinct native reads at00fd73/00fda9/00fdd3, constructor input read029eca, actual bound-port store024676 before completion02472c, and startup config writes01a516/01a52a before worker/hook entry. Principal Evidence IDs: `ev_839077f99c02612bc8d409358a72a00b4078cf9a201c931361fd8e1c4381c349`, `ev_4e8773ec4bcaae1dbadc4f0ef3ba6465ed4df416f35cdf6cc0f9d1a5444a8d2f`, `ev_afc5c75bd9f7deae42be2664dc9dd14d0f680948b60c4987de25a7b9c7a02b5e`, `ev_875cf78952cc594dcb532411ad3a7808330fabe6372fbb126f62bff806f11043`.

Verification was source review and bounded text inspection only. Assertions were not compiled. No project tests, compilation, linking, DLL execution, original-binary changes, version-binary access, or ZIP changes occurred. Original Domain exact-cell binding, Original lane live-reference forwarding/completion rules, and integration cases belong to the parent stage. These edits alone do not establish completed callback concurrency or ABI validation.
