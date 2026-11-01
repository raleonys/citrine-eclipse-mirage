# Production Asio IOCP transport — preloader_l 3.2.5 SQUALERIS HOTFIX 16

## Shipping ownership and legacy boundary

Windows `ReleaseServer` uses `start_release_asio_windows`, `drive_release_asio_windows`, `stop_release_asio_windows`, and `shutdown_release_asio_windows`. Asio 1.38.2 owns acceptor, session, cancellation, read/write callbacks, timer deadline and event delivery. IOCP is mandatory; `ASIO_DISABLE_IOCP` is disallowed.

Production uses WSA startup/cleanup and last-error through a lifecycle-only `SocketRuntime` adapter; no legacy `open/bind/listen/accept/send/recv/close` callbacks in the shipping provider. Legacy worker/acceptor/listener/endpoint/transport TU's are excluded from the Production graph. Native fault-injection and Debug retain compatible legacy code only where test coverage requires it. Windows socket OS imports remain expected. Physical 0x548 service model ABI and public exports do not change.

## Measured acceptance

- Receive idle: 30.0s (IDS06 Windows source-linked acceptance, two profile runs).
- Absolute logical frame completion: 10.01–10.02s (IDS06 two profiles); one-byte trickle cannot reset the absolute deadline.
- Post-timeout reconnect / 24 reconnect cycles, shutdown drain: PASS in both IDS06 profiles.
- IOCP write backpressure 5.009s `/MT`, 5.015s `/MTd`: IDS07 native backpressure harness PASS. Native test-only ShowIGO prepared send timeout 5.015s with timeout_count=1, parallel peer/no event leakage PASS.
- Full IDS08 source QA 8/8 + exact ZIP clean extraction 8/8 PASS; Windows path/locale 4/4 PASS on real MSVC x64.
- Production/Golden/Debug/Native/Static CRT previously PASS with Windows shipping DLL SHA-256 `5f442bc1fd740e3bac907d67142be50a50ce5dfce9120796c0e4a0760179d939`. HF16 release packaging does not require a fake fresh Windows build when no binary-impacting source input changed; carry-forward freshness is explicitly conditional.

## Explicit limitations

Game(s) reported normal by user; exact CollegeFB27/Madden27 instrumented full event/ACK matrix is not demonstrated. Native backpressure harness plus prepared-send injection does not prove a shipping Response/Event remained kernel-pending in `async_write`. These are **not** open IDS08 source-QA gates; new integrations must validate affected runtime surfaces separately. For provenance see `docs/RELEASE_PROVENANCE.md`.
