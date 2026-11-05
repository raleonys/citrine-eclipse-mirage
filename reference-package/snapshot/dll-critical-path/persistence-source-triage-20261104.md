# Selected achievement persistence — source triage and closed candidate

Read-only audit of current selected business persistence source and retained reports. No production/test edits, REA query, original-binary/user-file access, compilation, tests, link, DLL execution or packaging. The earlier business-gap-triage left write-result behavior unknown; existing retained evidence now answers that narrow question. No new native session is justified for this candidate.

## One bounded comparison question — CLOSED

For GrantAchievement030010 with a nonempty achievement XML path, if03c1d0 returns false without throwing, should the native caller fail the protocol response, roll back progress, skip Steam INI persistence, or retain a retry/save flag?

**Already answered: none of those changes is supported.** `anadius-achievement-runtime-source/flow-audit.md:32` cites the exact retained030010 path: XML call at `0x180030ac1` does not inspect BOOL; INI01f9a0 follows at `0x180030b4c`; response construction follows effects. Its independent evidence IDs are `ev_8e0c6e33949acd640bbdc56b56ceea86f8bb373eca849ee8c6a34d8a0ac1b175` (pseudo) and `ev_62f5fc49da4ec38f00325c7e0b857c87556da27030afd17618aec1e4238cb16c` (assembly).

INI01f9a0 likewise discards the lower BOOL, documented by `anadius-achievement-writer-source/README.md:3`, `writer.hpp:16` and independent `review-storage.md:9`. Its retained assembly places the03c1d0 call at `0x180020855`, followed by NOP and string-capacity cleanup at02085a/02085b/02085f, not an AL-result branch. This is the writer's large normal body, not a new020855 function. Writer evidence is retained under `ev_62b9a1da24b690b13ccc9ef87ff538b194425801a3a0857c3220da35a50c024f` / `ev_488d569fd12cfa9fe60d4d35d04b56c9d44b52396427f3060d14d2a7e0125a5a` as referenced by the existing storage audit.

`anadius-file-source/io-evidence.md:28-40` separately establishes03c1d0's boolean meaning: initial stream state at03c257/03c25c selects the path; bad initial state returns AL0 at03c2ba; write03be80 at03c2cc and close03ae70 at03c2d6 run on the other path; it returns AL1 at03c378 without a new final-success decision. Evidence `ev_97eaf0d1a5c0d1e7ad14855daa321d9f92e4aabb4a1d614bc467ef53f3010812` / `ev_69190e83bcae202b03f4fa8834b567484e7a048bd0cad2004da94e2895f74606`. Thus even true is not a durable-write guarantee. Ignoring this bool in the caller is recovered behavior, not an unimplemented error handler.

## Actual selected composition

`anadius-application-effects-source/factory.cpp:44` constructs Content from the shared state, shared achievement paths and selected Online backing. Worker moves that provider into Graph and binds Graph's achievement-file reference to the same paths (`anadius-application-worker-source/worker.cpp:302-313`). Graph's Content state references include the actual configuration achievement map and XML path (`anadius-application-graph-source/graph.cpp:334`). This is the concrete provider, not a legacy unused effect wrapper.

Grant030010 (`anadius-content-handlers-source/content_handlers.cpp:113-146`) updates the shared achievement in place under the achievement mutex: missing ID returns false; missing Progress uses1; unknown ID gets total=initial increment; signed progress<total gates updates. Progress uses modulo32 addition, UTC timestamp/epoch are updated, completion sets Count1 and caps progress, then queues the completion event. If XML path is nonempty it serializes/writes XML first (`:139-141`); Steam persistence is next (`:143`); response achievement element is produced last. The mutex covers that sequence. Normal bool failure does not roll back state or event. Completed achievements skip later side effects on repeated grants while still returning their response. There is no achievement save-success/dirty/retry flag added here. saved_avatar_size belongs to QueryImage and is not an achievement persistence flag.

`Content::write_xml` and `save_steam` (`anadius-content-effects-source/effects.cpp:92-93`) call the guarded Engine/PersistenceEffects. `anadius-achievement-runtime-source/runtime.cpp:32-39` invokes the actual binary writer and explicitly discards its bool; Steam save checks the map is the shared configuration map, then calls the real01f9a0 writer. GuardedFiles (`anadius-application-io-source/files.cpp:25-46`) forwards open/write/close under existing source checkpoints. A nonzero initial output state marks its raw output safely retirable; it is not converted into a protocol failure. Null backend objects and thrown/unknown prefixes remain integration/error outcomes, not bool false.

`anadius-file-source/binary_io.cpp:18-27` matches the retained initial-state-only decision. Selected NativeFactory (`native_io.cpp`) opens binary output with truncate, writes the explicit byte length and closes. The backend remains an acknowledged std::fstream compatibility adapter; this source audit does not expand CRT/share/exception ABI coverage.

## Representation, order and error behavior

- XML (`content_handlers.cpp:31-42`) emits Achievements with GameName and ContentId, then key-ordered records having signed Progress !=0 (including negative), with Id/Count/Progress/Grant/Name. The attribute adapter uses a C string, so embedded NUL terminates individual attributes. PersistenceEffects' document uses the existing MemoryPrinter/HandlerWriter and extracts bytes only through the first NUL, excluding its final terminator (`achievement-runtime-source/runtime.cpp:12-24`). This differs intentionally from the raw explicit-length INI writer; it is not one shared string truncation rule.
- INI (`anadius-achievement-writer-source/writer.cpp:15-43`) iterates achievement map key order but uses steam_id as section name. It emits CRLF, Achieved as signed field04, CurProgress/MaxProgress only when signed total>1, and UnlockTime as unsigned field70, ending each section with a blank line. It preserves explicit steam_id bytes, including NUL/delimiters, and duplicate section names are possible. Empty path returns with no effects; otherwise parent-directory creation precedes serialization/write. These decisions already have retained writer golden-byte/review coverage; they are not new proposed gaps.
- The load/update path (`anadius-achievement-storage-source/storage.cpp:52-94`) keeps previous XML path when set is empty, uses C-string XML parsing/error guard, imports direct children with existing Id/default/duplicate semantics, computes INI path and every epoch before the status gate, preserves kind2 existing regular files and writes for other permitted kinds. Exact status mask1/9, field defaults and ordering were independently closed in `storage-audit.md` and `review-storage.md`; do not repeat them as unproved gaps.
- A returned false XML write still permits INI and response. A returned false INI write still permits response. Directory/stream/allocator exceptions propagate through the existing Content poison boundary; already updated achievement/event state is not rolled back and no synthetic retry is installed. Bool discard does not mean exceptions are swallowed. Existing reports explicitly leave broader allocation/EH/backend equivalence outside their normal-path proof.

## Recommendation

Close the earlier “discarded XML/INI bool may be missing save handling” candidate using the existing evidence above. No production change and no fresh030010/03c1d0/01f9a0 query is recommended for that question. The focused scan found no replacement bounded business discrepancy in this selected chain that is both concrete and not already covered; inventing a save flag, retry policy, different persistence order, or unrelated CRT/filesystem framework would diverge from the recovered behavior. This closure does not claim exact original XML-version/backend/ABI equivalence or runtime validation.

## Current source SHA-256

| File | SHA-256 |
| --- | --- |
| content-handlers-source/content_handlers.cpp | 4cb847e398b3a81b4c9149a711fecfe0532265fb5c974f33aed73591700e947f |
| achievement-runtime-source/runtime.cpp | 5ec1e7c2fc2dced97d781450ad15058bc9cc2c9f5486d7ff3c35fe81bfd9e2b7 |
| achievement-writer-source/writer.cpp | 20bd58a3fb48d3db046725fa6c154da475072c9dbd29f21e7c6a34c617319b87 |
| file-source/binary_io.cpp | f95365aa9dc8f2f3ab9136e9d5122f754c0d11270cda529d577b61f1dee34222 |
