# Non-CRT input-domain triage — 2026-11-04

Read-only comparison of exactly 20 retained native bodies, with current source and mapped callers. No new behavioral defect was established in this bounded sample. No REA query, build, test, binary execution, production edit, status rewrite or packaging occurred. The existing 1,512-address heuristic unmapped count does not establish 1,512 missing business algorithms. Classification of support routines below is not an ABI/equivalence completion claim.

## Behavioral comparisons

1. **Signed achievement fields, 0x1800348d0.** The native signed-low32 decimal conversion supports negative values. Current `anadius-content-handlers-source/content_handlers.cpp:27` accepts `int32_t` and uses signed decimal conversion; `achievement_element_02c080` and persistence preserve signed Total/Count/Progress fields. Mapped callers are 02c080 and 030010. No missing negative-value domain was found. The decompiler's small temporary-array declaration is insufficient evidence of a native value-domain restriction.
2. **Response-derived cipher key, 0x180034c30.** Current `anadius-crypto-source/crypto.cpp:107` matches signed-byte extension, seed 7, one initial LCG step, reseed and 16 output bytes. A one-byte string supplies the terminating NUL as byte 1. Empty input's unspecified native second byte is not treated as a confirmed supported-input discrepancy. Caller 02d7e0 is mapped.
3. **QueryImage owner/size/cache path, 0x1800314f0.** Current `anadius-content-handlers-source/content_handlers.cpp:147` preserves required Width/Height/ImageId, colon owner, own/default avatar choice, signed width/208, raw-width cache name and own-avatar size update. The legacy path-only throwing overload in `anadius-content-effects-source/effects.cpp:72` is not the selected missing algorithm: AvatarRequest forwards owner/size through Graph and Online. Retained instructions at 031aa0/031aa3 load R9D=size and R8=owner, followed by the indirect call at 031aad. The pseudo-code's abbreviated indirect-call arguments do not overturn those instructions.
4. **Aggregate release, 0x180037f70.** Current `anadius-config-defaults-source/teardown.cpp:29` explicitly represents the named storage release sequence. Its retained helper calls are container/path support. Exact native allocator/SSO/tree layout remains outside this input-domain comparison; no new user-facing feature is inferred from those helpers.

## Exact native-body inventory

All addresses below have image base 0x180000000. EIDs refer to full retained pseudo-code replies embedded in `anadius-all-functions.pseudo.c`; no fresh native session was needed.

| RVA | Classification / inspected purpose | Retained EID |
|---|---|---|
|0348d0|signed integer decimal conversion|ev_c8956389f06090304d78bbbaeb7ba946fd58733924c38886a9cab4869d2c9cd2|
|0349a0|byte-vector insertion/growth|ev_e6d957b61c80f59949680c1d5a1143d1d5230dd78fef566737610c1f8471ea3f|
|034af0|byte-vector reserve|ev_c9097d390582e074c889496c9f5b981a585cb438aad0d2a806a7472cf2eba9cf|
|034e60|QWORD-vector insertion/growth|ev_9a86750a40c5525bc56bc0cb0fe30a6a55d8dbc04c12df3149cb6bd736cab45b|
|035120|QWORD-vector reserve|ev_d637a413ba839e365d8ccaa17e69ee20551d9620661d21e14fef287d5c35d6b1|
|036ac0|recursive tree destruction|ev_235856aecd30798d50812c9436127ef065e68c9b9637e5e866dca435e2dc7619|
|036b40|tree payload destruction|ev_25d3c1d88be97fe8f1b5792a7e3d7bea1145ecd436726f6247a7146714d87b3e|
|037d90|content-vector payload release|ev_78bed49b7813b85241d8bd2e933138f18493abe8336889edb3edc7ec93171f1d|
|037e20|entitlement-vector release|ev_e4b31623a4b9c89493458ecf4fc29f7a3a13ec301c95dce5f1781cf391ada869|
|037ee0|string/path-vector release|ev_32b5b7d8ce74105dd5392f27eef09c5e0c1e42c3e6c609ba2af934e38a866c9f|
|0394a0|string-vector insertion/growth|ev_dc8899b64860e522a9ab91f57ba4243ce07c56bb05abbf5b3bcab2ddeb199174|
|03cd30|counted string find|ev_00d705658e3778339e7f9a047ca60dcdbb8ff410872efb2e82602665b446d314|
|03c9e0|string-vector growth/copy|ev_cfe7859a5ebca5102db4986343bf035972b9614a6a955c459ddb8aa42c57ec53|
|03cb70|string copy/SSO allocation|ev_8d09165107f6f5af7f5a5bd042e1acf3d059d42b3b51a9984fb369a18fb96042|
|034c30|response-derived cipher key|ev_5f20ce94b6a641740f78ab38c80064dc65fea9e0d9c897af06b24bef8f8b2d0b|
|02b300|string reserve/shrink|ev_035e10f60a4facdf90b41d0d45af7f9a50b80c7a8ef00061d18d0bf367821008|
|02b3b0|C-string to wide-path helper|ev_d4e9f6f3f23c282b59d83ddf0a34d1a4fbea68acaabd5bc75f894551de723b38|
|02b3f0|counted-string to wide-path helper|ev_57d7e0f9d878bf5b1f86c139d90c8a53c1635ff4ec9c2a162b236cec975b6999|
|0314f0|QueryImage|ev_2e2e94933ee58f5567373dbd0e0d2f7bcd6173760dd3b9b93dbd5f4724cbc149|
|037f70|aggregate storage release|ev_c03fec66056719abadb29e535fb59a20827bd1c74e0bbe959590c8170e39714b|

Support-routine mapped callers: 0349a0/034af0←034c30; 034e60/035120←035850; 036ac0/036b40/037d90/037e20/037ee0←037f70; 0394a0←039720/03cf00; 03c9e0/03cb70/03cd30←03cf00; 03cb70 also←03c9e0; 02b300←02d7e0/0320c0/032d00/034cf0; 02b3b0/02b3f0←0320c0. Empty heuristic caller lists for explicit QueryImage/teardown definitions are not an unreachability proof.

## Source/evidence SHA256 authority

| Path relative to outputs | SHA256 |
|---|---|
|anadius-all-functions.pseudo.c|26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb|
|anadius-all-functions.asm.txt|9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42|
|dll-critical-path/current-full-source-map-after-connect-lane/functions.json|a6478d89e763cc6755a85cbffa5c76298f98512cf8ddad3e98abe49b01bdc563|
|anadius-content-handlers-source/content_handlers.cpp|4cb847e398b3a81b4c9149a711fecfe0532265fb5c974f33aed73591700e947f|
|anadius-crypto-source/crypto.cpp|cd493597464ba22c7c11557c36f033387770e0cc62d4fbbaefc1676bbfabd436|
|anadius-content-effects-source/effects.cpp|8224ff1c1b10c5a3bb3d91aa8762f574a657a17dc4a83dee59dc19b745e5a510|
|anadius-config-defaults-source/teardown.cpp|7166c7b23b61fa4f00f03451b38d9d39728d65f0865a1d95afe7d038e4983401|
|anadius-online-factory-source/factory.cpp|f6a613a5c9cc007a722298efdc0c181c1270c496a179eb952a0f12fbbb8b95c9|
|anadius-application-graph-source/graph.cpp|fc90cf952eb5bb9230421d8dc97981ed6c7b60cf94d5b18f0cbd695ea0fdaf84|

Recommendation: do not patch these comparisons or reopen the already-closed XML-status/config-duplicate/response-family questions. Continue a separate bounded sample of application/Online/activation producers beyond this inventory; a new patch needs an observed value/option/effect discrepancy. This negative sample does not establish full-DLL completion or remove remaining native support/ABI work.
