# Default environment classifier review

Independent static source/native review, 2026-11-04. No source changes, compilation, project tests, linking, target execution, packaging, or new REA session were performed for this review. Fresh native records and their limitations are retained in `native-findings.md` in this directory.

## Finding

There is a concrete selected constructor-branch mismatch. The native initial accepted-socket environment is context-bearing but classifies as the empty/sentinel type for `0x180023dd0`. The default HOST environment deliberately classifies as active. That distinction reaches production because the OriginalApplication route currently selects the same default HOST provider. A bounded explicit original-default constructor-classifier option is justified; replacing the whole HOST capability adapter is not justified by this evidence.

This establishes a difference in allocation, dispatch, and exception paths. It does not establish an observed business-payload failure or prove equivalence of the still-unexpanded native response factories.

## Native proof

- `0x180023000` allocates the session owner and calls `0x180023d20`. Their fresh records establish environment E at owner+0x78, through socket+0x68; copying preserves the initial family. Evidence: `ev_118a0cfef34aca9cd2f70a9f902647bc2e8014332037d147b693b6afa2c3fa5a`, `ev_7b2eb7e1eff80d28c8357649ce44845d3f1f2ae103f9f9bc728f878f799acb0e`.
- `0x1800211b0` emits initial operations `0x180084928`, schedule `0x180084908`, and executor `0x180084860`. E contains a real context, flags zero, and a self pointer. It is not a zeroed HOST row. Evidence: `ev_4bfe7f01a2866644c84442648a9a9a6b05c04a4e72bacc315bb3393fdb8f17eb`.
- The schedule's type-query target `0x180023890` is the complete LEA/RET leaf returning `0x1800a25c0`; adding eight yields `0x1800a25c8`, the equality literal used by the retained `0x180023dd0` constructor evidence. The initial family therefore selects the empty branch, absent an intervening family rewrite. Evidence: `ev_1111199304b117e12a6173706805b0dc91d27124a93de4813b26e339f32541d6` and the retained table/constructor evidence identified in `native-findings.md`.
- Initial copy/move target `0x1800228c0` copies context and flags and sets the destination self pointer; the complete 17-byte body has no work increment. Its typed source body already exists in `outputs/anadius-socket-move-source/move.cpp`; the lack of a Ghidra Function object is not a newly missing body. Evidence: `ev_79550a46046c957e71260f8fa4d9b7fd4df33f9ececfd07dca26ee75bae21135`.
- Initial executor preparation `0x180023520` uses `0x180020c80` without a work increment. Factory `0x180023580` instead retains native work and emits the tracked family. Tracked type query `0x180023880` returns distinct `0x1800a2690`. These are separate families; a global HOST classifier option must not be described as a reconstruction of every tracked native environment. Evidence: `ev_79b335ce03113d93d67c6e8fc484230ca1c0f7431c65304cb7fb643a852a9f98`, `ev_53d6ef0e2c9f71bff9e0f9cf81dd87e2c07090b15e7f168b9cb3017938219bd1`, `ev_8daf0429b5eb6b0229da4d9d300f2d2be8ac43491bc1ebba32f6f12078297bfd`.

## Selected source chain and consequence

1. `outputs/dll-link-gate-source/entry.cpp:76` calls `configure_native_worker`. `outputs/anadius-resources-factory-source/factory.cpp:89-109` selects native worker IO and OriginalApplication lifecycle. `outputs/anadius-application-graph-source/graph.cpp:558-570` transfers profile settings but supplies no custom environment provider.
2. `outputs/anadius-session-profile-source/profile.cpp:766-768` consequently constructs default `owned_environment::Environment`. `outputs/anadius-owned-environment-source/environment.cpp:26-27,216,247-249` supplies `active_type()==1` for its nonempty token and `empty_type()==0x1800a25c0` for the empty token. Equality is therefore false for the HOST root schedule.
3. `profile.cpp:464` creates a real HOST session row. The persistent constructor-source registry maps owner+0x78 to that Slot's environment. `outputs/anadius-session-flow-source/coherent_io.cpp:14-24` and `outputs/anadius-socket-source/environment_constructor.cpp:66-76` resolve allocator tokens through those root entries. Derived temporary response/factory rows are not added as constructor sources in this selected Profile.
4. `environment_constructor.cpp:36-62` already models the equality branch: equality produces an empty destination; non-equality calls factory58 and yields a nonempty owned row. The constructor body itself need not be rewritten.
5. `outputs/anadius-socket-source/io_completion.cpp:90-139` invokes send/receive directly for state zero, while nonzero environments enter scheduling; receive also prepares executor slot40. `outputs/anadius-socket-source/handler_dispatch.cpp:126-178` takes the HOST owned-task branch, allocating a handler task before the HOST handler invokes it inline. Thus the current active classifier adds allocation/cache ownership and associated failure/order paths even when normal payload output happens to agree.
6. Receive/send initiation already adds one scheduler work item independently of this classifier. The HOST factory does not emulate the separate native `0x180023580` work retain. For the initial native empty branch, both selected paths therefore still start with one IO work item. This review does not claim a missing default work pin.

## Roots, copies, responses, and job scopes must remain usable

The HOST rows provide capability validation, reference accounting, session origin, and protected job-scope authority. They must remain real root objects even if IO-constructor classification changes.

- `outputs/anadius-socket-source/message_dispatch.cpp` copies owner.environment_78 twice through the selected copy adapters before response dispatch. Those HOST proof rows must remain valid; zeroing every Slot root would break this path and the scoped authority checks.
- `outputs/anadius-socket-source/response_dispatch.cpp:65-105` uses executor slots20/88, copies/moves, and response scheduling. The selected HOST response adapter force-posts its owned task. The fresh native audit did not expand native slots20/88; current evidence is insufficient to replace that complete policy or claim it native-equivalent.
- A posted write later constructs IO from its owner allocator token, resolving the original Slot root. The relevant constructor source is not the temporary response factory row. This is the explicit boundary permitting a bounded default classifier option.
- `profile.cpp:631-657`, `JobHandoff::clone_environment`, enters an EnvironmentJobScope rooted at the real Slot environment and calls the generic constructor. It currently queries the clone unconditionally at line654. An empty constructor result must skip that query while preserving destruction and exact scope closure. Actual `NativeJobs::process_message` does not use this helper as its main dispatch body; nevertheless the public helper must remain coherent.
- The existing accept row is created with state zero at `environment.cpp:269`, through `profile.cpp:447`. Preserve its ownership and capability behavior; this finding does not require a separate accept-root replacement.

## Minimal proposed change, subject to root decision

Add an immutable `OriginalDefault` constructor-classifier mode alongside the current legacy HOST mode. It should select the existing empty-type query for the HOST schedule token while retaining the same root rows, context, copy/response adapters, capability ledger, and job scopes. Do not cast native virtual-table addresses into callable HOST pointers.

Propagate the explicit option through these files only:

- `outputs/anadius-owned-environment-source/environment.hpp` and `environment.cpp`: immutable option and configuration query selection.
- `outputs/anadius-session-profile-source/profile.hpp` and `profile.cpp`: dependency option, default-provider construction, and empty-clone handling.
- `outputs/anadius-application-graph-source/graph.hpp` and `graph.cpp`: profile setting and dependency propagation.
- `outputs/anadius-resources-factory-source/factory.cpp`: select the option in the actual original native-worker configuration.

Preserve legacy HOST defaults, explicit custom providers, original synchronous attach, and current retirement policy. Deriving the choice implicitly from every OriginalApplication lifecycle would be a wider behavior change than explicit selection. No generic constructor, response factory, native work-tracking framework, or unload redesign is warranted by this finding.

## Validation required before any runtime claim

The user has not authorized a new build in this task; these are proposed source cases, not results:

- Legacy HOST configuration remains active and retains existing copy/response/job-scope behavior.
- Original-default configuration retains a real session root but constructor output is sentinel/state zero; factory58 is not entered and no extra HOST row reference is retained.
- Selected Profile receive and send completions use their direct bodies without allocating an owned handler task; initiation's one IO work item and dispatcher decrement remain balanced.
- Root message copies, response force-posting, and queued-write root lookup still work; custom provider configuration is not silently overridden.
- The clone helper accepts an empty clone, avoids query_slot18 on it, destroys it through the sentinel operations, and closes the protected scope with no live temporary.
- Configuration propagation selects only the intended original worker; original synchronous startup is unchanged.

This report is a bounded static finding and design recommendation. It is not a runtime test pass, a proof of complete native environment equivalence, or an assertion that the tracked-family and response slot20/88 adapters are finished.
