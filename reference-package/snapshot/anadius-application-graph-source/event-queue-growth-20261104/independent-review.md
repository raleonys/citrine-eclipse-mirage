# Independent static review — 2026-11-04

Reviewer: `registry_sdk_integration_review`, separate from production and test-source authors. Static review PASS for the bounded source change. No compiler, project tests, REA session, target execution, linking or packaging was run by this reviewer. This report does not establish a passing executable result, exact native deque ABI or runtime equivalence.

## Production source

- The insertion predicate `count >= map_size - 1` matches the observed034660 one-spare-slot growth condition for this validated nonempty, bounded power-of-two projection. Growth chooses at least8 and otherwise doubles; container maximum sizes/doubling are checked before allocation. `event_capacity` remains the initial map size rather than a live-entry ceiling.
- Both new vectors and every new map pointer are prepared before old ownership changes. Existing admission is rechecked after allocating. Allocation failure or an allocation callback that poisons admission cannot leave a partly transferred old ring.
- Only trivially copyable/no-throw-assignable string prefixes move. Logical entry i transfers from old[(head+i)&old_mask] to new[head+i]; head is the normalized old physical offset. Since count<=old_capacity and new_capacity>=2*old_capacity, every destination fits. This preserves wrapped FIFO order and the physical head. Old prefixes are reset without releasing the moved heap buffers.
- No throwing operation lies between first ownership transfer and coherent publication of the swapped vectors, queue span, map size and head. The queue object retains its address. A post-commit admission failure may retain the enlarged projection but cannot expose half-moved ownership through this code path.
- `Strings` tracks large allocation header/data addresses rather than prefix-object addresses. Those heap identities do not change during relocation. Inline bytes move with each prefix. NativeProfile's Invocation refresh reconstructs event inline regions after successful lower calls from the current stable queue object and new map.
- Growth requires the Semantic lock; the Native pop lock therefore retains its head reference through copy/encrypt/destruction. The source native message loop reloads its current front after opaque effects. The new structure check also verifies vector sizes and a nonempty power-of-two map.

Failure limits remain material: Graph's existing error path can poison and retain a held lock/prefix; no reusable Graph retry or native EH-unwind equivalence is promised. NativeProfile still rejects maps larger than1048576 slots, and owned-string/profile resource limits remain. This is not an unbounded end-to-end queue claim. The source projection relocates prefix objects, whereas native deque map growth retains native node allocations; exact object/allocator ABI is outside the claim.

## Test source

The new plaintext scenario constructs70 actual source ShowIGO/ShowIGOWindow requests and checks a71-entry backlog including DOWN, with no event consumption and exact successful FIFO response frames. Its close assertions inspect normal graph/root retirement, not compiler-vector allocation internals.

The encrypted scenario starts with an8-slot map and DOWN plus six events, consumes three real native events, then appends six more. This forces growth with a nonzero wrapped head and live16-byte/4095-byte/5000-byte payloads, SSO and embedded NUL. The source checks all13 exact event logs,26 independently decrypted response/event frames, FIFO ordering and normal Parent allocation retirement. It would detect lost prefixes, stale inline mappings and double retirement if executed successfully.

These scenarios are NOT COMPILED OR RUN. No allocation-failure injection or maximum-map-limit test is added here. The obsolete active overflow605 expectation is removed. `graph_integration_tests.cpp` is unchanged; its historical overflow helper remains historical rather than being rewritten as current validation.

No concrete blocking defect was found in the inspected final patch. Compilation and actual scenarios remain unverified pending an explicit rebuild instruction.

## Reviewed source hashes

| File | SHA-256 |
| --- | --- |
| graph.cpp | b2915e54e501bb05a87fc51b98c34c6d88b01881fae27462b7f328239740406c |
| graph.hpp | ca762275c8c5dad511395f932ad0fca4e2f50773f0050f04bb525ff43ec20d2a |
| graph_tests.cpp | 017b75df2008e2a9507b3203bdb635a2338f3287ddf7726dc82aa8c93784cec1 |
