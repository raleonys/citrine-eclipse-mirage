# Wildcard result storage: `0x18006e800`

This module reconstructs one local function from the retained Ghidra 12.1.4 static analysis of `anadius64.dll`. It appends a newly allocated narrow string to a three-qword pointer vector. It is an independently compilable C++20 body with mandatory storage providers; it is not the original source or a complete DLL.

`storage.hpp` exposes `anadius::crt_wildcard_storage::append_06e800(source, prefix, prefix_bytes, vector, effects)`. All `Address` values are opaque 64-bit storage identities. The `Vector` fields represent the original begin, end and capacity-end qwords. The function never casts original preferred virtual addresses to host pointers. `read_word` is available to consumers sharing this vector facade; append itself only needs `write_word`.

## Observed behavior

- At `0x18006e82e–0x18006e848`, scan the source to its zero terminator, include that terminator, and return `12` if that byte count exceeds the bitwise complement of the prefix byte count. No allocation or free occurs on this early return.
- At `0x18006e86a–0x18006e8b1`, request `calloc(prefix_bytes + source_bytes_including_NUL + 1, 1)`. Copy the prefix only when its length is nonzero, then copy the source including NUL. The final extra byte comes from zero initialization. The extra allocation `+1` has no separate overflow check in the observed assembly; the unsigned wrap is retained.
- The character allocation result is **not checked for NULL** before copying. Any nonzero 32-bit copy result invokes the mandatory nonreturning Watson policy. This path adds no frees or C++ cleanup.
- At `0x18006e8b7–0x18006e8f7`, an empty vector receives four zeroed pointer slots, with a visible `free(NULL)` after publishing `begin` and before reloading it. The pointer allocation failure returns `12` after freeing the just-created text and the final NULL cleanup call.
- At `0x18006e8f9–0x18006e951`, a full existing vector uses wrapping subtraction followed by signed arithmetic shift right by three. Negative signed distances become unsigned values above `INT64_MAX` and fail. Otherwise call `recalloc(old_begin, 2 * slot_count, 8)`. A failed reallocation preserves the vector and frees the text. A successful call publishes relocated begin/end/capacity, then calls `free(NULL)`.
- At `0x18006e956–0x18006e968`, append the text address to the current end slot, increment end by eight, call `free(NULL)`, and return zero. There is no appended NULL pointer sentinel here; later packing is a separate function.

The named allocator/copy/free/Watson operations are required effects. This source does not claim their native implementation or ABI has been supplied. Providers must implement zeroed allocation, reallocation preservation, bounded source length excluding NUL, storage range checks, and the chosen fatal policy. Opaque integer address arithmetic preserves the target's 64-bit wrapping operations; invalid storage remains a provider-domain error, not invented native success. Corrupted-vector and enormous-address fixtures examine call arithmetic with explicit opaque providers and do not allocate enormous buffers.

## Evidence and validation

`evidence.json` identifies the target SHA-256, complete retained exports, selected excerpt digests, and claim ranges. `selected.asm.txt` is authoritative for register widths and call order; `selected.pseudo.c` is corroborating decompiler output. `collect-evidence.ps1` verifies both retained file identities before re-extracting the bounded excerpts. No binary was opened, executed, loaded or changed for this module.

Run `build.ps1` with the existing llvm-mingw toolchain. The standalone strict C++20 build passed **14,148 checks** (`-O2 -Wall -Wextra -Werror`). Synthetic fixtures cover content/NUL placement, prefix lengths, all three allocation states, pointer preservation across growth, guard boundaries, initial/reallocation failure cleanup order, unchecked text allocation failure, and both copy fatal branches. No original or rebuilt DLL/game, OS search provider, native heap, or actual Watson termination is executed. These checks establish the source model's behavior under its fixtures; they do not establish binary equivalence or whole-DLL completion.
