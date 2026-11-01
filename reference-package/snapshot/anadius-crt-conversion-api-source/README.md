# CRT conversion API wrappers

Rebuilt C++20 normal-flow source for three entries in the hash-bound `anadius64.dll` image. This is a partial source module, not a standalone DLL. Native character conversion remains a required `Effects` implementation.

| Original VA | Recovered behavior |
| --- | --- |
| `0x18006ccc8` | Filters flags, then returns the complete signed 32-bit result of `MultiByteToWideChar` through IAT `0x18007f178`. |
| `0x18006cd58` | Filters flags, handles UTF default-character pointers, then returns the complete signed 32-bit result of `WideCharToMultiByte` through IAT `0x18007f1f0`. |
| `0x180066c1c` | Calls the existing recovered resolver `0x1800669b0`, procedure slot0, one candidate module16 (`kernel32`), name `AreFileApisANSI`. Invokes the resulting provider address and returns its complete 32-bit BOOL; unavailable function returns1. |

For the first wrapper, flags become zero for code pages42,50220,50221,50222,50225,50227,50229,57002 through57011,65000. Pages54936 and65001 retain only bit3 (`flags & 8`); all other pages retain every flag. The omitted pages50223,50224,50226,50228 are significant.

For the second wrapper, those zero-flag pages plus54936 and65001 receive zero flags. Other pages clear only bit7 (`flags & 0xffffff7f`). Exactly65000 and65001 force both default-character pointers to NULL. A non-NULL caller `used_default_character` receives a four-byte zero **before** the conversion call, including if that call fails. Page54936 does not clear either pointer. Input and output addresses, signed input lengths and output capacities pass through unchanged; the wrappers add no validation or error mapping.

The retained Ghidra pseudocode declares the conversion wrappers `void`. Their assembly ends with tail jumps; the provider's EAX is therefore the result consumed by callers. The rebuilt interfaces preserve all32 bits, including negative values. `AreFileApisANSI` similarly preserves noncanonical BOOL values such as `0x100`, rather than narrowing them to a byte or C++bool.

Candidate bytes at VA`0x18008b8f8`, raw`0x8a0f8`, are `10 00 00 00`; end pointerVA`0x18008b8fc` establishes exactly one entry. The ASCII name beginsVA`0x18008b900`. `collect-evidence.ps1` validates the original and retained evidence hashes, derives the file offset from PE sections, and records the bounded24-byte table/padding/name slice. No original image is loaded or executed.

`selected.asm.txt`, `selected.pseudo.c`, and `evidence.json` retain the bounded source evidence and its evidence IDs. TargetSHA256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

Run `./build.ps1` with the existing pinned LLVM-MinGW toolchain. Validation compiles one production object under C++20 `-O2 -Wall -Wextra -Werror`, then runs a static fixture executable linked to the actual recovered runtime resolver source. **3,935,369 checks pass.** Tests cover every16-bit code page against five flag patterns, selected high32-bit values, all default-pointer presence combinations, exact signed argument/result forwarding, write-before-call order, cache hits, permanently missing exports, failed loads, native-name error87 fallback and full32-bit BOOL values. Only synthetic conversion/storage/module/function-call providers execute. Original compiler, ABI and DLL-wide behavior equivalence remain unproven; exception/SEH unwinding is outside this module.
