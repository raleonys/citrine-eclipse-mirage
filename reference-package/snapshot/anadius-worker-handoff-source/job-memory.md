# Explicit job-memory capability — source only

Status: **implemented, uncompiled and unexecuted**. No compiler, test, linker or
binary was run. Existing evidence.json/build receipts and the ZIP describe the
previous source snapshot. An explicit user rebuild request is required first.

The original PreparedMemory::Borrow still belongs to one actual Domain worker.
Its existing memory(), retirement and Parent routes continue to require that
worker's exact thread/fiber/PTD. A new move-only JobMemory offer may separately
lend the same controller-owned ApplicationMemory to a distinct same-Domain
NativeThreads worker. No aliasing shared_ptr transfers memory ownership.

## Narrow use

```cpp
// Original admitted worker, while its main Borrow is live:
auto offer = borrow.prepare_job();
// Transfer that move-only token to exactly one caller-owned native job.
// On the actual distinct same-Domain job worker:
offer.acquire();
auto& memory = offer.memory();
// Perform explicitly serialized borrowed-memory work.
// Stop all access through memory before logical return:
offer.retire();
// No use of the token or retained memory reference after return.
```

The class never creates a thread, schedules a callback, registers message logic
or executes an original image address. The caller owns token transfer, native
launch/SDK/code backing and all independent join/source obligations. Serialize
all calls and transfers of each token; do not concurrently mutate one object.
Caller must serialize shared allocator/fallback/OOM/new-handler and source
effects. Domain PTD admission alone proves no general concurrent safety.

Preparing an offer is passive: allocate a unique shared JobState node and
register it under the existing gate. At most 64 unreaped nodes are permitted;
this is a selected HOST bound, not a recovered native constant. Move invalidates
the donor. Unique pinned nodes are never reused in place, so a stale row index
or generation cannot accidentally identify a later job.

acquire atomically reserves Offered → Acquiring, verifies a distinct real
same-Domain PTD with a NativeThreads context marker, and duplicates the actual
current native thread handle. Neither the controller nor original worker, a
compiler-only foreign thread, or a repeated acquire is accepted. The duplicate
is adopted before postchecking admission; unknown outcomes retain ownership.
A known DuplicateHandle failure restores the Offered state for explicit retry.

memory validates acquired row, root state, thread/fiber and exact PTD. It grants
only the backing reference. One job worker may hold multiple distinct offers;
each independently retains its real self handle. A changed admitted fiber/PTD
poisons the root; wrong-worker probes reject without transferring authority.

## Logical return and physical exit

retire ends the job's memory permission and invalidates its token. It may leave
an allocation deliberately transferred to the original source owner. The main
Borrow still refuses return while any offer/acquire/job is outstanding, the
backing has live allocations, or its outcome is unknown. An unreturned job or
unresolved offer destructor poisons the retained root; it does not close an SDK
handle or free backing on the job worker.

cancel is limited to an Offered token on the original worker. Before canceling,
the caller must exclude pending users of that token and any future acquire. It
does not join an independently launched-but-not-yet-acquired thread. Canceled
rows have no admitted native handle and remain counted until reaping.

The original worker may reap_jobs outside any Parent Operation. A zero-timeout
wait checks returned children; logical return alone cannot recycle a row.
Only an actual signaled thread handle permits its duplicate to be closed and
the row removed. Timeout preserves it. Failed/unknown wait/close poisons and
retains the unknown suffix. The reservation spans polling and retirement, no
metadata gate is held across SDK wait/close, and successful close prefixes are
committed to null before any later failure can be observed.

The controller's final close also waits for every admitted child after the
original worker exits. A returned main Borrow and signaled original worker do
not substitute for child thread/FLS exit. A finite close timeout is one shared
wait budget across original and child handles. Timeout retains every necessary
owner; verified/closed child prefixes are not replayed on retry. Backing is
released on the original controller only after all actual exits are observed.
LastError remains preserved by these control operations.

## What remains open

The next source checkpoint now supplies an explicit pinned whole Parent
Operation: see `../anadius-session-memory-source/job-parent.md`. It is still
unbuilt/unexecuted; default Parent entry remains worker-restricted. Profile,
Graph and Policy access/serialization and actual message entry/thread-exit
cleanup remain open. Default InlineJobs behavior is unchanged. This original
backing layer alone never grants implicit access to those higher source paths.

No native function, export, loader hook or CRT-equivalence claim is added. The
opaque PreparedMemory/Borrow facade sizes remain unchanged; private State grows
and new APIs/types require their consumers to be rebuilt together when allowed.
Independent image/foreign/kernel finality and code lifetime remain required.

Unexecuted test source now describes:

1. Capacity 64, never-acquired cancellation and record recycling; moved token,
   original/controller/foreign rejection, and repeated acquire rejection.
2. Two actual exited-child reaping rounds and stable original backing identity.
3. Live allocation transfer, logical return while native child remains alive,
   original worker exit before child exit, and final retained-backing close.
4. A separate abandoned-job process retaining its exact record and allocation.
5. Legacy sibling rejection for Parent entry and the original one-worker path.

Test failure paths stop before unwinding wait owners under still-live test
threads. These scenarios have been inspected as source; none is a passing
runtime result. Dynamic failure/timeout/fiber/PTD/SDK and shared-budget checks
remain pending with the rest of the source changes.
