# Independent root static review — OriginalDefault regression

STATIC PASS for the final environment_tests.cpp diff authored by another agent. No compiler, project test, linker, target/DLL execution or ZIP update occurred. This review does not claim passing executable results.

The fixture's classifier argument defaults to Host, preserving the existing4100-reference and eight-negative-case scenarios. The new OriginalDefault case keeps its real source row/context/capability alive, invokes the actual recovered023dd0 constructor, and checks the exact empty/state0 sentinel result with no extra owned factory/copy/move count. Empty query rejection is a source boundary and leaves the real provider Bound; the diagnostic clone is then released.

The receive/send portion uses two real OwnedIoMemory projections, transferred handler strong references, typed manually armed loans and the actual027710/027a20 completion bodies. It does not call native async-start imports. The count check is consistent: captured root strong1, two handler strongs and two SDK-loan strongs produce5; receive body sees4 after its SDK loan/allocation retires, send body sees2 after the remaining loan/allocation retires, and both handler releases return to1. Pending/range-pin/allocation counts return to their captured baselines. The fixture worker has no active TLS cache in this direct completion phase, so allocations end before callbacks. Scheduling methods fail if reached; each direct body must run once. Work_count remains unchanged by this bounded constructor/projection/completion scenario; actual async-start/dispatcher work accounting is not claimed.

The same-mode message copy remains a registered state1 projection. The real response schedule consumes its PostedFunctor through the retained force-post path; the fixture's existing source worker drains it once. Root/copy rows are released before the session strong reference, with exact ledger totals. Source addresses remain valid for all operations.

Graph integration bytes remain identical to their before copy. The proposed worker-delay fixture is Explicit, so it was correctly preserved rather than converted to OriginalApplication. Graph's new dependency selection is independently statically reviewed; no new functional OriginalApplication Graph or child JobScope test is claimed here. The direct empty-query case motivates the production helper's sentinel check but does not execute JobHandoff::clone_environment.

Reviewed source SHA-256: `86fd0192a44cd64493a4d838f61a7f69cad1af298de1bf1453be314c5c6b7252`.

Unchanged graph_integration_tests.cpp and before copy: `6d6ca930e82f20dc81c9d0390f53bf117e17be501e171bd835010ae8f98357ad`.
