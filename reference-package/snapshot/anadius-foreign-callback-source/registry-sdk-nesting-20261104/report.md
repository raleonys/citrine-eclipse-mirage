# Registry nesting during original SDK calls — 2026-11-04

The selected OriginalCrt source now admits same-thread/fiber general registry descendants only while their immediate parent body is inside its actual typed original SDK invocation. Previously the single-active general guard rejected that path. This corrects a bounded source restriction exposed by native call ordering; it does not prove Windows actually reenters these APIs.

## Native basis

Six fresh REA6.8.0/Ghidra12.1.4 Evidence records and exact addresses are indexed in `native-findings.md` and retained in `rea-evidence-bundle.json`. Open A/W calls originals at0x18000f823/0x18000f983 before configured-key compares; query A/W snapshots capacity, calls originals at0x18000fb01/0x18000fba1, then enters replacement helpers. Real close calls original at0x18000f6bb after completing fabricated membership lookup; successful return reloads client/language sets. No source container operation remains active across these source SDK calls. The original file and native session identity were checked, and the session was closed after export.

## Implementation

- `anadius-native-source/native_callbacks.hpp`: noncopyable automatic SDK/body frame tokens, optional Admission operations, and a once-only passive OriginalEffects-to-Admission binding. The SDK guard surrounds only the typed `fn(args...)` call after provider resolution and boundary prechecks. It is destroyed on normal return or exception before the existing boundary postcheck.
- `anadius-native-source/native_callbacks.cpp`: native wrappers retain a counted Context lease, try existing pure-forward/connect selection first, then optional nested registry admission. Child leave precedes Context lease release. The nested route still executes the full ordinary registry body rather than directly forwarding it.
- `anadius-native-context-source/context.cpp`: actual Owner passively binds its owned Admission to Effects before publication. Admission is declared before Effects and therefore outlives it. Standalone/default policies retain null/declined phase behavior.
- `anadius-foreign-callback-source/foreign.hpp/.cpp`: SDK and child-body TLS frames retain exact Domain, callback, thread, fiber and parent-body identity. Child admission reserves Checking before hook-publication and existing-PTD checks; success becomes Full. A mere ancestor SDK marker cannot admit callbacks from a child's resolver, proof, CRT/log/set body. Each child's own SDK invocation can admit the next depth. Same-callback recursion is distinguished by frame identity, not just callback enum.

The root general admission remains active throughout every descendant and alone owns CallbackThreads entry/exit and any acquired PTD. Children validate and borrow it; they never create or retire a second PTD. Exact LIFO cleanup restores parent metadata even on known denial or lower exceptions. Existing poison handling and checkpoint semantics remain. Full permission remains present during an admitted SDK interval, consistent with the prior early-registry integration source. Metadata preserves LastError; SDK/provider effects retain their own semantics.

Only general open A/W, query A/W and real-close original calls select the new parent interval. Pure-forward/connect parents, arbitrary general body reentry and cross-thread shared-set concurrency are outside this correction. Existing typed API signatures, synchronous startup and application-detach behavior remain. No new readiness/unload requirement was added. Internal source Admission vtable and Effects layout changed: all consumers require a consistent future rebuild.

## Test sources and review

Two existing test sources gained unexecuted scenarios. `foreign_tests.cpp` checks exact frame recursion, child Checking isolation, same-slot identity, missing receipt, PTD restoration, foreign overlap rejection, pure-parent exclusion and metadata LastError. `foreign_integration_tests.cpp` uses actual native_context::Owner, concrete Policy and a Publication proxy with explicit Domain scopes. Its synthetic typed originals form openA -> same-slot openA -> queryA -> queryW -> real close, checking arguments, status bits, caller outputs, one root PTD and per-entry Context leases. Controlled resolver/encoding callbacks check that ancestor SDK permission cannot leak into child body phases. A direct Effects call checks synthetic lower-exception guard restoration without crossing the native wrapper's noexcept terminal boundary.

That integration source exercises Owner auto-binding, not the outer foreign_callback::make_factory wrapper verbatim. It uses synthetic SDK boundaries and does not establish actual Windows reentry, original EH/CRT ABI, arbitrary concurrency or production DLL runtime validity. Typed OpenW nesting is not positively covered; new cases do not yet cover every input combination or aliasing of caller query buffers.

Root implemented native integration, one subagent implemented Domain frames, another added test sources, and an independent subagent reviewed the combined source. Exact pre-edit files and separate implementation/review notes are retained here. No compilation or project test execution occurred; static review is not a passing executable test result.

## Build and delivery boundary

Five production and two test source/header files changed in this stage. Together with the two earlier pending files, nine canonical source/header files differ from the last actual successful full build. File counts do not measure whole-DLL completion. No numerical completion increase, new normal native algorithm recovery or ABI closure is claimed.

No build, link, reconstructed DLL execution, original mutation or ZIP update. The existing DLL and ZIP remain the verified prior delivery and do not contain these edits. Frozen build/link/package proofs retain their historical hashes. Explicit user rebuild instruction remains required before compilation/tests/link/execution/package work.
