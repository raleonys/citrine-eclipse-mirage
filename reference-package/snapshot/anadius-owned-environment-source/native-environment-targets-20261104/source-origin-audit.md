# Environment field origins and existing target bodies — source audit

Independent read-only source audit, 2026-11-04. No code edits, REA/session calls, original-binary access, build, tests, link, DLL execution or packaging. The native agent owns the fresh origin/vtable evidence. Addresses below are existing source selectors or explicitly identified prior saved evidence, not new observations from this task.

## Selected source flow and offset labels

1. `anadius-owned-server-source/server.cpp:213` initializes the pending socket through `initialize_0211b0`. That recovered constructor (`anadius-socket-source/lifecycle.cpp:23-35`) stores opaque table identities084928 at table_78,084908 at table_88 and084860 at table_90, plus context, zero_74 and self_reference_80. It does not create an OwnedEnvironment capability/table.
2. Selected Resources (`anadius-resources-factory-source/factory.cpp:68-75`) installs RecoveredSocketExecutor for Server cleanup and Runtime's socket move. Runtime creates DomainFlowApi with that ExecutorMove (`anadius-server-runtime-source/runtime.cpp`, bind). `DomainFlowApi::construct` (`anadius-session-flow-source/coherent_io.cpp:34`) calls SessionMemoryDomain::construct_session; `anadius-session-memory-source/memory.cpp:357-369` invokes actual `construct_session_owner_023000` using retained construction scratch.
3. `anadius-session-owner-source/owner.cpp:13-30` requests0x1c8, creates the control/owner, and invokes023d20 twice: pending -> temporary -> owner.socket. `anadius-socket-move-source/move.cpp:59-82` copies table78/table88, invokes observed table78 slot+8 on executor payload, reloads table90 after the callback, then performs028950 socket/list/control transfer. This preserves the concrete socket executor's native-origin table labels through owner creation.
4. Accepted-session flow invokes greeting before initial0280d0 read (`anadius-session-flow-source/flow.cpp`, accept_session_025720). Profile greeting deliberately creates an independent `Slot::environment` via selected OwnedEnvironment (`anadius-session-profile-source/profile.cpp:464`), then binds allocator label owner.identity()+0x78 to that sidecar (`:469-470`) and the message owner.environment_78 to the same sidecar (`:474`). It does **not** reinterpret or extract owner.socket.executor into an IoHandlerEnvironment.
5. Read0280d0 labels the stream as owner+0x10 (`anadius-socket-source/frame_read.cpp:60-65`). Composed read adds0x68 to that stream for021d10 (`composed_read.cpp:105`), yielding owner+0x78. Composed write passes its allocator_identity_68 (`composed_write.cpp:121`), explicitly bound to owner+0x78 by Profile. CoherentIoMemory/RecoveredConstructedWriteEffects resolve that label through the source registry and execute recovered023dd0 against the host sidecar (`coherent_io.cpp:13-24`, `environment_constructor.cpp:66-76`). The number is an identity/routing label, never an actual host member address or layout overlay.
6. Server accept is a different base: rearm_accept0270e0 passes FlowServer.identity+0x68 (`flow.cpp:44-46`). Profile::construct_accept_environment (`profile.cpp:447`) and OwnedEnvironment::accept validate exactly Runtime server_identity+0x68 and construct an independent host accept environment. Do not conflate this server+0x68 with session-owner+0x78.

Offset provenance inference to check against the native agent: session SocketObject begins at native owner+0x10; its executor payload starts socket+0x68, hence owner+0x78. IoHandlerEnvironment's operations+0x10 then corresponds to socket.table_78; schedule+0x20 to socket.table_88; executor+0x28 to socket.table_90. The current source labels support this arithmetic, but source typed fields are not a native ABI layout proof. Fresh023000/023d20/0211b0 evidence must confirm the complete origin chain before assigning concrete virtual targets to023dd0/028d30.

## Host projection versus recovered class

| Selected source item | Provenance / limit |
| --- | --- |
| Socket ExecutorFields table_78=0x180084928, table_88=0x180084908, table_90=0x180084860 | Recovered native-origin identity tokens set in0211b0 and propagated by023d20. They are not executable original pointers. |
| OwnedEnvironment Control.token / row.cap / Origin / job_epoch | Deliberate host capability and lifetime policy; globally generated IDs, not recovered native payloads or RTTI. |
| Control::Table (destroy identity1), move/query/copy | Host ledger operations: move changes registered address, query returns row.cap, copy creates a new capability/ref row. Their bodies are **not** established as84928's concrete native class. |
| Control::executor + factory/prepare/response20/response88 | Host bound functions routing through query_owner and make. No claimed original slot40/58/20/88 target addresses. |
| active_type()=1 and selected Types comparator | Host two-token classifier for the selected constructor branch, not recovery of original04d2d0 type_info comparator or concrete active RTTI. |
| empty sentinel fields named846c8/846e8/84770 | Selected configuration supplies `&Empty`, schedule0 and `&empty_executor`, not native image addresses. Empty's destroy_identity is017a30 solely for the recovered guard-equality decision; its C++ clearing behavior does not become the native017a30 RET body. |

`IoHandlerEnvironment` and `IoHandlerVtable` (`anadius-socket-source/io_completion.hpp:5-33`) explicitly model defined fields and functions separately from native storage/vtable ABI. The native-copy slot+8 is supplied separately via MessageEnvironmentCopyBinding, not through the host C++ vtable index layout (`message_dispatch.hpp:16`, `message_dispatch.cpp:32-41`). Existing algorithm bodies preserve native caller ordering but leave concrete target identities supplied by the selected bindings.

## Concrete body already available:0228c0

`anadius-socket-move-source/move.cpp:4-7` implements recovered0228c0: copy executor context and dword zero_74, set destination self reference, leave source unchanged. `move.hpp:14-23` maps only observed084928 cleanup slot0 to017a30 and slot+8 to0228c0. RecoveredSocketExecutor checks that exact table identity before using those source bodies. This selector is installed in the selected Resources path, not an unused legacy wrapper.

Prior saved `anadius-socket-move-source/selected-executor.md` (2026-11-03; adjacent17-record native bundle/session) explicitly records first **three** QWORDs of084928 as017a30,0228c0,0228c0 and the six decoded0228c0 instructions: qword payload+0 copy, dword payload+0xC copy, destination+0x18=self, RET. Consequently operations slot+8 COPY and slot+0x10 MOVE are the same recovered leaf **if the fresh origin chain confirms084928 is the active environment's operations table**. Do not report these as wholly missing algorithms. Their existing implementation uses ExecutorFields; no current production IoHandlerVtable/MessageEnvironmentCopy adapter maps that concrete body onto the active environment projection.

The host active environment's state_18=1 and capability payload differ from0228c0's concrete self-address/state semantics. Replacing its ledger copy/move with that leaf by name alone would silently change authority and payload meaning. A bounded concrete adapter needs the actual field/target proof; this audit proposes no edit or invented class.

## Recovered callers already implemented; concrete target unknowns

| Native selector | Existing source body | Concrete dependency still supplied |
| --- | --- | --- |
| 023dd0 | socket-source/environment_constructor.cpp:36-63 | schedule slot0 type query, active ops query+18, executor factory+58, ops move/destroy. Correctly caches factory before query callback. |
| 023cb0 | socket-source/io_completion.cpp:28-39 | ops move+10, native empty sentinel identities. Copies/reset defined prefix, invokes move, reloads executor after callback. |
| 028d30 copy/dispatch portion | socket-source/message_dispatch.cpp:47-89 | ops copy+8 via explicit binding; does two copies and reloads executors before021420 and paired destroys. |
| 0229d0 | socket-source/handler_dispatch.cpp:152 onward | executor prepare+40, ops query+18, resulting schedule. The algorithm exists; Control::prepare is host target binding. |
| 021420 | socket-source/response_dispatch.cpp:65 onward | executor factories+20/+88, ops query/move/destroy, resulting schedule. The two-stage recovered algorithm is already present. |

Current production source search found no concrete native implementation of84928 query slot+18,084908 schedule/type-query slot0, or084860 executor factory/prepare targets. The original04d2d0 comparator is still an interface in constructor/service/scheduler registries; the selected OwnedEnvironment two-token comparator is not its native body. Only Empty and Control::Table provide production IoHandlerVtable subclasses in this source set; test vtables do not close these unknowns.

Precise table-entry selectors for the native agent, **not guessed target functions**:084928+18 =084940 (query);084908+0 (type query);084860+40 =0848a0 (prepare), +58 =0848b8 (constructor), +20 =084880 and +88 =0848e8 (response factories). Empty tables846c8/846e8/84770 require separate evidence if used as concrete classes. Read pointers first; do not infer nearby function addresses from host labels or declare every indirect target new/missing before checking the already recovered0228c0 leaf.

## Separate native-agent update

After the source scan, the native agent reported fresh resolution of084928 as {017a30,0228c0,0228c0,006180},084908 slot0 as023890 (returns0a25c0), and084860 prepare40/factory58 as023520/023580. It also reported tracked tables084a18/849f8/84950 with copy0228a0, move0231b0, destroy0229a0, query006180 and prepare023310/factory023360, plus supporting020c80/020c10 construction bodies. The adjacent query files and exported bundle were produced by that agent's owned session, now closed. This source auditor did not run or independently validate those native queries; consult its final native findings and root's review for exact field/flag/work-count semantics.

These addresses resolve the earlier native-selector questions, while the source finding remains: the current selected source has0228c0 through ExecutorFields, but no concrete active IoHandlerEnvironment adapters for the newly resolved untracked/tracked tables. Do not mark023dd0/028d30 algorithms missing or duplicate the recovered0228c0 leaf; use the newly resolved bodies to determine the smallest concrete environment implementation after evidence review.

## Audited key source hashes

| File | SHA-256 |
| --- | --- |
| owned-environment-source/environment.cpp | e7293bf8bfcb67ec566e8518b34f6c76719ac9810dcd25e3f3242b74183aff3f |
| socket-move-source/move.cpp | a190efa2630a4f43de29a432236c34dbe5562029acc9145d1b2f6794a5faf39a |
| socket-source/lifecycle.cpp | 6f5f684e1fb3224a5a9fd92ebf51f4b495effecf0f5ee7aad7fa7bc92fbb52c8 |
| session-owner-source/owner.cpp | 31b749d9bbe830e42dd13b7d04be651fb5ce1237f27d2e2ba9a4fe39ab232f3c |
