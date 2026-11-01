# Multibyte system code-page and publication support

Reconstruction from static Ghidra evidence for `anadius64.dll`, SHA-256
`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
This module adds three function bodies; it is not a complete DLL or recovered
original source. The source API is a new testable interface, not an ABI claim.

| Address | Rebuilt function | Observed behavior |
| --- | --- | --- |
| `0x18006efd0` | `system_cp_06efd0` | Always enters LocaleUpdate with NULL, resets system-code-page flag, resolves -2 through OEMCP, -3 through ACP and -4 through locale-info record +0xC; conditionally clears PTD flag bit 2. |
| `0x18006ee00` | `publish_06ee00` | Locks caller-selected lock, copies type/case tables, decrements current global record reference, conditionally frees it, publishes current PTD record and increments its reference; unlocks caller-selected lock. |
| `0x18006532c` | `mark_locale_changed_06532c` | Exchanges locale-changed DWORD with 1 and returns its previous value. |

`publish_06ee00` takes references to the caller's mutable PTD and current-slot
address locals. It intentionally reloads those locals and referenced storage
at the positions used by the assembly. The old count comparison is with the
pre-decrement value 1; reference arithmetic wraps modulo 2^32. An initial static
record is excluded from freeing, but not from decrementing. There is no
same-record shortcut when old and new identities match.

The type and case table lengths are 257 and 256 bytes. Each 16-byte vector load
is represented by two ordered qword reads captured before two qword writes.
Copies proceed forward in the original 2-by-8 chunk order, including aliases;
the type-table trailing byte is read after the first 256 bytes. A NULL output
table writes errno 22 and invokes the invalid handler. A source address which
wraps to NULL first zeroes the corresponding output before the same error path.
A returning invalid handler allows subsequent operations to continue.

Effects must provide coherent bounded storage, stable identities for image
globals, and the native/heap/atomic/locale dependencies. Sparse test addresses
are owned fixture tokens; original image virtual addresses are never
dereferenced. The expanded zeroing loop captures memset byte results, not its
optimized access footprint. Split qword effects likewise do not reproduce
hardware vector atomicity, partial faults or native fault behavior. Only normal
flow is reconstructed; exceptions deliberately do not trigger invented RAII
unlock or LocaleUpdate cleanup.

Run `./build.ps1` with the already bundled LLVM-MinGW toolchain. The strict
C++20 build uses `-O2 -Wall -Wextra -Werror`; tests cover sentinel and ordinary
code pages, unsigned return bits, flag cleanup, table boundaries, 16-byte read
ordering, forward aliasing, mutable pointer locals, re-read free arguments,
counter underflow, initial-record guard, returning/throwing invalid handlers and
normal-flow exception boundaries. Only the synthetic test EXE executes.
`validation.json` records compiler identity, result and output hashes.

`selected-evidence.json` carries exact retained evidence IDs, full selected
function listings and source-listing digests. Ghidra's library match labels are
inferences; the explicit address-level instructions determine this code.
