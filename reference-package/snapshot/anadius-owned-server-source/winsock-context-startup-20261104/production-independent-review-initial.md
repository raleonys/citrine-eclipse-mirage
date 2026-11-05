# Independent WSA production review

No blocking source defect found in the four final files below. This is static comparison against this stage's `before/` copies, the 19 fresh native records and the additive retained CRT proof at `../winsock-crt-order-followup-20261105/findings.md`. It is not compile, test, link or runtime validation.

## Checked behavior

- `server.cpp:22–43` keeps common state passive (`constinit`), with an immutable typed startup/cleanup pair. Acquire increments atomically before startup, requests WORD `0x0002` only for the first owner and caches that return atomically. The caller's held flag is committed before the opaque startup. Cached-status inspection is a separate XADD-zero operation. There is no invented wait, retry, version validation or failed-acquire rollback. Release consumes the flag immediately after decrement, invokes cleanup only for old count one and ignores its returned status.
- `server.hpp:98–134` appends the explicit Original profile and borrowed state while preserving the Host default. Both backend constructors reject foreign profile/state/import combinations. The documented state lifetime must cover every retained successful or failed Core claim. Host retains its former per-context WORD `0x0202` behavior.
- `server.cpp:228–245` restores Original registry gate → shared acquire/cached error → existing scheduler/service construction. The registry gate is distinct from both existing gates. Nonzero status uses the native system category and exact `winsock` context. False initialization with error zero does not acquire a new artificial rejection. Existing phase/Parent checks and sticky failure policy remain.
- `server.cpp:303–319` retains socket/IO/domain retirement, then releases Original shared WSA before current internal service/IOCP teardown; the added registry gate is destroyed last. The former Host cleanup branch remains separate. Consumed release is not replayed after a throwing lower effect.
- `factory.cpp:90–116` checks a fresh Host/unbound selection and verifies the existing kernel pair matches the default module before provider allocation and again before publication. Thus rejection leaves the actual inputs uncommitted. It publishes Original plus the passive module state and performs no startup itself. `entry.cpp:23–35` alone registers the release callback before its unchecked module claim, ignores registration's return and performs no cached-error check. The callback follows normal compiler-CRT termination; no recovered outer CRT is replayed. The constinit common state avoids a cross-TU dynamic-initialization dependency.

## Evidence and limits

Fresh selectors: `0241b0`, `023f30`, `001310`, `029ea0` and raw/decoded `07df30`, indexed in `native-evidence-index.json` (SHA256 `840c3f8b74a300df823b5af90e87e92052ed68cfa3eaa89d9d546602238b1ddb`). Retained successful attach traversal proves slot `7f668` → `001310` occurs before application `038870`; the follow-up is additive and does not rewrite the earlier audit's timing uncertainty.

Native internal socket-service versus scheduler/IOCP destructor order is still uninspected; this change does not establish exact registry/class layout or those virtual destructor effects. Exceptional constructor cleanup, native C++/CRT/EH ABI and actual compiler/DLL execution remain unproved. No originals, frozen build, ZIP, tests or production files were changed by this reviewer.

## Final source SHA256

| File | SHA256 |
|---|---|
| `outputs/anadius-owned-server-source/server.cpp` | `bcbf7ab75389dfc833adb72eba6f120613a0e9ace205e236fb00fa1bdf21804e` |
| `outputs/anadius-owned-server-source/server.hpp` | `2a6a7ad756bb473c09e9c909bb5fc8a7f6465b44f65a3ec9022e434bcd62c452` |
| `outputs/anadius-resources-factory-source/factory.cpp` | `3210fd8e2711603aba00d1f536299a606ce164a5806d7bd1153e94fbd89b1dc5` |
| `outputs/dll-link-gate-source/entry.cpp` | `c0810bfc385c7159bdc559ad7bb3329799ca212d17ae4aba7fa49cd2243e09ca` |
