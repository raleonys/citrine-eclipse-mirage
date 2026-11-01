# REDFIELD RF32 — maintained architecture

LOCAL_BUILD / NOT_RELEASE. This is the current architecture entry point.
The [RF20 TCP delivery note](history/RF20_TCP_DELIVERY.md) is preserved verbatim
as historical design context. Current closure work is recorded in
[the refactor status](REFACTOR_STATUS_TR.md) and [review metadata](../metadata/REFACTOR_REVIEW.json).

## Identity and ownership

REDFIELD is the project name. Application namespaces, include paths and build
identifiers use `preloader_l`. The production DLL is `preloader_l.dll` and its
three public C exports remain DllMain @1, SURGE @2 and DllShutdown @3. SURGE is
passive. The independent compatible configuration filename is `surge.cfg`.
GPL-3.0-only markers, original notices and pinned upstream licenses remain.

All maintained application files have the same review and ownership rules,
regardless of origin. Exact reference files and replay recipes preserve ancestry;
they are not a second executable application tree. External dependencies retain
their pinned upstream layout. `metadata/BUILD_GRAPH.json` is the shared build list;
`metadata/ARCHITECTURE.json` binds the maintained application inventory.

| Area | Responsibility and lifetime boundary |
|---|---|
| `src/windows` | Process composition, attach scheduling and explicit shutdown; executable work belongs outside loader-lock callbacks |
| `src/platform` | Seven existing MinHook detours and registry policy/gates, owned by WindowsHooks; retain ownership on incomplete teardown |
| `src/config` | Bounded Config2 parsing, unpublished projection staging, atomic immutable snapshot publication and explicit hook configuration binding |
| `src/core` | Worker/service ownership, query formatting, per-session records and transactional deliveries |
| `src/net` | Socket/peer/buffer ownership, response writes, commit/event ordering and pending completion drain |
| `src/runtime` | Listener/timeout and transient memory support; generic allocator/PTD implementations are regression support only |
| `src/xml` | One TinyXML2 DOM engine and source/semantic text boundary; bounded framing and serialization |
| `src/handlers`, `src/router` | Bounded handlers and exact-name dispatch; 42 descriptors, 35 current plaintext bindings |
| `src/hash`, `src/codec` | Deterministic compatibility hash and strict byte conversions; hex input/output ranges must be disjoint |

`ConfigStore` publishes only complete owned snapshots. Raw quoted spelling is
preserved; typed projections copy borrowed parse tokens before publication.
Content ignores empty updates, Entitlement accepts empty replacement, and
Achievement preserves its documented totals/name/duplicate policy. Borrowed
`ConfigSnapshotView` and setting values require a retained immutable generation.
Hook configuration remains pinned until the hook gates drain. `surge.cfg` is
loaded only beside an absolute trusted module path, without CWD or DLL-stem fallback.

The transport commits a prepared delivery only after a full successful local
response write. Async callbacks keep their borrowed buffers alive. Local write
completion is not remote application acknowledgement or network exactly-once
delivery. Shutdown must retain owners until callbacks and physical releases drain.
The remaining module reviews still examine these implementation contracts.





## RF32 composition and build authority

Production/native use separate entry TUs with one shared WindowsWorker template.
The production observer owns no state;native fault/counter/export code is only
under tests/native. WorkerOwner owns thread/pin/cleanup duties;observation merely
gates close/unpin attempts andnever becomes a resource owner. Static constinit
per-DLL composition retains loader-lock scheduling andexplicit shutdown rules.
No native macrovariant of production entry isaccepted. Native entry has an
independent source role rejected byproduction/debug profiles. CMake uses the
same separate entry. Shared core andtest-support isolation remain unchanged.

RF31 session/resource/link/Golden/atomic-dist authority remains. Exact old29
native/CRT scenarios and4debughosts are mapped inACCEPTANCE_COVERAGE.json with
explicit pending gaps. Mapping isnot runtime parity.97freshportable regressions,
modeledworker runs andsyntheticPE QA do not close actualWindows acceptance.
298code+4vendor qualifications and16module reviews remainpending.

## Registry value output — RF33

The policy owns value selection, conversion and temporary allocation. One private
header owns allocation-free REG_SZ byte serialization. It borrows spans only for
the synchronous call; it never retains them or assumes typed/aligned output
storage. Conversion/heap helpers and hooks do not enter the portable writer.

## Worker and transient heap lifetime — RF34

M02 owns serialized join,session retirement,physical-release finalization,handle
close andmodule unpin duties under one deadline. M11 keeps callback descriptor
values inside each explicitly constructed intrusive allocation owner;context/code
lifetime spans both queued anddetached retries. The queue owns failed releases;
queued metrics are not a quiescence proof. Generic CRT/PTD stays test-support only.
