# Automatic listener port policy

The owner selects the Brownfield ascending listener range: **3226 through 3236,
inclusive**. This is a deliberate product policy over the original RE/Bluefield
configurable port path. The reference is a source policy, not proof that the
consumer rejects all other ports.

`AsioService` tries each candidate with the existing loopback address and backlog.
On Windows each candidate uses `SO_EXCLUSIVEADDRUSE` before binding, preventing
`SO_REUSEADDR` from sharing an already listening endpoint between service
instances. The pure RE server projection still passes its original reuse flag;
the concrete Windows adapter enforces exclusive ownership for this fixed range.
Each attempt owns an IPv4 acceptor and, when the host supports it, an IPv6-only
`::1` companion on the same port. Both use exclusive ownership. A failed bind or listen
releases that candidate before the next attempt. Only successful bind **and**
listen of both available families publishes the listeners. A companion conflict
also releases the IPv4 candidate before advancing. Unsupported IPv6/address
unavailability permits IPv4-only operation; other errors retain the stated policy. Occupied ports advance to the next candidate.
Windows can report exclusive or reserved ports as `access_denied`; that error is
also skipped inside this fixed range. Other errors fail immediately. If no
candidate succeeds, startup throws with the range in the diagnostic. There is no
port-zero bind or fallback to another range.

The selected real port is still obtained from `local_endpoint`, stored in
`configuration::State::server_port` and published as `EALsxPort` before the source
server-ready flag. Failed selection does not publish a fabricated port or ready
state. Zero in the initial C++ state now means **not bound yet**.

Both config application routes stop reading `Emulator/ServerPort`. Old keys,
including `default`, `random`, numeric or invalid text, are ignored like other
unused keys. They neither override the selected range nor trigger numeric errors.
No standalone CFG or embedded VDF config was present in the preceding source
package; there is no newly shipped port-setting template. Regression fixtures
retain obsolete keys only to verify their lack of effect.

The existing connect callback is unchanged, including its IPv4/IPv6 handling,
legacy port3216 comparison, selected-port state, waits and narrow counter lock.
The later broad audit found the IPv6 redirect had no live IPv6 listener. The
concrete adapter now supplies the same-port loopback companion; this repairs the
already-approved IPv6 path without changing the connect predicate or range.

## Source provenance

The supplied Brownfield archive
`HF20_REGISTRY_B_ARCH_B_STAGE03_ISSUER_SCOPE_FULL_SOURCE_CANDIDATE_NOT_RELEASE.zip`
(SHA-256 `2f32febfcd7ed330cb84b7fcf876ae4bdc8037ce4d7826c921f775c7b6dc6e67`)
sets `first_port=3226`, `last_port=3236` in
`src/runtime/release_server.cpp:215` and iterates these candidates in the active
`src/runtime/asio_release_bridge.cpp:148` path. That path retries address-in-use.
The current application adapts this range policy in its existing Asio/RE owner;
it does not copy the surrounding server/model application. Retrying access-denied
for exclusive/reserved candidates is a documented compatibility addition.

## Verification

`listener_port_contracts` uses real Windows sockets and exclusive blockers to
occupy the whole range. It checks exhaustion without an arbitrary fallback,
unchanged readiness/environment on failure, first-port selection, fallback to
the inclusive last port, service greetings over both address families, IPv6-only candidate conflicts,
release of the failed IPv4 candidate and reuse after shutdown. IPv6 assertions
are explicitly skipped only if a separate host capability probe reports the
family/address unavailable. Connection refusal remains a failure. It runs serially because it intentionally occupies all eleven ports.
Config tests cover ignored legacy keys in scalar/hash routes and main/override
files; the original partial-state failure test now injects a path-resolution
failure rather than relying on removed numeric-port parsing.

Execution results and platform limits are recorded in
[AUTOMATIC_LISTENER.json](../metadata/evidence/AUTOMATIC_LISTENER.json).
