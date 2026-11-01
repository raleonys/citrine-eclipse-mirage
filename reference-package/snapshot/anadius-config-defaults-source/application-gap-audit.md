# B28 independent application scope audit

Reviewed retained evidence only. No original execution, new target queries, production edits or test authorship.

The 61 canonical functions in 0x180030000..0x180038fff comprise 32 source-definition mappings, 22 unmapped definitions and 7 module-only records. Of the 22 unmapped, 19 are container/string/allocator/mutex cleanup helpers. Generic CRT/container work can remain paused.

The strongest application candidate is 036d20: an actual constructor that sets configuration policy. The independent field map proves all 41 current `configuration::State` fields and 6 + 14 attribute pairs, including zero `server_port` at +0x2b0. No outside-constructor input is needed for current State. An explicit factory would add useful completed-State semantics, while whole native constructor/ABI/global lifetime closure remains open.

`01390` passes global base 0x1800a39e0 to 036d20, then registers 07e0c0 through 04b770. The standalone cleanup thunk is not independently resolved by retained canonical function records. `037f70` is the aggregate cleanup candidate by structure; its exact registration linkage remains a gap.

| Address | Classification | Purpose |
|---|---|---|
| 0x1800349a0 | library_container_or_CRT_helper | byte-vector growth/insert |
| 0x180034af0 | library_container_or_CRT_helper | byte-vector reserve/reallocate |
| 0x180034ba0 | library_container_or_CRT_helper | byte-vector old-allocation release and storage commit |
| 0x180034de0 | library_container_or_CRT_helper | qword-vector allocator with large-allocation alignment |
| 0x180034e60 | library_container_or_CRT_helper | qword-vector insertion |
| 0x1800350f0 | library_container_or_CRT_helper | byte memmove/copy wrapper |
| 0x180035120 | library_container_or_CRT_helper | qword-vector reserve |
| 0x180035190 | library_container_or_CRT_helper | qword memmove/copy wrapper |
| 0x1800351d0 | library_container_or_CRT_helper | qword-vector storage release/commit |
| 0x180036ac0 | library_container_or_CRT_helper | recursive Achievement tree erase/clear |
| 0x180036b40 | library_container_or_CRT_helper | recursive string-pair tree erase/clear |
| 0x180036bc0 | library_container_or_CRT_helper | map<string,string> initializer-list helper, not default policy |
| 0x180036d20 | application_policy_and_aggregate_construction | Global configuration and runtime aggregate constructor; all41 current State fields plus20attribute defaults can be modeled as an opt-in semantic projection. Native aggregate fields and static lifetime remain open. |
| 0x180037b20 | library_container_or_CRT_helper | native mutex construction helper |
| 0x180037b60 | library_container_or_CRT_helper | string-pair map destructor |
| 0x180037b90 | library_container_or_CRT_helper | Achievement payload string destructor |
| 0x180037ce0 | library_container_or_CRT_helper | pair<string,string> payload destructor |
| 0x180037d90 | library_container_or_CRT_helper | vector<Content> payload destructor |
| 0x180037e20 | library_container_or_CRT_helper | vector<Entitlement> payload destructor |
| 0x180037ee0 | library_container_or_CRT_helper | vector<Path> payload destructor |
| 0x180037f70 | application_aggregate_lifetime_mostly_library_cleanup | Whole aggregate destructor appears structurally paired with036d20; exact07e0c0 registration thunk path is not independently resolved by standalone retained function records. Prioritize after application behavior. |
| 0x180038920 | application_fatal_log_then_abort | Log Unhandled exception using006880/016630, then terminal05f03c; smaller genuine app body but existing termination source route delegates lower Services. |

The 7 module-only records contain 0348d0 integer formatting and 6 existing bootstrap behaviors already represented in `outputs/anadius-source/bootstrap.cpp`; do not count them as fresh application gaps. B27 already binds all 7 online SOURCE calls.

Secondary genuine application body 038920 logs “Unhandled exception” and aborts, reached from 038890; its current Services implementation remains delegated. This is smaller and less valuable than the constructor policy projection.

No new native closure is claimed. Original MSVC string/map/vector/mutex layout, full aggregate fields, static initializer/atexit execution, and allocation/exception timing are outside the proposed completed-State projection. Exact evidence IDs/callers/hashes are in the JSON and field map.
