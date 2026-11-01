# REDFIELD RF31 — maintained architecture

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




## RF31 current build and Golden authority

One build_common.BuildSession owns compile/link/input/tool/object/resource/output
hashes fordeveloper aggregate andfour separate operator profiles.Production alone
compiles pinned compatibility RC andbinds actual MSVC link audit tofive real
Golden functions.A bounded immutable PE model isshared.Only URSYNEX owns allowed
byte mutation;later stages assert read-only hashes.Typed live Golden outcome is
required before atomic production dist/readback.All failures/cancellation clear
that attempt's dist.CMake stays developer aggregate,not another dist authority.

Policies/references arepinned anddestination binding isexplicit.Local source
package hygiene retains ancestry,not official flat release acceptance.262app files
areexact RF30.294code+4vendor qualifications and16module reviews remainpending.
Actual Windows/resource/layout/data acceptance andfull debug/native/CRT coverage
andcomposition remainopen;Python syntheticPE/mocks donot close them.
