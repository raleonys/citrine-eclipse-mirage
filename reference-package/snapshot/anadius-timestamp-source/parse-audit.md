# Timestamp parser audit: 0x180041e10

Scope: retained static evidence only; no original execution, new REA requests, real configuration reads, or production edits. This audit closes the **classic ASCII, base-facet ISO-format branch** sufficiently for a narrowly named provider. It does not recover general STL/CRT or claim arbitrary-locale `get_time` parity.

## Evidence and call graph

Input DLL SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

Retained pseudo listing: `outputs/anadius-all-functions.pseudo.c`, SHA-256 `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`.

Retained assembly listing: `outputs/anadius-all-functions.asm.txt`, SHA-256 `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

| Function | Role | Retained pseudo evidence |
|---|---|---|
| `0x18001eb00` | Storage caller; call at `0x18001f884`, source node+0x50, result node+0xb0 | `ev_f7cbf2f5b9d3d17681ffb7f9a17c6ed5cf2952ce07087685262cb3dccfe53b3e` |
| `0x180041e10` | Zero tm, length-bearing stream, fixed format, state gate, calendar call | `ev_80df4678956b9b7658e3d0b5553defdaa1244bbcda5a6974597668481318d415` |
| `0x18003d310` | get_time extractor and locale facet lookup | `ev_f4f2a1eb680a7fef54367c61f2f66e8b895c9fcacdc2fd88f07732159de7dea9` |
| `0x18003ab40` | Stream sentry, leading whitespace and state | `ev_08fd4b0566af9e3f0ea136dd39e7b88a9172df6fd8206e34250626949e30f50a` |
| `0x18003e870` | time_get base facet factory | `ev_631fc88d3cc57123aa3a67ab95f8f9d818ae598689c4b56a5e8e6b9fd15d50e0` |
| `0x180041760` | Format runner, literal folding, early state exit | `ev_d534decdbd37d824b4edeba1aa8b87354fb56ff73514b0f0a0b5a017fcfdf952` |
| `0x18003f190` | time_get virtual directive dispatcher | `ev_a47bfad1bd8e9cd6e0bac59d9333647dcc99a349b817cd04a45e6332d2dbb91d` |
| `0x18003d8f0` | Width-limited whitespace/sign/decimal field parser | `ev_5cc1de95c661c1e2eb1d38d7c96f2d8aa607569c484e2e5f97434d7d1283b971` |
| `0x18003ef80` | Numeric wrapper used for H/M/S | `ev_5fbe31765cdd7795f98eddfcfbec7eb0678d480053ad7aa5506d40c6dca2323d` |
| `0x18000dfa0` | ctype scalar lowercase virtual slot | `ev_9b4b1a32f0a862824ec4da2540b2d55e696972bcec1b6847ddcfedb2b5d4fc98` |
| `0x18004864c` | Lowercase helper; null locale-name branch folds ASCII A..Z | `ev_f04d99b928a795f51510ab4db03476ce2913b487b026df04b614f15e1c240aec` |
| `0x18000d1c0` | ctype scalar narrow virtual slot, identity | `ev_2e35e32c5b8b27f492e61aaa51033078878993339ad9896ff295c8ccb7e89070` |

`outputs/anadius-type-recovery/msvc-rtti.json` resolves time_get COL `0x1800948e8` → vtable `0x1800869f0`. Slot9 at `0x180086a38` (file offset `0x85238`) points to `0x18003f190`, matching the `+0x48` call at `0x180041b26`. ctype COL `0x180092f28` → vtable `0x180080350`: slot4/+0x20 points to `0x18000dfa0`; slot10/+0x50 points to identity narrow `0x18000d1c0`. RTTI's overall table-length heuristic is not needed to resolve these individual slots.

## Wrapper invariants

`0x180041e41..0x180041e4e` zero 36 bytes: all nine tm fields. String input length comes from the source string's size field at +0x10 (`0x180041efb`), selected data uses the usual +0x18 capacity discriminator, and `0x180041f17` initializes the string buffer with that explicit length. Embedded NUL is therefore input data, not a terminator.

Fixed format at `0x180086978` is `%Y-%m-%dT%H:%M:%S`; the wrapper scans only the format's C terminator. It invokes `0x18003d310` at `0x180041f49`. At `0x180041f57`, `TEST state,0x6` accepts states0/1 and rejects fail2/bad4; accepted parse calls `0x180041ff0` at `0x180041f62`. Rejected parse returns zero. No full-consumption test follows extraction.

Native bits in this branch: eof1, fail2, bad4. The extractor merges the parser result with stream rdstate &0x17, adds bad4 if streambuf is absent, and honors the native exception mask. A normal bounded provider should expose the parser state separately from the wrapper's resulting integer because epoch zero and parse failure share the return value.

## Exact fixed-format numeric branch

All six directives use `0x18003d8f0`; H/M/S pass through `0x18003ef80`. Field width is derived from the maximum, **not a minimum number of digits**.

| Directive | Min..max | Width | Successful write |
|---|---:|---:|---|
| Y | 0..9999 | 4 | tm_year = value−1900 |
| m | 1..12 | 2 | tm_mon = value−1 |
| d | 1..31 | 2 | tm_mday = value |
| H | 0..23 | 2 | tm_hour = value |
| M | 0..59 | 2 | tm_min = value |
| S | 0..60 | 2 | tm_sec = value |

Year call/range/write: `0x180040324..0x18004034f`; month call/write: `0x18003fcc9..0x18003fcf5`; hour numeric wrapper call/state OR: `0x18003fc17..0x18003fc30`; minute max59 at `0x18003fd0f`; second max60 at `0x18003fdcc`. No combined calendar date validation occurs here. wday/yday/isdst remain at their initial zeros for this format.

Bounded equivalent pseudocode for **identity narrow and classic ASCII classification**:

```text
parse_number(min, max, target):
    width = max < 10 ? 1 : max < 100 ? 2 : max < 1000 ? 3 : 4
    used = 0; buffer = ""
    while !end && used < width && is_classic_space(peek):
        consume(); used++
    if !end && used < width && (peek == '+' || peek == '-'):
        buffer += consume()            // sign does NOT increment used
    while !end && used < width && peek == '0':
        consume(); used++
    if used > 0: buffer += '0'           // includes prior whitespace count
    while !end && used < width && '0' <= peek <= '9':
        buffer += consume(); used++
    if used == 0: buffer = ""            // even a sign is overwritten
    value = signed_base10_Stolx(buffer)
    state = end ? 1 : 0
    if no_conversion || conversion_error || value < min || value > max:
        state |= 2                      // target UNCHANGED
    else:
        target = value
    return state
```

Width dispatch is `0x18003d953..0x18003d978`; whitespace-count loop starts `0x18003d986`; sign handling `0x18003db34..0x18003dbfb` does not increment the count; zero count increments at `0x18003ddd1`; synthetic zero at `0x18003dca0..0x18003dca9`; `_Stolx` call at `0x18003dd0e`; final bounds/write/failure at `0x18003dec8..0x18003def3`. The native temporary buffer is32 bytes, but this ISO branch can produce at most sign+4 digits, so saturation/overflow paths do not affect this bounded domain. `-0` succeeds for min0 fields; genuinely negative values fail. Whitespace consumes width and can itself produce zero for min0 fields. The exact classic whitespace set for an explicit ASCII provider is TAB/LF/VT/FF/CR/space; arbitrary native locale ctype tables are outside the provider's claimed domain.

EOF state depends on the remaining explicit input length, independently of width exhaustion. Using all allowed digits with remaining bytes leaves state0; reaching the buffer end yields eof1. Invalid fields leave their destination unchanged. Y/m assign a local result to tm only when fail2 is absent; d/H/M/S similarly write their target only on range/conversion success.

## Format control and critical EOF behavior

Sentry `0x18003ab40` skips unlimited leading locale whitespace before the format begins. Empty/all-whitespace input reaches eof|fail3. Numeric fields may then independently consume whitespace up to their width. Literals do not skip whitespace in this fixed format.

At `0x180041800..0x180041805`, the format runner **exits on any nonzero state before checking the next format element**. If input ends while a format element still needs to begin and state is stillzero, it produces eof|fail3 (`0x18004189c`). A directive successfully reaching EOF instead yields eof1; directive epilogue `0x18004038d` ORs eof1, and the runner exits without visiting the remaining format. Since the wrapper accepts eof1, EOF immediately after an earlier valid numeric directive is accepted.

Bounded driver:

```text
tm = all_zeros
skip all leading classic ASCII whitespace
if end: return state3
state = 0
while format elements remain:
    if state != 0: return state
    if end: return state3
    if directive:
        state = parse_number(field_min, field_max, field_or_local)
        if !(state & 2): perform Y/m offset assignment if applicable
        if end: state |= 1
    else:
        if ascii_lower(peek) != ascii_lower(format_literal): return state2
        consume()
return state
```

Literal comparison invokes ctype+0x20 on both input and format, so `t` matches `T` in the recovered classic branch. Separator matching remains '-' and ':'. Trailing bytes, including NUL, are accepted after the final directive has parsed successfully. NUL before a required literal is an ordinary mismatching byte; NUL does not produce EOF.

The following are **static branch-derived expectations**, not executions of the original DLL:

| Input | State/result consequence |
|---|---|
| `2024` | eof1 accepted; year124, all other fieldszero |
| `2024-02` | eof1 accepted; year124, month1, remainingzero |
| `2024-02-03` | eof1 accepted; day3, timezero |
| `2024-02-03T04` | eof1 accepted; hour4, min/sec zero |
| `2024-02-03T04:05` | eof1 accepted; min5, seczero |
| `2024-` | eof|fail3; literal consumed before missing next field |
| `2024-02-03T` | eof|fail3 |
| `2024-02-31t04:05:60tail` | success0; per-field ranges pass; trailing input retained |
| `2024-02-03T  :05:06` | success/eof1; two hour spaces consume width and yield hour0 |
| `2024-02-03T04:05:06\0tail` | success0; explicit NUL is trailing ordinary data |
| `2024\0-02-03T04:05:06` | fail2 at the required '-' |
| `+2024-2-3t4:5:6` | eof1 accepted; widths are upper bounds, signs permitted |
| `20240-02-03T04:05:06` | fail2; year width4 leaves extra '0' before '-' |

## Provider decision and limits

A specifically named **NativeClassicIso** provider can implement the recovered fixed-format control and decimal rules above for explicit ASCII inputs under the base classic-facet assumptions. It should expose unsupported-domain results for bytes>=0x80 or unmodeled locale/stream conditions rather than silently claiming native locale parity. Non-ASCII trailing bytes are also outside such a whole-input ASCII contract, even though the native final directive can leave them unconsumed.

The wrapper should retain a mandatory parser backend boundary. A host `std::get_time` implementation is an explicit compatibility backend; agreement on canonical timestamps alone does not establish these native EOF, sign, width, whitespace, or case-folding edges. No unconditional `std::locale::classic()` invariant was established for every possible native call: the actual extractor selects a stream locale facet. Native custom locale facets, allocation and exception behavior, iterator ABI, arbitrary formats/modifiers, date-order branches, and native error propagation remain outside this bounded reconstruction.
