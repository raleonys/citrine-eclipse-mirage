# Stateless concurrent headers and exact view handoff

2026-11-05. SOURCE ONLY. This author changed seven production files: Parent memory.hpp/.cpp, Profile Strings.hpp/.cpp, and the three remaining socket header consumers message_dispatch.cpp, response_dispatch.cpp and owned_response.cpp. Root captured their actual baseline in this stage's `before` directory at the 90-pending checkpoint. Root owns Profile wiring/publication; the other agents own Deques and regression sources. No compiler, tests, link, DLL execution, original mutation or ZIP update occurred.

Actual production policy remains Serialized. This is another lower prerequisite; per-message NativeProfile/Graph/provider integration and actual same-session/distinct-session progress remain open.

## Strings mode and observations

The immutable mode derives from Parent.operation_policy(). Serialized keeps the existing header vector, bound live spans, growth/reuse/prune and live_headers count. Its current tests with more than256 aligned headers retain their original contract. The old short track paths and all native allocation/free string algorithms are preserved for that mode.

Concurrent constructs no mutable header ledger. track validates only the supplied string's exact header through Parent's authoritative AllocationHeaderResolver, then the existing alignment terminal check. It does not publish, duplicate or prune a shared vector. read/assign/copy/release continue through the same Parent byte authority and actual owned string prefixes. free_sized removes no unrelated header state; Parent's live allocation is authoritative.

Concurrent prune performs the required Parent actor admission check and returns without scanning a union of headers. It neither pins unrelated payloads nor certifies drain. The legacy observations bindings(), bind_header_views() and live_headers() explicitly throw in Concurrent mode. They do not fabricate an empty catalog or zero count. Actual allocation/drain observations belong to Parent's real ledger. header_resolver() is null in Serialized mode and returns the exact Parent capability from an admitted Concurrent operation.

Root's Profile mode branch must use literal empty legacy spans plus this resolver in Concurrent FrameRead/WriteFinish contexts and skip bind_header_views. It must keep legacy spans/binding in Serialized mode. This author's changes do not silently enable the unresolved upper route.

## Exact operation-view release

New Parent API:

```cpp
void relinquish_operation_view(std::uintptr_t address, std::uint64_t bytes);
bool owns_output_map(std::span<WriteFinishString*>) const;
```

relinquish_operation_view requires the current explicitly admitted actor, locates exactly one Live nonbusy mapped ticket covering the requested window, and removes only that actor's deduplicated ticket view. This is **FULL-TICKET last use**, not a range-local release: every old producer span/alias of that allocation must have ended. It creates no replacement borrow and does not free payload, end a JobScope, invalidate a permit, or decrement any SDK/other actor pin. If the actor has no view for that live ticket it is an idempotent metadata no-op. Miss/ambiguity rejects before changing ownership. Borrow-node retirement happens outside Parent gate with LastError preserved.

Strings::relinquish(prefix) supplies this exact operation for a Concurrent owned heap string's data/capacity+1 window after validating its defined size/capacity. It does not clear the prefix or free it. SSO has no Parent heap ticket; Serialized is a compatibility no-op. A caller cannot use it to release another actor's view or a pending SDK range borrower.

This closes a necessary lower ownership-transfer mechanism: if Root assigns/copies a fresh request, its byte/header views otherwise last until Root Operation exit. Native starts/rearms before that exit, so a child can legitimately mutate/free its moved request while Root is still processing the completion. Requiring Root's whole Operation to end would invent a progress dependency. The explicit release instead records actual last use before the consumer becomes eligible.

## Precise current Profile transfer boundary

The current source chain is Frame.construct_message → move local message into FrameMessageClosure → Frame.begin_frame_thread → NativeJobs.start_prepared → Application.prepare_job → native thread launch; detach subsequently opens the child barrier. Root can call Strings::relinquish(row.closure->message_00) in prepare_job after the offered capability is adopted and match_handoff postchecks succeed, inside the existing failure guard and before returning to launch. No eligible child exists yet.

After that point the Root preparation/detach checks inspect the stable closure pointer, owner/function tokens, job/session generation and strong ownership; they do not read request heap bytes. Request snapshot/prefix comparison in complete_job occurs only after the child Returned state/receipt. The old local frame string is already reset to SSO, so its subsequent cleanup does not borrow the moved allocation. The shared session input remains a different allocation and keeps the views required for consume/rearm.

For the CURRENT reverse handoff, child process_message finishes track/snapshot work, then its Parent Operation is destroyed before return_memory can mark the acquired JobMemory Returned (the JobScope enforces this). NativeJobs marks Stage::Returned/posts its receipt only afterward. Original completion therefore receives the child result after those actual child views ended. No additional wait is introduced. If later native asynchronous response publication makes a new actor eligible before that producing scope ends, the same explicit last-use release must be wired at that real boundary; these seven files do not claim every future async transfer is closed.

## Header-consuming normal algorithms

The three remaining direct header-span loops now call lookup_allocation_header with their existing FrameReadContext resolver. The owner is authoritative when nonnull; Serialized null-owner fallback retains the existing bounded span semantics. Each consumer preserves its exact miss/ambiguity message, capacity threshold, bytes+39 arithmetic, alignment invalid_parameter ordering and immediate sized-free call. This is logical owner lookup, not a native std::string/CRT ABI claim.

Affected consumers are message request/response cleanup in message_dispatch.cpp; response-dispatch local cleanup in response_dispatch.cpp; and moved owned-response task/local cleanup in owned_response.cpp. Previously converted FrameRead/WriteFinish helpers remain unchanged here. All five selected consumers can consequently follow one Parent allocation across distinct Strings instances without a stale duplicate ledger.

## Deque preflight predicate

owns_output_map performs an admitted metadata-only query over Live nonbusy typed Map tickets. Both the exact array object and full element count must match; a prefix/subwindow is not accepted. It creates no map or payload pin and returns no borrowed catalog. The Deques author uses this only for the initial queue preflight. It must not be inserted at every growth callback: recovered026670 legitimately frees the old map before updating the owner's old map fields.

## Frozen source SHA256

- memory.hpp: `32e37805f2b01e992b4607a25bdff114dcac3f71d43280c3b48ffaed21dcdc19`
- memory.cpp: `84cb386fa4f0093c431376bf14e292e379c19777ec2f9d51577e6a7769987276`
- strings.hpp: `444f6d71099926110e8781309f2fca9398dad5350ff6e6d0bfd34f9c3577053c`
- strings.cpp: `ac810821216394fcfbb741e5639f866fded3dc15ef2d3179d86a15d4eff52bed`
- message_dispatch.cpp: `a406a1c96205865e25a13be944ba77a1a0404c0455022661e2a2e5fe4c69acaf`
- response_dispatch.cpp: `20a45675686aaaa6fcc243de1b5ff7350277dac6f21f2dc01fd911c48e6e0214`
- owned_response.cpp: `59f6e5610bb81faab3df29a31211e45a77bb8cd3e2f0985320dcc34239af019c`

Independent static review of these seven files and Root's four wiring files is `root-and-strings-independent-review.md`, SHA256 `fa70abe2a60fb15711cb6d28a0ce3c23c8bc5751095beb049ece8eca9cfb81ac`, with no blocking finding. Proposed/added source cases are not execution results. Exact view storage lifetime does not authorize arbitrary shared payload writes, clone shared application/Online roots, infer native same-session strands, or provide new readiness/unload conditions.
