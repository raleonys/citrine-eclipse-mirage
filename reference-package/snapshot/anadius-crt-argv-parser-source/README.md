# Recovered narrow argv parser

`parse.cpp` reconstructs the normal flow of `anadius64.dll!0x1800636c0` as one independently compilable C++20 body. It is based on retained Ghidra disassembly and pseudocode bound to target SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. It is not original source, a complete DLL, or evidence of runtime equivalence.

The assembly is authoritative where the byte/sign/width and operation ordering matter. `selected.asm.txt`, `selected.pseudo.c`, and `evidence.json` retain the exact evidence identities and input digests. No target was opened or executed in this subtask; retained evidence was reused.

## API and ownership

`anadius::crt_argv_parse_recovered::parse_0636c0(input, argv, text, counts, effects)` accepts stable bounded byte input and independently optional output spans. An empty output span models the original NULL output pointer, including a non-NULL zero-length span. The input, text, pointer array, counts and effect storage must be disjoint and remain stable during the call. Output points into borrowed caller-owned text; the parser allocates and owns no memory.

`Counts::pointers` includes the terminal NULL pointer, so an empty command line returns two pointer slots: an empty program argument and NULL. `Counts::characters` includes the argument NUL terminators. A counting call passes both output spans empty. A writing call normally receives exactly those counted capacities. The two modes are also separately supported: pointer output without text stores NULL for every argument; text output without pointer output still emits the text.

The mandatory provider `Effects::is_mb_lead_06f9a8(int32_t signed_byte)` receives an explicitly sign-extended 8-bit value, as both original `MOVSX ECX, byte ptr [...]` instructions require. Any nonzero 32-bit result classifies the byte as a lead byte. The provider is called even for the first argument's copied delimiter or NUL, but is not called for quotes removed by syntax, copied backslashes from a slash run in later arguments, or later argument terminators. The provider can inspect progress but must not mutate input, outputs or counts. The locale/multibyte classifier body remains a separate dependency; this module supplies no native locale API.

## Recovered decisions

| Address | Observed behavior |
|---|---|
| `0x1800636ed`–`0x180063701` | Reset character count, set pointer count to one, then publish the program-argument pointer. |
| `0x180063708`–`0x180063775` | Program argument strips every syntactic quote by toggling quote state. Backslashes have no quote-escape role here. A copied SPACE/TAB outside quotes becomes the argument's NUL, without another character-count increment. A copied original NUL is counted and the input pointer is backed up. |
| `0x18006371f`–`0x180063750` | Program byte count increments before the corresponding write and lead-byte predicate. A lead byte copies and counts the next byte without interpreting it as quote/whitespace syntax. |
| `0x18006377b`–`0x1800637a8` | Skip only SPACE/TAB; publish each later argument pointer before incrementing the pointer count. Leading command-line whitespace therefore yields an empty program argument. |
| `0x1800637ac`–`0x1800637f9` | Count backslashes in a 32-bit unsigned register. Before a quote, emit half that run; odd runs emit a literal quote, while even runs normally toggle quote state. A doubled quote already inside quotes emits one quote and preserves the state. |
| `0x18006380e`–`0x180063842` | Later ordinary byte is written before calling the predicate and before its normal count increment; lead-byte trail is counted and copied first. This order differs from the program-argument loop. |
| `0x180063847`–`0x180063864` | Emit/count argument NUL, write final NULL argument pointer if requested, then increment pointer count for that sentinel. |

No argument wildcard expansion, environment parsing, argv allocation, locale initialization, calling-convention shim, SEH/unwind cleanup or original DLL execution is implemented here.

## Explicit model boundaries

Out-of-range input reads, missing required trail/lookahead bytes, and insufficient nonempty output spans throw `std::out_of_range`. These are **new reconstruction guards** for unsupported caller inputs, not a claim that the original function throws that C++ exception. There is no prescan, hidden count pass, transactional rollback or invented cleanup: mutations already made before a provider/guard exception remain visible. Empty output spans mean NULL, rather than zero-capacity writing buffers.

The input must contain all bytes that the selected provider causes the original state machine to consume, including lead-byte trails and a reachable terminating NUL after any consumed trail. Embedded trail bytes are copied opaquely. A normal DBCS provider does not classify NUL, quote, slash or SPACE/TAB as a lead byte; the recovered core does not add that assumption as an extra branch.

## Validation

Run `./build.ps1` with the existing LLVM MinGW toolchain. Production source compiles with C++20, `-O2 -Wall -Wextra -Werror`; the synthetic standalone suite passes **57,718 checks**. Tests include hand-written quoting/whitespace vectors, slash-run lengths 0 through 32, program-name special rules, 1,555 literal values of length zero through four encoded by a separate inverse quoting function, all four output combinations, exact capacities and untouched canaries, every syntax-permitted single-byte provider value, DBCS trails that resemble syntax, callback-visible progress, and unsupported-storage guards.

These fixtures execute only the new parser and synthetic providers. They do not execute the original DLL, a rebuilt DLL, a game, or any native API. See `validation.json` for the source and test executable digests and `collect-evidence.ps1` for reproducible retained-evidence extraction.
