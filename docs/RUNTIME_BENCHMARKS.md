# Runtime load measurements

This opt-in diagnostic measures the actual AsioService/Session implementation,
production XML reader/printer and native atomic-file backend. It does not load
the application DLL or the original donor DLL. Production keeps two handler
workers, a 128-job queue and the existing session/deadline policy.

## Existing coverage and optimization questions

| Area | Existing evidence | Measurement or next decision |
| --- | --- | --- |
| XML parser, reference decoding, allocations and storage compatibility | [XML backend comparison](XML_BACKEND_COMPARISON.md) already has paired timing/allocation measurements | Retain that suite; the new TCP workloads measure scheduling around the current parser |
| Worker pool and per-session FIFO | Native `handler_pool_contracts` tests bounded workers, overload, order, failure isolation and shutdown | Compare 1/2/4/8 workers and a per-message-thread control with 1/8/32 simultaneous connections |
| Shared handler state | Application serializes registered handlers, while parsing/cipher work occurs outside its lock | Use a deliberately serialized sensitivity control; profile actual Application lock duration before changing lock ownership |
| Atomic achievement persistence | Existing source/native rollback and retry tests | Measure real two-file replacement of existing files, including staging, backup and flush costs |
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
only, without confidence intervals. A cross build verifies compilation;
Wine or Linux execution is not accepted as native performance evidence.
Source archives can run without Git, identified by their whole-source manifest;
Git checkouts require committed clean inputs. All generated files stay under
`build/`; no release, upload or application configuration is modified.

## Workloads and controls

Each native child process starts the real loopback service and warms every
connection with four verified responses. Client threads wait at a common gate
before timing. Every reply must exactly match its complete request, in order;
measured request counts, errors and worker bounds are checked before timings
are accepted. A deliberately corrupt response must fail as a negative control.

| Workload | Measured work | Boundary |
| --- | --- | --- |
| `echo` | TCP framing, queueing, Session send/receive and exact echo | Minimal custom Handler, not a production LSX command |
| `xml` | Same transport plus 16 actual XML parse/view/printer operations per request | Fixed 1 KiB generated payload; controlled CPU sensitivity |
| `xml_locked` | Same XML work under a shared mutex | Deliberately serialized upper-bound control; production does **not** hold its handler-state lock around this XML parsing |
| `atomic_pair` | XML work plus two actual native atomic file replacements | Shared NativeFactory and per-connection destinations, existing files warmed first; not full GrantAchievement routing |

Full runs contain 5 rounds x 4 workloads x 3 connection counts x 2 pipeline
sizes x 5 scheduling candidates = **600 measured child processes**, plus one
response-corruption control. Each connection sends 256 measured requests,
or 16 for the flush-heavy atomic workload. Pipeline 1 is closed loop; pipeline
8 sends a batch and records each reply's elapsed time from that batch send.
Burst latency therefore includes time behind earlier requests in that batch.
Cases and candidate order are shuffled with a fixed seed each round. Each
candidate sees identical inputs and sample counts for its paired case.

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
- Throughput: completed requests divided by the common measured interval.
- Message-thread creations and observed thread identities: exact creation
  count for the control versus zero per-message creations for pool profiles.
  Windows can recycle thread IDs, so unique observed IDs are not creation counts.
- CPU time/cycles: **whole child process**, including clients, server and helper
  threads. GetProcessTimes can quantize short runs to zero; cycles are also
  retained, not converted to wall time or compared across different CPUs.
- Peak working set: process lifetime peak, including setup/warmup. Private bytes
  are sampled after replies complete with service/peers still alive.
- Start/stop: service bind/thread start and quiescent service shutdown; not full
  DLL/provider startup, active-handler cancellation or game startup.

Summary percentiles are medians of per-round nearest-rank percentiles, not a
pooled percentile. Ratios compare paired rounds with two workers. Throughput
above 1 is faster; p99 below 1 is lower latency. Bootstrap intervals resample
paired rounds and describe this host/run only. Five rounds give limited
uncertainty evidence; repeat on target hardware before selecting a default.
`rounds_under_100ms` flags short timing windows; low-sample p99 and short echo
runs cannot establish tail-latency or CPU rankings. Neither longer queues nor
more workers are declared winners automatically.

This suite does not prove donor equivalence, real Consumer/game performance,
full Application lock contention, encrypted LSX cost, adversarial saturation,
long-duration leak freedom or power-loss atomicity. Existing correctness gates
cover different failure contracts and remain required. Extend measured cases
when actual traces identify those workloads; do not replace missing coverage
with a universal performance claim.
