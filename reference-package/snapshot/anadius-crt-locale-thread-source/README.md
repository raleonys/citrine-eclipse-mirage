# CRT thread locale / multibyte source reconstruction

This module reconstructs four normal-flow function bodies from the retained REA/Ghidra evidence for `anadius64.dll`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. It is independently compilable C++20, not original recovered C++ or a complete DLL.

| Original VA | Source function | Established behavior |
| --- | --- | --- |
| `0x18006f540` | `update_thread_multibyte_data_internal_06f540` | PTD flags `+0x3a8` and non-null locale `+0x90` preserve a thread-specific multibyte pointer `+0x88`; otherwise update under lock 5. |
| `0x18006f658` | `update_thread_multibyte_06f658` | Acquire required PTD, pass the identity of current global multibyte pointer cell `0x1800a59d0`, tail-call the internal function and preserve its RAX result. Ghidra's `void` return is corrected from assembly. |
| `0x180070968` | `update_thread_locale_070968` | Acquire required PTD; preserve a non-null per-thread locale when masked flags match; otherwise update from the current global locale under lock 4. |
| `0x1800709d8` | `update_locale_nolock_0709d8` | Reject null destination/replacement; store replacement before adding its references; release the captured old locale and free only a non-static old object whose post-release count at `+0x10` is zero. |

`0x18006f597..0x18006f5b4` implements an atomic decrement using the **previous** unsigned 32-bit refcount; only a previous count of 1 can free the old object. The static initial multibyte identity `0x1800a1c70` is exempt from free. `0x18006f5b9` reloads the live current pointer after release/free effects, stores it in PTD, and increments that object's count. The source retains the captured result across unlock; it does not reread PTD after an unlock callback.

`0x1800709fc..0x180070a29` establishes store/add/release/check/free order. The locale helper returns the captured replacement pointer even when a provider changes the destination later. `0x1800709a1` reads current global locale **after** locking. Both update wrappers abort after unlocking when the final result is null (`0x18006f5d0..0x18006f5f2`, `0x1800709b3..0x1800709d0`).

All memory operations use required abstract providers; the C++ never dereferences numeric binary addresses. `current_multibyte_slot_0a59d0()` returns a live pointer-cell identity, while `current_locale_0a52e0()` returns current pointer contents. The deep reference/free bodies `0x180070634`, `0x1800708c0`, `0x1800706c0`, global identities, PTD lookup, synchronization, atomic operations, heap free and abort remain required providers. There are no production success stubs.

Run `./build.ps1` with the existing bundled Clang toolchain. The standalone test result is **15,204 checks PASS** using synthetic bounded address maps and observable providers. Tests cover mask combinations including high bits, null locale overrides, equal/replaced/null pointers, static object exemptions, count wraparound, previous-value atomic behavior, global mutation under locks/free, source write order, post-release count mutation, preserved return values across provider callbacks, repeated PTD acquisition, and abort-after-unlock ordering.

Synthetic throwing providers represent explicitly unsupported fault or exceptional paths in two tests, confirming no invented RAII cleanup. The implementation claims only normal control flow. A replacement current multibyte pointer of null reaches the original atomic dereference; the source does not add a new guard. Real SEH, original unwind behavior, production concurrency, original ABI/compiler equivalence and full native locale implementation remain unproven. Only rebuilt test code ran; no original DLL or native locale/heap provider was loaded or executed. Inputs were read only.

`selected.asm.txt`, `selected.pseudo.c` and `evidence.json` retain bounded source evidence and parent bundle SHA-256 values. `validation.json` records compiler identity, options and test/object digests.
