# Actual prepare40 consumer seams

Source-only implementation; no compiler, project tests, linker, DLL execution or ZIP update.

Root-owned source freeze under outputs/:

| File | SHA-256 |
| --- | --- |
| anadius-socket-source/io_completion.hpp | 3428c513b838f673a0b20bb61684cc436ccf7370662ca5a15caafb278162fd09 |
| anadius-socket-source/io_completion.cpp | 3d51a40ebf0d40fc30c1cb2f27d41a8c144acff37ca4806b7dc6765568bb36c6 |
| anadius-socket-source/handler_dispatch.cpp | 82e5e253170f7b97f28cd5acaa25d07c1223ad39d149de18931dd9d1290a62a3 |

The shared prepare_io_environment_40 helper now owns the actual synchronous preparation segment at BOTH send0229d0 and receive027710 consumers. It caches slot40 before query can delete/change the executor, invokes that cached function with query/opaque scratch, moves the real temporary through023cb0, then reloads the temporary's live destroy identity for conditional cleanup. Scheduling and final prepared-output destruction remain in their original outer callers. Receive's earlier operation cleanup and all later handler/control cleanup are unchanged; the helper holds no operation/task reference.

A stack Body owns the actual temporary before selecting any owner, so an owning provider can claim its ADDRESS along with source/output before the first checkpoint/cache/query. The temporary remains alive through the whole hook and final owning admission. Before admission, the accessor grants its address only, not authority to inspect unspecified fields.

Explicit IoEnvironmentPreparation is selected before ALL source field reads. Otherwise an optional HOST source-table hook can reserve the whole operation; default false retains the same raw body. Private IoEnvironmentPrepareOperations carries query/cached40/move/destroy calls on an already admitted effect. These are HOST typed interfaces, not recovered native ABI/vtable slots.

The body consumes one attempt before reading executor. A repeated call, including fallback after a malformed run-then-false hook or retry after an exception, rejects before a second query/factory. Explicit calls must return the exact output after the body returned; a handled Root hook must have completed the body. Unknown query/factory/move/guard outcomes preserve their actual preceding effects without invented normal cleanup or replay.

No child facade is stored in IoCompletionEnvironment, HandlerTaskContext or any deferred task. Existing function signatures and deferred context lifetimes remain unchanged. Scope pure preparation can select this same synchronous helper, but this stage does not grant child scheduling, complete-IO ownership or response20/88 authority. The actual outer consumers still use their existing Root scheduling composition.

Fresh addressed REA6.8/Ghidra12.1.4 evidence is saved in this directory and exported/closed: send ev_c3016d57cf23af5fa783396940f99c700701bbb46e18247c1af6ebb1aae50d11; receive ev_6ff5df9f920a7b81c7935c726699b51d98c9cb814b0887bf86d738158d86a902. See native-findings.md for exact instructions and body extents. No new recovered-function credit, compiler/runtime result or overall completion percentage is claimed.
