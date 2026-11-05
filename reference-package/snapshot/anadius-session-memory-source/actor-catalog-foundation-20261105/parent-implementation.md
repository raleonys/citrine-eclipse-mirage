# Parent actor and authoritative catalog foundation

2026-11-05. SOURCE ONLY. This author changed only `outputs/anadius-session-memory-source/memory.hpp` and `memory.cpp`. Root captured actual pre-edit bytes in this directory's `before/anadius-session-memory-source__memory.hpp` and `__memory.cpp`. Socket resolver interfaces, ApplicationMemory and test sources are owned/reviewed separately. No compilation, project tests, linking, DLL execution or packaging occurred.

This is a disabled lower foundation. Both constructors default to `OperationPolicy::Serialized`; no production assembly selects `OriginalConcurrentActors` in this change. The user-visible native concurrency gap remains open until the coupled Profile/NativeProfile/Graph/providers/IO consumers and NativeJobs integration are complete. In particular, neither distinct-session nor same-session A/B progress has been run or claimed solved.

## Implemented API and actor lifetime

- Immutable policy is exposed by `operation_policy()`. The owned ApplicationMemory constructor also accepts an explicit policy for same-domain owned/callback source fixtures; it retains the existing sole-owned, clean backing requirement. The prepared Borrow constructor uses the same third argument. Unknown enum values reject before ownership is consumed.
- `enter()` and `enter(JobMemory&)` are the only admission producers. A nonallocating stack Admission is linked under the metadata gate before actor/list allocation, so the same actual thread cannot recursively mint another Operation through a compiler allocator callback while its admission is preparing. Independent threads do not reject merely because another admission exists.
- Each stable shared ActorState captures actual thread/fiber, FLS index/PTD, optional exact JobScope and separate mutation/construction/view-reservation state. Metadata registry lookups select only an explicitly admitted record; FLS never mints authority. A separate recursive body gate is owned only by Serialized operations. Concurrent operations do not retain the metadata mutex for their bodies.
- JobPermit now retains its exact ActorState, not a single Parent current-job slot. It rejects wrong Parent, thread, fiber, ended actor and unhealthy actor state. `Operation::job_permit()` rejects a foreign caller before touching JobScope validation, so that invalid request cannot poison a healthy Parent through another actor's lower API.
- Operation exit invalidates/removes only its own record, releases only its own view counts and moves its JobScope out for destruction after releasing the Parent gate. Copied permits retain inactive metadata/Parent identity, not the actual job scope or permission. B can logically return while an inactive copied B permit survives; A's live seal is independent.

## Short transactions and native prefixes

Metadata uses a nonrecursive mutex. Actor/list/seal/lease/borrow preparation, typed destruction, backing allocation/free, lifecycle callbacks, PTD/Prepared checks and OS reaping occur outside it. Potentially allocating runtime-error construction explicitly unlocks through the private reject helper; node ownership is spliced out before destruction. Fixed/trivial legacy catalog refreshes have capacity reserved outside the gate.

Access holds a per-actor mutation reservation rather than a gate across effects. Pure exact lookup is allowed during that actor's normal Buffer copy/fill; public nested mutation/context access still rejects. Captured PTD is checked before reporting a busy actor, preserving sticky detection when an injected lower catches the rejection and restores a corrupted slot before the outer effect returns. Observed corruption poisons before constructing its diagnostic.

Allocation reserves a stable Ticket before calling ApplicationMemory. Its own returned raw pointer/key is recorded while the row remains Reserved, before post-effect validation/publication. Other traversals check Live state before private key/typed fields, so Reserved fields are not concurrently inspected. Unknown outcomes retain their reservation and poison; a healthy no-return failure removes only that attempt. The global `outstanding_allocations()==before` rollback inference was removed: a healthy sibling's allocation/free is irrelevant to this attempt.

Free first reserves one exact ticket and checks actual SDK/other-actor borrowers. It consumes the freeing actor's old view pin, destroys typed storage once, unmaps it, then immediately invokes backing free. There is no deferred HeapFree, guessed rollback or replay. Known retained-live failed free stays Retired for the existing explicit retry; unknown or physically completed but invalid postcheck remains poisoned and nonretryable. Construction scratch remains stable through partial lifecycle prefixes as before.

Additional metadata checks, publication and Access destruction locally preserve LastError. Backing wrappers still capture and restore the actual SDK result/error; no new outer guard restores the pre-call value across a source allocation/free. Construction/free metadata retirement is cleared under localized preservers outside gates.

## Authoritative exact catalogs

The concurrent Parent owns one stable stateless Resolver implementing the socket agent's agreed `ReceiveBufferResolver`, `AllocationHeaderResolver` and `DequeMapResolver`. `buffer_context().memory` selects it only in Concurrent mode. `allocation_header_resolver()` and `output_map_resolver()` expose the exact owner resolver to future consumers.

Byte/header/map lookup returns Miss/Found/Ambiguous and writes output only on Found. It selects exactly one Live nonbusy Ticket under the gate, pins that ticket for the requesting actor, prepares its extra borrow node outside the gate, then revalidates and commits. A temporary count prevents physical retirement while the node is being prepared. Repeated views of that same ticket by one actor reuse its pin. It does not pin a union of all allocations, retain unrelated sessions' buffers or publish an all-ticket snapshot.

The Concurrent `allocate_output_map()` returned typed span is itself pinned after Live publication; metadata failure after that point retains the actual allocation prefix. Own explicit free consumes its own old views; foreign active actor and SDK borrows reject premature free. Resolver pins establish storage lifetime, not permission for arbitrary simultaneous writes to a cipher/string/deque or unknown provider memory.

Serialized mode continues to publish legacy regions/maps and retains its separate whole-operation body gate. Existing selected NativeProfile's copied Parent regions/header/map spans therefore continue to use their old valid serialized contract. Concurrent `output_map_bindings()` rejects the unsafe borrowed union interface; the exact resolver must be used. This change does not quietly activate unresolved catalog consumers.

## Maintenance and close

Concurrent original-worker reaping can coexist with other active actor rows. It retains only a maintenance reservation and excludes reentry by that same maintenance thread; it does not exclude sibling actor entry/allocation. PreparedMemory still selects only Returned/Canceled rows and polls physical exit outside Parent. Serialized retains its prior idle requirement. Root owns the corresponding PreparedMemory metadata-allocation refinement.

The coupled reap catch validates the exact borrowed ApplicationMemory identity/current worker/PTD plus healthy backing outside the Parent gate. A known pre-effect lower snapshot metadata failure can propagate without adding Parent poison. Lower native wait/close unknowns fail this capability check and poison; an already observed Parent poison is never cleared. No exception-type or global-count inference is used.

Close returns false promptly while any actor/admission/actual borrower/allocation/partial construction remains and does not wait for a handler. Only a zero-active, empty ownership commit changes Open to Retiring; provider destruction and Borrow return happen after releasing the metadata gate. Existing controller physical thread/FLS exit ownership remains with PreparedMemory. No new unload/readiness condition, child join in a source body or hidden cleanup framework was introduced.

## Evidence and limits

Fresh native lineage is `../parallel-frame-admission-20261105/native-evidence-index.json` (12 records), with full retained caller/FH4 bodies in `../handler-admission-overlap-20261105/`. 025480 copies each frame to its own closure, starts/detaches and rearms without joining; 021770 invokes captured028d30, and028d30 runs033d70 before response executor scheduling. The warmed plaintext UnknownMessage/GetConfig trace is valid for distinct sessions and the same session, as documented in `memory-design-review.md` and `integration-plan.json`. This lower policy does not invent a per-session strand.

Current source-only cases authored by other agents may assess exact actor/seal independence, sibling allocation failure/count changes, views/free, map/header lookup and reaping. They have not executed. Shared fallback errno domains, arbitrary OOM-handler/foreign-provider behavior, native EH/atomic ABI, upper payload synchronization and final production concurrency fidelity are not claimed complete by this foundation.

Final frozen source SHA256:

- `memory.hpp`: `9dabf8e664583ee13973522382d8023a8dd31b57b85c7274e66e2e9f8f523c71`
- `memory.cpp`: `ad221cc300d5bfdf1c2b17391fc7b999e86513b3c818b05fdd6cca846f91abb0`

Independent review is recorded separately in `parent-production-independent-review.md` (SHA256 `79f1ebf4d7ab50f313f49761df8dc5056d5dd9dddfb84fe21ed205a4cd7f87eb`), with no blocking finding and no execution claim. Its earlier review round is preserved. Source remains frozen unless a concrete coupled finding requires a successor.
