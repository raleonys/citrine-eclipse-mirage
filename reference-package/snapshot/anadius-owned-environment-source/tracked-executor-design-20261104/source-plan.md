# Concrete tracked executor integration — selected-source design decision

Read-only design audit of current production sources and the closed20-record environment /11-record response evidence. No source/test edits, REA calls, original access, compilation, tests, linking, DLL execution or packaging.

## Recommendation: defer implementation until a real tracked consumer is selected

No current actual OriginalDefault production call chain inspected here reaches the tracked family. Initial Slot sources have flags0 and the initial schedule classifier;023dd0 takes the evidenced empty branch. JobHandoff::clone_environment takes that same optimization. Message028d30 still copies its context-bearing initial Slot environment, but native021420 factory20/88 remain **untracked**, producing flags1. The fresh response audit explicitly closed that selected route as NO CHANGE; its one posted work pin belongs to028ca0, not a tracked environment.

The exported configuration factory58 is a real callable **adapter interface**, but after the OriginalDefault correction it is not invoked by the current initial-source023dd0 path. Directly calling it from a new test would create a tracked-input scenario, not prove that normal OriginalImage/Profile currently uses one. Host configuration still uses its deliberate factory/ledger policy and must remain unchanged. Native tracked bodies are now recovered evidence; adding unused host wrappers for them would not close another demonstrated production behavior gap.

A tracked-family implementation is reasonable if root explicitly selects support for those nondefault input environments or identifies an actual runtime family rewrite. It should then be a coherent family/pin stage, not disconnected leaf helpers or a false percentage increase for the default route.

## Current real consumers and boundaries

| Entry / source | Current selected behavior |
| --- | --- |
| Profile Slot source (`session-profile-source/profile.cpp:464-475`) | Host context/capability root, classifier OriginalDefault for OriginalApplication; no native tracked family selected. |
| Constructor023dd0 (`socket-source/environment_constructor.cpp:36-63`) | Initial type0a25c0 matches the empty branch and bypasses factory58. The nonempty tracked branch exists generically but no current original initial registry source selects it. |
| Message028d30 / response021420 (`message_dispatch.cpp:47-89`, `response_dispatch.cpp:65-107`) | Context-bearing initial copies; fresh native factory20=023400 sets bit0 and factory88=0234c0 clears bit1, preserving initial family. Host final response flags1 is already matched. |
| Initial executor factory58 binding | Control::factory currently creates a host ledger clone. This differs from concrete023580 for an explicit direct invocation, but such invocation is not reached on the selected default constructor route. |
| Current Host mode and direct configuration consumers | Deliberate host contract; source comments disclaim native class/ABI equivalence. Do not retrofit native retains into Host make/copy/destroy. |

## Minimal coherent representation if nondefault tracked support is chosen

Extend private Row metadata for OriginalDefault only with a Family (Initial/Tracked), DWORD flags, and explicit owned-work-pin state. Keep current stable external IoHandlerEnvironment addresses and globally unique capabilities/Origin/job_epoch. Query may remain a checked host capability resolving the exact row; it must not pretend to be native006180's raw pointer or cast original addresses. No external Row spans/deque framework is needed.

Tracked rows need a distinct schedule identity/type-query binding for native0a2690, separate from the initial0a25c0 classifier. The current single Control.token schedule cannot classify every family as empty. Native RTTI/name-equivalence limits must remain explicit; a distinct pointer is evidence of a different tag, not by itself complete arbitrary04d2d0 compatibility. Operations can dispatch by the checked private row family rather than adding an ABI overlay. All row/source/protected-source/growth proofs must include the added family/flags/pin state so metadata movement cannot lose ownership.

Concrete transitions from the fresh evidence:

| Operation | Initial family | Tracked family |
| --- | --- | --- |
| copy slot8 |0228c0 copies defined context/flags; no work retain |0228a0 copies and retains one work unit when context exists |
| move slot10 |0228c0 leaf copies; outer wrapper replaces source prefix |0231b0 transfers context/pin and clears source context; no extra retain |
| destroy slot0 |017a30 no native work release; host ledger still retires metadata |0229a0 releases exactly its owned work unit; old count1 calls02a190 |
| prepare40 |023520 clears flags bit0; remains initial, no retain |023310 clears bit0, retains and remains tracked |
| factory58 |023580 preserves flags, retains and returns tracked |023360 preserves flags, retains and remains tracked |
| response factory20 |023400 sets bit0, remains initial, no retain |023220 sets bit0, retains and remains tracked |
| response factory88 |0234c0 clears bit1, remains initial, no retain |0232c0 clears bit1, retains and remains tracked |

Final schedule.owned10 is022f90 into022e40 and borrowed18 is null for both evidenced families. A family-aware response publisher would use its actual row flags rather than constant1 for arbitrary inputs. The existing022e40/028ca0 algorithms already cover matching-TLS inline dispatch, forced post, ignored bit1 hint, one posted-operation work pin and failed-post queue ownership. Do not add a second posted pin in the environment layer. Tracked factory pins are separate from that posted pin, source owner strong refs and host ledger refs.

## Allocation, commit and failure order

Prepare metadata growth and a new capability outside the nonrecursive gate before any new retain; revalidate exact source/destination, same selected JobState/permit/protected source and current rows. Retain can then be one atomic DWORD operation followed by a no-allocation publication commit. Existing make's candidate/retired-vector cleanup and active-depth/reentry rules remain applicable, but family/flags must be current data, not cached stale source values. Existing private IDs may skip on failed unpublished attempts.

A tracked copy/factory must own one pin, a move must transfer that same ownership once, and destruction must never decrement twice. Last release may call an opaque stop/wake path, so it cannot execute while holding Control.gate or retaining a Row reference that an admitted callback could invalidate by growth. It needs a bounded per-row release/in-progress prefix or equivalent exact-cap reservation: record that the atomic release was consumed before a throwing stop effect, preserve that prefix on failure, and reject duplicate destroy/retype while the release is in flight. Do not guess a rollback increment or replay02a190 if an SDK callback fails. This is more than adding three unused arithmetic helpers.

## Existing stop effects and actor limitation

The actual stop algorithm exists in `socket-source/worker_run.cpp:5-11`: exchange stop34, exchange wake38, post the stop packet and translate an observed post error through its required effects. Runtime already uses it through a local RecoveredDispatcherWorkerEffects adapter (`server-runtime-source/runtime.cpp:322-333`), backed by Profile WorkerControl (`session-profile-source/profile.cpp:153-159`) and selected Policy exchanges/raise (`session-policy-source/policy.cpp:148-149` and raise).

However, current `rt::Ports` (`server-runtime-source/runtime.hpp:22-27`) exposes WorkerRunApi, DispatcherApi, IO environment and scheduler, **not** WorkerRunEffects/WorkerControl or a raw-worker channel. Ports.worker is Runtime::WorkerApi (`runtime.cpp:32-39`), whose post_stop routes through Runtime completion. While the worker runs, that completion points to AdmittedDispatcherApi. Its require method (`owned-server-source/server.cpp:31-35`) checks the original worker thread/fiber and held Parent Operation. Selected Policy::call also requires its original thread/fiber (`session-policy-source/policy.cpp:35-43`). Consequently a child job's last tracked release cannot safely call02a190 by simply reusing these wrappers. The raw configured Runtime Dependencies.worker is private behind Runtime; it is not ports.rawWorker.

Existing NativeJobs pins work from start through returned receipt (`owned-jobs-source/jobs.cpp:193,217`), so balanced temporary tracked retains inside that exact job often cannot be the final scheduler pin. That does not prove the general Environment::enter_job API can never observe old count1: its JobPermit/source-control authority does not promise a scheduler work pin. Do not silently suppress the last-release stop, restrict a formerly valid child scope, or declare that branch unreachable for every caller.

For a deliberately original-worker-only nondefault consumer, a small explicit stop-effects binding from Profile could reuse the existing algorithm. Full child last-release support additionally needs a narrow actor-appropriate stop publication/error channel with actual lifetime/admission proof, distinct from the original worker's live admitted dispatcher. That crosses Environment/Profile/Runtime or worker wiring, and requires testing both actors. It is not justified merely to claim unused tracked helper coverage, and no new unload/external-drain condition is recommended.

## Scope, cost and validation decision

If root selects the wider nondefault contract, expect a coordinated Environment family/flags/pin implementation, a small original/child stop-effect wiring change, and source tests across Environment/Profile/worker boundaries. It is not a single-body patch. Required source scenarios include explicit initial factory58 -> tracked, tracked copy/prepare/factory58, move without duplicate retain, both response flag transformations, tracked023dd0 constructor path, ordinary versus final destroy, selected child final-release behavior, and retained failure prefixes. Assert native-work pins independently of host refs and posted pins. Preserve all existing Host and default-empty tests.

For the currently selected OriginalImage default path, recommendation remains **NO PRODUCTION CHANGE in this design stage**. Close the real initial-response question with the11-record NO CHANGE proof, retain concrete tracked evidence as a future input-domain expansion, and pursue another demonstrated selected consumer instead of implementing unused helpers or enlarging stop/concurrency policy without need.
