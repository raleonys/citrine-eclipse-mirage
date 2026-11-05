# Prepared deque/string fixture and coupled source review

2026-11-05. No remaining blocking finding in the reviewed production changes. STATIC SOURCE ONLY; NOT RUN. This reviewer authored only handoff_tests.cpp after the verified90-pending checkpoint. Its two new default-main cases are frozen at714af17f024cd4143874e969e2f27bf75e6d00142adbc3584758ae26cf8ee94d; independent fixture review belongs to the peer/root, not an independence claim of this authorship report. No compiler, tests/process launch, linker, original/reconstructed DLL execution, REA query, network or ZIP update occurred.

Before/current proofs for all14 candidate sources are in sdk-review-source-hashes.json, SHA256 a0b947324906542839f0b74ff3c1f10a1c83e9ad0101805961e35d2b8964f067. The table below records current bytes; historical build proofs are untouched. The peer independently reviewed exact714af17f fixture bytes with no blocking finding; handoff-fixture-independent-review.md SHA256 adf7fcad8229532a80b92267c6da1b53d51bce3dd4cb179cf4e0b52460900af7 records that distinct source-only review.

## Actual prepared deque case

parallel_owned_deques() reuses the existing real DomainStorage/NativeThreads/Event/Task/PreparedMemory helpers. Original constructs two real Parent control-owned queues and one Deques under explicit Concurrent Parent policy, then seeds both maps8 through actual026670. A and B acquire distinct real offers/JobScopes and typed permits.

The existing native HeapAlloc pause hook is parameterized by extent; default113 preserves the earlier IO scenarios. Only this fresh fixture selects128 and A's actual thread. A is held after physical heap allocation inside the actual026670 map8-to16 path, with its old queue prefix and a private Parent Reserved allocation. It is not held merely before source entry.

B's same-queue mutation attempt rejects without poisoning A. B then runs actual026670 on its different queue, disposes that actual control/map through strong/weak source release, returns its Operation/offer, and physically exits the native wrapper/FLS path. Original joins and reaps B BEFORE releasing A. While held, Parent has only A's old live map plus A's Reserved allocation; cumulative successful/allocated counts are3 (two seeds+B). Original provider close returns false promptly with Ready/active preserved. A then completes with its permit valid; final cumulative counts4/4 and one live map are checked, followed by actual owner/pending-socket disposal and normal provider/backing drain.

All waits/native joins have the established finite bounds. A is never released just to turn blocked B into a delayed pass. Exact per-queue growth conflict is not presented as a Session/Logic body strand or permission to mutate an arbitrary shared queue.

## Large header/ownership transfer case

parallel_string_transfer() keeps the ORIGINAL actual Parent Operation alive while children are eligible. Original assigns two aligned large heap strings, records exact raw-base/extent proofs, creates a real Parent SDK-range lease on the transfer payload and prepares two real child offers. Strings.relinquish(transferred) declares the end of ALL original aliases to that allocation, while original keeps its unrelated request view and its Operation.

A calls Concurrent prune and reads only the immutable request, then remains admitted. B first proves relinquish did not release the actual SDK range lease by a guarded direct Parent free. Original releases that real lease while its Operation is still active. B then physically replaces/frees the transferred original allocation, proving it did not wait for Root scope exit and that A housekeeping did not pin unrelated strings. B performs actual009eb0 response copy; relinquishing B's request view still cannot release original/A views. Its Operation ends before actual JobMemory return; B native exit/reap occurs before original response consumption.

Original consumes the transferred child response through actual020cb0, real OwnedIoMemory task ownership and actual022280 discard. FrameReadContext carries the exact Parent header resolver. TransferApi checks the actual raw base/sized extent and physical heap retirement once, while A's unrelated payload remains live. The envelope is owned/freed by the real IO provider. Null response control bounds this case to header/payload/task cleanup; it is not claimed as a full session strong-owner or network response test. A subsequently validates its permit and rereads the actual Parent request bytes before final request drain.

The three Concurrent legacy header-observer APIs explicitly throw; no empty span or zero count is treated as drain. Authoritative Parent buffer/allocation/SDK-borrow counters prove the intended prefixes. Coordination flags are observational test barriers only; actual source admission comes from real Parent/JobScope/PTD, with no scheduler work/readiness/PTD mutation.

## Coupled implementation assessment

Reviewed the full two Deques files and changed Parent/Strings/header consumers, Profile producer handoff and three controlled socket-test diffs. No concrete blocker remains.

Concurrent Strings has no mutable header union. track resolves only the requested exact header; prune performs admission without scanning/pinning other actors' allocations. Host ledger growth, duplicate/free-slot reuse, bound span publication and metrics remain on their prior branch. Header observers throw in Concurrent mode rather than invent a zero metric.

Parent relinquish resolves one live exact ticket and removes only the CURRENT actor's one borrow node, retiring metadata outside its mutex. One node covers all that actor's aliases to that ticket, so the documented precondition is last use of the ENTIRE allocation, not one independent subrange. Other-ticket, SDK and foreign-actor pins are untouched. owns_output_map is an exact element-count metadata predicate without payload pinning or union-catalog exposure.

Deque Concurrent admission reserves exact queue/actor membership before metadata allocation. Per-call checkpoint, permit, map resolver, byte/slot adapters and unknown prefix remain separate stable rows. No body mutex covers allocator/checkpoint/source work. Same-actor nested growth stays sticky; a foreign exact-queue conflict is a nonwaiting precondition rejection. Successful rows/targets retire outside the mutex after source/permit checks, while unknown descriptors remain retained and nonreplayable. Healthy aggregate phase stays Ready and active reflects actual admitting/running rows. Host retains its Growing/global-active and cumulative metric behavior.

All three normal destroy helpers use authoritative allocation-header lookup with Miss/Ambiguous rejection, followed by unchanged alignment/base/extent/free order and no legacy fallback. Profile installs the resolver before publication and relinquishes the child closure's distinct owned heap at prepare_job's last Root payload use; this does not drop the separate receive-input allocation that Root must continue rearming.

This prerequisite checkpoint does NOT enable actual Original Concurrent selection or close upper NativeJobs/Profile/NativeProfile/Graph per-message ownership and same-session/distinct-session runtime validation. Arbitrary cipher/event/shared payload mutation, foreign SDK synchronization and native ABI/EH equivalence remain unproved. All new assertions are unexecuted.

## Exact candidate bytes

| File | SHA256 |
| --- | --- |
| `outputs/anadius-session-profile-source/strings.hpp` | `444f6d71099926110e8781309f2fca9398dad5350ff6e6d0bfd34f9c3577053c` |
| `outputs/anadius-session-profile-source/strings.cpp` | `ac810821216394fcfbb741e5639f866fded3dc15ef2d3179d86a15d4eff52bed` |
| `outputs/anadius-session-memory-source/memory.hpp` | `32e37805f2b01e992b4607a25bdff114dcac3f71d43280c3b48ffaed21dcdc19` |
| `outputs/anadius-session-memory-source/memory.cpp` | `84cb386fa4f0093c431376bf14e292e379c19777ec2f9d51577e6a7769987276` |
| `outputs/anadius-owned-deque-source/deque.hpp` | `cf38e757e6189ca3a6a2cd3c9016c504401c6941b7a50cff10c1f225aa3cb35e` |
| `outputs/anadius-owned-deque-source/deque.cpp` | `35bcec259f98a2f65e136f6d257b36cdf1be699dba689d6b814b50cc7474d10d` |
| `outputs/anadius-session-profile-source/profile.cpp` | `7413d158e406bdcdca2b308f495dbc4a71db265df313566f17e741f4cb892676` |
| `outputs/anadius-worker-handoff-source/handoff_tests.cpp` | `714af17f024cd4143874e969e2f27bf75e6d00142adbc3584758ae26cf8ee94d` |
| `outputs/anadius-socket-source/message_dispatch.cpp` | `a406a1c96205865e25a13be944ba77a1a0404c0455022661e2a2e5fe4c69acaf` |
| `outputs/anadius-socket-source/response_dispatch.cpp` | `20a45675686aaaa6fcc243de1b5ff7350277dac6f21f2dc01fd911c48e6e0214` |
| `outputs/anadius-socket-source/owned_response.cpp` | `59f6e5610bb81faab3df29a31211e45a77bb8cd3e2f0985320dcc34239af019c` |
| `outputs/anadius-socket-source/message_dispatch_tests.cpp` | `5980c32365669191912281ec39e7ccbc74dc9cddff27a53e43313504694f87e2` |
| `outputs/anadius-socket-source/response_dispatch_tests.cpp` | `b35b224166ecc0403fa64f0e61c218db0ed2be2c46bb79e71af7f14cf47ac0a4` |
| `outputs/anadius-socket-source/owned_response_tests.cpp` | `a8d5a1ca491d13081ad693ab9ae5513fb4450f65d7e8548e2cf65621e38a90be` |

