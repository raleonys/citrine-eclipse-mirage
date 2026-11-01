# Narrow CRT environment reconstruction

This module reconstructs five Windows x64 function entries from the retained, hash-bound `anadius64.dll` analysis. It supplies independently compilable C++20 control flow; required platform, allocation, conversion and fatal-handler dependencies remain pure provider interfaces. No original DLL or game is loaded or executed.

| Original VA | Rebuilt function | Evidence-supported behavior |
|---|---|---|
| `0x180063a70` | `initialize_063a70` | Skip if narrow environment already exists; initialize multibyte state; convert and build; publish initial/current pointers on success only. Return low EAX `0` or `0xffffffff`. |
| `0x180064054` | `thunk_initialize_064054` | Actual assembly is a tail `JMP 0x180063a70`; the decompiler repeats the body. |
| `0x18006f9e4` | `convert_block_06f9e4` | Acquire UTF-16 block; count through its final terminator; query conversion size; signed EAX-to-64-bit allocation; convert; release the acquired wide block on each normal path. |
| `0x180063b54` | `build_array_063b54` | Count and copy all nonempty entries whose first byte is not `=`; allocate `(count+1)` pointer slots with 8-byte stride; preserve duplicates, order and strings without an equals sign. |
| `0x180063d80` | `free_environment_063d80` | Skip NULL array; free each entry in increasing order until NULL, then free the array. |

`environment.cpp` preserves the observed calls to `free(NULL)`, including two on a string-allocation failure after freeing the partial array, and the additional initializer cleanup. Conversion failure frees the allocated buffer; successful conversion instead calls `free(NULL)`. The return of `FreeEnvironmentStringsW` is ignored. On failed array construction, neither global is written: assembly at `0x180063ab2..0x180063ac7` skips both stores, even if a provider mutated their relocated state during the call. The decompiler's apparent self-assignments are not used.

The conversion provider represents `0x18006cd58` with code page 0, flags 0, NULL default-character and NULL used-default arguments. Those constants are implicit in the typed interface. A query's nonzero signed 32-bit result is sign extended before the malloc dependency; a negative result is not silently normalized. No actual code-page conversion is claimed by this module.

`Buffer` and `Environment` carry bounds and ownership metadata added by this reconstruction. Their C++ layouts are not original ABI layouts. Providers must return live byte buffers and live, zero-initialized pointer arrays; allocations must remain valid until their observed free calls. `State` relocates the two original pointer globals, rather than binding preferred virtual addresses. Provider lifetimes and synchronization remain the caller's responsibility.

Malformed/unterminated spans, insufficient provider capacity and unterminated pointer arrays raise explicit `std::domain_error` guards. These are unsupported-input boundaries of this bounded model, not recovered original validation. No SEH/unwind cleanup has been invented: throwing providers propagate directly, and a nonzero `strcpy_s` result calls the required nonreturning Watson provider without publishing the current string or adding cleanup. Wide environment initialization, later environment mutation, multibyte initialization internals, the conversion helper, native providers and heap composition remain separate work.

Run `./build.ps1` with the supplied local llvm-mingw toolchain. `validation.json` records the source and synthetic-test executable hashes. The fixture suite checks precise normal-path ordering, all four initial allocation failure positions, conversion acquisition/query/fill failure, signed allocation size, empty and drive-prefixed blocks, duplicates, partial-array cleanup, fatal copy behavior, publish behavior under provider mutation, and 128 generated multi-entry blocks. Tests read no process environment.

`evidence.json` and `selected-assembly.txt` tie these statements to the original artifact and retained analysis hashes. Library names emitted by Ghidra are labels, not proof of an exact original CRT version or source identity.

Input byte/wide spans must remain stable for the entire call, including allocator/copy/provider callbacks; bounds and capacity metadata stay valid throughout. Arbitrary concurrent or aliased mutation of input blocks is outside the supported reconstruction domain.
