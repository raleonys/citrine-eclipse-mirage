# B30 independent constructor evidence/API audit

One normal typed body, `023000`, is supported by retained evidence. Use a caller output pair reference and existing IoReference/FrameReadOwner, B29 socket moves, actual cipher reset and actual temporary destructor. Full native allocator, vtable disposal and ABI/lifetime closure remain explicit.

The owner begins at native control+0x10. Its weak-self is owner+0/+8, socket+10, input+a8, deque+c8 and handler+f0. The first move at2306b occurs **before** weak-self zero stores23078/7b; second move is23086. Input has16zero inline bytes, size0 and capacity15. Five deque qwords are zeroed; the16-byte proxy has slot8 zeroed, then header.proxy is published, then proxy[0] points to header. Reset230e2 precedes temporary destruction230eb.

Output object/control stores230f9/fc occur **before** weak-self setup and callbacks. Skip replacement when an existing weak control has nonzero strong. Otherwise increment allocated strong, cache live output.control, increment that weak, write weak object, load displaced weak control and write new weak control. Delete displaced weak onlast, then release the SAME cached published strong, dispose onlast and reload/decrement its weak before delete. Ordinary successful exit is strong1/weak2. No owner access follows final release.

Saved table084750 slots00/08/10 are026570,017420,026290. Dispose026570 receives CONTROL base: deque+d8, input+b8, socket+20 and weakself+18 correspond to owner+c8/a8/10/8. Delete017420 dispatches virtual slot10 with EDX1;026290 restores table identity and sized-frees0x1c8 whenflags&1. These destructor/deleter bodies are not currently source-recovered and must not be counted as constructor recovery.

Existing WriteFinishOwner covers map_d0/map_size_d8/offset_e0/count_e8 but not proxy_c8. A new owner wrapper should add an explicit proxy/backpointer projection and reuse all existing mutable state. Actual allocator requests0x1c8 and16 require nonnull-or-throw. The apparent allocation-null branch does not yield a valid empty owner. Normal retained ASM does not establish full native exception cleanup or allocation rollback.

The constructor's selfweak does not protect existing detached raw-owner jobs. All owner/context/code lifetimes remain caller obligations until actual asynchronous work is drained. No host shared_ptr substitution or quiescence claim is justified.

Address phases, evidence IDs, field/control coordinates, current source hashes and API/failure recommendations are recorded in JSON. No original input read/execution, new target query, production/test modification or generic CRT work occurred.

The proposed embedded SessionControl/SessionOwner plus persistent single-use Scratch fits this normal algorithm. Delete Scratch copy/move because its socket references internal state. Preserve FrameReadOwner.input_a8 when clearing only the native weak pair. Opaque owner tokens require typed relative-address bindings, not host offsets. Retain partial allocations/scratch through separate cleanup after failures, and never dereference retained control pointers after final release may delete them. This proposal review is not a production or final frozen-source review.

Current `anadius-session-owner-source/owner.hpp/.cpp` received an early independent read-only review with no blocker. It preserves two-move/weak-clear order, explicit proxy initialization, actual reset/temp destruction, output publication and cached callback-sensitive control release. Only independent scratch metadata is written after possible owner/control deletion. Output is raw constructor storage, not an existing RAII owner; its prior control is not released. Production hashes are recorded in JSON. Final tests and common build are not yet reviewed.
