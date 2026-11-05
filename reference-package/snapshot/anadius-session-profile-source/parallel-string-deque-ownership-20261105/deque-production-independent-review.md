# Independent Deques source review

2026-11-05. Read-only review of the Deques author's two-file candidate against this stage's actual before copies. This reviewer authored Parent/Strings/header consumers, **not** the Deques files. No source edit, compiler, test, link, DLL execution or package update was performed.

**Result: no blocking finding in the inspected scope.** This is static source review; actual Original production concurrency remains disabled and unvalidated.

Frozen SHA256 rechecked:

- `outputs/anadius-owned-deque-source/deque.hpp`: `cf38e757e6189ca3a6a2cd3c9016c504401c6941b7a50cff10c1f225aa3cb35e`
- `outputs/anadius-owned-deque-source/deque.cpp`: `35bcec259f98a2f65e136f6d257b36cdf1be699dba689d6b814b50cc7474d10d`

## Authority and concurrent call state

Mode is immutable and derives from the same Parent. Concurrent entry checks the copied exact child JobPermit or the original provider's actual worker/fiber; subsequent admits also check explicit Parent Operation. A thread's FLS state alone does not create a Deques call. Passive metrics and pointer identity are not source permission.

The stack Admission at cpp:133–145 reserves the exact queue and actor before compiler allocation. Same-actor allocation/checkpoint reentry is sticky poison; a healthy foreign actor's collision on an already growing queue rejects without poisoning the independent call. Distinct queue/actor entries keep their own stable Call, maps, allocation/slot adapters, checkpoint and copied permit. There is no shared `retained`/Growing body reservation in the Concurrent branch and no metadata mutex spans Parent allocation/free, checkpoint or recovered026670.

Fully constructed Call descriptors are spliced to owned storage before a sticky preparation failure can unwind their captured targets. On a normal success only that exact row is removed. Admission removal uses no allocating list operation and cannot remove a sibling's row. Call actor/thread/fiber/running membership is checked under the provider gate; Parent/permit validation runs outside it.

## Map proof and source ordering

Initial nonempty queue validation requires coherent native power-of-two/count/offset fields and the new metadata-only Parent `owns_output_map` predicate. The predicate accepts the full exact typed array, not a prefix or arbitrary same-address span. No all-map catalog is borrowed or pinned.

`Call::admit()` installs the exact Parent map resolver in Concurrent mode. It intentionally does not re-run the initial map predicate after every effect. Recovered026670 frees the old map before updating the owner's old pointer fields; inserting that predicate there would reject a valid normal prefix. Byte/header reads and pure slot copy/zero remain bounded by the actual owned maps. Native allocation/free and owner-field update order are unchanged.

The normal map allocation returned span pins only that actual Parent ticket for the current actor; own free consumes that view, while other actor/SDK protection remains in Parent. This is storage lifetime, not synchronization for arbitrary concurrent writes to one deque. The real caller must retain the session/control and queue through its own call.

## Target retirement and failures

The checkpoint target is emptied and destroyed outside the gate before final actor/Parent/permit validation (cpp:170–174). A target destructor can reenter or revoke the actual permission; the final checks and subsequent provider-phase check prevent publishing a clean success after those observed failures. The retired Call row is spliced out under the gate and destroyed outside it with metadata LastError preservation. No callback-bearing unique_ptr/function target is deleted under the nonrecursive gate.

Unknown failures retain the actual Call descriptor and completed map prefix, mark that call nonrunning and poison the provider. They do not roll back/replay026670 or free newly allocated storage speculatively. Pending list nodes/fully constructed descriptors are either retained or destroyed outside the gate; the actual Parent allocation ledger still prevents false drain.

Concurrent close returns false while admission/call rows exist instead of poisoning or waiting on an opaque growth body. Only the bound original actor can close; Parent admission is validated after the short Retiring reservation and the owning lease is released outside the gate. An unsupported/unknown prefix cannot be certified closed.

## Serialized and remaining boundaries

The default branch retains the old one-active-call phase, worker affinity, legacy map binding span, exact growth algorithm, metrics and close semantics. Shared helper adaptations select the resolver only for Concurrent Parent. No production policy switch, SDK serialization, per-session message strand or new unload proof is introduced here.

The planned live two-queue/real-allocation regression and existing Serialized child-permit cases were consulted only as source scenarios. They were not run, and the new test author's in-progress bytes are not certified by this production review. This review does not validate the final NativeJobs/Graph same-session/distinct-session progress trace, arbitrary foreign checkpoint mutation, native atomic/EH ABI, or the later per-message provider integration.
