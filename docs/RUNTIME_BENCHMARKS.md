# Runtime load measurements

This opt-in diagnostic measures the actual AsioService/Session implementation,
production XML reader/printer and native atomic-file backend. It also runs the
real Application, MessageHandler and registered command router through TCP,
with plaintext and negotiated encrypted LSX. Application uses isolated fixture
configuration/data paths and `install_hooks=false`; it does not load the
application DLL, install hooks or load the original donor DLL. Production keeps
two handler workers, a 128-job queue and the existing session/deadline policy.

## Existing coverage and optimization questions

| Area | Existing evidence | Measurement or next decision |
| --- | --- | --- |
| XML parser, reference decoding, allocations and storage compatibility | [XML backend comparison](XML_BACKEND_COMPARISON.md) already has paired timing/allocation measurements | Retain that suite; the new TCP workloads measure scheduling around the current parser |
| Worker pool and per-session FIFO | Native `handler_pool_contracts` tests bounded workers, overload, order, failure isolation and shutdown | Compare 1/2/4/8 workers and a per-message-thread control with 1/8/32 simultaneous connections |
| Shared handler state | Application serializes registered handlers, while parsing/cipher work occurs outside its lock | Measure actual handler-state lock wait/hold separately from queueing and complete handler duration; retain the deliberately serialized component control |
| Atomic achievement persistence | Existing source/native rollback and retry tests | Measure both component two-file replacement and real GrantAchievement routing; check every progress response and final XML/INI against the stopped Application state |
| Connection framing and output | Existing partial-send, receive, timer and event-receipt correctness tests | Compare closed-loop requests with bursts of eight; investigate framing/allocation/Nagle effects if measured delays dominate |
| Provider/bootstrap and hooks | Existing startup, rollback, quiescence and SAFE-FREEZE diagnostics | Measure hook-disabled offline Application startup; live provider, hooks and full DLL startup remain separate fixtures |

These are investigation candidates, not findings that a production optimization
is already safe. More workers cannot remove serialized persistence or shared
state. Queue size, lock granularity, batching, allocation reuse and socket
options must retain existing ownership, ordering and durability contracts.
There is no automatic change to any production default.

## Run the focused Windows measurement

The existing manual performance workflow selects one suite. Its default remains
the XML comparison; `runtime-load` selects only the new native Windows job:

```sh
gh workflow run xml-backend-comparison.yml --ref <reviewed-branch> -f suite=runtime-load
```

This is a manual component diagnostic, not a full-validation or merge gate.
It adds no push, PR, schedule or initial-label execution. Source QA and the
existing eleven required checks keep their normal synchronization policy.

Local native Windows commands, from a committed clean source checkout:

```sh
cmake -S tests/diagnostics/runtime_load -B build/runtime-load/binaries -G "Visual Studio 17 2022" -A x64
cmake --build build/runtime-load/binaries --config Release --parallel 2
python -B tests/diagnostics/runtime_load/run.py --build-dir build/runtime-load/binaries --output build/runtime-load/results --repeats 5
```

Use a new output directory per run. `--quick` runs correctness smoke coverage
only, without confidence intervals. The default `--suite all` runs both component
and Application workloads; `--suite application` or `--suite component` selects
one complete paired matrix. The hosted runtime job runs `all` explicitly.
A cross build verifies compilation;
Wine or Linux execution is not accepted as native performance evidence.
Source archives can run without Git, identified by their whole-source manifest;
Git checkouts require committed clean inputs. All generated files stay under
`build/`; no release, upload or application configuration is modified.

## Workloads and controls

Each native child process starts the real loopback service and warms every
connection with four verified responses. Client threads wait at a common gate
before timing. Every reply must exactly match its complete request, in order;
measured request counts, errors and worker bounds are checked before timings
are accepted. Component replies must equal the complete request. Application
replies are parsed and compared against independently specified complete XML
trees, including response IDs, sender, command, all attributes and children.
The fixture's eight content IDs/values are checked independently; expected
response order follows their loaded immutable configuration order. Extra,
missing, corrupt or out-of-order responses fail. Timestamp syntax is checked
for grant responses; persisted dates/progress are checked against stopped state.
Deliberately corrupt component/Application responses and Application XML and
INI files modified separately after shutdown must fail as four mandatory
negative controls.

| Workload | Measured work | Boundary |
| --- | --- | --- |
| `echo` | TCP framing, queueing, Session send/receive and exact echo | Minimal custom Handler, not a production LSX command |
| `xml` | Same transport plus 16 actual XML parse/view/printer operations per request | Fixed 1 KiB generated payload; controlled CPU sensitivity |
| `xml_locked` | Same XML work under a shared mutex | Deliberately serialized upper-bound control; production does **not** hold its handler-state lock around this XML parsing |
| `atomic_pair` | XML work plus two actual native atomic file replacements | Shared NativeFactory and per-connection destinations, existing files warmed first; not full GrantAchievement routing |
| `app_profile` | Real Application GetProfile over plaintext LSX | Configured profile with complete response checks; no synthetic 1 KiB padding or repeated parsing loop |
| `app_profile_encrypted` | Same command through actual hex/AES/session negotiation | Alternating fixed-key version 2 and derived-key version 3 peers; includes server decryption/encryption and client codec cost |
| `app_mixed_encrypted` | Repeating GetProfile, GetSettings, QueryContent and QueryAchievements | Deterministic fixture command mix, eight content items and one achievement per peer; not a captured game trace |
| `app_achievement_encrypted` | Real GrantAchievement with progress increment 1 on a peer-owned achievement | Shared Application locks, XML/INI serialization and native atomic replacement on every request; final acknowledged progress verified in both files |

Full `all` runs contain 5 rounds x 8 workloads x 3 connection counts x 2 pipeline
sizes x 5 scheduling candidates = **1,200 measured child processes**, plus four
negative controls for `all`. Each selected four-workload suite has 600 measurements.
Each connection sends 256 measured requests, or 16 for `atomic_pair` and
`app_achievement_encrypted`. Pipeline 1 is closed loop; pipeline
8 sends a batch and records each reply's elapsed time from that batch send.
Burst latency therefore includes time behind earlier requests in that batch.
Cases and candidate order are shuffled with a fixed seed each round. Each
candidate sees identical inputs and sample counts for its paired case.
Application's `payload_bytes=0` means command-sized LSX, not a padded component
payload. The Application mix and achievement count depend on the selected
connection count, identically for all five candidates in that paired cell.

Application starts once per child with hooks disabled and no online provider
or extra DLL configured. A dedicated encrypted peer verifies and drains the
initial shared DOWN event before warming measured peers. Each peer then warms
four requests. Grants use distinct IDs and a total of 1,000,000, so each request
persists a new progress value without completing the achievement or enqueueing
a completion event. This bounds the workload; completion-event delivery remains
covered by correctness tests rather than being silently discarded here.
After quiescent stop, every achievement must have exactly `4 + measured requests`
progress, zero completion count and a valid grant timestamp. The complete XML
and watcher INI must match that state; a fast response without persistence fails.

Generated Asio copies insert the same queue clock/function wrapper and
compile-time control branches in every candidate. Pool profiles change only
the worker-array count. The `thread-per-message` profile creates a fresh thread
for each message, releases its handle immediately, and retains in-flight
function/Client ownership until an explicit drain completes. It preserves the
same FIFO, TCP, XML and storage paths, isolating scheduling cost. Scheduler
contracts check completion, exceptions, capture lifetime and idempotent stop.
It is a controlled alternative, not original Anadius DLL execution or a claim
that every donor failure path is identical. The JSON `workers=0` sentinel
identifies that dynamic control; use the explicit `profile` field in comparisons.
Instrumentation has overhead, especially for echo. Original source/vendor bytes
and the shipping build graph remain unchanged. Generated-copy hashes,
source-manifest identity and executable hashes are retained.
Generation recognizes the current private OwnedThread selector and requires
its ordinary std::thread alias unchanged. Missing, duplicated or changed alias
and worker/queue anchors fail generation. The selector remains in every copy;
only the diagnostic worker count and declared instrumentation/control insertions
change. Reversing these insertions restores the complete current source.
The generated Application copy adds only prepare-response observation, actual
handler-state lock timing and the response corruption control. It retains lock
ownership and statement order; parsing and cipher work stay outside that lock.
Source-anchor drift fails generation. Before execution the runner recomputes
both Application/Asio transformations and verifies every generated copy against
its receipt. The standalone Application target reuses the accepted explicit
application/VDF source list and pinned dependency build recipes, omitting only
the original two files replaced by measured copies. No root DLL target changes.

Generation preflights the complete output set and rejects linked/nonregular
destinations before writing. Copies are staged exclusively and replaced, so a
borrowed hardlinked inode cannot be truncated through a generated filename.

## Results and interpretation

The artifact contains `results.json`, `summary.csv`, raw per-request samples,
the ten selected executables, their post-link build receipts, input/source pins,
an offline audit report and CMake compiler settings. Partial samples are preserved
with a FAIL receipt if a child or measurement check fails. No timing result
alone is a pass/fail threshold. Results retain source identity and CPU count,
without adding machine names, owner paths or regional timezone information.

- RTT p50/p95/p99: exact request/reply latency at the client.
- Queue p95: enqueue wrapper to worker entry, including uniform instrumentation.
- Handler p95: handler entry through response preparation; separate from send.
- Application lock-wait/lock-hold p95: waiting for and executing under the actual
  shared handler-state mutex. Every measured command must have exactly one
  observation. Each wait+hold interval must fit inside its handler duration.
  Component cells use null for these summary fields. Raw handler/queue/lock
  arrays share record indices, while client RTT samples are a separate sequence.
- Throughput: completed requests divided by the common measured interval.
- Message-thread creations and observed thread identities: exact creation
  count for the control versus zero per-message creations for pool profiles.
  Windows can recycle thread IDs, so unique observed IDs are not creation counts.
- CPU time/cycles: **whole child process**, including clients, server and helper
  threads. GetProcessTimes can quantize short runs to zero; cycles are also
  retained, not converted to wall time or compared across different CPUs.
- Peak working set: process lifetime peak, including setup/warmup. Private bytes
  are sampled after replies complete with service/peers still alive.
- Start/stop: service bind/thread start for component cells; full hook-disabled
  Application start/stop for Application cells, including its actual offline
  provider/extra-library phases. It excludes hooks, DLL entry/loader work,
  active-handler cancellation and game startup.

Summary percentiles are medians of per-round nearest-rank percentiles, not a
pooled percentile. Ratios compare paired rounds with two workers. Throughput
above 1 is faster; p99 below 1 is lower latency. Bootstrap intervals resample
paired rounds and describe this host/run only. Five rounds give limited
uncertainty evidence; repeat on target hardware before selecting a default.
`rounds_under_100ms` flags short timing windows; low-sample p99 and short echo
runs cannot establish tail-latency or CPU rankings. Neither longer queues nor
more workers are declared winners automatically.
Application throughput includes request construction, client encryption,
decryption and complete response validation. RTT starts after the request batch
has been encoded and ends after response decryption, before XML validation;
later replies in the batch can include time spent validating earlier replies.
These client costs and common probe overhead are part of this diagnostic.
Do not subtract independent p95 values to estimate missing processing stages.

The current receipt schemas are `citrine.runtime-benchmark-inputs.v2`,
`citrine.runtime-benchmark-build.v1` and `citrine.runtime-load.v3`. Earlier
artifacts retain their original source head/schema and coverage. The strict
v3 auditor does not silently upgrade older results or borrow their PASS.

This suite does not prove donor equivalence, real Consumer/game performance,
production command frequencies, provider/hook contention, adversarial saturation,
long-duration leak freedom or power-loss atomicity. At most 32 sessions each
have one active handler; the 128-job queue is not saturated. Application lock
and encrypted LSX costs are measured for the declared fixtures only. Existing correctness gates
cover different failure contracts and remain required. Extend measured cases
when actual traces identify those workloads; do not replace missing coverage
with a universal performance claim.

## Reproduce and audit the proof

Successful linking invokes `run.py --record-build` from the standalone CMake
target's POST_BUILD command. Each sidecar binds the exact executable SHA-256,
candidate kind/worker count, Release/Debug configuration, compiler ID/version,
C++20 and prepared source/generated-copy identities. Measurement requires the
selected Release sidecars, source identities and actual executable bytes to
match before any child runs. Reconfiguring source inputs while retaining an
older executable cannot give that executable a new source identity. Builds
and archives require their own receipts; none are inferred from file dates.

`identity()` accepts a Git commit only from a Git marker at the selected source
root, verifies the Git top level and rejects a dirty checkout. Inherited Git
repository selectors cannot redirect these local queries. A source archive
nested beneath another checkout records a null commit and its own manifest
identity rather than adopting its parent's HEAD.
The disposable Git fixture also clears repository-local Git environment before
creating its temporary repository, so pre-push cannot redirect fixture setup
into the source checkout.

After native measurement, the workflow runs the committed offline auditor:

```sh
python -B tests/diagnostics/runtime_load/audit.py \
  --results build/runtime-load/results --build-dir build/runtime-load/binaries
```

Download the complete `native-runtime-load-evidence` artifact and check out its
identified source commit. Anyone with that checkout and Python can repeat the
audit without Windows, network access or running the executable:

```sh
python -B tests/diagnostics/runtime_load/audit.py \
  --artifact /path/to/native-runtime-load-evidence.zip \
  --artifact-sha256 <digest-from-the-GitHub-artifact-metadata>
```

The auditor verifies source/vendor/generated-copy identities, all selected
executable bytes and link receipts, required corruption controls, canonical
workload recipes and complete candidate/case/round coverage. It verifies each
raw sample hash and its agreement with the compact receipt, then recomputes
every summary field, paired interval and CSV cell. A random per-run identity
and executable hash on each raw row reject accidentally spliced runs/binaries;
this identifier contains no machine identity. Each Application sample must
record exactly one handler-lock acquisition; equal totals cannot hide a missing
observation in one request and a duplicate in another. Duplicate JSON keys,
nonfinite numbers, missing rows, stale binaries, changed summaries and unsafe
ZIP/member paths are rejected. ZIPs are read without extraction, with bounded
member count and decoded size. The ordinary five-round matrix has 1,200
children, 3,214,400 measured request/replies and 240 summary cells; the quick
matrix has one round and is explicitly smoke-only.

`PASS_INTEGRITY_NOT_EXECUTION_ATTESTATION` means internal/source consistency.
It is deliberately distinct from native execution. Verify the GitHub run's
head SHA, successful native build/CTest/measurement/audit steps and independently
supplied artifact digest before citing native results. Local hashes and
self-reported JSON cannot authenticate a malicious builder, prove compiler
reproducibility or establish that invented samples were executed. A fresh clean
CI build supplies execution provenance; no signing or release permission is
introduced here. Source archives with null Git commits must be identified by
their exact source-manifest digest, not an inferred repository commit.

## Questions, answers and decision criteria

| Question | Answer and evidence | Decision boundary |
| --- | --- | --- |
| Does merging this PR make the DLL faster? | No production tuning is included. `git diff main -- src include third_party CMakeLists.txt` must be empty; the standalone targets and source manifests make this reviewable. | Merge value is a repeatable way to reject bad tuning and investigate costs. It has value only if maintained and used for later decisions. |
| Is this a real runtime path? | Application cases construct the actual hook-disabled Application, negotiate actual LSX and execute Session/MessageHandler/router and native persistence. The component and Application source lists are explicit in the diagnostic CMake file. | No DLL entry, installed hooks, online provider or game is exercised by this benchmark. |
| Is the comparison fair? | Within one case/round the five candidates use identical fixture commands, connections, pipeline and request counts; the full matrix and unique candidate order are audited. Cases/candidates are shuffled with seed 48731. | Comparisons across workloads, heads or machines are not paired evidence. Client/probe costs remain in the process. |
| Is thread-per-message the old donor implementation? | It is a controlled source alternative with FIFO and explicit in-flight ownership/drain. Exact creations are counted independently of reusable thread IDs. | It proves this scheduling comparison, not donor timing or failure equivalence. |
| Can an old EXE or changed report pass? | Linked build receipts and the offline binary/raw/CSV/JSON audit reject stale or inconsistent evidence. Infrastructure mutations exercise these refusals. | Trust in the CI execution itself remains separate from consistency hashes. |
| Could fast responses conceal wrong routing or missing writes? | Complete response trees/IDs/order and every acknowledged grant progress are checked. Final XML and INI must equal stopped state. Four distinct corruption controls must fail. | This does not prove power-loss atomicity, physical media guarantees or every rollback path. |
| Does every request really measure the lock? | Raw per-request `lock_calls` must be 1, total calls must equal requests, and wait+hold must fit the same record's handler duration. | RTT arrays have independent client ordering; do not add/subtract unrelated percentiles to invent stage costs. |
| Is the Application parsing under that mutex? | The generated copy times the existing registered-handler lock only; XML/cipher work stays in its original per-session location. A reversible-generation regression checks retained statements/lock ownership. | The deliberately locked XML component control cannot prove that production's lock should be removed. |
| Do more workers always help? | Compare all connection/pipeline cells and both throughput and tail latency. A paired interval spanning one supplies no clear directional evidence for that cell. | Shorter queues alone do not establish better complete responses. Preserve the existing two-worker default until separately reviewed target evidence justifies change. |
| Are p99 and CPU values strong evidence in every cell? | `samples_per_round`, `p99_rank_per_round`, `samples_at_or_above_p99_rank`, duration, short-round and zero-CPU-tick counts expose their limits. With 16 samples, nearest-rank p99 is the maximum observation. | A low-count maximum or quantized zero is not a reliable population-tail or CPU ranking. Five rounds do not capture between-host variance. |
| Is the instrumentation overhead zero? | It adds clocks, wrappers and counters uniformly. Costs are declared; no uninstrumented calibration is present. | Ratios describe these measured copies. They are not an absolute production-throughput estimate or a proof that tiny differences exceed probe effects. |
| Are deadlock, overload and active shutdown proven? | Quiet benchmark retirement is checked here. Native [handler-pool contracts](../tests/native/handler_pool_tests.cpp) separately exercise bounded admission, overload, FIFO, cancellation, active-worker drain and queue discard; their current-head native result is required independently. | Neither a quiet drain nor a finite set of tested schedules is an all-schedule deadlock proof. Hook SAFE-FREEZE remains its own diagnostic. |
| Does this prove real game/FPS performance or a safe library migration? | Those workloads and hooks are outside this fixture. Existing Native/Golden gates check different contracts. | Obtain identified game/Consumer traces and target-hardware measurements before making such claims. This PR does not change vendors, ABI, CRT or language standard. |
| What are the costs and extra dependencies? | Python standard library plus the already pinned C++ dependencies; no new vendor/service dependency. Full native measurement and audit are opt-in/manual; normal PR tests run the inexpensive infrastructure regressions. | Maintain the small source transformations when their anchors change; manual runs consume runner time and artifacts expire after the configured 14 days. |

For a proposed optimization, retain a current-head correctness/Native/Golden
baseline, define the actual target workload and hardware, then run the same
paired case matrix for both implementations with source/build receipts. Compare
throughput, p99, CPU, memory, errors and shutdown separately; preserve durability,
FIFO, rollback and callback ownership. Require a repeatable improvement relevant
to the target, including independent runs/hosts where needed. A neutral result
can justify retaining the current design. A fixture result cannot close the
game, provider, hook or all-schedule proof boundaries above.
