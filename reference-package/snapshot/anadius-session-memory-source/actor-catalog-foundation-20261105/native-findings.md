# Handler entry surface: fresh exact reference classification

2026-11-05. REA server identity6.8.0 at C:/tools/rea-6.8.0/node_modules/rea-agents/scripts/rea.mjs; active provider candidate Ghidra12.1.4 at C:/tools/ghidra_12.1.4_PUBLIC, Java25.0.5. readiness-projection.json is an explicitly labelled projection, not an additional native Evidence record. Original DLL SHA2569e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c. Session exported and closed without target execution or binary change.

Question: before enabling independently owned message invocations, does Ghidra identify another direct entry caller that would require a different source admission route, or are the apparent two incoming references code plus unwind metadata?

Observed: xrefs(180033d70) returns180028d99 and1800a8a90. Typed inspection at entry033d70 classifies the first as UNCONDITIONAL_CALL and second as DATA. Listing at028d99 is CALL0x180033d70 (bytes e8d2af0000), inside028d30. Type at0a8a90 is /PE/_IMAGE_RUNTIME_FUNCTION_ENTRY, size12, fields BeginAddress0,EndAddress4,UnwindInfoAddressOrData8. Thus the metadata row is not a second executable caller. In this analyzed exact-entry reference inventory there is one observed direct code site: the detached message receiver already selected in the Profile/NativeJobs reconstruction.

This supplements the retained full028d30/033d70 bodies and prior twelve capture-site records in ../parallel-frame-admission-20261105. It does not re-prove every body, prove absence of unresolved computed/external calls, establish global lock-freedom, observe runtime overlap or claim a newly reconstructed native function. The warmed same-session and distinct-session counterexamples remain bounded by those existing body/catch/cipher proofs. The current source foundation is a coupled ownership prerequisite, not final Original route enablement.

Fresh inline records saved in full:

- `xrefs-033d70.json`: `ev_4f3e6892e2e85123d38d02e20bfb8e7a46bac6b9c92d8357e56241e138b165e2` SHA256 `b38992490e97459a0f1cc36f353f910678771f48211d0fe67825c68b48bad1ce`
- `instruction-028d99.json`: `ev_cb0ee34f06df65ba1992a016b84adf153fd0a1a5ca21a069ea80232e320420d8` SHA256 `842e8217c8d7a087856a81ec21d147b2bf0eb6183878829a7b71f959faa5ddb7`
- `instruction-033d70.json`: `ev_c319ae4c895520aa767331f28657e409824bf516bc09cfd0de892d226b28d911` SHA256 `885409841e2eafb623dbed306fa06dc6ea87d611c470426d22ed30438e11ec59`
- `data-type-0a8a90.json`: `ev_baa2bce6c557f49d7e0eeb0f5a2f4bd53efeb7ae95db3281b3f3aa70a41139b7` SHA256 `5bb39b57c6d24e7d7d536bb2b39f71ca35c6a86f761c2a2185c061de2c2c2cdd`
