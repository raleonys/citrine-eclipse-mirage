# Busy-aware original-worker Parent maintenance

Source-only change, 2026-11-05. No compiler, project test, linker, DLL execution or ZIP operation.

SessionMemoryDomain::reap_jobs rejects an unavailable Parent and its own active or pending Operation before acquiring the recursive Serialized gate. It validates the actual original PreparedMemory::Borrow thread/fiber/PTD and exact backing before any busy-zero return. A rejected foreign caller is outside the later OS-outcome catch; wrong-thread misuse does not poison a healthy Parent. Borrow retains its own sticky fiber/PTD failure state, which still throws rather than reporting healthy busy.

Serialized maintenance tries the existing whole-Operation serial_gate without waiting. A foreign active body yields zero after another exact capability/health/metadata check. A successful try owns that same exclusion until after the maintenance RAII clears reaping. Thus a new Serialized child waits outside Parent metadata and enters only after that reservation clears. OriginalConcurrentActors takes no serial gate and retains the existing foreign-actor overlap rules. No all-sibling native-exit prerequisite or new scheduler/body mutex is introduced. Parent metadata is not held over lower snapshot allocation or native wait/close.

The existing parent_job_borrow SDK test now holds the child's actual Operation across two finite event handshakes. The actual original worker checks busy-zero, Open phase and preserved LastError; the foreign controller checks rejection of busy maintenance without poisoning. The child cannot unlock before both checks, and the existing self-reap rejection, moved JobMemory, wrong-root/default-entry rejection, source allocation and native/FLS distinction assertions remain. Assertions were authored and statically reviewed, not run.

Frozen source for this bounded review:

* `outputs/anadius-session-memory-source/memory.hpp`: `73bb13f7c083b0546b6e45e52e67fa5fbdd8aadbc761217fa5af55f7bfbb5392`
* `outputs/anadius-session-memory-source/memory.cpp`: `2ca6df7d1ed04b7e32cf0db904824729fdfac20c735d1f07624b76f5073ae73a`
* `outputs/anadius-worker-handoff-source/handoff_tests.cpp`: `de2dbd557873984bdae5dbd5a2704ca19f87d6400e5974ab5f4d4e6e907be2b0`

This fixes maintenance admission; it does not select Concurrent for existing production callers, prove allocator/foreign SDK simultaneous safety, or close full native response-tail/tracked-environment integration.
