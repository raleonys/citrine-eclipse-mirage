# Multibyte control reconstruction

This module recovers the normal flows of `0x18006f2d0` and the multibyte initializer `0x18006f5f8` from the exact supplied `anadius64.dll` SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. `evidence.json` contains the bounded retained Ghidra pseudocode and assembly with their original evidence identities and full retained-corpus hashes. This is reconstructed C++, not the original author's source or a complete DLL.

`set_multibyte_06f2d0` invokes the thread updater before code-page resolution, allocates 0x228 bytes only when the resolved page differs, snapshots the old record before writing the replacement, resets its reference count, and calls the mandatory record-builder provider. Only full EAX equal to `0xffffffff` means builder failure. Other 32-bit statuses survive the entire successful path. Failure writes errno 22 and frees the replacement. Allocation failure calls free(NULL) and preserves the allocator's error effects.

The successful path marks the locale changed only when the input initialization byte is zero, decrements the currently reloaded PTD record's reference count, conditionally frees a freshly reread PTD record, installs the new record with one reference, and publishes its global type/case tables only when `(locale_mask & ptd.flags) == 0`. It calls the actual reconstructed support helper with lock ID 5, sets the initial global pair only in initialization mode after publication, and finally calls free(NULL). The source deliberately preserves observable reads around provider callbacks.

`initialize_multibyte_06f5f8` initializes case/current-record/type globals in machine order, obtains the required PTD, calls the setter with request -3 and initialization byte 1, and sets the initialized byte even when allocation or record building fails. Repeated calls skip all initialization effects. The original guarantees AL=1; this API returns `bool` and makes no claim about the original upper RAX bits.

## Build and verification

Run `./build.ps1` from PowerShell. It uses the existing pinned LLVM-MinGW toolchain, C++20, `-O2 -Wall -Wextra -Werror`, compiles the production object, and runs only the generated synthetic fixture executable. The linked `multibyte_support.cpp` is the actual reconstructed code-page/publish/changed-flag support source.

The fixture passed 128,916 checks: equal-page and allocation/build failure exits, full status comparison, all 0x228 copied bytes, complete-read-before-write snapshot ordering, zero/one/two old references including unsigned wrap, initial-record free exclusion, flag-mask publication, three initialization-byte values, actual table copies and lock pair, global-pair timing, callback mutation of PTD/source/status/error cells, initialization failure marking, and idempotence. The fixture address space owns bounded byte arrays; it does not read process or user profile memory.

## Remaining providers and limits

`build_multibyte_06f674` is a mandatory provider boundary; its builder implementation is not claimed here. Thread updating, allocation/free, raw PTD acquisition, atomics, lock calls, native code pages, and address-space storage are also required bindings; the root integration can bind recovered source modules. Tests use synthetic providers for those dependencies. The fixture does not establish native SDK behavior, original fault behavior on invalid addresses, SEH unwinding, or binary ABI equivalence. Neither original DLL nor game is loaded or executed. Original inputs are untouched.
