# Independent Information and immutable-context review

Status: no blocking source finding in the four exact files below. Read-only static review against the stage's Information before copies and newly added context header; no compilation, test execution, link, DLL execution, REA query or packaging. This report does not independently review my own Content changes.

`CallContext` captures one actual thread/fiber, exact Parent and copied JobPermit plus a borrowed checkpoint. Constructor/`validate()` checks do not invoke that checkpoint. `check()` validates, calls the admission checkpoint outside metadata gates, then validates again. The final header adds an Error restorer around this metadata/admission sequence, preserving the caller's LastError; actual clock, file, stream and formatter effects run outside it. The permit and context copy do not extend an Operation or the checkpoint backing. Foreign/expired factory use rejects before calling stale checkpoint backing.

Information's owner registry has an intrusive admission reservation before allocations, with published preparing state before caller checkpoints/wrapper allocation. Execution states have nonowning owner references; wrappers separately retain owner and state, avoiding registry/owner strong cycles. The mutex protects only registry/phase metadata. Actual clock, formatter, stream and checkpoint work occurs outside it. Private per-state clock/formatter adapters retain their exact context while sharing the same actual lower roots. Same-actor active/preparing/checking/closing and legacy-job overlap is sticky; merely idle handles and different actors do not create a body strand.

Normal close requires the exact live context, keeps its state published through the final checkpoint, then retires only that row. Closed wrapper destruction later touches retained owner metadata only. Unclosed destruction sets shared unknown health and retains the registry prefix without probing expired callback backing. Creator retirement still refuses live rows/admissions and uses its existing affinity/backing-retirement rules. Host default methods and synchronous child methods retain their existing narrow interfaces.

The Info test diff adds `sdk_error_survives_metadata()` and its main invocation only: injected actual clock/formatter and stream-flush errors must survive subsequent provider metadata checks. Existing handler/XML/UTC, null-gmtime, checkpoint/reentry, affinity and attached last-owner-retirement cases remain. This unit file does not itself prove parallel views; the separate handoff fixture supplies source coverage and has its own review. Nothing here claims runtime PASS or whole Graph/NativeJobs concurrency. Final context review includes the root's three-line admission-check LastError guard revision; the earlier unrecorded `41c8cb47…` candidate is superseded, not final proof.

| Reviewed source | SHA-256 |
|---|---|
| `outputs/anadius-information-effects-source/effects.hpp` | `3a77b6470f2e0f0751498a14559c1274f6ea8b469cd8eda60dba53ed8298186b` |
| `outputs/anadius-information-effects-source/effects.cpp` | `a01949674e6f3ac384c31fc534f1e1ed1ff9abc32b824d7423ccaf6715fbdb39` |
| `outputs/anadius-information-effects-source/effects_tests.cpp` | `78eea811a3067953f6fac0ed80c6e97c263163b637e63d43752e13f162ee6bc4` |
| `outputs/anadius-application-io-source/call.hpp` | `2d2fd2f4bfc1b4abaad932a88b93dd1ad9f03240ce3cc5561b252d733ab096d4` |
