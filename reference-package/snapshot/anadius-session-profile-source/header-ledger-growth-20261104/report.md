# Grow large-string header metadata — 2026-11-04

The selected production Strings adapter now grows its allocation-header ledger instead of rejecting a valid backlog when all initial256 entries are live. It also republishes both long-lived Profile views before old metadata storage is destroyed. Payload allocation bases, ownership and the five primitive header lookup algorithms remain unchanged.

## Native basis and concrete restriction

`native-findings.md` distinguishes eight supporting fresh REA6.8.0/Ghidra12.1.4 records from two incidental address checks. Original identity/hash verified; complete replies retained in `rea-evidence-bundle.json`; native session closed.009eb0 allocates/copies by length and stores the aligned raw base at data-8;028270 dynamically grows the output deque;025a70 and00aa50 read each large allocation's base directly before freeing. None of those examined bodies uses the source's fixed256-row ledger.

The former source track() threw when all header rows were live. A slow consumer with a few hundred pending large replies/events could therefore fail even though the queue itself supported growth. Default headers=256 remains an initial allocation setting, not a maximum live-header count.

## Coherent production change

- `strings.hpp/.cpp`: Strings identity is noncopyable/nonmovable. Passive once-only bind_header_views registers the actual FrameReadContext and WriteFinishContext span fields. bindings() is explicitly a snapshot API; retained unregistered span copies must not cross growth.
- On exhaustion, track() allocates a doubled replacement ledger with vector-size/overflow checks while the old ledger/views remain valid. It revalidates the Parent operation and table shape, then repeats prune/duplicate/free-row lookup to account for same-shape compiler-allocation callbacks. If a row is now reusable, it updates that row and discards unused replacement metadata.
- Otherwise it copies current rows and installs the new header in the replacement, then performs a noexcept swap and publishes both registered spans before the old metadata vector dies. There is no opaque call between swap and publication. A local WinLastError guard covers only this additional metadata allocation/destruction path; existing raw-header validation is outside it.
- `profile.cpp`: binds frame.allocation_headers and effects.finish_context().allocation_headers after both fields exist, before the final potentially throwing Deques construction. All other selected response/message/Runtime adapters retain those contexts by reference. The normal Strings destructor never writes through the span pointers after their fields have been destroyed.

The five header lookup loops finish iteration and cache only scalar bases before opaque calls. They therefore need no algorithm rewrite. Graph uses the same Profile-owned Strings object, so its large events use the growing ledger too.

Allocation failure preserves the old ledger and views and propagates after any already-completed payload allocation; it does not free/replay that payload or promise an unpoisoned retry. Existing Parent/source poison/retention rules remain. Arbitrary concurrent or recursively growing use is not newly guaranteed. Initial host-setting validation and natural allocator/vector limits remain; no global readiness/unload/CRT framework was added.

## Unexecuted regression source and review

`profile_tests.cpp` manually registers real FrameReadContext/WriteFinishContext fields, holds260 aligned strings from initial256, then uses actual025480 error handling and025a70 success completion to retire different headers beyond the old span boundary. It checks raw aligned allocation bases, byte counts including39-byte overhead, hole reuse and final retirement. The error-category mock moves its donor prefix so ownership is not duplicated.

`graph_tests.cpp` exercises actual Profile constructor auto-binding:260 large queued events plus the initial DOWN,261 encrypted Native033d70 requests, independently decoded response/event pairs, FIFO and final allocation retirement. This covers actual Profile/Graph write completion, while the large FrameRead error case above uses manual context registration. An accidental omission of the unrelated fiber608 invocation was detected and restored before final hashes; all prior cases remain. These are source scenarios, not passing executable results.

One subagent implemented production changes, another added tests, and an independent subagent reviewed all five final source files. Full before copies, production/test notes and independent-review.md are retained.

## Delivery boundary

Three production and two test files changed in this stage. Since graph_tests.cpp was already pending,17 canonical source/header files now differ from the last actual successful full build. No compile, project test run, link, reconstructed DLL execution, original mutation or ZIP update. Existing DLL/ZIP and frozen build/link/package proofs keep their historical hashes. No numerical whole-DLL completion percentage or native ABI closure is claimed. Continuous REA/Ghidra/subagent work remains active under the user's instruction; explicit rebuild authorization remains required.
