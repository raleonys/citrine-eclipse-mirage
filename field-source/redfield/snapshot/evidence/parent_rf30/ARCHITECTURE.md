# REDFIELD RF30 — maintained architecture

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



## RF30 current build profile authority

One build_common.BuildSession now owns compile/link/hash/process/environment
behavior for the developer aggregate andfour independent operator profiles.
Each profile recompiles the same production sources under its own fixed build
root andCRT.Debug alone uses /MTd+symbols;production/native/CRT use /MT+GS.
Test support/regressions never join operator DLLs.Native entry define islocal.
The generated build/dist roots are excluded from immutable source inventory;
source packaging refuses generated output inclusion.Nonproduction dist writes
are staged/hash-bound/atomic/readback-checked.Production shipping dist isblocked
until actual destination-bound Golden integration in M20.3.No override exists.

CMake remains a developer aggregate withthe common source-contract preflight;
it does not publish/finalize production dist.Native/CRT operator checks reuse
current hosts only;full old coverage andnative/debug composition are pending.
All281 maintained-code qualifications and4 vendor integrations remain pending;
4 old module source reviews do not imply new-rule semantic qualification.
