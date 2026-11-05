# Independent Policy and server composition review

2026-11-05. Read-only independent review of the four root-authored files below, compared with actual stage `before/` copies. All four before-copy hashes match `before-manifest.json`. I authored the separate Profile changes and do not label those independent. No compiler, project tests, linker, DLL execution, packaging, source edit or new REA session was performed for this review.

| File | Exact reviewed SHA256 |
|---|---|
| anadius-session-policy-source/policy.hpp | `720a15b840365f5e87fe3cc7e0ae4e98b88df195dd5d1f078aea723eff854aea` |
| anadius-session-policy-source/policy.cpp | `6338a6ecb4d7c3d736057da418a49fe18debb300ff58131ff85468248554be23` |
| anadius-owned-server-source/server.hpp | `eca6cb891d6561312320431556c48857c55ddd946381e3619f7180a2589f5cff` |
| anadius-owned-server-source/server.cpp | `9e96247b84ff6396809c4f5d2bfda12b887b564fa8ea3437ac010a69878b4c9b` |

Result: **no blocking finding in this source scope**. This is static review, not a concurrency execution result or a complete native-ordering claim.

## Policy actor admission, retirement and actual cleanup

The child path validates the real Parent permit before reserving a nonallocating exact thread/fiber stack admission. It detects both pending and published same-actor reentry. List node and CleanupCall allocation occur outside Policy.gate; publication splices the prepared node before removing the pending admission. The per-call permit/Parent/thread/fiber, registered identity, checkpoint state and shared sticky Policy phase determine authority; Concurrent cleanup does not borrow the original worker's `active` flag or locate a current child through TLS.

Each CleanupCall retains the existing provider lease and immutable actual Parent/permit. Its metadata rejects closed/failed/wrong-actor/wrong-Parent calls. Validation surrounds actual source observation and the borrowed admission callback. Callback reentry is sticky and failed prefixes stay registered. The actual `v.exits.cleanup_current_thread()` remains at the same semantic point and still uses the existing ExitWaitRegistry/recovered cleanup implementation; only the registry's real narrow native synchronization is shared. No handler/body mutex, replacement wait registry, readiness barrier or deferred exit cleanup was introduced.

The normal cleanup count/cleaned prefix commits under metadata after the actual effect. Opaque checkpoint target retirement occurs while the exact call remains registered, followed by live validation. Registry unlink is a metadata splice; list/call/lease destruction occurs outside Policy.gate. Unknown registered prefixes stay registered and Policy remains sticky; no second cleanup or guessed native rollback is performed. Admission/checkpoint ownership remains published across metadata preparation and opaque target retirement. Metadata guards preserve incoming LastError; root cleanup's newly guarded counter update preserves the actual prior leaf error state. Existing child cleanup's outer error-preservation contract is retained.

Concurrent original calls and healthy sibling cleanups have separate execution state. Serialized still rejects overlap through the registry/admission checks. Passive Snapshot.active reflects pending/registered child work; cleanups is updated under the same metadata gate on both root and child paths. Close and external quiescence return false for healthy registered work before setting closing, and final close/attestation rechecks cover a child admitted during earlier external observations. A successful closing reservation prevents later child publication from racing optional Ports/lease retirement.

## Explicit Parent policy composition

The new server dependency is appended and defaults to Serialized. Both owned and Prepared Borrow constructors validate its enum before constructing/adopting the Parent and pass the exact copied selection into SessionMemoryDomain. Selection is stable in the owned Core; it does not use thread-local or per-callback switching. Existing default callers retain Serialized because neither resources-factory native selection nor DLL entry was changed in this stage. An explicitly supplied Concurrent value enables only this Parent construction choice and does not attest the rest of the provider graph.

## Limits kept explicit

The separate singleton `finish_environment_work`/OwnedEnvironment execution restrictions remain unchanged; this review does not certify arbitrary concurrent tracked release, shared licensing mutation or generic custom providers. NativeJobs.source_gate and the source's current original-worker response/021770 tail remain separate outstanding integration/order boundaries. The new actual-Prepared fixture and Policy source tests require their own static reviews, and none has been executed here. Unknown compiler/exception cleanup and native ABI parity are not established by this source projection.
