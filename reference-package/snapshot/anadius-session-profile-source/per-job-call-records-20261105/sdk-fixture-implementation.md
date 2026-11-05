# Prepared Profile/Policy call-record source fixture

Authored 2026-11-05. **Source-only validation: not compiled, linked, or run.** No DLL, original binary, ZIP, production policy selection, or historical proof was changed by this author. The only authored source is `outputs/anadius-application-graph-source/graph_integration_tests.cpp`. Whitespace/diff checking produced no diagnostics.

## Actual composition and route

The new isolated `--prepared-profile-parallel` mode is also invoked by the existing default executable harness. All prior ordinary/70-message NativeJobs modes remain registered and retain their previous assertions. The shared `ConcreteProviders` fixture accepts an optional Dialog API, constructing one Dialog and the same existing Runtime/configuration/Online/AttemptState/provider owners once, before publication.

The new mode uses a controller-owned real DomainStorage/NativeThreads and PreparedMemory, a real original native worker, the existing actual Graph/Backend/Runtime/Profile, one Session/Logic, and real native child threads. `Dependencies.operation_policy=OriginalConcurrentActors` is passed to the SAME Backend's actual Parent constructor. No Parent or ServerView is substituted. All production callers still select the default Serialized policy. `OriginalApplication` selects the existing measured OriginalDefault Environment route; the adapter checks the actual Runtime's matching `environment_work_stop` capability. The real response/environment continuation is executed, not omitted or replaced with a fake capability.

The controlled adapter implements BOTH Jobs and PreparedJobs. It captures the actual prepublished Submitting job via `Application::prepare_job`, preserves exact token/session/closure identities, launches the recovered native wrapper, acquires the real JobHandoff, calls its actual `process_message` and `return_memory`, and posts the real prepared receipt to the actual IOCP. Each actual delegated launch introduces one legitimate scheduler work pin, following NativeJobs' source protocol; receipt handling invokes `Application::complete_job`. The actual dispatcher owns the corresponding decrement. There is no synthetic receipt or manually restored counter.

A receives a large owned UnknownParallel request and reaches the concrete Licensing/Core/Dialog diagnostic. Its actual dialog is held. The first `start_prepared` waits for this entered diagnostic before returning. B uses the same actual Session/Logic and a distinct large GetConfig request/response. The second launch waits for B's actual body AND memory return before `start_prepared` returns and before detach. Stable source row pointers survive the initial job capacity 1→2 growth; the test checks distinct current tokens and two live source jobs.

The original worker consumes B's actual receipt and response continuation. Its normal SDK send completes while A remains held. Root snapshots, the same session owner identity, and the exact live Parent-backed A request remain accessible. At the real outside-Parent maintenance seam, B's actual native handle/FLS exit is joined, its JobHandoff is retired, and Parent.reap_jobs runs. The controller-owned PreparedMemory observer must then report exactly ONE outstanding job (A), after both Policy probe rows and B have been reaped. Only after B response, physical exit, and this reap proof does the fixture release A. A's supported unknown-message control route produces no fabricated XML response. The final response bytes contain exactly B's single NUL-framed large response.

## Actual Policy overlap

Before frame delivery, two additional real PreparedMemory child offers exercise the SAME Policy and Parent. Each child locks/registers its own actual WaitTarget and calls `cleanup_job_exit_waits` with its real permit. A is held in its nonmutating checkpoint; B's actual cleanup and native exit finish first. While A's target is still locked/registered, the original worker calls the ordinary `cleanup_exit_waits`, and reads actual Policy passives. A's foreign target must remain untouched. The test then releases/joins A and defers physical prepared-row reaping to the outside-Parent Runtime seam. Seven final cleanups are required: two frame-child calls, two original continuations, two probe-child calls, and one ordinary original probe call. No cleanup counter is set/reset by the fixture.

Normal termination uses the existing actual final pending accept's typed packet, recorded cancellation intent and software status 995 after sessions/jobs drain. Source stop, zero wake, cache cleanup, Profile/Policy/Graph retirement, prepared return, native/FLS joins, provider leases and Domain closure remain actual existing paths. The adapter's close/closed-maintenance operations are idempotent. Finite native waits fail the isolated process; an unknown child prefix is not converted into normal cleanup.

## Limits

These assertions are authored and statically traced, **not runtime results**. The new adapter intentionally does not impose production NativeJobs' remaining body gate/detach barrier; therefore this is not evidence that production NativeJobs or the production assembly now permits parallel execution. It is a bounded same-session two-message Profile/Policy composition, not arbitrary concurrent cipher/configuration/licensing mutation, tracked-family Environment concurrency, network/game SDK thread-safety, or original binary ABI proof. SDK receive/send and dialog are controlled boundaries; actual source parsing/033d70/032d00, concrete providers, Environment continuation, native thread/FLS ownership and IOCP receipts remain in the route. Existing 70-message production NativeJobs scenarios remain separate source cases.

## Frozen bytes

| File | SHA-256 |
| --- | --- |
| Graph integration before | `13094a04164a2890cafbfa9637ed38e1b801c2b7c15daf3a4326af211d0a99d3` |
| Graph integration final | `b80e0d82a7ee1c2a09f2ecd588148f7891d04936b9b7dd0f702ed757e1088354` |
| Profile header | `ede20dcc14bcfb1d4859f91b2f9222051c43ad7044947badc853133c65d49c5a` |
| Profile source | `55b0964eb026adea5fb7a8acf793fdbd818b6a12a6c65c65f497503dac6cc69b` |
| Policy header | `720a15b840365f5e87fe3cc7e0ae4e98b88df195dd5d1f078aea723eff854aea` |
| Policy source | `6338a6ecb4d7c3d736057da418a49fe18debb300ff58131ff85468248554be23` |
| Server header | `eca6cb891d6561312320431556c48857c55ddd946381e3619f7180a2589f5cff` |
| Server source | `9e96247b84ff6396809c4f5d2bfda12b887b564fa8ea3437ac010a69878b4c9b` |

The six production hashes identify the coupled API reviewed for this fixture; those production files were authored by other agents/root. This report and the final test source are frozen at publication. Subsequent findings require a separate successor or addendum.
