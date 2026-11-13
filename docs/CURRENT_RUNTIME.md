# Current application integration

The current machine-readable record is
[RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json);
its task and validation fields retain earlier Asio/XML migration checkpoints. Current
validation belongs to the selected head's GitHub Actions results, following
[the synchronization workflow](GITHUB_WORKFLOW.md). The
[native-allocation/VALIDATION.json](../metadata/evidence/native-allocation/VALIDATION.json)
record describes an earlier checkpoint.
Earlier evidence is retained with its original source snapshots.

One Application owner manages configuration, Asio service, handler/event/
achievement storage, provider, companion DLL, and hook lifetimes. Its startup
worker runs outside loader lock. Production exports are **DllMain@1, SURGE@2,
DllShutdown@3**; the C++ source API is a separate `citrine_eclipse_mirage.source.dll`
target. There are no readiness/port test exports; tools observe existing
`EALsxPort` and LSX responses.

An earlier correction changed only MSVC aligned-CRT allocation injection
in the native `net_failure` test; product `src/include` bodies were unchanged.
The owner's four successful Windows profiles, Native Acceptance 204/205 result,
and local validation of the correction are separately documented in
[NATIVE_ALLOCATION_FIXTURE.md](NATIVE_ALLOCATION_FIXTURE.md). A new native/game
PASS cannot be inferred from the earlier results.

The application is RE-based. Bluefield supplies selected hook 11/lifetime
dependencies; Brownfield supplies infrastructure/metadata and approved
resilience approaches. Entire application runtimes are not imported.
The standard compiler CRT is used. The current partition is **62 Production / 78 reference CPP files**: 60
production object sources, one VDF source and one DLL entry source, as declared
in [BUILD_PARTITION_POLICY.json](../metadata/BUILD_PARTITION_POLICY.json).
Reference sources were not deleted. Four model compositions
were removed from Production linkage, and mixed tests were separated.
The crypt diagnostic TLS guard remains Debug-only.
See [cleanup scope](BUILD_CLEANUP.md).

Four owner-supplied vendors remain active: Asio 1.39.0, MinHook 1.3.4,
PEGTL 4 (4.0.2 header on the owner-uploaded 4.x snapshot), and pugixml 1.16.
Selected archive bytes are immutable; current snapshot identities are in
[VENDORING.md](VENDORING.md). The project XML
compatibility layer uses the A12 depth-100 profile (98 paired levels accepted,
99 rejected) and corrected
reference decoding, with no TinyXML2 production fallback. See
[XML_BACKEND_COMPARISON.md](XML_BACKEND_COMPARISON.md). SAFE-FREEZE applies only to the pinned MinHook build copy. Windows I/O
uses standard filesystem/fstream and approved atomic persistence adapters.
Gulrak is not an active dependency.

The cached offline identity format is exactly `persona;user;username`.
The offline loader checks for exactly two semicolons before invoking the
shared unrestricted splitter, avoiding a discarded vector proportional to
malformed field counts. Valid three-field bytes, empty fields and embedded
NUL behavior remain unchanged. Malformed caches retain prior identity cells;
authentication logging, avatar fallback and cleanup keep their existing order.
The worker fixture measures a 50,000-delimiter cache against a 512 KiB
temporary-allocation budget; it does not bound the initial file read itself.

Configuration is optional: `surge.cfg` beside the EXE, followed by
`surge_override.cfg`. SDK identity `10.6.2.31` and wire version
`10,6,2,31` are fixed. Former ServerPort/SDK CFG keys do not apply.
The listener selects ports 3226–3236 in ascending order; startup fails
if all are occupied. Connect redirection applies only to
`127.0.0.1:3216` / `[::1]:3216`, using a copy of the caller's const
address. The counter lock does not enclose startup waiting or the
original `connect` call.

There are two handler workers, a bounded queue, and a per-session FIFO.
Event reservations are committed only after complete transmission and
remain retryable on failure. Greeting is asynchronous; session/frame/output
limits and receive/frame/send deadlines remain enforced. Achievement XML/INI
writes use an atomic batch with rollback and retry ownership; no claim of
power-loss atomicity across multiple files is made.

The grant persistence path validates and serializes the complete watcher INI
before creating its parent directory. Unsafe section names in any achievement
therefore reject the grant before directory creation or atomic writes; the
grant restores its state and releases its lock without publishing an event.
A corrected input can retry the same live state.

The concrete Windows batch backend first recovers retained work, then rejects
empty/embedded-NUL destinations, comparison lengths beyond the Win32 domain,
and case-fold-equivalent destination strings before staging. Comparison uses
locale-independent `CompareStringOrdinal`; comparison failure also rejects the
batch. This conservative admission can refuse distinct case-sensitive names;
it does not resolve every hardlink, reparse, relative/absolute or 8.3 alias.

Nonempty Config2 `AchievementsSet` and `SteamAppId` identifiers are checked for
Win32-forbidden punctuation, path separators and C0 controls before achievement
file I/O or retained-path updates. The bare `SteamAppId` watcher directory also
rejects reserved device names (including `CONIN$`/`CONOUT$` and their device
extension forms), leading/trailing spaces and trailing dots. Its
filesystem-codepage-converted UTF-16 name is checked again, including COM/LPT
superscript device digits. Both checks share the same allocation-free lexical
classifier; narrow high bytes are not treated as Unicode digits before conversion.
Ordinary lookalikes (`CONIN`, `CONIN$_`, `COM10`, COM plus U+2074) and valid U+202E
names remain allowed. Safely wrapped XML names such as `achievement-CON.xml`
remain accepted. Path conversions complete in XML filename, INI filename, app ID,
watcher-directory order before XML import, so conversion failures cannot leave a
partial import. These lexical checks do not provide reparse-point isolation.

For native socket closure, a documented would-block result is retried using
the exact handle. Other close failures have uncertain-quarantine semantics:
do not retry a numeric handle blindly, and do not report successful cleanup.
Client/listener and temporary rollback resources are retained; retirement
is not accepted until read/write/timer callbacks and I/O context draining
finish. See [socket retirement](SOCKET_RETIREMENT.md).

`DllShutdown` retains the same Application and worker image lease after
incomplete cleanup. Hook quiescence does not retry indefinitely within one
call; a later request can complete it. Uncertain socket results are not
automatically cleared, and Production has no force-clear export/CFG key.
The caller's timeout does not bound preemptive termination of provider/
handler work. Native callback Guard/Pause, LastError, trampoline draining,
and retryable unload-guard ownership remain in place.

Hook 11 retains the static `CRYPT32!CryptQueryObject` import, protected
UTF-16 selection, and the 32,768-unit/NUL-inclusive heap-free scan limit.
Avatar/fallback bytes are read-only C++ data; the only PE resource is
pinned VERSIONINFO. Golden metadata and import order apply to the finalized
copy, not the raw compiler output. Debug records actual request/response/
event/error/config values through the existing `showLSX` gate; Release
neither creates `surge.log` nor appends to an existing Debug log.

Local LLVM-MinGW/Wine results are not native Windows/MSVC, Golden, or game
acceptance. Hosted native and Golden results must be checked for the selected
source head; historical NOT_RUN fields are not evidence that no later native
execution occurred. Real game acceptance remains owner-run and DLL-specific.
Donor retirement is unapproved; registry provenance, aggregate memory
budgets, shutdown liveness, and remaining source/test/tooling reviews are open.

For native directory-leaf review observations, the existing manual Environment
diagnostic accepts `operation=path-probe`. It runs the owned-directory
`tests/diagnostics/windows_paths/probe.py` without the Environment build matrix.
The default operation remains the existing Debug/Release Environment tests.
Observed names are evidence, not a new Unicode rejection policy.
