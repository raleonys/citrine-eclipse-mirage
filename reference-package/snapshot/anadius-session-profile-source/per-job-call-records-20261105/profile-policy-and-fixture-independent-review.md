# Independent Profile, Policy, composition and fixture review

2026-11-05. Read-only review against the actual stage `before/` files. All 12 captured before-file hashes match `before-manifest.json`. I authored none of these seven changed source files. No compiler, project test, linker, reconstructed/original DLL execution or packaging ran. Read-only CR-aware diff whitespace checks produced no diagnostics.

| Reviewed source under outputs/ | Exact SHA256 |
|---|---|
| anadius-session-profile-source/profile.hpp | `ede20dcc14bcfb1d4859f91b2f9222051c43ad7044947badc853133c65d49c5a` |
| anadius-session-profile-source/profile.cpp | `55b0964eb026adea5fb7a8acf793fdbd818b6a12a6c65c65f497503dac6cc69b` |
| anadius-session-policy-source/policy.hpp | `720a15b840365f5e87fe3cc7e0ae4e98b88df195dd5d1f078aea723eff854aea` |
| anadius-session-policy-source/policy.cpp | `6338a6ecb4d7c3d736057da418a49fe18debb300ff58131ff85468248554be23` |
| anadius-owned-server-source/server.hpp | `eca6cb891d6561312320431556c48857c55ddd946381e3619f7180a2589f5cff` |
| anadius-owned-server-source/server.cpp | `9e96247b84ff6396809c4f5d2bfda12b887b564fa8ea3437ac010a69878b4c9b` |
| anadius-application-graph-source/graph_integration_tests.cpp | `b80e0d82a7ee1c2a09f2ecd588148f7891d04936b9b7dd0f702ed757e1088354` |

Result: **no blocking finding remains in this bounded static source scope**. This does not establish an executed concurrent schedule or complete native equivalence.

## Profile capture, admission and publication

Stable JobRecord/SlotRecord addresses and the prelaunch immutable capture carry the exact closure, owner/control, token generations and Logic pointers. The child acquire/process/return paths use captured rows; the reviewed child region contains no deque traversal, `slot()`/`job()` lookup or replaceable closure-table indexing. Growth repairs the captured row's actual public binding pointer under State.gate before retiring the old table, retaining the binding-tamper check. Direct source controls, weak-self identity and native entry remain checked outside metadata gates, with current metadata checked again afterward.

Concurrent authority accepts the exact prepublished Submitting row plus acquired JobMemory/permit before original start-return/detach. This follows the fresh launch evidence: capture stores at `180025522`/`18002552d`, zero launch flags at `180025543`, launch at `180025556`, later detach at `18002557f`; complete EIDs are retained in `native-findings.md`. Serialized keeps its earlier admission contract. Known returned launch/detach outcomes are adopted before opaque postchecks; original return does not replace an already advanced child message state.

ChildUse owns an exact handoff epoch, per-slot call count and processing reservation. Retained checkpoints require their own live epoch rather than a sibling's aggregate count. Final source/Parent observations are followed by exact handoff/capture checks before Prepared publication. Global counts remain observations/retirement constraints, while root_calls separates original reentry from healthy children. Slot retirement still requires its own jobs/calls/strong ownership to drain.

The narrow ContainerMutation reservation covers actual original-worker deque resize and rollback operations. Root metadata rejects before structural access during allocator reentry; captured children and passive count snapshots do not traverse that container. Allocations, opaque checkpoints, source providers and ownership destruction are outside State.gate. Original completion moves retained objects out before their destruction; unknown handoffs retain their source prefix with sticky failure.

## Policy and actual Parent composition

Cleanup admissions reserve the exact actor before allocation, publish a stable call with copied permit/lease/checkpoint, and remain published through checkpoint-target retirement. Concurrent child validation uses its own registry membership and actual Parent/permit, independently of original active state. Existing current-thread ExitWaitRegistry cleanup and native synchronization are retained. Root and child cleanup counters use the metadata gate; the original-counter race found during review is fixed.

The final CleanupCall postcheck clears checking and revalidates phase/open/failure/binding/closing under the gate. The final close reservation also rechecks pending/published calls immediately before closing=true; a newly admitted healthy child causes normal false deferral. External attestation commits perform the corresponding final membership checks. These fixes prevent later effects after a known poison/close boundary without holding metadata over opaque callbacks. Failed calls remain registered; no cleanup replay or inferred rollback occurs. Metadata LastError guards leave actual SDK effects outside their scope.

The trailing server dependency defaults to Serialized. Both constructors validate its enum before adoption and pass the same immutable selection into the actual Parent constructor. No production caller opts into Concurrent here, and no substitute Ports tuple or special test getter is introduced.

## Fixture source validation

The isolated `--prepared-profile-parallel` mode uses the actual Domain, native thread wrappers, PreparedMemory, Backend, Runtime, Graph, Profile, Policy and concrete provider call views. A controlled PreparedJobs adapter supplies exact prelaunch claims and real IOCP receipts. It intentionally does not substitute for an actual NativeJobs concurrency result.

Two actual children process separate long XML requests on the SAME Session/Logic. A enters the real UnknownMessage diagnostic path and is held in the injected Dialog SDK leaf before lower start returns/detach. B processes actual GetConfig and returns memory before its lower start returns; initial job capacity1 grows while A's captured row remains live. Root consumes B's real receipt and normal response, validates A's retained request bytes and passive session identity, and retains A as the sole source job. Each delegated job owns one real work pin; the actual Runtime dispatcher consumes its receipt/decrement once. No manual scheduler/owner/count reset is used.

Native joins and Parent reaping occur outside the original Parent Operation. After B's actual exit and reap, the controller-owned passive PreparedMemory observer must report exactly one outstanding job while A remains held. Thus the assertion observes retirement rather than merely calling reap. Only afterward is A released. The final response has B's long ID and one NUL frame, with no fabricated A response.

Two additional real prepared children exercise overlapping Policy cleanup: B completes its own registered target while A's exact checkpoint/target stays live; an actual original cleanup also progresses without consuming A's target. Normal execution accumulates seven real cleanup calls. All native/FLS joins precede handle/backing/provider retirement. Assertion count is atomic; shared log trace counters are protected, and final observations follow actual joins. The Dialog pair is constructed once before provider publication. Existing seventy-job and terminal modes remain registered; none was run.

## Explicit remaining boundaries

Actual NativeJobs still has the global source_gate and detach barrier; this stage does not prove its parallel body/reap progress. OwnedEnvironment execution and Policy environment_work_active remain separate singleton restrictions. The basic process_message route does not invoke clone_environment/enter_job/finish_environment_work, but arbitrary concurrent tracked-family release is not certified. Prepared response/021770 completion still runs on the original receipt tail; full native child-tail ordering, arbitrary shared business mutation, generic custom providers and compiler ABI/EH remain outside this result. Source fixture assertions are unexecuted; an explicitly authorized rebuild is still required for runtime evidence. Original binaries, historical build proofs and delivered ZIP are unchanged.
