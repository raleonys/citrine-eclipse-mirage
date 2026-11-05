# Independent static review — session registry growth

Production source result: PASS in the bounded reviewed scope. No compilation, project tests, linking, DLL execution or packaging was performed. The fresh six-record REA bundle supplied by root was inspected as saved evidence; this reviewer opened no REA session. Test-source additions were still in progress when this production review was recorded and are not covered by this report.

The native accept025720/session023000/greeting02d540 evidence supports removing the examined source-only fixed session-table rejection. It does not prove unlimited connections or native ABI/runtime equivalence.

## Stable objects and table publication

Profile's deque append preserves addresses of existing Slot objects and their receive/send implementations, environment, queued/message owners and heap Logic objects. SessionToken indices and generations are retained. Eight replacement vectors are allocated before reading/copying live records; admission and table shape are checked after allocations and deque append. The append rollback guard removes only new unpublished Empty slots on a precommit failure. Existing source poison/unknown-prefix handling remains; no replay or native unwind claim is introduced.

The eight swaps, captured span updates and queued stream-pointer repair form one nonallocating publication sequence with no opaque provider call. Reviewed consumers: frame.owners; read.strings/read.streams; composed write streams; finish owners; queued owners; response-queue owners; message owners; Effects environment sources; and CoherentIoMemory environment sources. The added setters are passive noexcept assignments. Existing context objects remain fixed, so the preceding header-ledger view-field registrations continue to refer to the same fields.

QueuedWriteOwner.stream_10 pointers into the retired send table are translated by their old element index, preserving an existing cross-row alias. Old table storage remains alive until all spans and these pointers have been updated. Pending IO uses stable owner/implementation objects and source identity tokens. Prepared handoff jobs retain stable session/control/Logic identities; the independent Job/closure arrays are unchanged. Owned Environment source-address registrations remain valid because Slot.environment does not move.

Graph growth allocates default rows, rechecks admission, copies existing token/raw-Logic rows and swaps under metadata. No Row reference spans allocation or Logic construction. Subsequent checks reacquire by token index. The private registered flag prevents an unpublished Logic from retiring an unrelated empty row during prepublication unwind; published Logic retirement still checks exact pointer/liveness. Old Logic objects and generations survive metadata vector relocation.

## Review finding addressed

Previously returned public RegistryView spans can be invalidated by later registry growth. Root updated profile.hpp to explicitly describe borrowed metadata snapshots, require reacquisition after a potentially growing greeting, and prohibit treating the views as owner/control lifetime permission. Internal captured consumers are actively rebound as described above.

No unresolved production blocker was found in this bounded static review. Natural container/allocation limits, independent job/environment limits, exact native allocation timing, exception ABI and arbitrary concurrency remain outside this result.

## Reviewed source SHA-256

| Source | SHA-256 |
| --- | --- |
| anadius-session-profile-source/profile.cpp | c45a3535583b0e8827b594c92a192ee209713e74746b49512f31b3d3f6e71a6e |
| anadius-session-profile-source/profile.hpp | ad4b9f2e74b1ba01f34e7a32f9b9a167373625b72a381d9fce452c054b5dc07c |
| anadius-application-graph-source/graph.cpp | f16b7a4dd9cbc8557792da98a38a0243471228f18f52e7d0f5b18a8a677dabed |
| anadius-application-graph-source/graph.hpp | de539b9a711a527a824ff55d9b22ac67c6098fb3657ab3d0ee49bc0c0d853aaa |
| anadius-session-flow-source/coherent_io.hpp | 46e63f7427823f3222aa0137197c55890bfdb98ef3ab8ce04705a95a0fa2f2c3 |
| anadius-socket-source/environment_constructor.hpp | eb1dea6575b80cd4610029dc24f7440a9b9bde016e10fe045d34b597844c5ae5 |

These hashes identify the reviewed source bytes only; no compiler/runtime validation is inferred.
