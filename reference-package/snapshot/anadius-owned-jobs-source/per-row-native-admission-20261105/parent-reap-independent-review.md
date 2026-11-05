# Independent static review: original-worker Parent maintenance

Status: no blocking finding in the reviewed three-file change. This is source inspection only; no compiler, project test, linker, or DLL execution was used.

Reviewed bytes (SHA-256):

| File under outputs/ | SHA-256 |
| --- | --- |
| anadius-session-memory-source/memory.hpp | 73bb13f7c083b0546b6e45e52e67fa5fbdd8aadbc761217fa5af55f7bfbb5392 |
| anadius-session-memory-source/memory.cpp | 2ca6df7d1ed04b7e32cf0db904824729fdfac20c735d1f07624b76f5073ae73a |
| anadius-worker-handoff-source/handoff_tests.cpp | de2dbd557873984bdae5dbd5a2704ca19f87d6400e5974ab5f4d4e6e907be2b0 |

`SessionMemoryDomain::reap_jobs()` validates open lifecycle, an existing borrowed backing, and absence of this actor's actual Operation or pending admission before entering maintenance. The own-actor check precedes `try_lock()` because the selected Serialized gate is recursive. A successful recursive lock must not turn own-Operation misuse into authorized reaping.

The exact original-worker Borrow is checked outside Parent metadata, including backing identity and allocation-tracking poison. Only then can Serialized mode return a healthy zero for a foreign body holding its existing Operation gate. The busy branch repeats lower capability/health and Parent metadata checks before returning; a wrong controller cannot obtain the same success merely because that gate is busy. A lower fiber/PTD mismatch throws and stays sticky in Borrow. This does not claim that every preflight exception changes Parent's own phase.

Serialized maintenance uses the already selected Parent gate with nonblocking `try_lock()`; it adds no NativeJobs body lock or all-sibling thread/FLS prerequisite. Once acquired, that lock excludes a new Serialized admission until the maintenance reservation clears. The local reservation destructor precedes destruction of the serial lock, so a later Operation cannot acquire the gate while observing the old reservation. Concurrent mode takes no serial lock and retains its existing foreign-actor admission behavior during original-worker reaping. Parent metadata is released before lower reaping, native wait/close, and allocator effects.

The authored `parent_job_borrow()` schedule holds a real child Parent Operation with an actual allocation while the original worker requires zero from maintenance, an open Parent, and preserved LastError. The child waits for both that check and a controller's rejected maintenance attempt before leaving its Operation. Existing nested/self-reaping rejection, moved permission, same-Domain wrong-root rejection, logical-return versus native/FLS-exit, and exact original/controller closure checks remain present. Event publication keeps shared Parent owners live throughout the controller probe. These are meaningful source assertions; they have not been executed in this stage.

The bounded native motivation remains the saved Ghidra 12.1.4 evidence for original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`: `18004a458..18004a472` closes the original native handle without a detach event, and `18005e8e0..18005e9b9` forwards the launch flags to CreateThread and returns its handle/id without waiting for detach. This Parent maintenance adjustment supports removal of the reconstructed scheduler's extra global gate; it is not independent proof of complete native behavior or full reconstructed DLL equivalence.
