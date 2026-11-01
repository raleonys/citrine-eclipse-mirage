# Service registry retained static flow audit

Scope: original normal procedures `0x180022fa0` and `0x1800239c0`, with `0x1800228e0` constructor/provider and `0x1800211b0` caller context. Reviewer: `service_registry_audit`. Original DLLs, games, user configuration, tokens, achievement stores and networking were not executed or read. The two complete retained listing SHA-256 digests were independently checked.

## Fixed key and search

`022fa5/022fb6` writes the opaque native RTTI descriptor identity `0x1800a2620`; `022fb9` writes zero to the identity field. `0239e0..0239f0` first zeros the local 16-byte key then invokes this helper. The name inferred by Ghidra is `asio::detail::typeid_wrapper<asio::detail::win_iocp_socket_service<asio::ip::tcp>>`. This is identity evidence, not a native RTTI object reconstructed by host C++ RTTI.

The registry is borrowed context state: `0239fd` enters the registry gate and `023a08` reads the live head at native `registry+0x30`. Key values are cached in RSI/RDI at `023a0c/11`. The native entry projection has type at `+0x08`, identity at `+0x10`, next at `+0x20` and virtual table pointer at `+0x00`. Both identity-fast-path fragments (`023a20..31`, `023a97..a8`) require the fixed requested ID to be nonzero; the requested ID is always zero, so neither can select an entry here. Nonzero entry IDs must not accidentally produce a shortcut.

For nonnull entry type and nonnull requested type, `023a45/49/4d` and `023ab8/bc/c0` call `04d2d0` with **entry type + 8 as left/RCX**, **requested type + 8 as right/RDX**. Only the returned low 32-bit EAX is tested; zero matches. Pointer equality is not an independently implemented matching rule. The compare effect is required even when both type identities happen to be equal. A null entry type skips the comparator.

After each failed comparison, `023a5a` / `023ac9` loads the current entry's **live** next pointer. A comparator may mutate this link; caching next before the call changes observed normal effect order. On first-search match, `023b17/1a` leaves the gate and `023b20` returns the cached selected entry. A gate-leave callback changing the head does not change that winner.

## Creation, second search and insertion

First-search miss leaves the gate at `023a66`. `023a71/74` invokes the mandatory `0228e0` provider using the original context argument. The factory is outside the registry gate. `023a79` caches the returned candidate and `023a7c/81` copies the complete 16-byte key into candidate+8 **before** the second gate enter at `023a88`. A factory null return is dereferenced by native key assignment; there is no native successful null/empty-result path. A host explicit UnsupportedInput boundary may reject that unsupported state but must not claim to recover a native exception or null-success contract.

Second enter is followed by a fresh live head read (`023a8e`). The second search follows the same fixed-key and RTTI rules. If no match remains, `023ad2` re-reads the registry head **after the last comparator callback**; it does not insert against a cached initial head. `023ad6` assigns that live value to candidate.next, `023ada` publishes candidate as registry.head, `023ae1` leaves the gate, and the cached candidate returns. The head read and next assignment must remain live and ordered.

If a second-search winner is found, the loser is discarded **while the second registry gate is held**: `023b05..13` fetches candidate.vtable slot 0, sets EDX to 1, and calls that deleting effect on the candidate. The winner remains cached in RBX. The code then leaves the gate and returns RBX, even if the discard effect changes registry.head. No recovered unlink, duplicate-key cleanup or allocation rule beyond this deleting call is established.

## Context and explicit boundaries

`0228e0` requests 0x80 bytes from `04b0bc`; initializes context, scheduler/service link, mutex (`027be0`), error category (`028620`) and the final service vtable; initializes its service socket-list head at `+0x78`. `023b30`, allocator, gate construction and failure cleanup are provider boundaries. This stage does not count `0228e0` as recovered.

`0211c8..d6` obtains registry from context[0], passes the context argument to `0239c0`, and stores its returned service before socket field initialization. `021281..12ac` subsequently enters that returned service's **separate** `+0x50` gate and links the socket under its `+0x78` list. A reconstruction can compose a recovered resolver with existing lifecycle/closing/cancellation/recycler sources; native registry/service memory overlays, allocator, scheduler, executor, RTTI implementation and export/entry ABI are not implied.

Only normal effect order is established. Native exception/SEH/unwind behavior, including factory/comparator/deleting/gate failures and native lock cleanup, is unproven. The reconstruction must not add RAII exception guarantees and describe them as original behavior. Input graph entries, service objects, gates and context bindings must remain valid and caller-owned. No full DLL equivalence is claimed.

## Retained evidence

| Procedure | Pseudocode Evidence | Assembly Evidence |
| --- | --- | --- |
| `0x1800239c0` | `ev_c935d546ffcfd9a015759eba276b6b86808a598ce42947d5ddbe6ef939153abb` | `ev_245079b916f12d89c98287073bb82e89b00650e14f7fa967621e26699d2344a4` |
| `0x180022fa0` | `ev_a9b75f1a923d0914dee2554ddc6d7d96c4d1579172c994ac25615f02b63b747b` | `ev_a68b9b82503b9c9f1b9172f2e3d616c2b12fc05a0e32ab0ac818548e3209af0a` |
| `0x1800228e0` context | `ev_22c4bffe54fe0b986545326a4020802e4e98041a480e8cafc64b603ecd05d1ef` | `ev_cccd315f694dc43aeae87a61fc8912482989a03586a119141e1b2a2d8f2bdd7b` |
| `0x1800211b0` caller | `ev_0a99c668ad187f12ba22bcb5a084965bb3619bfa398a64dd105f65647c8a7c33` | `ev_632ecefa1757518f06b13606a94918118593165c553357228e6939dd09997b9d` |
