# Explicit job capability to whole Parent Operation — source only

Status: **implemented, not compiled or executed**. The user's explicit rebuild
instruction is still required before compilation, tests, linking or binary
execution. Existing build receipts and ZIP remain on their prior snapshot.

Subsequent source checkpoint adds copyable JobPermit metadata whose authority
expires with this exact Operation. It is used by explicit child deque growth;
see `../anadius-owned-deque-source/job-deque.md`. Operation's HOST layout changes
again; all consumers await an authorized rebuild.

The preceding JobMemory layer admitted only direct backing-memory access. The
new SessionMemoryDomain::enter(job) overload admits a whole Parent Operation
using a capability for this exact prepared backing root. Default enter() and
owner lifecycle methods retain their original worker/thread/fiber/PTD policy.
No Profile, Graph or Policy affinity check was removed.

## Permission and ownership

On the original worker, prepare_job() requires an existing whole normal Parent
Operation and the prepared Borrow route. Preparation is passive. A distinct
same-Domain native job then acquires the move-only offer as before:

```cpp
// Original worker:
std::optional<worker_handoff::PreparedMemory::JobMemory> offer;
{
    auto operation = parent.enter();
    offer.emplace(parent.prepare_job());
}
// Explicit caller-owned transfer/launch to the distinct native job worker:
offer->acquire();
{
    auto operation = parent.enter(*offer);
    auto& buffers = parent.buffer_context();
    // Actual typed Parent source allocation/access/free remains under Operation.
}
offer->retire();
```

The caller still owns native launch, transfer serialization, code backing and
source cleanup. Source callback/Graph execution is not supplied by this API.
Once Parent owns the backing, callers must use its tracked typed allocation
routes; direct untracked backing allocations are not Parent-owned tickets.

Borrow::pin_job compares the actual shared prepared State, rather than merely
Domain identity, before pin construction. Two independent prepared roots in one
Domain do not share authority. A JobScope pins that exact State/JobState, checks
the acquired child thread/fiber/PTD, and increments its active scope count.
Logical JobMemory::retire returns false while any pin exists.

Operation owns a stable heap JobScope independently of the movable JobMemory
object. Moving the token during an active Operation cannot invalidate the
scope or bypass its retirement guard. Destroying an unresolved active token
still poisons the root, and the pinned State/row remain alive for rejection.
No raw pointer to the JobMemory object is stored in Parent.

The Parent recursive gate covers the whole source chain. Only after successful
root/permission/PTD checks does construction publish active scope/thread/fiber.
Failure releases a provisional pin without publishing a partial Operation.
Source effects recheck the stable scope permission and exact backing as well as
PTD. Scope exit validates permission/PTD nonthrowingly, poisons Parent on loss,
and clears the Parent pointer before destroying the pin while the gate remains
held. Lock order is Parent then prepared-root; root code never calls Parent.

JobScope is noncopyable/nonmovable. Token methods/transfers remain serialized,
and raw borrowed references cannot be used after scope/token retirement. This
operation serializes Parent state, not every Domain allocator/new-handler or
external provider. Existing shared fallback/OOM and source synchronization
obligations remain.

## Source close and actual native exit

Parent.close now checks that all child offers are canceled/returned with no
scope/control reservation **before** moving Borrow or destroying lifecycle
providers. A pending child therefore returns false and preserves an open Parent.
Active operations, tracked allocations and provider/buffer leases still block
close under their existing rules. Retirement exceptions or failed final phase
checks cannot publish Closed over a poisoned prefix.

Parent.reap_jobs is an original-worker operation outside any whole Operation.
It reserves the existing mutation state, releases the Parent gate while the
underlying zero-timeout OS polling/handle close runs, then postchecks and clears
the reservation. Concurrent lifecycle/admission cannot retire the borrowed
owner during that interval. Unknown underlying outcomes also poison Parent.
The returned count is not proof that all external source work has drained.

Successful Parent.close is source closure and logical return to PreparedMemory.
It does not release the controller's backing or prove native child/FLS exit.
The existing PreparedMemory.close still observes every actual native thread
handle before backing release, including a child that stays alive after the
whole Parent Operation and logical token return.

## Scope and pending validation

SessionMemoryDomain::Operation gains an owning pointer, changing its HOST size.
Rebuild every allocating/passing/embedding consumer together when authorized.
The opaque Parent/PreparedMemory facade sizes and original virtual interfaces
are unchanged. No translation unit, native procedure, export or loader hook was
added. Runtime ABI/code/image/foreign/kernel completion remains independent.

Source scenarios were added but **not compiled or run**:

1. Two prepared backing roots in one real Domain: reject wrong-root explicit
   entry before poisoning otherwise valid Parent; default sibling entry rejects.
2. Pending job preserves an empty Parent and its lifecycle/Borrow owners.
3. Whole-operation permission blocks token return, survives token movement,
   rejects nested entry/child preparation/reaping, and supports typed buffers.
4. Original worker observes/frees the child's typed allocation; logical Parent
   closure still cannot substitute for live child native/FLS exit.
5. An isolated active-token abandonment poisons Parent/root and retains exact
   typed allocation and child row without a dangling permission pointer.
6. Existing one-worker and unauthorized sibling behavior remains in test source.

No passing test count or total DLL percentage is inferred. Fiber/PTD changes,
SDK/allocator failure, pin allocation failure and reentry/timeout checks remain
pending runtime validation. Native message dispatch still needs explicitly
admitted Profile/Graph/Policy access and matching closure/lifecycle ownership.
