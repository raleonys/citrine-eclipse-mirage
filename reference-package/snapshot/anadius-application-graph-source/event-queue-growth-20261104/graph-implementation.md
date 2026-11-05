# Owned Graph event map growth: source implementation

Changed only graph.cpp and graph.hpp; exact pre-edit bytes are retained beside this note. No compiler, project test, link, target execution, REA session or packaging was performed by this subagent. Native034660/026670 findings were supplied by root and are retained separately in this stage.

`event_capacity` now names the initial map allocation, not a maximum queued-event count. The existing initial-input contract (nonzero power of two, at most65536) remains; subsequent growth is not capped by that initial limit. Default64 remains. Before each insert, map_size<=count+1 triggers growth, matching the supplied native one-spare-slot rule, and offset normalization follows growth. Minimum growth size is8; otherwise capacity doubles. Both vector max_size limits and doubling overflow are checked before allocation.

Growth prepares a new values vector and pointer map before modifying ownership. It then calls existing admit() after allocations, so a reentrant compiler new-handler that poisoned source state cannot lead to a later mutation. Only the trivially copyable ReceiveBufferString prefix moves. Derive head=old_offset&(old_capacity-1), then transfer each live logical entry to new physical slot head+i. This preserves the head and FIFO order across an old-map wrap while using the newly available second half. Payload allocations and aligned-header addresses never change, and old source prefixes are reset without freeing their moved payloads. Swaps and queue span/map-size/offset publication use nonthrowing metadata operations. A post-publication admit() preserves existing checkpoint behavior; a later error can retain a grown queue but cannot leave a half-moved ownership prefix.

Held-reference audit: Graph's queue object has stable identity and only its map span is rebound. Native pop snapshots its head index only while LockMode::Native is held; growth is restricted to the existing Semantic insertion lock. NativeMessage processing reacquires its current front after opaque callbacks and no growth is admitted during that Native lock. Graph retirement holds a prefix reference only during string release while mutation admission is closed. Strings tracks aligned heap-header/data addresses rather than vector prefix addresses. NativeProfile::Invocation::refresh rebuilds inline memory regions from the current queue map after lower calls; it does not retain old event prefix pointers as queue ownership.

This remains a host projection of deque map/string prefixes, not original MSVC map/node allocator ABI. The separate active NativeProfile guard rejects maps over1048576 slots; this stage does not remove it or claim an unbounded native route. Existing owned-string header/profile resource limits also remain. Allocation/checkpoint failures retain the existing Graph poison/retention behavior; no new lifecycle or unload protocol was introduced.

Final source hashes before independent review:

- graph.cpp: `b2915e54e501bb05a87fc51b98c34c6d88b01881fae27462b7f328239740406c`
- graph.hpp: `ca762275c8c5dad511395f932ad0fca4e2f50773f0050f04bb525ff43ec20d2a`

Static source inspection only. Test additions and independent review are owned by other agents; no passing executable result is claimed here.
