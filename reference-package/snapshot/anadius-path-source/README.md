# Windows lexical path source

`paths.hpp` / `paths.cpp` reconstruct six path routines from the static `anadius64.dll` evidence. They use the existing `anadius::configuration::Path` (`std::u16string`) and live in `anadius::path_recovered`. They do not call `std::filesystem`, query the host filesystem, open target files, or normalize paths.

| API | Original VA | Behavior recovered |
| --- | --- | --- |
| `root_name_length_00bb30` | `0x18000bb30` | UTF-16 root-name parser: ASCII drive letters, UNC server, device prefixes, both separators |
| `is_absolute_0195b0` | `0x1800195b0` | Drive paths require a separator after the colon; other nonempty root names are absolute |
| `filename_00e120` | `0x18000e120` | Skip root name and all root separators, then scan back from the end |
| `parent_path_019820` | `0x180019820` | Preserve the root prefix; discard the final component and its preceding separator run |
| `append_016b50` | `0x180016b50` | Absolute/different-root replacement, rooted replacement after the left root name, relative append |
| `join_016b00` | `0x180016b00` | Copy the left operand and call append |

The filename and parent routines use the stored string length, not a terminating NUL search. Embedded NUL and unpaired UTF-16 surrogates therefore remain ordinary code units. No `.` / `..` reduction, slash replacement, case folding, or UNC share special treatment is added. A single leading slash does not make `is_absolute_0195b0` true. `//server` does. Root-name comparison in `0x180017e20` is exact and case sensitive: joining `C:a` with `c:b` replaces the first path with `c:b`.

Append preserves the original separators and inserts a backslash only when required. For example, `C:/a` plus `/b` produces `C:/b`; `//server/share` plus `/b` produces `//server/b`; `a` plus an empty path produces `a\`. A bare drive root `C:` plus `b` produces `C:b`.

## Self-alias boundary

The assembly captures the right operand's pointers before changing the left operand (`0x180016bcb`–`0x180016bf1`). With the same path object on both sides, rooted truncation writes NUL into that saved source before the append (`0x180016c5b`–`0x180016c6e`, `0x180016ce8`). Thus self-append of UTF-16 `/a` yields `{NUL, 'a'}` with length two. The source preserves this quirk, along with absolute self-append and self-append that requires no separator insertion.

For self-append that inserts a separator, the saved source may refer to old storage after `0x1800121a0` grows the original MSVC string. Its outcome depends on original capacity and allocation state, which `configuration::Path` does not carry. Such input throws the new `OutOfEvidenceAliasing` exception before changing the value. That exception is a reconstruction boundary, not an error observed in the DLL. `join_016b00(path, path)` copies first and remains supported for these values.

## Validation and limits

Run `build.ps1` with the workspace's portable LLVM-MinGW compiler. It compiles with C++20, optimization, and warnings as errors, then executes only the generated synthetic test program. `validation.json` records **328 passing checks** against fixed path/decomposition/join expectations, including drive-relative, UNC/device, mixed separator, embedded NUL, surrogate, long-path, and alias cases. These tests do not execute the original DLL or establish runtime equivalence with it.

`evidence.json` binds the original SHA-256, retained assembly/pseudocode digests, function Evidence IDs, and the instruction ranges behind the behavior. MSVC object layout, allocator behavior, capacity growth, exception ABI, extreme length failures, and operating-system path resolution remain outside this module.
