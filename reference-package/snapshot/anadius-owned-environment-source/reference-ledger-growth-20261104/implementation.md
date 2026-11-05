# OwnedEnvironment host reference-ledger growth

Implemented only `environment.cpp` and `environment.hpp`; before copies are retained in this directory. No compiler, tests, linker, DLL execution, original binary access, REA queries or ZIP/package action was performed by this implementation task. Independent static review is separate and does not imply runtime PASS.

## Native comparison boundary

The adjacent fresh native findings retain eight complete replies: seven relevant constructor/move/copy-caller/prepare-caller replies and one incidental021770 trampoline reply. Constructor023dd0 queries the operations at `0x180023e6f–0x180023e7a`, calls the cached factory at `0x180023e87`, moves at `0x180023ebc`, then calls023cb0 at `0x180023efa`. Primitive023cb0 invokes move at `0x180023cee` and reloads executor at `0x180023cf9–0x180023d08`. Message caller028d30 performs environment copy calls at `0x180028e18–0x180028e64`, then destroys the two locals at `0x180028e8e/0x180028e9c`. These observed callers do not expose the host4096 ledger admission gate.

The unknown virtual factory/copy/move targets are still unknown. This change removes an adapter metadata ceiling on the selected host operations; it does not prove unlimited native capacity, full original factory/copy bodies, ABI or EH equivalence. String byte-count comparisons with0x1000 and incidental021770 cleanup are not counted as environment-reference proof.

## Implementation

The private vector<Row> remains the ledger. Existing environment/table/executor addresses, unique capabilities, state, Origin, job_epoch and counters are copied unchanged into prepared doubled metadata storage. The public constructor count is documented as initial storage. The sole growth ceiling is actual vector max_size; native/host resource failure remains possible.

make and copy now share private publication machinery. Copy still changes only the payload/state under its caller-provided partial native prefix and increments copied; regular construction publishes the complete environment and increments created. Existing row moves do not allocate or change reference counts. Only a new projection receives a new monotonic capability; copied old rows never receive new caps. Capability issuance is outside the nonrecursive Control.gate, so capability exhaustion cannot allocate an exception during the new commit. Unpublished private values may be skipped after failure.

When full, make reserves a dedicated growing flag under gate, capturing old data/size, the existing active-call depth and the same selected JobState retained across the allocation. A candidate vector is allocated with gate unlocked. New perform entry during that reservation marks phase Poisoned under gate, then constructs its rejection exception after unlocking; swallowed allocator reentry cannot silently resume the transaction. checking remains owned by ordinary admission probes. Existing legitimate nested active depth is preserved rather than requiring active==1.

After allocation, the owner runs ordinary Parent/actor/permit/checkpoint/IO/backend admission outside gate using its captured selected JobState. It rechecks live session control ownership/strong/embedded-owner/weak_self where a session Origin exists. The final locked section computes nonthrow validity predicates: Bound phase, reserved flag, same table shape/data and active depth, same selected identity/open state/origin, complete protected-source fields/cap/origin/epoch, destination freshness, and immediate source identity/full fields. Only then does it copy CURRENT rows, swap and publish the single new projection/ref/counter update without allocation or opaque callback. Failure throws after unlocking.

Both failed candidates and retired old metadata are destroyed outside gate while the growing reservation is still held; RAII then clears the reservation with a short gate scope. Added growth work preserves LastError across allocation, validation and retirement. Failure before commit retains old projections/counters. If a retirement callback poisons after commit, the newly published row/counter prefix remains retained for the established perform failure policy; it is not rolled back or replayed. No guessed source destruction or JobScope cleanup is added.

## Authority and freshness details

construct(query) retains the complete by-value source Row proof through make, instead of discarding its capability and keeping only state/origin. copy likewise carries a current full-source proof. Post-allocation matching requires the same address, cap, state, Origin, job_epoch and complete source payload/table/schedule/executor fields, preventing same-address recreation with a new cap from reviving an old query. Output freshness is checked again including absence of an already registered destination address.

For selected child projections, the protected source itself is checked in addition to the immediate copied/queried source. The same JobState must remain selected and open; its existing check validates permit and live session/control around the checkpoint, and the ledger match validates the actual protected environment fields that JobState.check alone does not inspect. The protected-source cap/Origin and existing job epoch survive metadata movement. Original-only session/accept entry and exact accept allocator token validation remain at their existing public entry points; growth introduces no child session/accept/handler/response/close authority.

The source objects remain caller-owned and must retain their supported lifetimes. No external Row pointer/span is published and no external rebinding, stable deque, unload condition, new source pin, CRT path or general concurrency framework was added.

## Source fingerprints

| File | Before SHA-256 | After SHA-256 |
| --- | --- | --- |
| environment.cpp | af5bb3c6c5b03714818d616759968b428afe96e6078e81d8b67b6324056be6dc | e7293bf8bfcb67ec566e8518b34f6c76719ac9810dcd25e3f3242b74183aff3f |
| environment.hpp | cbb3048100274a1710b458ea5f1aa8a73528a8cd9cccd6510db2134c5cac887f | 48d78dc79ed9bad3fbdc5f8d8171db335c16c001403a75ebcf7cd11f08bf7d02 |
