# Retained CRT dispatch proof for the WSA module owner

**Resolved:** on successful original DLL process attach reaching application038870, initializer001310 executes first through the C++ initializer range. No new REA session, target read, source edit, build/test/link or execution. This additive retained audit does not rewrite the frozen19-record WSA audit's earlier uncertainty.

## Exact bounded chain

1. Retained dllmain_dispatch04bbc8 handles reason1 through04b9e0 at04bc37, tests its result, and only then calls application038870 at04bc52.04b9e0 reason1 routes to04ba30 at04ba1c. Earlier failure can prevent the application call; this is a normal successful attach ordering statement.
2.04ba30 performs its C initialization checks. On success,04bab7 loads end **0x18007f6d0**,04babe loads begin **0x18007f5d0**, and04bac5 calls064614. Only after return does04baca set startup state2.
3.064614 walks this end-exclusive range in QWORD steps, skips nulls and invokes every nonnull callback (064629–064648). Thus the256-byte range contains32 slots.
4. Slot19 is7f5d0+19*8=**7f668**, containing **001310**. The retained PE table selection states this explicitly; the preceding fresh WSA audit independently read that exact pointer in its7f660 bytes (ev_f9dc0dfd16a685cc89e66459ab27ed85afaf9819c61cb5fa4dc1b2996bb3f1ad).

Therefore001310's already-measured registration of07df30 **before** its unchecked shared WSA claim belongs before actual application entry on that successful native DLL path. This supports a bounded DLL-only compiler-CRT initializer effect; it does not justify executing the recovered outer CRT around the rebuilt compiler CRT or starting WSA in common passive factories.001310's unrelated sentinel write is not reconstructed by this ownership proof.

## Retained authority

Full normalized extracts and source-line/EID indexes are retained here. Original aggregate SHA256: pseudo26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb; assembly9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42. Original target digest remains9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.

| Function | Pseudo EID | Assembly EID |
|---|---|---|
|04bbc8 |ev_91e8bcee0b250a2874d0a05b4c68a1b1f88ae3412e43503f48e378bfefef7bc9 |ev_8a4fcef44115565710b6684a71424833dbc39067b1acfa4284be158f1b0a4630 |
|04b9e0 |ev_8d1fa77bcddd0eb58c0f3d1e7f8490c49640d9be72cd8a44223093f1aff2aaf2 |ev_14275c8735da0d792f1a57803773b974be6d9f92b190fabb9089a5b31f489c71 |
|04ba30 |ev_969fa08f3000af55fdfbefaf21047d6514908cf7dfb8ad03cc181894922a872d |ev_e313cdf981deab3b2e56b180e7becbd1b6a3095a19b2b7eeb786758fe475a6b1 |
|064614 |ev_d3385bc6670ce172cf01545c87b7fb8b2b1601d79878e7016455aedca6c70b19 |ev_9db292af08a5935c9ea1770203044e37dc4ee9439454178adf306f44360f68b4 |

Table authority: outputs/anadius-crt-initializers-source/initializer-tables.json SHA256c5af6f3fa9b7d798359499309555146029ff94ad03b82ac217141dbe9e6c6682. It is retained PE mapping, not a new REA record. Existing crt-entry-source/entry.cpp also mirrors the CRT-before-application order; it is source support rather than new native evidence.

## Registry teardown boundary

The preceding fresh029ea0 instructions establish: destroy both sockets; decrement shared WSA/last-owner cleanup; traverse registry list invoking every entry's virtual+8 in list order; traverse again invoking virtual0 with flags1 and updating head; finally DeleteCriticalSection/free0x38 registry gate. All shutdown calls precede deletion calls; registry gate is last. This proves WSA release before both passes. Exact internal socket-service versus scheduler/IOCP-port destructor effects/order were not independently resolved here and must remain bounded, rather than assigning unsupported fixed internals to those virtual calls. Native exceptional cleanup, arbitrary initializer failure, class/CRT/EH ABI and runtime parity remain unproved.
