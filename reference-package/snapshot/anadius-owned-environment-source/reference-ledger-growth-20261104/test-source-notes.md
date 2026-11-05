# Environment reference-ledger test source

Only `environment_tests.cpp` and `environment_integration_tests.cpp` were edited. The stage's existing before copies were preserved. No compilation, project test execution, linking, DLL execution or package update occurred. All behavior below describes authored assertions, not observed passing results.

## Core source coverage

- Existing `moves_and_copies()` now starts the real EnvFixture Environment with one row instead of 32. Its existing actual `023dd0`, `023cb0`, message copy, prepare slot40, response factories, capability lifetime, empty-sentinel and LastError assertions remain. This forces the selected make/copy operations to grow from a small initial ledger.
- New `reference_ledger_growth()` starts with 4096 rows and holds 4100 metadata projections simultaneously: one real session source plus 2050 actual `construct_io_environment_023dd0` results and 2049 message copy-slot results. These are references to one fixture session, not 4100 network clients. Every destination and both capability buffers are allocated before registration; the destination vector is never resized or moved while registered.
- The new case checks nonzero/unique capabilities, defined state/table identities for every projection, unchanged original source fields/capability, unchanged oldest constructed projection fields/capability, exact live reference and factory/copy counters, and no poison or retained active scope.
- It releases three selected old/new destinations, reconstructs them at the same addresses, rejects their expired capabilities, checks new capabilities differ from the expired ones and remain unique among all 4100 live references, and checks exact reference counters. Finally all 4100 references are released and the total destroyed count must equal created+copied = 4103 (including the three reused destinations). The fixture's actual source worker/IO graph is then drained through its existing finish path.
- Removed only obsolete `--capacity`/ExitProcess306, which expected a fixed one-row copy failure. The other eight isolated retained-prefix cases (301–305, 307–309) and their assertions are unchanged. Main's advertised isolated-case count changed from nine to eight.

## Integration source coverage

The existing two-peer concrete Profile/native Runtime/NativeLogic integration now sets `profile_deps.environment_capacity=1`. Its original source pipeline, plain/encrypted responses, backlog, wrapped deque growth/FIFO, exact injected SDK calls, actual heap/FLS/IOCP cleanup, native disposal and resource-drain assertions are unchanged. The test's own metadata growth differs from its existing application/deque assertions; it does not replace their implementation with a fixture.

These additions use the original admitted worker path. They do not assert child JobScope growth coverage, expand child authority, introduce real network clients, or claim native ABI/RTTI/vtable/unwind equivalence. Fresh native comparison and unresolved virtual-target limitations are in `native-findings.md`.

## Static checks and identity

Compared both files to the preserved before copies using read-only diff. The integration diff contains only the one dependency assignment. The core diff contains the positive growth cases, needed standard includes, removal of the obsolete capacity branch, and main wiring/count wording. No production source was edited by this test author.

Final SHA256:

- `environment_tests.cpp`: `145dec7a3ce33a856c9bb9a221bf9d8204ee1414fe26875c50cc99dd72897a7d`
- `environment_integration_tests.cpp`: `8b60026b2caa037148a4aa077065be9b941f03949ded696b9c74032d6415e28f`

Before-copy SHA256:

- `environment_tests.cpp.before.txt`: `570631227d1e8b4628c214f59a64326fa677e6b5fa95f8c5529986c333574cf8`
- `environment_integration_tests.cpp.before.txt`: `64202a8b0f2428ded3ed358c4a9978379e36d0277b7111256f0c9e3833d8155d`
