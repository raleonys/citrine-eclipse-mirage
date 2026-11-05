# Accept sidecar growth — source implementation

Scope: `outputs/anadius-accept-source/owned.cpp` and `owned.hpp` only. No compilation, project tests, link, DLL execution, package update, or REA session was performed by this agent. Native evidence is the separate root-owned bundle in this directory. This is a source-owned metadata implementation, not a claim of native ABI/container identity.

The constructor's capacity is now a positive initial allocation, at most 65536; it need not cover the full base table. On an index beyond current coverage, the sidecar doubles its `deque<Row>` until the requested base slot fits, subject to container limits. Existing Row, AsyncAcceptOperation, Task, packet-field, and actual OVERLAPPED addresses remain stable. Existing generation and submission validation, authoritative accept buffers, pending/pin checks, SDK sequencing, and native retry/completion flow remain in place.

Growth holds the existing recursive gate and requires the existing Parent Operation. A bounded `growing` reservation rejects source reentry (including snapshot traversal) while deque metadata is being appended. After allocation the code rechecks Parent admission, sticky state, and the original base slot/generation before returning a row. Added allocation bookkeeping preserves Win32 LastError. Failure during append/postcheck poisons the sidecar; no reusable retry guarantee after allocator failure is asserted. Empty appended rows may remain in a failed prefix, while existing row objects remain at their original addresses. Close declines during growth.

After installing accept dispatcher metadata and `op.packet`, project calls the base's keyed `bind_external_packet(storage, op.packet)`. Base table relocation updates that exact stable field before freeing its old packet table. Retirement first applies existing pending/pin checks, then always calls alias-only `unbind_external_packet(old->packet)` before destroying the variant. This safely handles a base-cleared null field, including a prior base free, without dereferencing stale OperationStorage. The final base API also forgets a registered null-tampered field; a foreign nonnull field is rejected. Existing dispatcher entries naming the retired operation are then cleared.

Borrowed table-span APIs are documented as valid until base growth; persistent consumers and live-view wiring are the root's separately reviewed change, not duplicated here. No new stop/unload conditions or registry framework were introduced.

Static implementation inspection covered append stability, no Row reference acquired before index growth, generation recheck, post-append failure state, base-free/null alias cleanup, project/retire ordering, LastError scope, and the unchanged pending/retry/completion checks. Independent review was requested from the root's source reviewer. This note does not claim runtime validation.

SHA256, production source:

- `owned.cpp`: `ec4beab122e4402f8163ce2d8a95c9a4970bccac3d6a9a3575883a15bbed9edc`
- `owned.hpp`: `b77f2418372bab73dd313a62f4a92b0686e622cf575fbcd44a05c0e542686d13`

Preserved before-copy SHA256:

- `owned.cpp.before.txt`: `aaa6e0c1cb126a1ac6b6515a5062a75a4c1f4cfa38bf2e7debccc8941732324c`
- `owned.hpp.before.txt`: `011cf682eb0f9ffb30575ded5a184ca366ea88bf605421ee24eda5cfb72c74f9`
