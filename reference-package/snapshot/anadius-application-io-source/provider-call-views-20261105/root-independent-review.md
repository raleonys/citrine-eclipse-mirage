# Root independent provider and prepared-fixture review

2026-11-05. No remaining blocking finding in the five files below. Static source review only, NOT RUN. Root authored the separate new CallContext header; its independent review belongs to the peer reports, not this review. The two provider unit-test files are also covered by peer reviews.

Information has registry-owned view states referencing one owner, with a separate wrapper backing pin. Content likewise uses a nonowning execution-to-root link and wrapper-only root ownership: no root-registry/execution strong cycle. Both share actual selected lower identities, source roots and sticky failed-owner health. Each private execution owns its own active/checking/closing state; Content's GuardedFiles and Engine are private and retain the exact original Runtime/Paths/Online references. Same-actor checks cover pending preparation, other active handles and legacy child scopes without excluding a different actor or mere idle open view.

Allocation/exception construction, checkpoints, provider/SDK work and captured-target retirement are outside new registry gates. Views remain registered through successful close validation/retirement; unknown outcomes retain their own actual prefix. Normal closed handles no longer invoke source authority or destroy creator-owned options. Root facade destruction seals and retires its original options only after registry drain. The shared CallContext's final metadata LastError contract matches existing admission behavior while preserving actual SDK effects outside checks.

The realPrepared fixture holds an adopted raw output at its initial-state call, and uses a second actual native child/view to finish a distinct counted XML write plus shared-owner Information/UTC work. Coordination orders the non-threadsafe test clock/stream counters; file rows are mutex-protected. Original default work is exercised while B views remain open. Foreign and expired factory contexts reach no borrowed callback. Review found and fixed reaping inside the original Operation; all physical joins/reaps now occur outside it while preserving B-before-A-release.

The704 variant tests an exact unknown exception from A's already-adopted raw output after B has finished. A's still-valid context is denied by shared owner failure; its raw stream is not written, closed or destroyed. After actual A native/FLS exit and reap, Root also rejects a legacy call before intentional process exit. No guessed cleanup or normal final-drain proof is claimed for that failed case. The final B-context test independently mutates admission LastError and verifies entry preservation before factories, then resets its actor-owned test field.

This is lower provider/ownership coverage, not actual Graph/NativeJobs, network, arbitrary business-state or SDK concurrency validation. All assertions remain unexecuted; production policy remains Serialized and historical build/DLL/ZIP proofs remain unchanged.

| Reviewed file | SHA256 |
|---|---|
| outputs/anadius-information-effects-source/effects.hpp | 3a77b6470f2e0f0751498a14559c1274f6ea8b469cd8eda60dba53ed8298186b |
| outputs/anadius-information-effects-source/effects.cpp | a01949674e6f3ac384c31fc534f1e1ed1ff9abc32b824d7423ccaf6715fbdb39 |
| outputs/anadius-content-effects-source/effects.hpp | 5904d4c70e8910b63f10be9215076680300ab0089990e8cccf1f028690f24a91 |
| outputs/anadius-content-effects-source/effects.cpp | 58b482063f9fa7d4263de0f6f21aa33cc5d5983b38ac2f96dc4b287cbe4b1812 |
| outputs/anadius-worker-handoff-source/handoff_tests.cpp | 267d414545fff25d4b09e7ff62562ddefce8ccc525e4267e0e749a6bfbef3f97 |
