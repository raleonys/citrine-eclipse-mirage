# VCRT 128-byte PTD and lifecycle reconstruction

This is buildable C++20 recovered from retained static REA/Ghidra evidence for
`anadius64.dll`, SHA-256
`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
It is independently written source, not the original source, and is not a complete
replacement DLL. The original DLL and game were never loaded or executed.

`vcrt_ptd.cpp` covers the normal control flow of these twelve entries:

| Original VA | Rebuilt function | Behavior |
| --- | --- | --- |
| 0x18004d55c | initialize_04d55c | Locks, PTD initialization, lock rollback |
| 0x18004d584 | thread_attach_04d584 | Required allocation without abort; boolean result |
| 0x18004d598 | thread_detach_04d598 | Clear and free current PTD; true AL result |
| 0x18004d5a8 | uninitialize_04d5a8 | On nonterminating detach: PTD then locks |
| 0x18004d5c4 | late_uninitialize_04d5c4 | PTD only; true AL result |
| 0x18004d5d4 | free_callback_04d5d4 | Free every nonnull pointer except static PTD |
| 0x18004d5f4 | free_current_04d5f4 | Get, clear slot, then callback semantics |
| 0x18004d63c | get_required_04d63c | Noexit getter, abort on null |
| 0x18004d658 | get_noexit_04d658 | Preserve LastError; sentinel; calloc; publish |
| 0x18004d718 | get_noinit_04d718 | Preserve LastError; sentinel becomes null |
| 0x18004d768 | initialize_ptd_04d768 | Allocate FLS, publish static PTD, set frame state |
| 0x18004d7b0 | uninitialize_ptd_04d7b0 | Ignore FlsFree result, invalidate index |

`Ptd` is exactly 128 bytes with a signed 32-bit frame state at offset 0x78.
It is separate from ACRT's 968-byte PTD in `anadius-crt-ptd-source`.
`State` retains the initial index -1 and a caller-owned static PTD identity,
relocating original globals `DAT_1800a15e0` and `DAT_1800a4d90`.
Only the known frame-state field is named; the other bytes stay opaque.

The allocating getter places pointer -1 in FLS before calloc(1, 128), blocking
recursive allocation. It writes frame state -2 only after successful publication.
The successful allocation path calls free(nullptr), as does allocation failure.
Publication failure tries to clear FLS and frees the allocation even if clearing
fails. The free callback exempts only null and the static PTD: if handed -1, it
forwards that token to the free provider without dereferencing it. No additional
success, failure, or safety policy is invented.

Invalid-index getters return without touching LastError. Valid-index getters
save and restore it after FLS/allocation operations. Current-thread cleanup and
global initialization/uninitialization do not add error preservation. Initialization
does not check whether an index already exists. Locks and shutdown synchronization
must follow the surrounding recovered lifecycle; this module adds no hidden locks.

All FLS, heap, locks, error and abort operations are mandatory `Effects` methods.
The FLS allocation callback uses a semantic token for the rebuilt 04d5d4 shim;
it is not a preferred-base code address. `free_05caec` corresponds to the proven
direct wrapper that calls 0668c0. `_calloc_base` at 05cae4 likewise forwards to
066848. Provider return values are observed only in their recovered widths; the
boolean functions model AL and make no claim about upper RAX bits.

Run `./build.ps1` using the existing toolchain. The 565 checks cover allocation
and publication failures, sentinel reentry, visibility before frame-state write,
static PTD identity, ignored cleanup failures, rollback, abort, terminating detach,
repeated init/uninit and LastError differences. Test providers operate exclusively
on synthetic caller-owned records and pointer tokens. No native FLS, heap,
original DLL, game or provider code runs.

`evidence.json` binds retained pseudocode and assembly identities;
`assembly-evidence.txt` keeps the selected instruction sequences. `validation.json`
records compiler identity and object/test hashes. Native callback binding,
runtime initialization tables, real FLS cleanup, SEH and full CRT lifecycle
integration remain separate work. Original-source identity and runtime equivalence
are unproven. No C++ exception rollback or SEH handling was inferred.
