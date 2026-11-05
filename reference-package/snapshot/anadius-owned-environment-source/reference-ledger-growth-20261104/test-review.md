# Independent root static review of environment regression sources

STATIC PASS for the two test diffs authored by another agent. Compared exact before copies, final source and current fixture/native-constructor interfaces. No compilation, project tests, linker, DLL execution or package operation occurred. These are unexecuted regression sources, not passing runtime evidence.

The existing moves_and_copies fixture now starts at1, forcing both constructor and copy growth while retaining existing capability, move, stale-query and cleanup checks. The new reference_ledger_growth fixture starts at4096 and holds4100 environment projections: one source,2050 actual023dd0 constructor results and2049 message copy-slot results. All registered destinations and capability/sort buffers are allocated before registration; the destination vector is never resized or moved while rows refer to it. Snapshot copies contain value fields and are not separately released as owners.

Checks cover unique nonzero live capabilities, original source and old constructed projection fields/addresses, three released old/new destinations, new capabilities at reused addresses, rejected expired query tokens, uniqueness after reuse and exact4103 total generation destructions. The source SessionOwner/control stays live until all environment projections are released. Factory stale-query lookup rejects before an admitted mutation, preserving the existing Bound state. Final counts distinguish created from copied and retain no active perform scope.

Only obsolete fixed-capacity306 child behavior and its invocation were removed. The other eight retention cases301–305 and307–309 remain, and main's count is updated. The core test is4100 host metadata projections, not4100 clients or OS threads. Allocation-failure/reentry injection and child JobScope growth are not exercised by these new scenarios.

The existing two-peer actual source integration changes only Profile environment_capacity to1; SDK, native parsing/message, FIFO, lifetime, transport and cleanup assertions remain. It exercises the same selected original-worker composition with a small initial ledger. It does not prove original binary execution, native vtable/RTTI/EH ABI or arbitrary callback/concurrent behavior.

Final independently read source hashes:

- environment_tests.cpp: `145dec7a3ce33a856c9bb9a221bf9d8204ee1414fe26875c50cc99dd72897a7d`
- environment_integration_tests.cpp: `8b60026b2caa037148a4aa077065be9b941f03949ded696b9c74032d6415e28f`
