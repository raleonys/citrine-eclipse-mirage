# Typed acceptor and socket-service diagnostic binding

SOURCE IMPLEMENTED, NOT COMPILED OR EXECUTED. Only server.cpp/server.hpp changed. Root's stage/before copies matched both files before editing. No tests, link, execution, ZIP, version.dll/original access or new REA query. Tests/review are separate assignments.

| File | Final SHA256 | Before SHA256 |
|---|---|---|
| outputs/anadius-owned-server-source/server.cpp |7aa4b341b8ef7e96e62833f320f5d30ad43386aa869155a4275e28a7b5bd2343 |db9337ccfff364b63c9dfd269822166e6d8bf04181ddf841f20543f43fa3a42d |
| outputs/anadius-owned-server-source/server.hpp |b847284e3498e43e34213ec84125197111bd27aff3d3aa9ffcaf16db540e98f1 |d34244ef4eb162705a9aa449a24db140f7b179121765da42f00f2bfce1293af7 |

## Concrete gap and change

Actual020e20 produces028620 system errors for nonzero socket-open, IOCP-attach, reuse-option, bind/listen failures; already-open1 uses0285b0/asio.misc. Actual0228e0 produces nonzero service-gate status with028620/mutex. Complete retained callers and fresh020950/compiler audits establish the remaining wrong-message-provider binding independently of the separate scheduler024990 fix.

Added the explicit overload:

`SourceError(Error, std::string_view operation, const std::error_category&, uintptr_t context_address=0)`

It constructs the standard base with the supplied category and retains typed Error, owned operation and optional numeric context. Existing contextual/address construction delegates using its unchanged selector; bare028210 construction is unchanged. The added symbol requires consistent future rebuilding, not a new exception layout/framework.

Only the two measured concrete effects now select the stable module-native category explicitly:

- AcceptRuntime::raise028240: nonzero + logical system + exact open/set_option/bind/listen operation.
- Construction's string raise028240: nonzero + logical system + exact mutex operation from0228e0.

Selection is local, not global logical-system/text classification. String effects preserve context_address0, operation, signed code, opaque token and standard catch compatibility. Already-open misc1, zero, unknown text/families, context-free, integer-only SDK and unrelated HOST paths retain their previous providers.

## Caller and prefix boundaries

Private Core AcceptRuntime is reached through actual020e20's nine raises: misc1 open, nonzero open WSA/attach, invalid-socket10009 or nonzero option/bind/listen. LifecycleRuntime supplies initialize/reset/destroy only. No generic success/code0 seed reaches those raises; unknown HOST inputs remain fallback.

SDK/error capture, close/gate/socket transfer, guards, poison/cleanup and loader/work counters are unchanged. The passive getter reuses the module pool; nonzero standard-exception construction calls020950 once. The pool outlives Core/Policy during module life. Explicit callers must keep their category alive through exception/error_code copies.

024990 scheduler gate is first;0228e0 socket-service gate is second, after scheduler/port success. A second-initialize failure regression exercises the latter while preserving its retained prefix. No normal teardown is invented.

## Evidence and limits

See evidence-links.md for exact EIDs/hashes/paths and supplementary stages. Complete retained callers already answered this binding; no ceremonial fresh query was made.

This fixes known caller/provider mismatches, not new algorithm bodies, arbitrary028210/generic/zero mappings, MSVC class/EH ABI, allocation/cleanup parity, imported-code lifetime or whole-DLL runtime equivalence. No percentage/runtime PASS is claimed.
