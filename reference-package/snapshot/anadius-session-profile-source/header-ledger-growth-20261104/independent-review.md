# Independent static review — 2026-11-04

Reviewer: `registry_sdk_integration_review`, separate from production and test-source authors. Final static review PASS within the selected source route. No compilation, project tests, REA session, linking, DLL execution or packaging was performed by this reviewer. Test scenarios described below are source additions, not passing executable results.

## Production source

The fixed reconstruction header-ledger ceiling is removed by growing its metadata vector when no matching or free entry remains. The constructor bound now limits the initial allocation; growth is checked against vector max_size and arithmetic limits. This grows reconstruction bookkeeping, not an asserted native256-row structure or recovered native allocator ABI.

Growth allocates a default/zero replacement before reading/copying current source rows. It then revalidates the Parent Operation and checks the old ledger's size/data identity, avoiding captured iterators across a compiler allocation callback. A concrete same-shape callback issue identified during this review was fixed: after allocation it prunes and rescans duplicate/free entries again. A callback that inserted the same header into a new hole therefore cannot cause a second appended record. If a callback relocated the ledger, the outer operation rejects instead of reading stale storage.

The completed source rows are copied after those checks. Vector swap and both registered span-field assignments have no intervening opaque callback or throwing allocation. Both live consumers see the new ledger before the replacement destroys the old metadata vector. Growth-specific PreserveError also spans metadata allocation/deallocation and preserves its incoming WinLastError. The existing payload allocation/free order remains selected normal-return behavior.

`Application::Impl` binds two stable span fields after Effects and Frame exist: FrameReadContext::allocation_headers and Effects::finish_context().allocation_headers. Strings is noncopyable/nonmovable. The actual Impl does not relocate these fields during use. Their member destruction precedes Strings, whose default destruction does not dereference the stored span pointers. The standalone bindings() API is explicitly a snapshot; callers must not retain copied spans across possible growth.

All five selected lookup paths were reviewed: frame_read, message_dispatch, owned_response, response_dispatch and write_finish. Four use the same FrameReadContext by reference; write_finish uses the independently rebound finish context. Each header lookup completes before an opaque invalid-parameter/free callback. No examined lookup retains a header iterator across such a callback. Nested composed/owned/queued contexts retain these context references rather than extra persistent header-span snapshots.

Failure limits remain explicit: metadata allocation can fail after the source payload allocation completed. Existing ledger/views stay unchanged and no completed payload allocation is replayed or automatically rolled back here; the source error/prefix-retention policy remains. Reusable retry, general concurrent/reentrant Strings use and original CRT/EH equivalence are not established. Ordinary Profile/Parent actor and lifetime requirements continue to apply.

## Test source review

The Profile core scenario holds260 large aligned strings simultaneously with an initial256-row ledger, binds both actual context types, checks rebinding and aligned raw-base provenance, then calls actual finish_frame_read_025480 and finish_write_025a70 paths for entries beyond the old span. It verifies sized frees and metadata-hole reuse before releasing all payloads. The retained grown snapshot is used only while those operations remain below the enlarged capacity.

The Graph scenario exercises actual Profile automatic binding:260 distinct large queued events plus DOWN, followed by261 encrypted native requests. It checks the complete event FIFO, independently decrypted response/event pairs and normal Parent/Domain retirement. This is a source-level integration scenario using controlled SDK boundaries; it is not observed original-DLL runtime evidence.

An unrelated deletion of the existing isolated fiber608 invocation was flagged during review and restored. Existing historical proofs were not rewritten. No allocation-failure injection, compiler new-handler reentry execution or arbitrarily large resource-limit test was run.

No concrete blocking defect remains in the inspected final patch. Compilation and actual runtime behavior remain unverified pending an explicit rebuild instruction.

## Reviewed source hashes

| File | SHA-256 |
| --- | --- |
| session-profile/strings.hpp | 18db9bc458ab1e14f50bbba812d4928ef8a02c7fb4b25d7b9fb860845caec7a5 |
| session-profile/strings.cpp | 48da5ac125507c245caf3cf7ba56b94112c79e27e4e327d954b4d30f9603d797 |
| session-profile/profile.cpp | ea0220ea3db901da0d0c43483bc9fc8167ff14652bb089d01c187ddf27d793e6 |
| session-profile/profile_tests.cpp | 007fd4e0fbb2e7bb9aac01d895778164263f991e6506489f2d8879ec1c1e23cf |
| application-graph/graph_tests.cpp | 732042f790d54a16472d76398eb570d40e29bbefaddac4a09b8294376fcfe606 |
