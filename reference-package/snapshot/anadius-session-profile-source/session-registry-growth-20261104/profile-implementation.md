# Dynamic Profile session registries

This subagent changed only `outputs/anadius-session-profile-source/profile.cpp`. Its exact pre-edit bytes are retained in `profile.cpp.before.txt`. Root owns the passive source-span setter headers, Graph rows and documentation updates. No compilation, project tests, REA session, linking, target execution or packaging was performed here.

## Stable identities

Session Slot objects now live in std::deque. Appending new empty rows leaves every existing Slot, receive/send implementation, environment, queued owner, message owner and unique Logic address intact. Existing SessionToken slot indices and generations are neither renumbered nor reset. Jobs and closure inventories remain independent and unchanged.

On greeting exhaustion, the method prepares eight replacement metadata vectors: frame owners, receive strings, receive streams, send streams, write-finish owners, queued owners, message owners and Profile State environment sources. It allocates them empty, then checks Parent/source admission before appending empty deque rows. Another admission/shape check follows the append. Only then are current records copied, so no iterator or stale row copy crosses allocation callbacks.

An append rollback guard removes only unpublished new empty rows if allocation or post-append admission fails. The original published metadata vectors remain intact until the complete commit. This does not replay or free the preceding native session-factory payload. Existing effect/Mutation failure-prefix semantics remain.

## Complete publication

Eight vector swaps and their span publications are nonallocating and have no opaque provider call between them. Updated consumers are:

- frame.owners;
- read.strings and read.streams;
- effects.write_context().streams;
- effects.finish_context().owners;
- queued.owners and queue.owners;
- message.owners;
- the private environment source spans in Effects and CoherentIoMemory, via root-provided noexcept setters.

Each existing queued owner's stream_10 pointer is translated from an old send-binding table element to the corresponding new element. The translation also preserves an existing valid cross-row alias rather than silently rewriting it to that queued owner's own index. Pointers outside the old table are not normalized. All old metadata vectors are destroyed only after publication and pointer repair.

Frame/context objects themselves remain in place. The two header-ledger view destinations introduced by the preceding stage remain bound to the same fields; this growth does not reconstruct or rebind them. Source error state is preserved across the entire metadata growth path. Greeting checks admission once more after growth returns and old metadata has been destroyed, before it increments the native control's weak count or adopts the new row.

## View lifetimes and limits

Public RegistryView values are snapshots. Runtime's stored Environment.sources is an initialization snapshot consumed when CoherentIoMemory is constructed; its only active post-publication consumer is the private source span that is republished here. Runtime flow, response, owned-response and FrameRead adapters otherwise retain stable context references.

Natural container/allocation limits still apply. Initial session capacity validation remains an initialization policy, not a live peer limit. Graph's independent token/Logic row inventory requires root's matching growth change. This patch does not expand job/environment capacities, introduce new readiness or unload conditions, or establish arbitrary concurrent mutation/ABI equivalence.
