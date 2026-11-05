# Original event peek, lock, pop and unlock ordering

2026-11-05. Eight fresh selected instructions were obtained through aligned REA6.8.0/Ghidra12.1.4. Complete inline replies and the eight-record export are saved. The session was closed; original SHA256 afterward is unchanged: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

`1800342fc CMP qword ptr [RAX+20],0` tests queue count before `180034317 CALL 18004a6ac` acquires the actual event mutex. After that call, `180034332` tests the SAME queue count again. `180034337 JZ 180034570` bypasses payload/pop work when another actor has emptied it before acquisition, but still reaches unlock. Thus a nonzero pre-lock observation does not entitle the reconstruction to demand a nonempty post-lock head.

On the pop path, `18003451b SUB qword ptr [RSI+20],1` commits the actual shared count. `180034522` resets the offset for empty result, or `180034528` increments it for the nonempty case. These writes precede `180034573 CALL 18004a6b4`. They are not an unlock-time transaction or a queue snapshot merged only after a callback returns. The unchanged typed033d70 body must preserve this committed prefix when unlock has an unknown result.

The C++ projection therefore uses one shared lock-free relaxed count cell, while map/offset/payload access and mutations remain under the actual queue lock. It does not acquire the event mutex merely to peek, clone queue ownership, or defer pop publication. The selected Win64 atomic wrapper has a compile-time lock-free requirement; this is a source memory-model mechanism, not a claim that its class is an original MSVC ABI overlay or that arbitrary concurrent writers are permitted.

The eight instructions alone do not prove the full function or helper synchronization. Explicit historical context is the complete033d70 retained assembly/pseudo and caller proof from handler-admission-overlap-20261105, with hashes in the native evidence index. Existing event/achievement lock identities and caller-owned lifetimes remain mandatory. No runtime schedule, native EH/SDK equivalence, additional reconstructed-function credit or full production concurrency follows from this query.
