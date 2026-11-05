# Root static review of NativeJobs and PreparedMemory changes

STATIC PASS for the four backend sources authored by a separate subagent. This review compared their before copies and current implementation; no compiler, project test, linker, reconstructed DLL execution or packaging was invoked.

NativeJobs list rows preserve raw Row pointers passed to native thread entries. Publication uses the same metadata mutex as row iteration/snapshots; a new node is prepared outside that mutex and admission is checked before splice. Child operations do not change list topology. Row ownership, state and receipt publication remain inside the existing active lifecycle reservation. The caller receives the exact published row pointer before later failure can reach failed(row), so unknown prefixes remain retained. Existing preallocated-row reservation does not acquire a new provider callback. Physical-row retirement still waits for the existing source acknowledgement and actual thread/FLS exit proofs.

PreparedMemory retains stable shared JobState identities in its existing list. The existing job_control reservation now spans allocation outside the mutex, worker/fiber/PTD/phase is rechecked before splice, and only unpublished state is destroyed on prepublication allocation failure. The reservation guard clears after the noexcept result construction, without reverting a poisoned phase. No capability exists before publication. Original child acquire/return/cancel, scope and physical-reap logic is unchanged; this review does not claim general concurrent allocator safety.

The removed64 checks were reconstruction limits on the examined per-frame native path. Other resource limits, InlineJobs policy and existing reaper implementation remain separate. Full selected behavior additionally requires the coordinated Profile job/closure changes, reviewed separately.

Reviewed SHA-256:

- jobs.cpp: `47f1098982d46ffc3cc2951bcce433f80d718cc0e14c98425aa85f5304cdedc7`
- jobs.hpp: `a2a1f920550b3d9ef9112aa0e312733c8430dc1617141082595e2bfbf944743d`
- handoff.cpp: `c082860b0727fbeb9cf506bda213f3c13778353c99e6ef1de73f2e6bcf09170b`
- handoff.hpp: `37078e266fad3e124c07a62192ad7f73963e437feff70939274de12e9460bbec`
