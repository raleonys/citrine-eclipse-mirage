# B7 Services allocation and final return

`SourceServices` selects three existing `bootstrap_owner::Services` routes:
`allocate`, `deallocate`, and `guard_return`. With B3's six application routes
and B4's two registrar routes this yields 11 bound routes and six remaining
policies: log insertion, log finish, thread error, CRT exit-wait cleanup,
terminate logging, and unexpected-terminate-return handling. Unselected methods
continue to the mandatory owned lower provider. `00e310` already reaches the
existing host initialization body through B3 and is not counted again.

## Actual domain ownership

Two prior production files, `anadius-thread-domain-source/domain.hpp` and
`domain.cpp`, are deliberately extended. `DomainStorage::make_memory()` may run
only in Ready on the original nonreentrant controller thread. It constructs an
`ApplicationMemory` with the same provider-lease mechanism as B6's actual thread
factory. The lease retains the root and prevents FLS/lock/runtime teardown while
the memory provider lives. No heap state or effects reference escapes publicly.

`ApplicationMemory::allocate` uses existing `malloc_base_06a308` and
`call_new_handler_0634b0` with that root's actual heap, error/PTD binding,
encoded handler, and ACRT lock 0. Nonzero allocations use flags 0; the existing
malloc source normalizes zero to one. Deallocation calls existing
`free_0668c0` through a stack observer of the same effects object. Size is ignored
as in the retained delete route.

Per-provider source tickets are allocated before calling the native allocator.
The ticket gate is released across native calls and user new handlers. It is
not the original CRT allocator layout or a new native function. Known null
outcomes remove the ticket; a throwing handler with a known null result may
propagate. A throwing injected heap boundary with an unknown outcome poisons
and retains the provider. Foreign, already released, concurrently released, or
other-provider pointers are explicitly outside this bounded ownership domain.

Known HeapFree failure still executes the recovered errno mapping, retains the
block, and raises a selected compiler `system_error` so the controller can retry.
This retry signal is an intentional rebuilt-adapter difference from the native
void free route. An unknown free outcome cannot be retried. Destructor with any
ticket or poison is terminal. This also exposes the original bootstrap's
unreleased payload on failed thread creation; it does not silently invent cleanup.

Worker calls do not read mutable domain phase. Callers must use the original
controller or workers created through this
same domain's recovered factory, with a present domain PTD. Foreign/CRT-only
threads and shared fallback-error/OOM concurrency are not established here.
B3/B4 must exclude new admission, join every worker including FLS destructors,
and release Runtime/Services/thread
providers before root close. Memory provider destruction must not race calls.
Direct users must enforce that same contract. Provider existence is not an
automatic worker join or automatic cancellation. Concurrent allocation does not
establish general CRT, new-handler reentrancy, DLL unload or loader-lock safety.

## Selected exception and extent policy

Exhausted handler uses the rebuilt compiler's `std::bad_alloc`; size `SIZE_MAX`
uses `std::bad_array_new_length`. This matches the source-level error choice,
not original MSVC exception object/EH ABI. Extents above PTRDIFF_MAX but at or
below original malloc maximum `0xffffffffffffffe0` remain explicitly rejected
by the existing native byte-array bridge; no enormous synthetic allocation is
performed. Above that original maximum the malloc-failure/handler path performs
no native allocation. No over-aligned allocation or cross-provider free support
is claimed. Internal tickets use the compiler CRT independently of returned
application storage.

## Retained static evidence

Target SHA-256 is
`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
No new native session, decompilation, original execution, or original-byte write
was needed; retained full assembly and existing recovered source were inspected.

| Address | Static observation | Assembly Evidence ID |
|---|---|---|
| `0x18004b0bc` | malloc then explicit handler retry; exhausted SIZE_MAX branch | `ev_4b9d54867bdd71cc2cd07945df16f49f84fdecaa3c3535386e97bd20f64624ec` |
| `0x180060044` | `_malloc_base` jumps to `06a308` | `ev_660e9d43688c584c1b643f3db3b1648df0d8479f4a4c0042af86c26725db0708` |
| `0x18006a308` | flags 0 heap allocation, zero normalization and malloc failure | `ev_fec2970bf0d499ba0715ef0f1dd0028bfcd6d27a246fd3e25106a2d5f9825b64` |
| `0x1800634b0` | explicit selected handler with requested size | `ev_feeb37c1659fa2ad6ed36fb2e17ad1622a1893e7adb59a2a704c97c32c5c11ba` |
| `0x18004b0f8` | jumps to `04b788`, then `05caec`, then `0668c0` | `ev_c742d3bdc5602e2293140e3408473befa826c5e7f02f988ef02afb1709aa3adf` |
| `0x18004b788` | intermediate delete thunk | `ev_9c1d7141369ea0b1b9c0ca37156bcfddbb25e1a30aaf0a61f46b64920290d220` |
| `0x18005caec` | final thunk to free | `ev_dc2e8860cfa1a06b464563b31c9a5f586dc49fb16cd3579156e95db913e59895` |
| `0x1800668c0` | null no-op; free failure maps LastError through errno | `ev_c99f204c542cccdad396d7bc71515f1b5731e73657cfccf84d0a121761e73e4a` |
| `0x180017a30` | exactly `RET 0x0` | `ev_4ef5a2bb53b7728331fdd5c8debedafc4f7ebcce78b3dd4a30154920bfc23a9d` |

The actual RET justifies an empty source `guard_return`; it does not implement
PE Control Flow Guard policy. Native thread-error `04a648` uses an indexed
MSVC error table and C++ throw machinery. It remains lower-provided rather than
being guessed from a Win32 error number or the compiler's platform errno table.

These are composition-only bindings and do not add mapped native bodies,
native closures or original ABI equivalence. Full DLL readiness remains open.
