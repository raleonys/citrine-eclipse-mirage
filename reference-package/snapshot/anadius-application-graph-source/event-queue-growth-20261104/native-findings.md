# Dynamic application event queue — fresh native findings, 2026-11-04

REA6.8.0 server identity was verified before opening original anadius64.dll with Ghidra12.1.4. Original SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Seven full fresh Evidence records are retained individually and in `rea-evidence-bundle.json`. The native session was closed after export. No target execution or mutation.

## Concrete production discrepancy

The selected Graph owns a fixed-size NativeMessageEventQueue projection. Before this stage, graph.cpp::push_raw threw `Bounded owned event ring is full` and never grew its vectors. The default64 slots can be filled by valid application requests, not only by hostile sizes: ShowIGO/ShowIGOWindow032b10 appends a DOWN event, while plaintext033d70 returns without consuming queued events. Lazy initialization contributes one initial DOWN. Without event draining, the old default source fails on the64th user enqueue.

## Native observations

- `034660`: compare map_size with count+1 at0x180034681; if insufficient, call026670(extra1) at0x18003468c. This is a one-spare-slot growth rule, not a fixed-capacity/full-only rejection.
- `034660`: normalize head offset with map_size-1 at0x180034698; select the tail ring slot; allocate a32-byte string prefix when the native slot pointer is empty; move the input prefix and reset its source; increment count at0x180034714.
- `026670`: start from existing capacity (or1), double until at least8 and enough new slots exist; allocate a larger pointer map, preserve wrapped pointer order and clear new map entries before publishing. Original allocator alignment/failure/overflow branches remain visible in the complete reply; a source HOST vector projection does not reproduce their exact ABI/timing.
- `02c400`: lazy queue constructor creates the initial68-byte DOWN event and the deque/proxy, uses026670 for initial map capacity and copy-inserts the seed before registering cleanup.
- `032b10`: emits success XML, locks the event mutex, gets02c400 storage, constructs the DOWN payload and calls034660. No achievement/persistence prerequisites are needed for this trigger.
- `033d70`: the plaintext response route does not drain events; the encrypted path accesses the global queue and processes a pending event. Complete fresh pseudocode is retained; this stage does not reinterpret Ghidra EH/no-return warnings as independent runtime proof.

These are shipped-artifact static observations. No actual request sequence or Windows DLL was run in this stage. The resulting source correction must preserve FIFO and Parent-owned payloads across wrap/growth and keep existing native-pop locking and projection refresh semantics.
