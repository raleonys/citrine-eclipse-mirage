# Original dispatch observation correction — 2026-11-03

TEST_SOURCE_ONLY_NOT_COMPILED_OR_EXECUTED. No production source changed here.

The preceding original-dispatch scenario incorrectly reused the cooperative
fixture's terminal995 retirement expectations. completion_dispatcher.cpp forwards
the real scheduler owner for a physical packet. The cooperative fixture's custom
select_owner then replaces it with null to obtain its terminal cleanup path.
OriginalApplication deliberately preserves that actual owner instead. The error
path in accept_session_025720 logs and rearms accept; error995 alone is not a
source server stop or a successful retirement. This is a source-test assumption
error, not evidence that production needs an extra stop/unload protocol.

The scenario now runs as an isolated child (expected observation exit825). It
uses actual Domain/NativeThreads and the existing controlled IOCP/socket boundary.
At the first physical dispatcher get, it checks Serving, both startup milestones
still absent, actual published port/readiness, armed accept, live PTD and retained
IO/source owners. It then exits that child intentionally before delivering a
completion. It does not claim normal return, FLS teardown, socket cleanup or safe
unload. The child has NOT been launched. No observed exit825 or PASS is claimed.

The bounded production-source review followed this order:

| Stage | Source condition / action |
| --- | --- |
| Policy original retirement | Observed worker return and actual local IO/work checks precede selected Graph drain. FALSE retained, TRUE rechecked. |
| Graph::Impl::drain | Exact backend/Parent checks, actual app reap, live logic/job checks, queued string release/prune, real Parent lease release. |
| Profile retirement | Actual jobs/environment/deques close before Policy closes its lease/ports. |
| Runtime retirement | Actual remaining owners checked, accept closed, application/flow graph released. |
| Graph server wrapper | Allows final Closed only from actual Drained or known unstarted Installed. |

No further mandatory cooperative proof was found in this bounded path. This is
manual source reasoning, not executed evidence of a complete session. The large
queued-event original-profile retirement integration case remains uncovered;
the prior note promising that extension did not result in such a test. Existing
cooperative large-event cases and original unstarted installation cases remain
source scenarios, with no rerun claim. Do not weaken production owner selection
to make a fixture terminate.

Both loader helpers remain source-defined. Current pending count stays55; only
worker_tests.cpp changed in this stage. Build/link/test execution and packaging
were not requested or performed. Frozen build/link/receipt/ZIP remain unchanged.
The current source milestone requires an explicitly requested consistent rebuild
for its next meaningful validation; further hypothetical lifecycle hardening is
not a substitute for that result.
