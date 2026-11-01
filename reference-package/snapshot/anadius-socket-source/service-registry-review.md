# Service registry frozen source review

Result: **PASS**, no material findings in the two new production files under the documented initialized, finite, borrowed graph and stable lifetime domain. Reviewer: `service_registry_audit`. This was an independent static review against retained full pseudocode and assembly; the reviewer did not execute tests, original DLLs, game executables, sockets, IOCP or user files.

## Reviewed files

| Source | SHA-256 |
| --- | --- |
| `outputs/anadius-socket-source/service_registry.hpp` | `5584fc37f54fa847efee7aa350fb51d2f82033d8826cca762f8e6e9f27752525` |
| `outputs/anadius-socket-source/service_registry.cpp` | `663e6f4799e9173722e0af62b05d7a4e96d069f4f0dc55b6777cc46e037cbc02` |

## Normal original source behavior

`initialize_service_key_022fa0` writes exactly opaque type identity `0x1800a2620` and ID zero. It does not dereference native addresses. `resolve_service_0239c0` creates this key internally, so no artificial input makes the native generic ID fast path reachable. The matching helper preserves the native nonnull guards and comparator argument order `entry.type+8, fixed.type+8`; equality is determined by the mandatory provider's uint32 result being zero, rather than by host pointer equality or an invented name table.

Both `find` loops initialize from the live head after enter and advance through the current entry's next field after comparison returns. A comparison that changes that next field therefore changes traversal, matching `023a5a/023ac9`. The first winner is cached across gate.leave.

First miss leaves the registry gate before mandatory `create_socket_service_0228e0(context)`. Candidate key assignment precedes second enter. The explicit null guard is accurately documented as an unsupported host domain boundary; native key stores dereference a null result and do not return null successfully. The implementation adds no fallback service or fabricated allocator.

The second search reads the head again. A final miss uses live `registry.head` in `fresh->next=registry.head` after all comparison callbacks, then publishes `fresh`, leaves and returns the cached candidate. A second winner invokes `fresh->scalar_delete(1)` under the second gate, leaves and returns the cached winner. Deleter mutations to registry.head do not cause an unintended new head lookup. The host virtual deleting effect is an explicit selected typed interface, not a native vtable overlay.

Only these **two original normal source procedures** are counted: `0x180022fa0`, `0x1800239c0`. Helper matching/traversal and composition methods do not add original procedure counts. No RAII gate guard or additional exception recovery is introduced. Native unwind/SEH and callback failure cleanup remain unproven.

## Selected composition

`RegistryLifecycleEffects` implements the prior lifecycle resolver with the actual recovered lookup. It validates the configured context identity, then maps the resolved entry through a borrowed table requiring nonnull service and close context. Native context[0], registry, entry and service layouts are not reinterpreted as C++ objects. Entry/service bindings must be unambiguous, stable and caller-owned. A missing projection is an explicit UnsupportedInput boundary.

The close method selects the corresponding service/CloseContext projection and calls actual `cancel_and_close_026fb0`, allowing the existing lifecycle, cancellation and recycler implementations to participate. It adds no second registry or cancellation engine gate. Executor cleanup remains a required delegated provider. The table owns or destroys none of the entry/service/context objects. Unsupported ambiguous/stale bindings, concurrent unsynchronized list mutation, invalid entry lifetime and reused factory results are outside the valid domain.

## Remaining proof boundaries

`0228e0` native constructor and `04d2d0` native CRT RTTI comparison are mandatory providers. Native service allocation/construction/destruction internals, registry teardown, allocator, scheduler/dispatcher, executor, lifetime/concurrency, exception/SEH/unwind and entry/export/DLL ABI are unproven. This stage is neither a reconstruction of native object memory layout nor evidence of full original DLL equivalence. CRT implementation remains deferred.

The supporting retained flow audit is `work/service-registry-flow-audit.md` and `.json`; it records exact address ranges, original evidence IDs, verified listing digests and normal-flow limitations. Production, tests and build files were not edited by this reviewer.
