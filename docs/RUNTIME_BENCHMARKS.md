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
| Provider/bootstrap and hooks | Existing startup, rollback, quiescence and SAFE-FREEZE diagnostics | Network service start/stop is measured here; provider and full DLL startup require a separate fixture |

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
Deliberately corrupt component/Application responses and an Application XML
file modified after shutdown must fail as three mandatory negative controls.

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
sizes x 5 scheduling candidates = **1,200 measured child processes**, plus three
negative controls. Each selected four-workload suite has 600 measurements.
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
input/source pins and CMake compiler settings. Partial samples are preserved
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

The extended receipt schemas are `citrine.runtime-benchmark-inputs.v2` and
`citrine.runtime-load.v2`. Older component-only artifacts remain evidence for
their original source head and four workloads; they do not validate Application
coverage or provide a cross-head paired comparison.

This suite does not prove donor equivalence, real Consumer/game performance,
production command frequencies, provider/hook contention, adversarial saturation,
long-duration leak freedom or power-loss atomicity. At most 32 sessions each
have one active handler; the 128-job queue is not saturated. Application lock
and encrypted LSX costs are measured for the declared fixtures only. Existing correctness gates
cover different failure contracts and remain required. Extend measured cases
when actual traces identify those workloads; do not replace missing coverage
with a universal performance claim.
