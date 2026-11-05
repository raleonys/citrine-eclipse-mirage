# Growing selected string-header metadata

Production changes are limited to `strings.hpp`, `strings.cpp` and `profile.cpp`. Exact pre-edit copies are retained in this directory. No compiler, project test, link, REA session, target execution or packaging was run by this subagent.

`Strings` treats its constructor size as initial header-table storage rather than a maximum number of simultaneously live large strings. Existing raw-header validation, duplicate update, prune and reusable-empty-row paths remain. A full ledger allocates a doubled replacement table, capped only by vector max_size arithmetic. It rechecks Parent buffer-context admission and the original ledger shape after allocation, prunes/rechecks duplicate and reusable rows again, and copies current existing rows only if growth is still necessary. It installs the new row, swaps and republishes the two selected live spans without an opaque operation in between. Only then does the old metadata vector die. If an allocator callback registered the same address or made a hole without moving the table, that current row is reused and the unpublished replacement is discarded. The payload bytes and their native backpointers are not moved, replayed or freed by metadata growth.

Only the full-ledger growth path has a local WinLastError preservation guard, covering extra metadata allocation and either old/replacement metadata destruction. Earlier raw-header validation semantics remain outside this guard. The guard outlives the candidate vector on all normal/exception exits.

If replacement allocation or post-allocation admission fails, the old table and its view fields remain intact. Any source payload allocation completed before track is retained through the existing failure-prefix/poison handling rather than being replayed or guessed-freed here. This is selected source ownership behavior, not a claim of original native EH equivalence.

## Exactly two cached views

Profile captures `strings.bindings()` at construction only in its `frame.allocation_headers` field and in `effects.finish_context().allocation_headers`. The constructor now registers references to those two span fields after Effects/frame/read linkage is constructed and before the final potentially throwing owned-Deques construction. Binding is passive, distinct and once-only, with immediate publication of the current table.

`Strings` is noncopyable/nonmovable so its bound-view identity remains stable. Both referenced span fields belong to the same stable Profile::Impl; they outlive active ledger use. They are destroyed before Strings itself, whose normal destructor merely destroys metadata and does not publish through those pointers. Partial-construction unwind likewise performs no publication after the span fields end.

The five primitive header consumers are `frame_read.cpp`, `message_dispatch.cpp`, `owned_response.cpp`, `response_dispatch.cpp` and `write_finish.cpp`. Each completes header lookup and caches only the scalar allocation base before an opaque invalid-parameter/free provider call. No header iterator survives that call. Message/response/owned-response/Runtime adapters retain the Profile frame by reference rather than copying its header span. Consequently publishing those two fields covers all currently selected production consumers without changing primitive algorithms.

Graph retains the exact Profile-owned Strings instance for its event data, so large queued events share the same growing ledger and publication. `bindings()` remains a snapshot API; callers must not retain an unregistered copied span across an operation that can grow. No new global readiness, concurrency, unload or lifetime-proof protocol was introduced.

## Limits

Initial constructor capacity validation remains bounded as before; it is not an event/response count limit. Metadata allocation can still fail at natural allocator/vector limits. Existing selected actor serialization remains; this patch does not establish arbitrary concurrent access or original CRT/string/node ABI equivalence. Root owns fresh native evidence, independent review, test-source integration and final hash registration.
