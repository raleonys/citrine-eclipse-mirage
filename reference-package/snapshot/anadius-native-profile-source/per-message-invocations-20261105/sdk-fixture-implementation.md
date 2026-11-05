# Same-session native invocation fixture — authored source only

The only authored source is `outputs/anadius-worker-handoff-source/handoff_tests.cpp`. Before SHA256: `714af17f024cd4143874e969e2f27bf75e6d00142adbc3584758ae26cf8ee94d`. Candidate SHA256: `3140116c5847c8f6529063b41d5638308eb6458b0449aa3306a9b1615eec5f33`. The before copy is retained in this stage's `before/anadius-worker-handoff-source__handoff_tests.cpp`.

## Actual selected source route

`parallel_native_invocations()` (line 564) reuses the existing actual Domain/NativeThreads/PTD/FLS, PreparedMemory, JobMemory, Parent Operation/JobPermit, Task and finite-event fixtures. It explicitly selects `OriginalConcurrentActors` on this test Parent; it does not enable the production Graph policy. A single source-constructed SessionPair, plaintext SessionCipher and NativeProfile Logic serve both real children.

The original prepares two large, separately owned XML requests and relinquishes both whole-ticket views before either child becomes eligible. Its original Operation stays alive through B's return and response consumption. A runs `process_native_call` with its own dependencies and actual XML backend, selects a supported UnknownMessage, and blocks inside that invocation's diagnostic callback. B must complete before A is released: its distinct explicit dependencies use the actual XML reader, recovered 033d70/032d00 path, dispatch Router and GetConfig handler 02e5f0. A 6,002-byte request id is naturally echoed into B's real response; no printer padding or fabricated business answer creates the large allocation.

B's request is released in its child Operation. B relinquishes its heap response, ends its Operation and returns its actual JobMemory. The original reads and physically releases that response while A still holds its diagnostic/request and the original Operation remains active. B's actual native/FLS exit is joined and reaped before A is released. A then completes its supported diagnostic, releases its own request, returns its offer and is physically joined/reaped. Final source control/proxy disposal, Parent allocations and real prepared/provider/heap ledgers drain through the established fixture.

## Assertions and provider separation

The constructor's default dependency set traps every attempted use. Per-call XML/effect providers are distinct views over one shared business/configuration root. B invokes the actual Router against those same roots; unused license/content/event/crypto leaves throw rather than manufacture successful values. Plaintext and show_lsx=0 keep those leaves unselected.

While A remains held, B checks `active_calls()==1`, healthy `failed_prefix()==false`, and `retained_prefix()==true`; B cannot clear A's active record or mistake healthy activity for failure. B's large framed response includes its own id/GetConfigResponse and excludes A's id. Original observes exactly one remaining Parent buffer after freeing B's request/response. Final provider counts require A parse=1, printer=0, invalid=1, diagnostic=1, handler=0; B parse=1, printer=1, handler=1, invalid=0, diagnostic=0. Default/unselected call count is zero. After A returns, active/retained/failed observations are all clear. Default main calls the new fixture at line 776; existing cases are preserved.

## Static composition check and limits

Current `outputs/source-workspace/build.ps1` already registers this test with `anadius_core_partial.a` (line 509), and lists NativeProfile (157), handlers/crypto/XML reader/tinyxml2 (280–284), content/misc handlers and Router (297, 312–313). Its archive construction uses those units. This is source inspection, not link verification; the build script was not modified or run. Typed signatures/aggregate construction were checked against the current headers. The before/current whitespace diff emitted no whitespace diagnostic.

No compiler, project test, executable, reconstructed DLL, linker or ZIP operation ran. The fixture provides authored lower-source coverage of two actual prepared child actors sharing one Session/Logic with explicit providers, not an executed concurrency result. It does not exercise the full Graph, actual kernel network IO, native jobs, a game SDK, encrypted/shared cipher mutation or arbitrary shared business/provider thread safety. Production Original admission remains unchanged by this test edit. Native evidence collected by the root is separate from this authored source validation.
