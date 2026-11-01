# CRT error cells and OS error mapping

`errors.cpp` recovers four functions from the hash-bound Windows `anadius64.dll`:

| Original VA | Source | Observed behavior |
| --- | --- | --- |
| `18005d650` | `errno_from_os_error_05d650` | 45 exact table pairs, then unsigned range fallbacks |
| `18005d698` | `map_os_error_05d698` | Write DOS code, map, independently look up PTD again, write errno |
| `18005d704` | `dos_error_cell_05d704` | PTD+`24` or global `1800a17ac` |
| `18005d724` | `errno_cell_05d724` | PTD+`20` or global `1800a17a8` |

Ghidra calls both accessors `__doserrno`. Their instructions select different
offsets; the names here describe those observed cells. No imported CRT accessor
replaces either lookup.

The mapping table is at VA `180089f70`, file offset `88770`, length 360 bytes.
Its SHA-256 is `11ed605c378b270d3886b5648fb8e8abd6d7e45a4c37e88e3bb333c6317cdb36`.
The fallback globals initially hold **errno 12 and DOS 8**, matching image bytes
`0c00000008000000` at file offset `9fda8`.

Each accessor calls the required `Effects::optional_ptd_064d08` once. A mapping
calls it twice, allowing different PTDs or null results. It writes DOS before the
second lookup, so a throwing second dependency leaves that write observable.
There is no added rollback or cleanup. Effects returns null or live borrowed
storage covering offsets `20..27`; ownership and TLS/FLS lifecycle stay external.
`Cell32` uses memcpy rather than aliasing a DWORD, signed int or unaligned byte
fixture as a uint32 reference. The recovered global state remains caller-owned;
this module adds no synchronization absent from the instructions.

Run `build.ps1` for **4,151 checks**: the complete low error domain through 4095
(including all 45 table entries), unsigned edge values, both range boundaries,
unaligned/native cells, every PTD/global selection pair, lookup exceptions and
integration with the actual recovered `begin_thread_05e8e0`. The launch failure
uses this mapper before releasing its synthetic module/context; invalid entry
writes errno 22 before the synthetic invalid-parameter handler and leaves DOS.

Production contribution: **one C++ object**, one ordinary synthetic test group.
Standalone tests compile the existing thread and bootstrap core as dependencies.
No DLL, game, hook or external OS API is loaded or executed. Original CRT ABI,
SEH, PTD creation/destruction and real thread integration remain unproven.
Exact retained REA evidence IDs, instruction anchors, binary data hashes and
input hash are in `evidence.json`; compiler invocation is in `validation.json`.
