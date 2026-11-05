# Shared Winsock/context test source

Edited only owned-server `server_tests.cpp`, `server_integration_tests.cpp`, resources-factory `factory_tests.cpp` and this note. **Authored/static source validation; no compilation or execution.** No project build/test/link, child process, DLL, packaging or REA query was run. Production server.hpp/cpp, resources factory.cpp and DLL entry.cpp belong to the separate implementation agent. The parent captured all seven before files and pending62 metadata before editing; none was overwritten.

## Primitive ownership tests

`shared_winsock_ownership()` uses the public WinsockModuleState API with a statically typed injected SDK pair, without private counter setters or a debug API. It checks passive construction/default-module lookup and import identity, exact WORD2 and nonnull WSADATA, a first unchecked claim followed by two independent claims, no second SDK startup while owners remain, last-only cleanup, and ignored nonzero cleanup return. The fake reports an intentionally different WSADATA version, which must not cause an invented version check.

A fresh0→1 epoch returns cached10091. A later claim sees that status without another startup, and releasing that nonlast claim does not undo the image claim. Explicit final release uses the measured primitive even for this controlled failed-startup epoch; it is not claimed as native constructor-EH cleanup. Another bounded case reenters acquire from inside the first injected startup: the second claim observes old cached0 before the first call publishes10091. This preserves the measured absence of a startup-completion wait/retry without constructing a thread framework or claiming execution of the race. Missing import pairs reject before claim/startup.

Two new own-child modes cover unknown injected SDK effects. Startup throw must leave held=true, count ownership sufficient for a later claim without another SDK call, and the unpublished cache0. Last cleanup throw must leave held=false; replay rejects before a second cleanup. Each exits with the retained prefix through the existing bounded child harness. These are controlled C++ exceptions in injected functions, not native SDK/EH or automatic-cleanup claims.

## Actual backend/profile tests

Existing HOST IPv4/IPv6, IOCP/scheduler-gate/service-gate/socket diagnostic and sticky-prefix cases remain HOST. Startup now explicitly checks the selected WORD: HOST0x0202 versus Native0x0002. HOST cleanup still returns its actual SDK result; its two-gate/port-before-cleanup expectations are preserved.

New cold Native mode retains an injected ModuleState in the existing State lifetime and supplies its exact same startup/cleanup pair in KernelImports. It first checks rejection of Original+null, HOST+native state, default real state with the fake pair, and an unknown profile before SDK effects or a Parent provider lease. Then the actual source server path runs with three gates. The first recorded registry gate precedes this isolated state's context acquire/startup; normal last release occurs before port/service/registry teardown. The fake cleanup performs the actual own WSACleanup effect but reports SOCKET_ERROR, which Native must ignore while reaching normal Closed and existing full resource assertions.

This cold-state order does not claim whole-module startup always follows a context gate. The separate cached-failure child explicitly takes an unchecked image claim before the context gate, returning10091 without a physical WSAStartup. Actual checked context initialization creates its registry gate, acquires another claim and raises native `winsock`/84500 from cached10091 without repeating SDK startup. Releasing the fixture's image claim must not clean up because the failed context retained its own increment; an extra explicit claim still sees10091 without retry and can retire nonlast. The poisoned context remains retained until process exit.

A second Native child fails its first registry gate. It requires native `mutex`/84428, zero startup calls/port attempts and no cleanup, distinguishing gate-before-context-acquire from the earlier unchecked image-owner case. Both failure children use the actual backend and exact SourceError code/category/name/context/fullwhat. Complete message expectations use one independent default-native SDK oracle, not counted interception of constructor formatting. No successful rollback or cleanup is manufactured after their poisoned prefixes. New normal/failure modes are registered in the default main and available as focused arguments.

## Resources factory tests

Actual configure_native_worker now must select Original and the passive default ModuleState matching KernelImports. Reconfigure preserves that exact authority. Preselected profile, preselected state and a nondefault startup/default-module mismatch must reject while all IO/executor/Graph targets remain unchanged and no injected startup runs. Selecting only native SDK adapters or only OriginalApplication lifecycle leaves HOST context policy unchanged.

The running full Resources/three-worker fixture intentionally uses its existing HOST configure path and now asserts WORD2.2. The existing `original_host_assembly` builds then closes a never-published Runtime; its native configure callback never runs and its kernel pair remains the default real pair, so it contains no real/fake WSA mix requiring a fabricated initial owner. No test in this executable links or executes DLL entry.cpp's compiler-CRT initial-claim registration. Primitive unchecked-owner tests and static DLL-entry review cover different boundaries; their evidence is not conflated.

## Baseline and candidate hashes

| File | SHA256 |
| --- | --- |
| before owned-server/server_tests.cpp | `0905773c98d744cd47dd3b47a740d14038b82e5e2de0b97399cba5c421e9dcfb` |
| before owned-server/server_integration_tests.cpp | `73679f010f505b6da45412f52121c84a7484fd6cb2dd5878635fcbcfbc590268` |
| before resources-factory/factory_tests.cpp | `247c7838c29b3b1790b327653bb8ffbc2d3f3a89ccebddbac118d11ff7b913c1` |
| before pending-source-status.json (62 sources) | `32f39d0a34781ed81008eef4c44f3b04b9ba1d620da0f6feb05723ff072020c9` |
| current server_tests.cpp | `dab286c792b921479cf35d562e10dcb4ef69021a890e226ab13c1d2ccc82f51e` |
| current server_integration_tests.cpp | `87bf17d709d9a9f508bb3f85489b6ef6ae4ec974f5d0f11db90dc58d0c908e8a` |
| current factory_tests.cpp | `992842d88053a6bb70621a080aa4d51132ab39ecf73a1cca5d7bede53f36b093` |

Static whitespace checks found no defect. Independent parent/alias review is pending at these candidates. No PASS runtime result, complete native CRT/EH/ABI equivalence, arbitrary module-unload safety or whole-DLL completion is claimed. An explicit user rebuild/test instruction is required before execution validation.
