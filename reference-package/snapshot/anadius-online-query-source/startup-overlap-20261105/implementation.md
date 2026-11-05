# Original Online concurrent-query admission — 2026-11-05

Changed only the five authorized production files: online-version `version.hpp/.cpp`, online-object `object.cpp`, online-query `query.cpp`, and online-factory `factory.cpp`. Root captured the actual seven-file/test-source snapshot and pending64 before authorization. This author changed no tests, status, unrelated production files, original binaries or ZIP, and ran no compiler, tests, linker, target execution or packaging. The fresh native audit was performed independently by the SDK agent, not duplicated here.

## Behavior and ownership

Binding now takes an immutable `QueryPolicy` with Serialized default and explicit Concurrent selection. Online's actual factory selects Concurrent for its existing OriginalCrt lifetime profile; Cooperative and legacy Binding construction retain serialized query admission. There is no new configuration flag, global-ready barrier, foreign callback wait or query mutex spanning a foreign call.

The former shared active_ Boolean remains exclusive lifecycle/export activity. A separate private size_t query counter covers status, getters, avatar, auth, owned result copies and existing object_call scopes. Concurrent admits additional queries only for the exact adopted, known live object/module with retained factory code; Serialized additionally requires zero queries. Load, resolve, version-call, factory lookup, create, destroy, free and last-owner Binding destruction all account for nonzero queries before effects/retirement. The private count cannot wrap silently; end_object underflow is a fail-stop invariant violation. No public query-count field was added; Snapshot.active remains its existing Boolean shape and now aggregates lifecycle activity or any live query.

QueryUse's bad-layout constructor catch retains poison/object-unknown state and releases only that failed constructor's reservation. It no longer clears a shared active flag that could erase a sibling's reservation. A constructor failure does not run its destructor, so the reservation is decremented once. Other admitted queries remain counted until their own normal/exceptional end. Metadata mutexes protect counter/owner changes only and are not held during vptr/slot reads, foreign calls, NUL scans, string allocations or code/owner retirement.

Existing Binding-held code_ and factory_code_ shared owners remain untouched while queries exist because lifecycle transitions reject that count. The actual typed module-reference lease therefore retains the code without per-query module acquisition, an owner registry or allocation pool. Online's facade already counts active calls and retains the Binding; no new ordinary-call thread/fiber affinity was added. Its controller-only close/physical-retirement contract remains unchanged.

Owned copy_getter/copy_auth guards continue through the returned foreign C-string's first-NUL scan, allocation and owned copy. Their outgoing per-thread LastError behavior and metadata Error guards remain unchanged. Raw borrowed getter/auth APIs retain their existing caller-controlled consumption contract. Counting does not establish foreign buffer immutability, external SDK internal thread safety, safe arbitrary object self-destruction or a guarantee that configuration strings can be mutated concurrently.

## Native scenario and bounds

The unchanged SDK `native-findings.md` / index / full12-record187323-byte bundle establish the caller-side window: native035bd0 publishes online before profile getters; native0314f0 can call object slot28 based on online without an OnlineReady wait; original server ingress can already be active. A slow **first** username getter leaves profile/path state unchanged while an eligible avatar request executes. The old bridge's unconditional busy exception poisoned that otherwise eligible source call. The new policy allows the original virtual invocation rather than predicting the unknown foreign provider's result.

The ordinary recognized hooked3216/nonweb connection path calls wait(-1), so it masks this initial window. The concrete eligible case is direct/unhooked listener ingress (and separately the proved no-wait80/443 route), not a claim about every first game connection. Native/foreign concurrency runtime occurrence, arbitrary shared-State publication/string mutation, complete foreign object/RTTI/EH ABI and DLL-wide equivalence remain unproved. The fix changes provider query admission only; it neither adds a readiness barrier nor claims those broader races solved.

## Exact source authority

| File relative to outputs | Before SHA256 | Final SHA256 |
|---|---|---|
|anadius-online-version-source/version.hpp|512c3f20766a3fd5691d06933e69c0ca5228aec3a16e5c156eb5cd9d5fa4537b|59be644b1ad458f14a35f6673fd795ff81eb3c247fbd1d9782fc3f1609aeeb4a|
|anadius-online-version-source/version.cpp|328443d0ac8d3bae0b5f12878c955087cf99a402beecf7e6879b345e16e9c7d0|8c4bedbcac6fe06ec876948feb49f37b2ec64eadf647594deb741e29bc067457|
|anadius-online-object-source/object.cpp|917a7503768e47250f94df059b2ad98fec03a022c264aa2a0566e9ae36dfce78|8ec06369de0d4bb2d327fb59cab61ad3d4ca13be86def5e161819d28c15b2064|
|anadius-online-query-source/query.cpp|55bf8cf5ffc9ac78dfd6d96956b62bc03cd06e6e0265877a86315d1dc36c36d3|aef7a62d8a77ca6426d88b3a5a9275d67230cc573f58022003c83dc374039287|
|anadius-online-factory-source/factory.cpp|f6a613a5c9cc007a722298efdc0c181c1270c496a179eb952a0f12fbbb8b95c9|f681a819749056ecb9a29c2f20dca74a1514974b75c561a1dc45c7f8210e724f|

Independent source review and regression source authoring are assigned separately. Their results do not imply execution or a newly rebuilt DLL.
