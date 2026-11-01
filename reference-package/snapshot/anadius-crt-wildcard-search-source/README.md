# CRT wildcard enumeration source

`search.cpp` reconstructs the normal flow of `anadius64.dll!0x18006e984`
using the retained REA/Ghidra assembly and the actual sibling
`crt_wildcard_storage::append_06e800` source. This is newly written C++20,
not recovered original C++ or a complete standalone DLL.

The input is an original pattern, a position within that pattern supplied by
the caller's wildcard scan, and a three-address output vector. The source
walks backward with the mandatory DBCS-aware `previous_077168` helper until
`/`, `:`, `\`, or the pattern start. The prefix includes a separator byte.
A colon outside byte offset one returns the literal pattern without finding
files or converting text.

The pattern is converted to wide text after its own locale selection region.
Conversion failure still calls FindFirst with NULL, as the assembly does.
An invalid handle appends the literal pattern. For a valid handle, each result
gets a separate locale selection and narrow conversion. Only `.` and `..`
are skipped; no file attribute check is invented. Every other result is
appended with the original prefix. FindNext tests its complete DWORD return;
the ending LastError is not queried. If the vector length changed, sorting
receives only the new segment, including a segment with one item.

Normal completion sorts, closes the handle, then frees the owned wide
conversion. Append failure retains prior entries, frees the owned narrow
conversion, closes the handle, then frees the owned wide conversion. Borrowed
conversion buffers are not freed. No automatic cleanup is invented for
throwing providers or hardware-fault paths.

The exact comparator identity passed by the binary is `0x18006e41c`.
Lexical ordering must not be inferred from a qsort-shaped call. This module
requires `sort_new_segment`; its fixture records addresses/counts only.
The parent integration may supply a separately reconstructed sort source.

## Evidence and boundaries

`evidence.json` records original SHA-256, retained assembly/pseudocode hashes,
REA evidence IDs, decisive instruction addresses and unresolved providers.
`selected-assembly.txt` and `selected-pseudo.txt` include the complete selected
function and bounded dependency functions. `collect-evidence.ps1` validates
the retained digests before creating those excerpts.

All addresses are opaque provider-owned storage identities. `Effects`
requires the actual DBCS previous helper, locale selection/cleanup region,
conversion helpers, find operations and sorting. The source supplies no
fake production successes. A narrow conversion failure causes an immediate
NULL dereference in the original assembly; the portable model rejects that
unsupported fault domain with `domain_error`, without returning a made-up
status or freeing resources. A nonprogressing/out-of-range previous-character
provider is likewise explicitly rejected. Original SEH, cookie checking and
exact compiler stack layout remain outside this normal-flow model.

## Validation

Run `build.ps1` with the existing LLVM-MinGW toolchain. It compiles one new
production C++ object plus the sibling storage dependency with
`-std=c++20 -O2 -Wall -Wextra -Werror`, then executes only the synthetic fixture.
`validation.json` records the current source hashes and result.

The fixture checks every nonzero byte for exact separator classification,
drive and non-drive colons, a DBCS trailing-backslash case, dot/dotdot
filtering, changing code pages, full-DWORD status/FindNext decisions,
existing-vector reallocation and sort boundaries, invalid-find literal
fallback, partial append failure, buffer ownership and cleanup ordering.
No original or rebuilt DLL is loaded. No native file enumeration or original
file write occurs. These checks do not establish runtime equivalence.
