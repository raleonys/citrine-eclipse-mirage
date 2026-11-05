# Original registry teardown integration test source

Edited only `anadius-owned-server-source/server_integration_tests.cpp` and this note. **Authored/static source validation, not compiled or executed.** No compiler, project tests, SDK fixture/process/DLL execution, link, ZIP or REA query was run. Production server.cpp belongs to the separate implementation agent. Parent-captured before files and pending64 metadata remain unchanged.

## Exact gate identities and live storage

The existing Native cold-context case now records actual CRITICAL_SECTION addresses at initialization. For this explicit Original profile the order is registry, scheduler, service. Deletion callbacks require those same addresses in service(index2), scheduler(index1), registry(index0) order; the final assertion also requires exactly two gate deletions between WSA cleanup and port close, then one registry deletion after port close. Existing HOST startup/two-gate/port-first cleanup expectations remain in their original branch.

Runtime.run stores `&view.scheduler`, which denotes the scheduler embedded in the actual retained Core. It does not store the address of the borrowed ServerView as teardown authority. The existing stack-view pointer is still cleared before run returns; subsequent teardown callbacks use only the actual scheduler pointer while the backend/Core remains alive. No access is made through that pointer after Core destruction.

After the actual worker returns and its queue/relay is drained, Native records its natural low32 wake1 and work0. The fixture explicitly seeds only high32 with0x5a5a5a5a while preserving the actual low wake and work count. This is an approved fixture input to distinguish replacement from OR, **not an observation that native normal execution produced that prior high value**. Each Native Delete callback requires high32 exactly1 and the recorded low wake unchanged. It therefore catches omission, OR-with-old-high, or accidental whole-word replacement that destroys low wake.

## Ignored port-close result

The same native IOCP import adapter uses a dedicated get_last_error spy. It records only actual IocpConstructorApi error queries, not every direct Win32 metadata GetLastError elsewhere in the fixture. The close callback invokes real CloseHandle on its own actual IOCP, captures its result and requires physical success in the Native case. Only then does it return a synthetic FALSE with error5. No handle leak is introduced merely to test a failed BOOL.

The future Native case requires normal Closed, one actual physical port close, one synthetic false return and zero IOCP-adapter error queries. Logical handle retirement is the source's separate effect after the ignored BOOL; it is not inferred solely from the physical close. Actual gate-deletion addresses/order and the existing complete provider/domain/resource balance assertions remain required. HOST continues returning the actual SDK CloseHandle result and retains its prior checked-result/port-first source contract. Existing constructor-error cases can still query the same adapter error spy normally; they are not included in the Native normal zero-query claim.

## Never-initialized Original cancellation

A tiny additional scope reuses the valid explicit Original dependencies before the normal case. It constructs a backend, calls its existing Constructed destroy_context route and requires Closed with no WSA start/cleanup, gate initialization/deletion, port attempt or error query. Its actual Parent provider lease retires before the initialized case begins. This guards the production live-gate conditions: an unpublished cancellation must not enter an uninitialized service gate or manufacture scheduler/WSA teardown effects.

The actual empty-service gate enter/leave remains **source-reviewed**, not separately intercepted by this fixture. Gate initialization/deletion observations do not prove that round-trip occurred. No new gate SDK framework, hidden-worker/timer/custom-registry teardown fixture, native dword overlay, EH cleanup or unload condition is introduced.

## Hashes and validation status

| File | SHA256 |
| --- | --- |
| before server_integration_tests.cpp | `87bf17d709d9a9f508bb3f85489b6ef6ae4ec974f5d0f11db90dc58d0c908e8a` |
| before server.cpp (other owner) | `8f46ebb2adfc054fd2a8710c4d7f4da2e2a6044b2e3ddeede7e6d973eba84c32` |
| before pending-source-status.json (64 sources) | `03f6bee83abef337f6010e3d8efcf7e17f3749e74d01a952788d41f512a5e053` |
| current server_integration_tests.cpp | `45e4627b42d3463fb0ae879168d7115e811d0b97289d001e38ab32e837c37e68` |

Static source/diff/whitespace inspection found no whitespace defect. The independent reviewer confirmed the exact current hash with no finding; `integration-independent-review.md` records the final audit. Test bytes are frozen. No runtime PASS is claimed. An explicit user rebuild/test instruction remains required before execution validation.
