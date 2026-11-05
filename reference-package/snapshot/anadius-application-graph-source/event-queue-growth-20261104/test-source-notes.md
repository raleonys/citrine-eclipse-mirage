# Event queue growth regression sources

Only `outputs/anadius-application-graph-source/graph_tests.cpp` changed. `graph_integration_tests.cpp` was inspected and its pre-edit bytes preserved but left unchanged: extending its tightly accounted prepared-child runner would require a separate scenario rather than changing existing protocol/packet expectations. No compiler, project test, link, REA session, target execution or packaging was run by this subagent.

## Positive scenarios

`plaintext_event_growth()` supplies70 alternating real `ShowIGO` / `ShowIGOWindow` requests through the existing Graph/Policy/Profile/Native033d70/XML/cipher/IOCP fixture with initial event capacity64. It requires all70 successful FIFO plaintext wire responses and71 pending events (initial DOWN plus one per command), with no event dequeue/log. Ordinary graph retirement must drain the enlarged backlog and release every Parent/Domain allocation.

`wrapped_event_growth()` uses initial event capacity8 and13 real encrypted GetConfig requests. Existing authoritative after-completion hooks first enqueue6 distinct events plus initial DOWN. After3 actual native event pops,4 pending events and the nonzero head remain. Enqueuing6 more events crosses the old wrapped map and requires growth;10 pending events must survive. The oracle checks the exact13-event FIFO, including inline SSO,15/16-byte boundary,4095/5000-byte heap/header boundary, and embedded NUL bytes. An independent client decrypts all26 response/event wire frames and compares them with expected response logs and FIFO payloads. Ordinary close and allocation/frees accounting check that moved prefixes are retired exactly once.

These use the existing actual Native033d70 path and therefore exercise event-map refresh/consumption in NativeProfile even though no separate prepared-child runner was added. The fixture SDK imports are controlled; the scenarios do not install real network hooks or execute the reconstructed production DLL.

## Obsolete failure removed

The former `retained_child("overflow")` branch, its capacity2 setup, forced overflow hook, exit605 assertion and normal-main `isolated(L"overflow",605)` invocation were removed. A normal growth test replaces the old expected rejection; no terminal expected-failure case is relabeled as a successful child.

Read-only search found the old overflow605 assertion in historical `work/b46-core-proof.py` and historical registration records. No active `outputs/source-workspace/build.ps1` or current graph build helper dispatches it. Those historical proofs/helpers were left unchanged and describe their old B46 source; the B46 helper must not be used to validate the new source.

Final authored graph_tests.cpp SHA-256: `017b75df2008e2a9507b3203bdb635a2338f3287ddf7726dc82aa8c93784cec1`.

Unchanged graph_integration_tests.cpp SHA-256: `a4ca98a08b131de32822d29df3ae5852629d5c2b38ec721667e211adbc8ae042`.

Manual author inspection only. No passing executable test result, original ABI equivalence, arbitrary concurrent mutation or completion-percentage credit is claimed. Root owns independent review and final source-hash registration.
