# Explicit selected Environment ownership integration

Status: source5 frozen for independent static review. Four production files plus one component-test source are owned by this author. No compiler, project tests, linker, reconstructed-DLL execution or ZIP update ran. SDK separately owns graph_integration_tests.cpp; no success/runtime coverage is claimed for those authored fixtures before execution.

## Frozen files

- `outputs/anadius-session-profile-source/profile.hpp` SHA256 `838a0f0acd22ef5c23973d3479d767cacbfa43ba754c3372dc82b332c7291f9c`.
- `outputs/anadius-session-profile-source/profile.cpp` SHA256 `c9efdbd1cbe629891b41250236a38106ced83946003242fc781467a17889a9d5`.
- `outputs/anadius-session-profile-source/profile_tests.cpp` SHA256 `91d359de8fc878baffcac3b8bdeec7570b292cfc1724d1324cd4d99967f3d4ba`.
- `outputs/anadius-application-graph-source/graph.hpp` SHA256 `7c0b23301f15f50d153e4ef7df8f7ea13a7561194e9b03d8a625225fdedb1c77`.
- `outputs/anadius-application-graph-source/graph.cpp` SHA256 `609d171d7c526dea6f50e5fef91b5e6896e7ef0f326f0396a378a3e53de9ceb1`.

## Concrete composition change

session_profile::Dependencies and application_graph::ProfileSettings now have an OPTIONAL TRAILING shared_ptr<owned_environment::Environment> environment_owner. The forward declaration lives outside the session_profile namespace; no circular owning-header include or getter is needed. Graph forwards the SAME selected object to Profile, never a replacement provider/business root.

Profile first computes its existing exhaustive legacy-environment-empty predicate. An explicit owner plus ANY nonempty legacy sentinel/types/query/factory/handler/response/copy/accept projection rejects invalid_argument BEFORE provider configuration or automatic default allocation. It then MOVES environment_owner out of the Dependencies slot into the existing State::owned_environment ownership path and exports THAT object's configuration. No duplicate Dependencies ownership slot remains in State.

The normal owned bind, session/accept construction, release/reap, clone_environment and final close paths are reused unchanged. Runtime Ports, actual concrete Policy, work/stop algorithm, Parent operation policy and existing selected callbacks are not wrapped or substituted. Retaining a provider pointer supplies object/code lifetime only; child calls still require real Parent JobPermit/CallContext admission.

An explicitly supplied provider's immutable classifier and actual capacity are deliberate. Profile's original_default_environment/environment_capacity remain automatic-construction settings and do not reconfigure/replace the supplied object. Graph::create also validates the automatic environment_capacity range ONLY when no explicit owner is supplied; null-owner validation is unchanged. Other registry/SDK/lifecycle configuration checks remain.

Null owner plus an empty projection keeps the previous automatic provider construction. Null owner plus an explicit custom projection keeps its existing legacy route. No new readiness/body/unload barrier, use_count gate, provider reservation or permission channel is introduced. The documented caller contract is exclusive binding of one fresh provider to ONE Profile/Graph. configuration() permits fresh exports and is NOT a reservation against a second unbound Profile. Bound/closed provider configuration rejects early without poisoning the existing provider; unsupported simultaneous sharing is not silently made safe by pointer retention.

## Authored component evidence, not executed

profile_selected_environment_owner is added to existing profile_tests:

- Three separate one-field legacy mixtures (sentinel scalar, type-query entry, copy binding) plus owner each reject, leaving the supplied object Fresh with unchanged token/no rows/calls/counters or legacy environment callbacks.
- Positive selection uses a Host provider of actual capacity1 while automatic original_default_environment=true and automatic environment_capacity=0. Exact exported schedule/function/executor identities prove the selected object/classifier is used; automatic capacity0 would otherwise reject.
- Actual Profile create binds that SAME object. Bound-provider reuse rejects without changing its phase, existing application's Open state or real Parent provider lease count.
- Real component greeting/session projection bypasses legacy Policy environment callbacks. Actual session strong release and Profile reap destroy the same provider's one projection, and existing Profile retirement closes that supplied object. Closed-provider reuse rejects without reopening/poisoning it.
- Existing null-owner/legacy/default tests and their unrelated function bodies remain. Original per-file line endings are preserved; only the constructor selection block, trailing declarations/docs, direct Graph forwarding, conditional unused-capacity guard and the focused test addition/main registration changed.

This unit harness remains a lower Profile component fixture; it is not relabeled as the one-selected-Environment actual Prepared Graph/Runtime scenario. SDK's independently owned graph_integration_tests fixture is responsible for that actual composition, real offers/permits/threads, tracked-family deltas, physical exit/reap and naturally established returned child old1.

## Evidence and limits

This is a typed source composition fix and creates no new native-function claim. It uses the retained current native constructor/move/copy/prepare/work-release evidence already recorded in preceding stages. No new REA target/session is opened merely for dependency injection.

Actual runtime execution, full child response/scheduling/021770 ordering, generic shared lower mutation and native EH/ABI equivalence remain unproved. Original binaries and historical build/link/delivery proofs remain untouched.
