# Brownfield metadata audit

The owner's instruction to check all metadata against
Brownfield, including Compilation Timestamp, was applied.
The former decision to preserve compiler timestamp,
linker, and Rich-header values was superseded.
The authority is the actual MUSTAVEX/ARYRA/URSYNEX
policies in the supplied Brownfield archive,
not parser fixtures.

`metadata/evidence/REFERENCE_METADATA_POLICY_AUDIT.json`
records exact comparison of all 19 declared policy groups
against Brownfield. The goal is not to make every PE
byte identical to the Brownfield DLL: code, addresses,
image size, and three explicit owner exceptions differ.

## Fields in the final DLL

| Field | Expected value |
|---|---|
| `machine` | `"0x8664"` |
| `pe_format_magic` | `"0x020B"` |
| `subsystem` | `2` |
| `os_version` | `[6, 0]` |
| `subsystem_version` | `[6, 0]` |
| `linker_version` | `[14, 0]` |
| `timestamp` | `"0xCD268EB1"` |
| `export_internal_name` | `"preloader.unsigned.dll"` |
| `rich_header` | `"ABSENT"` |
| `additional_resource_assets` | `[]` |
| `VERSIONINFO.fixed.dwSignature` | `0xFEEF04BD` |
| `VERSIONINFO.fixed.dwStrucVersion` | `0x00010000` |
| `VERSIONINFO.fixed.dwFileVersionMS` | `0x3FE24D7B` |
| `VERSIONINFO.fixed.dwFileVersionLS` | `0x9AF25405` |
| `VERSIONINFO.fixed.dwProductVersionMS` | `0xB54B7381` |
| `VERSIONINFO.fixed.dwProductVersionLS` | `0x01A10EC2` |
| `VERSIONINFO.fixed.dwFileFlagsMask` | `0x0000003F` |
| `VERSIONINFO.fixed.dwFileFlags` | `0x00000000` |
| `VERSIONINFO.fixed.dwFileOS` | `0x00040000` |
| `VERSIONINFO.fixed.dwFileType` | `0x00000002` |
| `VERSIONINFO.fixed.dwFileSubtype` | `0x6A1AAE62` |
| `VERSIONINFO.fixed.dwFileDateMS` | `0x00000000` |
| `VERSIONINFO.fixed.dwFileDateLS` | `0x00000000` |
| `VERSIONINFO.strings.FileDescription` | `EA Javelin Anticheat` |
| `VERSIONINFO.strings.LegalCopyright` | `Copyright 2023 Electronic Arts. All rights reserved.` |
| `VERSIONINFO.strings.ProductName` | `EA Javelin Anticheat` |
| `VERSIONINFO.strings.ProductVersion` | `3.9.5` |
| `VERSIONINFO.string_table` | `040904b0` |
| `VERSIONINFO.translation_language` | `0x0409` |
| `VERSIONINFO.translation_codepage` | `1200` |

Timestamp **0xCD268EB1** is Brownfield's fixed target value,
not the wall-clock time of the build. A PE viewer may
display it as a date using the viewer's local time zone.
Verification compares the raw 32-bit value.

## Other checks

| Group | Rule |
|---|---|
| Debug | No Debug Directory, RSDS, REPRO, or COFF symbol table; no packaged PDB |
| Signature | No Authenticode/certificate/security table |
| Resource | One VERSIONINFO leaf; no PNG/RCDATA/manifest; Brownfield RC bytes unchanged |
| Avatar | Owner's PNG in read-only C++ data; independent RE fallback retained |
| Sections | `.text,.rdata,.data,.pdata,.rsrc,.reloc`; /MT UCRT profile adds `.fptable` after `.pdata` |
| `.fptable` | Index 4, virtual 0x100, raw 0x200, characteristics 0xC0000040; writable/non-executable and all-zero data |
| Structural validity | Alignment, file extents, no raw/virtual overlaps, and RVA ordering checked |
| Excluded targets | Other section addresses/sizes, entrypoint RVA, and entire image size are not required to match Brownfield |
| Identity | Brownfield's declared blocklists for older project names/versions remain the same |
| Export | Preserve the owner's five C exports; internal module name `preloader.unsigned.dll` |
| Import | Owner's six modules, with only `CryptQueryObject` from CRYPT32; no OLE32 |
| CRT | Verify actual MSVC /MT, /GS cookie, and executable CRT entrypoint |
| Integrity | Compute PE checksum; reject byte mutations during read-only Golden stages |

## File under inspection

Do not modify the raw
`build/.../preloader.unsigned.dll`.
URSYNEX can modify only the four-byte timestamp,
two-byte linker value, Rich block, and four-byte
checksum in a separate candidate copy.
OS/subsystem version 6.0 and absence of the Debug
Directory are preconditions; they are not silently
removed to conceal a bad build. Reject finalization
if any other bytes change.

After all Golden checks and the final-named runtime
test, write `dist/preloader_l.dll`, its SHA-256,
and receipt. Renaming the file does not change
the internal export-module name. Inspection of
the old raw unsigned file does not show the final
metadata outcome.

```powershell
python -B tools/inspect_dll.py dist/preloader_l.dll --final --report build/final-metadata.json
```

This read-only check reports MUSTAVEX/header/checksum/
resource/avatar and the static CRYPT32 result.
The full Production pipeline is required for
ARYRA/CORVEXON/MSVC gates.
Raw DLL inspection without `--final` identifies
RAW_LINK_OUTPUT / NOT_FINALIZED differences.

## Evidence boundary

The policy comparison and negative PE regressions
were verified.
`metadata/evidence/REFERENCE_METADATA_PE_PROBE.json`
checks header/VERSIONINFO finalization on an explicitly
labeled **diagnostic fixture** created from the actual
LLVM development DLL. Only in preparing that test
copy were the development Debug Directory/COFF symbol
pointer fields cleared; the Production finalizer
does not do this. Because LLVM section/import/CRT
details differ, the fixture is not Golden acceptance
and is not copied to dist. The revised source
requires actual Windows/MSVC Production testing.

A subsequent owner instruction added a complete import
descriptor permutation to the permitted URSYNEX scope.
The Brownfield header/VERSIONINFO/layout requirements
were retained. The updated comparison is in
`metadata/evidence/WINDOWS_IO_REFERENCE_POLICY_AUDIT.json`;
the earlier 19-group report is the snapshot of the
previous task.
[Current change](WINDOWS_IO_FIX.md).
