# Exact Graph MessageCall implementation plan (read-only)
1. Authority: graph.cpp `fc90cf952eb5bb9230421d8dc97981ed6c7b60cf94d5b18f0cbd695ea0fdaf84`, graph.hpp `de539b9a711a527a824ff55d9b22ac67c6098fb3657ab3d0ee49bc0c0d853aaa`; no source/build/REA changes.
2. Replace the selected child `weak JobCall/job_binding` path (cpp32,55–105,370–415) with stable registered MessageCall rows, per-actor stack Admission before allocation, and explicit calls from existing np::JobLogic::process_native_job.
3. Preserve legacy creator execution separately; no TLS/currentjob lookup, temporary shared Dependencies swap, per-session strand or readiness barrier.
4. MessageCall stores exact Parent/Strings/Logic/SessionToken/control/JobPermit/thread/fiber, source checkpoint, own phase/checking/opaque/router flags and own writer inventory.
5. Registry owns shared MessageCall; call owner link is raw/weak, while the executing Logic/local owner pin retains Graph Impl and source lease. Do not make registry→call→Impl a strong cycle.
6. Under brief metadata gate validate row membership, Bound phase, exact token/Logic/control; outside it validate actual Parent permit/source, then recheck. New throws allocate only after unlocking.
7. Same-actor pending/active prefix aliasing rejects sticky; different actor calls on the SAME Logic/session remain eligible. Use Logic.failed_prefix(), not broad retained_prefix(), for sibling health.
8. Bind once to actual Parent/Strings; copy provider-lease ownership before metadata allocation, retain it through the entire registered call, no ambient authority from shared owner lifetime.
9. Add call-owned adapters for Effects, Random/guarded Clock, Events/EventMutex, AchievementMutex/SDK, Information, Content, Licensing, Node/Xml, Writer and no-response; each carries this MessageCall explicitly.
10. Build Info/Content/Licensing CallViews using one exact application_io::CallContext with raw checkpoint to MessageCall::admit; all Runtime/config/Online/attempt/auth/provider roots remain the originals.
11. Rebuild Router references for EACH dispatch from current shared configuration; its flags/writers belong to MessageCall. Do not clone configuration or silently bless simultaneous mutable licensing/string/map access.
12. Per-call random uses a private guarded Clock adapter over the same actual Graph TimeSource; no global Graph.opaque/checking field is used by child callbacks.
13. Local np::Dependencies owns call adapters and lifetime pin; MessageCall does NOT own that Dependencies object. Adapter backlinks are weak/raw; views borrow checkpoint, avoiding call→deps→call and GraphCall→view→GraphCall cycles.
14. Invoke Logic.process_native_call(parent,strings,out,req,chosenDeps,callCheckpoint); caller provides fresh/reset output, preserving native overwrite semantics.
15. Route only this call's observers/logs through its own opaque/admission guards, preserving source NoResponse/Unknown controls and actual shared-log locking/effect order.
16. Keep actual shared event_gate, event_values/map, initialized state, native_mutex and external Runtime achievement MutexState/Platform singular.
17. Event/achievement held/waiting flags and native pop old_offset/count/head are call-local; a foreign owner waits on the actual native mutex instead of triggering the old global nonrecursive guard.
18. Move event structure/map/offset reads under actual event ownership; map growth/payload ownership remain shared, with exact Parent resolvers and no whole catalog pins.
19. Native033d70 reads queue.count before lock: use ONE shared count_20 with relaxed atomic load/store at ALL source reads/writes; a small typed counter wrapper can preserve aggregate initialization/copy, comparisons/arithmetic, prefix increment/decrement and assignment.
20. Count becoming zero between peek and lock is legitimate; do not retain current lock_event's requirement for a live initialized head after another actor has consumed it.
21. No queue projection, pre-peek event lock, delayed pop merge or retry: native decrement changes that same shared counter immediately before unlock, retaining committed-pop prefixes if later unlock/error fails. Root owns neutral counter/header/source tests before Graph GO.
22. Never hold Graph metadata while waiting on event/achievement locks, running SDK, allocating, invoking source callbacks or retiring targets. Preserve LastError only around metadata.
23. Keep call published while retiring its writers, closing Licensing/Content/Information views under the live exact context, destroying local chosen Dependencies/adapters and source checkpoint targets, and performing final source/health proof.
24. Then unlink via nonallocating splice and retire compiler metadata outside the gate; unknown/partial retirement retains the exact row and poisons shared Graph health, without speculative unlock/free/view close.
25. Original make_logic, unrelated greeting, passive getters/snapshot/retained and source reaping do not fail merely because healthy MessageCalls exist; creator root admission uses its own active flag.
26. Session rows/growth remain short-gated and generation-checked; destroy a Logic only when THAT Logic has no active/failed invocation, not because another session has a job.
27. Snapshot reports aggregate activity without becoming a permission gate; its queued_events field reads only the relaxed atomic count (not map/offset), avoiding an added native event-lock wait or inversion.
28. Update/final drain/close remain creator lifecycle operations requiring no active/retained calls; normal drain can return false for healthy live work and reap outside its own Parent Operation, with no new unload condition.
29. graph.hpp changes document per-message source authority/root mutable-state bounds; do not expose MessageCall or a test-only raw Logic getter/public bypass.
30. Tests enter through actual np::JobLogic reached by real Profile JobHandoff/session construction; use existing prepared Graph fixture and concrete provider CallViews, not synthetic Effects replacing this route.
31. Current Profile/NativeJobs admission still serializes whole jobs: a simultaneous end-to-end Graph passing claim requires that coupled follow-up stage; Graph foundation tests alone must state this boundary rather than fabricate a public testing entry.
32. Meaningful eventual case is actual same-session A supported UnknownMessage dialog held and B actual GetConfig/Content/Core effect finished before A release, plus real native/FLS exit/reap and retained unknown prefixes.
