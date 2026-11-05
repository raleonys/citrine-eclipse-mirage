# Prepare40 caller ownership independent static review

Reviewed 2026-11-05. This reviewer changed no production or test source and ran no compiler, project test, linker, reconstructed DLL or ZIP operation. Review used the frozen source, captured prior source and saved native instruction replies; this reviewer opened no new REA target. No blocking defect remains identified within the bounded synchronous preparation change. This is a source-review result, not an executed PASS or native-equivalence/completion claim.

## Exact seven-file freeze

Paths are relative to the project root. Independently read and SHA-256 checked after all authors froze their files:

| File | SHA-256 |
| --- | --- |
| `outputs/anadius-socket-source/io_completion.hpp` | `3428c513b838f673a0b20bb61684cc436ccf7370662ca5a15caafb278162fd09` |
| `outputs/anadius-socket-source/io_completion.cpp` | `3d51a40ebf0d40fc30c1cb2f27d41a8c144acff37ca4806b7dc6765568bb36c6` |
| `outputs/anadius-socket-source/handler_dispatch.cpp` | `82e5e253170f7b97f28cd5acaa25d07c1223ad39d149de18931dd9d1290a62a3` |
| `outputs/anadius-owned-environment-source/environment.hpp` | `f45705884f794a56e11d511601d187d2111dab1dbc3ead78caf3210f3c4747fa` |
| `outputs/anadius-owned-environment-source/environment.cpp` | `332027af5206b7ba6aa4bd72e59ffa087a539d4d341608bba6b4cf1481de2968` |
| `outputs/anadius-socket-source/io_completion_tests.cpp` | `9782579292f1075f9684e237f2cc7e87ecef8db1bd4cf253f31cf63caf1d0f8b` |
| `outputs/anadius-owned-environment-source/environment_tests.cpp` | `03e95083dcf76d2002d7794daa10eddbf8c880536fa97b97ca736ff55f07170e` |

## Native evidence inspected

The stage's complete saved REA 6.8.0/Ghidra 12.1.4 instruction replies identify the original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

- `instructions-1800229d0.json`, Evidence `ev_c3016d57cf23af5fa783396940f99c700701bbb46e18247c1af6ebb1aae50d11`: one inclusive Ghidra function range `0x1800229d0..0x180022c3a`, 619 bytes. Slot40 FUNCTION cache `0x180022a02` precedes query call `0x180022a0e` and cached function call `0x180022a1d`. The actual move precedes the live moved-from temporary guard comparison/call at `0x180022a82` / `0x180022a8c`. Scheduling and final moved-output destruction follow preparation.
- `instructions-180027710.json`, Evidence `ev_6ff5df9f920a7b81c7935c726699b51d98c9cb814b0887bf86d738158d86a902`: one inclusive range `0x180027710..0x180027a15`, 774 bytes. The nested receive path caches40 at `0x1800278f0`, queries at `0x180027900`, invokes cached40 at `0x18002790f`, performs the move, then reads the temporary's LIVE destroy target at `0x180027965` and guard/call at `0x180027974` / `0x18002797e`. Receive scheduling `0x180027989` and final nested destruction `0x18002799a` remain outside the extracted preparation segment.

These are Ghidra function-body address-set and decoded-instruction observations. They do not independently establish runtime overlap, native ABI/EH equivalence, complete downstream scheduling behavior or an increase in total function coverage. The saved replies retain their provider limitations; instruction fast-path queries do not decompile or scan the whole program.

## Source conclusions

Both actual consumers now call the same `prepare_io_environment_40` helper: send scheduling and receive completion's nonempty nested path. The helper selects explicit synchronous Preparation before reading source fields. Otherwise its optional HOST owner hook precedes executor/cache/query effects; default unowned providers retain the raw algorithm. No caller signature, IoCompletionEnvironment or deferred HandlerTaskContext gains a child pointer.

The body owns a distinct real temporary for the entire hook invocation, including the owning provider's final admission. Its address accessor permits source/full read plus output/temporary write claims to be seeded before `Control::run` performs its first checkpoint. Native ordering remains cached FUNCTION before query, cached call without executor-this, move through the actual023cb0 helper, and LIVE moved-from destroy-target guard after the move. No invented source/output unwind cleanup was added.

Early review found the initial body could be replayed by a hook running it and returning false. The final Root source consumes `attempted` before executor access, including failed attempts, so fallback or repeated invocation rejects before a second native prefix. Explicit preparation requires the exact output reference and a completed body; a true owner hook also requires body completion. These guards were inspected in the frozen source.

Root Table and JobScope's stable JobState-owned facade share `prepare_whole`. One registered Effect claims exact source/output/actual temporary before callbacks or native field effects. A private stack Calls adapter binds that Effect; it validates the exact source query/capability, canonical cached prepare function, real temporary factory destination, exact output/temporary move and moved-from-only guard cleanup. It does not recurse through raw callbacks, resolve an ambient current Effect or temporarily swap shared context. Scope metadata validates the actual actor and lineage; foreign/wrong-scope inputs remain known preflight rejection. Root raw preparation remains original-actor-only.

Final proof checks temporary ownership metadata before inspecting its live fields, exact source/output generations, full source/output and canonical moved-from empty fields. All claims survive normal final admission. Registered projections remain checked across callbacks; the nonmutating checkpoint contract bounds arbitrary direct writes to the unowned canonical temporary during that last callback. The temporary remains alive until the hook returns. Body/Calls references are not retained. Unknown unwind retains failed address/prefix metadata, not replayable stack callbacks; shared failure/phase guards prevent later successful replay against an ended helper temporary.

The facade contains a raw JobState backlink, while existing scope/control/provider leases maintain lifetime; this introduces no shared ownership cycle. Scheduling authority and delayed task admission are unchanged. Existing task cleanup/self-delete boundaries are not crossed by a newly retained preparation reference. Tracked-family preparation uses the existing Transform::Prepare publication/work retain and transfers its existing capability/pin through the move; this review did not execute that path.

## Authored test coverage and open limits

Static test review covers actual public helper fallback, handled private-operations dispatch, cached function survival after query deletes its executor, LIVE post-move temporary destroy target, explicit refusal before null source field access, and the malformed run-then-false replay boundary. Four raw exception boundaries preserve query/factory/move/destroy prefixes without invented cleanup. The real selected Host Environment test forwards the SAME body, observes one active effect before publication, temporary-only factory publication, full output tail, exact source preservation, distinct capability, reference/accounting drain and LastError preservation. Passive observations alone are not proof of private reservation metadata; that proof comes from the separately inspected source seeds.

The tests were authored, not run. True-hook-without-body and explicit wrong-result guards have source review here, not newly authored test coverage. The owned positive fixture is Root/Host; it establishes no actual Scope/Concurrent Runtime overlap or tracked child-old1 result. Actual two-scope progress, selected-provider production injection, physical child exit/reap with a sibling held, child handler/response scheduling, response20/88 integration, full028d30/021770 tail and native ABI/EH/runtime equivalence remain OPEN. Original binaries, historical builds and delivered ZIP proofs remain unchanged.
