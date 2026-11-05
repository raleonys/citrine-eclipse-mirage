# Native IO identity and callback retirement boundary

2026-11-05. Original-only static REA6.8.0/Ghidra12.1.4 session, expected package/server path aligned by binary_session. Readiness projection identifies C:/tools/rea-6.8.0/node_modules/rea-agents/scripts/rea.mjs and supported C:/tools/ghidra_12.1.4_PUBLIC. Eight fresh selected instruction records are saved in full with EIDs, exported as a complete8-record bundle; session closed and original hash rechecked. No original/reconstructed execution occurred.

Question: the stable IO owner projection must preserve the actual kernel operation identity, and must not turn its metadata reservation into a lifetime/body strand after a completion has transferred/freed its envelope. Which native boundaries supply those requirements?

## Observed selected instructions and retained body context

- 027c10 passes its stack OVERLAPPED output slot to GetQueuedCompletionStatus; fresh027ed4 loads that result directly into RBX. In the retained full body, nonnull packet metadata and queued-CAS operate on that same raw operation. The host sidecar uses explicitly typed OVERLAPPED identity, without claiming the same object offsets or reinterpretation of arbitrary native bytes.
- Fresh027f5a loads callback from [RBX+28],027f62 passes that SAME RBX as RDX, and027f68 calls RAX. Fresh027f6a's next instruction loads the WorkerThreadState stack argument; the retained full normal/pending/return successors access thread/result/work state, not the potentially retired operation. The callback may end the operation; source metadata cannot inspect its prior slot/projection afterward merely because an outer completion scope is active.
- Fresh022f26 selects027120 as the posted completion address. The retained complete022e40 body immediately stores it to allocation+28 at022f2d; that store is explicitly reused evidence, not a ninth new observation. Thus the selected allocator/posted path has a concrete callback family.
- In027120 fresh027153 calls028fd0 cleanup BEFORE fresh027167 calls021ac0 consume/invoke of the moved functor. The retained full body moves the functor out and clears operation+38 first;028fd0 can cache-transfer the envelope or take its ordinary no-cache/full-cache free branch. This is not an unconditional native physical-free claim: logical envelope ownership can end before the user functor, with physical free only on the selected branch. Existing source complete_posted follows that ordering.

## Implementation consequences and limits

Use stable owner-resolved storage/packet metadata for live rows; do not patch unrelated packet pointer fields during registry-vector growth in Concurrent mode. Preserve actual generations, SDK loans, exact buffer/control pins and known/unknown prefixes. A callback Call reservation belongs to the captured Record AND its generation, while its shared_ptr retains only host metadata. Once source cleanup frees/transfers/reprojects that generation, another row/generation can progress while the old callback remains opaque. Do not add an operation-wide Parent byte view that defers physical HeapFree to outer Operation/callback exit.

Storage/dispatcher resolvers are reconstruction projections; the original performs direct typed operation access rather than this host registry. Static selected instructions plus the retained complete bodies support this ownership boundary, not arbitrary concurrent writes to a single live operation, foreign kernel behavior, raw ABI/exception equivalence or full Original message concurrency. Indirect call targets beyond the bound posted family remain qualified by the existing call graph. There is no new reconstructed-function count or observed runtime schedule here.

The authored concurrent source fixture deliberately uses the legitimate no-cache lower to assert actual envelope free before its held functor, and separately real same-Domain NativeThreads/PreparedMemory actors for disjoint growth/free/progress. It is not a full Runtime/Graph/kernel-receive execution; existing HOST pending-loan receive/send cases remain separate, and none of these assertions has run. Actual Original mode remains disabled pending coherent upper ownership and required same-session/distinct-session message fixtures.

Fresh records:

- `instruction-180022f26.json` ev_57f1f764332011c87054786ed2ac24ea63bd4ba54970d85751d971f9f4297347 SHA256 `6b82a4b1d2278d5326d71882d88db6ed55712cec3cd35a1a201cb21b3649ddc1`
- `instruction-180027153.json` ev_a46d00409474b6a7d04b71b679e8069c921bd3654d6ab5556b9c42db89bf49d9 SHA256 `53fc9cb07ed7438cf83a26933564b231b9467e18af31fc564f57350778a4c230`
- `instruction-180027167.json` ev_4bff8fc1f83cd91512e0d4572a10362eeffa7aa089b8018fe8c3d30b8f9a624d SHA256 `19776c346ccd8d48b2e803d67f75938720738c0e32225f89fb5c8c771802e2b2`
- `instruction-180027ed4.json` ev_2868ab1d4e96d433a2cba245fffb27496e0335c941e998e207bdceff2be6009b SHA256 `772a7ac7f306c7e58ea935d0e676c3b29fccb6082fff05a5460263add9ada672`
- `instruction-180027f5a.json` ev_f709969a88579b07252da35633665be7c07924363d041bf6eb7a4c700bf529a5 SHA256 `baab6a73e2373b77a0b9ef11312ad464a1c2c3bc00be9eabb49a987b55078c5e`
- `instruction-180027f62.json` ev_3aa41c09dccdac930446e0cee94d8e4eaca6b16a3442ae2159863ae9b8a100d6 SHA256 `209e2c2616ca70b3cfcff4dbf9c4dc0dcc7d71f079fa6d695ed9347f1d21f9aa`
- `instruction-180027f68.json` ev_3baf697df601243f2a01441d2f61ff4e0f2d96d8c8cc0e71535a78a0c4f24e38 SHA256 `d8646e63e675749d9c3ebc668474075b7cbd60832c74090086798e95cad07b63`
- `instruction-180027f6a.json` ev_7805190ebdb79475041beacf3778c36fc884ac084737ced2bd3057bf3604be93 SHA256 `56a1b432364aef40921df313b47088cedf808f9174b0b4fd7d8d2514a4f070ca`

Retained complete body files and hashes are enumerated in native-evidence-index.json; they are not fresh queries.
