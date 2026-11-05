# Next provider call-view plan — read-only design

No source changed, no REA session opened, and no compiler/test/link/DLL/package operation ran. This is a new unreferenced planning artifact after the NativeProfile stage froze; it does not alter that stage's recorded evidence.

## Concrete selected constraints

Information's `Impl` owns one `job`, `busy`, `checking`, `binding`, `checkpoint` (effects.cpp14–43); child entry reserves that singleton at61–68. Content has the same controls plus one `GuardedFiles`, Clock/Formatter/Encoding/Directories adapters and Persistence `Engine` (effects.cpp22–79); `with_job` still calls the original facade and installs one global synchronous descriptor (101–115). Licensing repeats this in both Handler and Core, including reciprocal `handler_context/handler_check` and `admission_context/admission_check` fields. Graph's persistent adapters install those singleton scopes (graph.cpp237–325), so changing only Graph's JobCall or removing its body gate is insufficient.

The actual mutable ownership requiring separation is **per-message execution state**, not Runtime, configuration, Online, Paths, AttemptState or the SDK/file/clock providers. Existing shared-log leases and actual Graph event/achievement mutexes remain the same native shared identities. They must not become per-message locks or be replaced by a whole-handler provider mutex. In particular Content's unknown-field logger releases its log lease before its dialog (effects.cpp66–69), and Licensing's supported unknown diagnostic must do likewise; an independent message can progress while that dialog is held.

## Proposed additive public seam

Use named per-call facades, preserving every existing HOST method/with_job/checkpoint contract. Suggested spelling, subject to root API decision:

```cpp
// One small shared typed header, e.g. application-io-source/call.hpp.
// Constructor performs no allocation/SDK and requires exact permit admission.
class CallContext final {
 public:
  CallContext(SessionMemoryDomain&, const JobPermit&, CallCheckpoint);
  void check() const; // exact actor, Parent+permit, borrowed checkpoint, recheck
  // immutable captured Parent, copied permit, checkpoint, thread/fiber
};

std::shared_ptr<Information::CallView> Information::make_call_view(const CallContext&);
std::shared_ptr<Content::CallView> Content::make_call_view(const CallContext&);
std::shared_ptr<Handlers::CallView> Handlers::make_call_view(const CallContext&);
```

Each public CallView implements the existing narrow virtual interface(s): Information Effects; Content Effects+AvatarRequest; Licensing LicenseHandlers+UnknownMessageEffects. A Core view, if exposed for existing direct Core tests, implements Effects+OwnedAuthEffects+UnknownMessageEffects. The call context is copied once into stable call state and shared **by identity** between a Licensing Handler view and its nested Core. A plain copied JobPermit extends neither an Operation nor source authority. Root-only historical calls keep their existing creator affinity/legacy execution; no invented root permit or ambient current-job lookup is introduced.

Graph's eventual explicit MessageCall owns these facade pointers and passes them into its per-call Router/effects dependencies. Calling a view never calls `owner.with_job`, never overwrites owner.job/checkpoint, and never redirects the owner's default facade. The exact public context could instead remain three existing typed parameters with one internal stable shared Context object, avoiding a new common header; the required invariant is identical context throughout nested execution, not a particular name.

## Internal backing/execution split

Retain one owner Backing containing immutable selected Options/shared provider identities and the actual borrowed state/paths/Online roots. Keep the legacy execution and controls as one separate execution object. A call view owns a separate execution object containing busy/checking/poison/retirement state, its immutable CallContext, all leaf adapters and (Content/Core) its own GuardedFiles. Persistence Engine references the SAME backing configuration/Paths but the **view-local** Clock/Formatter/Directories/Files adapters. No constructor I/O, Online load, configuration copy, authentication replacement or fake AttemptState is permitted.

Owner metadata needs a short-gated stable registry of view states and shared sticky failed-owner health. Publish a nonallocating exact actor admission before list/shared-pointer/provider metadata allocation, validate Parent+permit before callbacks, allocate candidate nodes outside the gate, then splice/recheck. Different admitted actors on the concurrent Parent may create/use views simultaneously; same-actor allocator/checkpoint reentry remains sticky for that execution and its owner. An unknown view outcome also poisons shared owner health, so the legacy root facade and sibling views cannot continue as though their common owner were healthy; an already-running sibling observes that at its next checkpoint. Healthy activity alone is not owner failure. Do not hold the owner gate over any allocation, exception construction, shared-target destruction, SDK, logging, directory walk, file, dialog, Online query or checkpoint. Metadata scopes preserve LastError locally.

Normal facade retirement keeps its state registered through callback-bearing target/writer retirement and final context checks. An unknown file or callback result retains that execution's actual prefix in the owner registry; merely dropping the facade must not invoke speculative raw-file cleanup. Root provider retirement rejects live/failed views using its existing lifetime discipline, rather than silently freeing their backing or adding a new OS unload requirement. Distinguish active and failed states so an independent healthy view is not mistaken for the current call's retained failure. Keep the creator-owned root facade/Options backing alive through all views and retire it on its existing creator actor; the last normal child facade must not unexpectedly destroy creator-affine backing. A view may strongly retain Backing, whose registry owns stable execution states referring back nonowningly; do not create a Backing-to-view strong cycle. The caller retains raw checkpoint backing through view retirement. In particular the view must not strongly own its Graph MessageCall when that same MessageCall owns the view in its dependency bundle.

The existing GuardedFiles implementation intentionally supports one stream owner and is not a shared multi-call object (`files.cpp10–15,39–50`). **Keep that contract and instantiate one guard per execution.** A raw factory remains shared; selected native file factory opens independent actual streams. Its custom HOST replacement is not declared thread-safe merely because the facade now permits independent executions. Raw file adoption before postchecks, initial-state/close authority, sticky unknown prefix and retained-file tests remain unchanged.

## Licensing identity and remaining shared-state obligations

Handler/Core reciprocal validation must become direct same-context validation. Do not point a shared Core's handler_context at whichever child happens to run. A per-view lightweight semantic `license_recovered::Handlers` can reference the same actual StateReferences/authentication cell while using the view-local Core. This is an executor, not a new business state: `used_` is HOST binding bookkeeping written on every semantic request (license.cpp39,57,124); sharing that executor without synchronization would itself be a C++ race. Bind the view executor to the **existing exact AuthenticationCode identity**, once, before its first request; do not reconstruct its fallback value or permit rebinding after publication.

AttemptState's attempted/blocked fields, denuvo token string and game attributes remain actual shared roots. Existing license bodies directly mutate/read them (license.cpp124–155,57–60). This foundation must not claim arbitrary simultaneous mutating licensing calls are safe by giving each view separate copies. Before enabling the full coupled Original route, shared scalar/string/map accesses need explicit audit and narrow native-consistent access projection where required; a whole licensing-request lock would reintroduce the measured progress defect. The measured UnknownMessage/GetConfig case does not access those mutable licensing cells, so the call-view foundation can be implemented and tested without inventing a license transaction model. This is an integration obligation, not a reason to restore a message-wide strand.

## Minimal coherent source and source-test groups

First group: Information/Content effects.hpp/.cpp (four production files), a shared typed context header only if selected, their two test files, and an actual PreparedMemory handoff fixture. Preserve old defaults, borrowed checkpoint binding and child-with_job failure children. Test A Content dialog or file effect held while B calls actual Information UTC/Content independent effect; check identical backing roots/provider identities, exact distinct permits, outgoing LastError, wrong/expired permit denial, swallowed view reentry and failure isolation. Reuse current real NativeThreads/PTD/FLS fixtures and finite events.

Second inseparable nested group: Licensing effects.hpp/.cpp and effects_tests.cpp; no change to semantic business algorithms solely to manufacture concurrency. Test same Context through Handler→Core and XML callbacks, existing exact authentication cell, same AttemptState identity, owned Online result reservation, supported NoResponse/UnknownMessage vs unrelated same-type throws, and separate raw-file owners. A retained failed-file child must not free its adopted raw stream; a healthy sibling need not inherit that execution's busy/checking state. Meaningful source cases must use the actual provider facades, rather than synthetic Effects that bypass them.

Then Graph MessageCall/adapters/Router/writers and Profile/NativeJobs integration: per-message facade pointers, context lifetime through NativeProfile chosen-dependency retirement, actual shared event/achievement locks with call-local ownership markers, no global current-job or deps swap. Keep actual OriginalConcurrentActors disabled until these coupled layers and shared business-state access are coherent.

## Exact inspected source hashes

| File | SHA256 |
| --- | --- |
| Information effects.hpp | `78f57e125b0f648ace5a42a875a91b6ceb8f792f528d7b67858d48f315b0b739` |
| Information effects.cpp | `11ae43c5d32eb83d9cd1768fef6d789d7db69ea200a5392bdf731905ab7a0e51` |
| Content effects.hpp | `b387ca6756b1898abe60f672bfdfa467de6a16117c33bcdb6333509630312242` |
| Content effects.cpp | `8224ff1c1b10c5a3bb3d91aa8762f574a657a17dc4a83dee59dc19b745e5a510` |
| Licensing effects.hpp | `7a84578546cecb74312818a094cdccdabbb641ab67f0c63787e7bfdf56684d07` |
| Licensing effects.cpp | `c66cf34ed9b2e23772c48e834810e5afca0200083dd14b30e0eb508547e7e5d9` |
| application-io files.hpp | `14136f9036d42c215b32051451af2d35930b94207a230d3ae492df85ad8a59e6` |
| application-io files.cpp | `72d45de14700e0336dfd1f4909b3a9e97b1e94b92531a58db447dfd8f0d34cd3` |
| Graph graph.cpp | `fc90cf952eb5bb9230421d8dc97981ed6c7b60cf94d5b18f0cbd695ea0fdaf84` |
| Graph graph.hpp | `de539b9a711a527a824ff55d9b22ac67c6098fb3657ab3d0ee49bc0c0d853aaa` |
