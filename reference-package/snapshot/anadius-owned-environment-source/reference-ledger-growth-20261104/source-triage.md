# OwnedEnvironment reference ledger — source design triage

Read-only current-source audit, 2026-11-04. No code edits, REA calls, binary access, compilation, tests, linking, DLL execution or packaging. Root assigned fresh native comparison separately; the source4096 ceiling below is confirmed, but this note does not independently establish native equivalence or a native discrepancy.

## Storage and authority map

`environment.cpp:57-60` defines private vector<Row> rows. Each value stores an external IoHandlerEnvironment address, globally unique capability, state, session Origin (SessionToken/owner/control), and job_epoch. The constructor sizes it once (`:88,182`; public default4096 in environment.hpp:17). `make` (`:136-141`) and `copy` (`:163-167`) scan for an empty address, then throw length_error when full. Moves update a row's address without adding a reference; destroy clears a row and decrements refs (`:144-148`).

No external span, pointer or reference to Row storage is published. query_row returns a Row by value (`:130`), construct explicitly saves a value (`:162`), and copy's `const auto source=c->row(...)` is also a value. Actual IoHandlerEnvironment objects, session owners, executors, tables and handlers live elsewhere. Therefore metadata-vector replacement can preserve every observable address/capability: no stable-deque conversion or external view rebinding is needed. Copy all **current** Row values only after the last allocator callback, then swap the vector without changing live addresses, caps, origins, states, epochs, refs or statistics.

Capabilities are globally monotonic via next_capability (`:29-32`); a row may reuse its empty slot but must receive a new cap. No vector index is a public generation. Growth must not call next_capability for copied existing rows. Issue one new cap only for the actual new projection; failed admission must never publish it.

Control.gate is a nonrecursive mutex. `perform` (`:105-113`) captures an optional exact JobState, increments active, probes admission without holding gate, executes the operation and probes again, with sticky poison on failure. `admission`/`probe` (`:91-104`) enforce worker/fiber or exact selected job, Parent Operation and IO/backend/Accept admission. JobState carries immutable source address/cap, origin, epoch, permit and actor identity; its check (`:47-54`) validates live Parent session/control. `allowed` (`:114-119`) restricts rows to that job's exact origin and source-cap/epoch and denies destructive operations on its protected source. Existing enter_job and JobScope close enforce idle reservation, scoped temporary drain and protected-source identity (`:216-272`). None of these authorities should be recreated or broadened by growth.

## Smallest coherent growth transaction

Keep the vector and existing APIs. Add a narrowly scoped ledger-growth reservation to Control, used only when make/copy find a full ledger. Reserve it under gate after validating the current operation, source, destination freshness and selected job; record the old table shape and captured selected JobState. Release gate before constructing a default Row candidate vector of checked doubled size, bounded by rows.max_size(). Preserve LastError across this extra metadata allocation and retirement.

While that reservation is active, new mutating `perform` entry must reject with sticky phase poison before executing admission callbacks; a swallowed allocator/new-handler rejection must still invalidate the outer commit. Existing checking cannot simply remain true over the allocation because the outer admission probe itself uses checking. A dedicated boolean is sufficient; no general framework is required. Snapshot may inspect the coherent old rows/counters. query_owner's capability lookup may read old rows under its existing registry/control locks, but the subsequent perform must reject the reserved transaction. Lifecycle attempts already see active>0 and fail; include the reservation in admission/close checks if needed so an incomplete transaction cannot publish after authority retirement.

After allocation, run the normal Parent/actor/checkpoint admission outside Control.gate using the **same captured** selected JobState, not a newly selected job. Reacquire gate and verify Bound phase, reservation, same table shape, expected active operation, same active job pointer/epoch/open state, and current source/destination fields. Copy current rows into already allocated candidate storage only after those checks, choose the current empty/new row, acquire the new cap without a callback-bearing allocation in the commit, and swap/install/publish the new row and destination fields in one no-throw metadata section. No probe, allocator, user callback or callback target destruction belongs in that section.

Keep both the candidate and retired vector outside the final lock scope: its destructor must run after gate unlock even when a validation exception is thrown. Keep the reservation until retired metadata destruction is complete, then clear it using a short gate scope on every exit. This avoids nonrecursive-lock deadlock from allocator/deallocator callbacks and prevents callbacks from observing or mutating a half-published transaction. Allocation failure before commit leaves all old rows and destination/source projections untouched; perform's existing exception policy poisons rather than simulating cleanup.

## Per-operation freshness that must be repeated

| Route | Before/after allocation proof |
| --- | --- |
| session -> make (`:206-208`) | Same SessionToken/owner/control origin; Parent still owns that live strong control and its embedded owner; weak_self object/control fields still match. Re-run fresh(out,false). Original-worker-only authority remains. |
| accept -> make (`:209`) | Same server allocator token and original-worker authority; fresh(out,false) still succeeds. No session origin may be invented. |
| construct(query) -> make (`:162`) | Preserve the original source cap/query, state, origin and job epoch through growth. Re-run query_row(query), allowed and actual source projection-field validation at its current address. Current code passes only state/origin into make; this is sufficient before adding a new callback, but loses the cap needed for post-growth validation. Extend the private route to carry the source query/value proof. A destroyed/recreated projection at the same address with a new cap must not authorize the old query. |
| copy(out,from) (`:163-167`) | Re-run fresh(out,true) and row(from,true), comparing current source cap/state/origin/job_epoch to the captured source value; do not retain Row& across allocation. This checks payload owner/cap, state, operations, schedule and executor, and rejects same-address destroy/recreate. Commit only out.payload/state, preserving the native copy prefix supplied by its caller. |
| selected child make/copy | Revalidate selected identity/open state/epoch and permit/checkpoint admission, plus the protected source projection's actual fields/cap/origin. Copy only the exact selected origin; new row.job_epoch remains selected.epoch. Never permit protected-source move/destroy, session/accept creation, handler/response scheduling or provider close as a side effect of growth. |

The destination object must still be unregistered after allocation, even if its raw fields remain superficially fresh. The source must still be registered under its exact capability, not merely equal state/origin. Retain existing full versus partial prefix semantics: native move uses an empty source prefix for row(from,false), whereas copy requires row(from,true). This growth concerns make/copy; move needs no allocation and can retain its existing locked path, subject to the reservation barrier.

## Avoided scope expansion and review limits

No external row-view system, new session ownership/pins, unload barrier, CRT recovery, arbitrary worker concurrency or rewritten scheduler is needed. Existing source handler/response callbacks deliberately release gate before executing tasks and cannot inspect projections afterward (`:168-179`); preserve that behavior. A failed metadata allocation is not proof of source cleanup, closed JobScope, or released strong references.

The projected native environment constructor/move paths already modeled as023dd0/023cb0 are useful context for root's independent native comparison; this private ledger is a selected policy implementation, not a claim that native has a corresponding std::vector or4096 counter. Fresh native evidence must determine the justified behavior domain before production edits.

## Audited source hashes

| File | SHA-256 |
| --- | --- |
| outputs/anadius-owned-environment-source/environment.cpp | af5bb3c6c5b03714818d616759968b428afe96e6078e81d8b67b6324056be6dc |
| outputs/anadius-owned-environment-source/environment.hpp | cbb3048100274a1710b458ea5f1aa8a73528a8cd9cccd6510db2134c5cac887f |
