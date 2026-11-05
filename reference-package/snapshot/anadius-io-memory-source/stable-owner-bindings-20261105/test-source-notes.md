# Stable IO owner regression source

2026-11-05. Authored only `anadius-io-memory-source/io_memory_tests.cpp` and `anadius-worker-handoff-source/handoff_tests.cpp` after root verified this stage's before copies (86-file pending checkpoint). No compiler, test/process execution, linking, reconstructed/original DLL execution, network or ZIP update occurred. Neither source case enables the actual Original Graph composition.

| Source | Before SHA256 | Frozen authored SHA256 |
| --- | --- | --- |
| io_memory_tests.cpp | `7ec876e70eac83d60838b56810436f857dfeca02376291c1cdad959b2aa10fbf` | `4bfd141e524e1f140ce03855762d63bd9e16e12a542795b995a41e80d3e062b1` |
| handoff_tests.cpp | `23fedea9c99480b33ef8e8c5b776effa9ef168c0d12e109b22c3239bad837f30` | `b676e53f01d240846fd0e0a3d268e38b8381110b2354d81525162b1a4fea1604` |

## Serialized exact resolver case

The existing single-thread real-heap IO fixture gains `exact_owner_resolvers()`. It checks passive resolver identity/current capacity, requires explicit Parent admission, resolves the exact OperationStorage and both typed OVERLAPPED/CancelOperation packet identities, and continues through index growth using the copied resolver-backed environment. `registry_capacity()` must observe the actual1-to2 growth rather than return a constructor setting; this matters to later Runtime/Stop receipt bounds. In Serialized mode `packet_binding()` and resolver results must still name the authoritative mutable legacy packet row. The old packet-tamper negative is unchanged, so a detached compatibility copy cannot silently make it irrelevant. Miss preserves an unrelated caller output. Freed storage identity cannot resolve through the owner.

Existing actual receive/send pending-loan/growth tests, range limits, extra owner strong pins, cancellation not implying completion, duplicate/stale completions, descriptor tampering, cache generation reuse, reentrant completion and isolated unknown SDK prefix cases remain intact. Their assertions are authored source and historical runtime proofs retain their historical hashes; no rerun is claimed.

## Failed strong-release retention

Independent production review found that the initial IO candidate detached the pending owner/range pin before checking the source strong count. A failed guard would then unwind the stack pin and lose the previously retained prefix. The authorized `--consumed-strong` regression uses actual PinnedAsyncReceiveApi to establish one pending receive, range pin and three strong references, then deliberately supplies invalid source count1 in its own isolated process. Actual completion must throw before callback/environment move, retain IO Poisoned with one record/pending loan/range pin and the exact unchanged packet/projection, and leave the count1 prefix untouched. The child terminates with `_Exit(0)` after those source assertions, without fake cleanup. Default main explicitly launches this mode and the unchanged `--unknown-sdk` mode; neither process has been launched during authoring. Production correction `812044f220b13cbb32d4d4e821879d80181801b6f1b10066b5a4f3e6ec3b763b` is reviewed separately; existing duplicate-completion and outside-CAS sticky-phase checks also remain intact. Test source bytes are frozen, with execution validation pending.

## Two real prepared child scenarios

`parallel_owned_io(false)` and `parallel_owned_io(true)` are separate fresh fixtures registered in default main. They reuse the already established controller-owned native Domain, NativeThreads, Event, Task and PreparedMemory helpers. The actual original worker creates Concurrent Parent and OwnedIoMemory(initial capacity1), then prepares two real child offers. No manual PTD, work-count, readiness or lifetime attestation replaces those authorities.

An independent guard row stays live throughout both scenarios. Original captures its actual storage descriptor, exact packet binding, OVERLAPPED, generation and one registered stable external packet pointer field, plus a by-value resolver-backed PostedEnvironment. These are source-owned projections, not a native ABI cast. B allocates seventy simultaneous Posted rows to force registry growth, resolves the exact guard identities and verifies packet contents/external alias remain unchanged. It performs a real Posted completion while A remains held, immediately frees its other rows, verifies stale generations reject and leaves IO Open. After B's Operation ends, the actual original native thread joins and physically reaps B before releasing A.

In the completion scenario A invokes actual `complete_posted_027120`. That source frees its envelope before invoking the moved functor. The test holds the moved functor, not the retired envelope; only the independent guard is inspected afterward. B may reuse the freed slot while A's completion Call is still active, and A's old explicit generation must remain invalid. This tests that neither a global IO gate nor a global active-call exclusion covers the foreign callback and that no old projection is accessed after callback-driven free/reuse.

In the allocation scenario an opt-in existing physical HeapAlloc adapter hook targets only A's113-byte application allocation. It registers the actual successful allocation under the existing Ledger mutex, releases that mutex, signals entry and waits finitely before returning the pointer. B's actual IO allocation/projection/free and original physical reap must finish while A remains inside that lower effect. This catches an IO mutex held across Parent/native allocation. The hook preserves the underlying SDK LastError and leaves every older fixture on its ordinary path. It is a known successful paused allocation, not an invented unknown allocation rollback test.

All event waits/native joins use the existing finite ten-second bounds. A is released only after B's required progress, native wrapper/FLS exit and reaping. A timeout cannot be converted into a pass by dismissing A first. Scenario failure exits retaining live wait/token owners. Successful paths drain real rows, close the IO provider, return the actual Borrow and verify controller/native heap cleanup only after native exits.

## Limits

Concurrent scenarios use Posted software work. Real pending kernel buffer/strong pins remain covered separately by the preserved receive/send source cases; they are not falsely described as concurrent kernel completion coverage. No actual networking, game/foreign SDK, fullGraph/NativeJobs same-session overlap, IOCP OS scheduling, Native ABI equivalence or arbitrary shared-payload writes are tested here. A source resolver pointer alone supplies neither caller admission nor an operation lifetime pin. Actual upper per-message/IO ownership integration remains pending. All new assertions are unexecuted.
