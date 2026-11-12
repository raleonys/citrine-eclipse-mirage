> **Historical checkpoint.** Use [VALIDATION.md](VALIDATION.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

# Windows acceptance and DLL size

The owner-provided Compile, Production, Debug, Static CRT, and Native logs
all concluded with PASS. Native passed 193/193, CRT passed 6/6, and all
six Production Golden gates passed. The uploaded DLL has the same SHA-256
as the final DLL in the Production log:

`4a0f37c9328ecf31d0815a9e9b85634bc481e77e2f07b636d4ea65bb4783f26b`

Read-only inspection of the uploaded final DLL metadata also passed:
the correct timestamp, valid checksum, no Rich/debug/COFF-symbol/certificate
data, three correct C exports and ordinals, required import ordering,
static CryptQueryObject import, and two correct read-only avatars were
verified. VERSIONINFO is the only PE resource.

Game acceptance was recorded as an explicit PASS statement from the
owner. No game name or per-scenario matrix was provided, and the game
was not rerun in this investigation. Earlier pending native/Golden/game
acceptance records remain historical evidence; the current status is
in [the acceptance record](../metadata/evidence/OWNER_NATIVE_ACCEPTANCE.json).
For that DLL, the three previously pending acceptance tasks were
closed by those specific evidence classes. This does not establish
unlimited equivalence with every original DLL function/ABI or every
OS/Consumer scenario.

## Measured difference

| Section / file | Original DLL, KiB | Product DLL, KiB | Difference, KiB |
|---|---:|---:|---:|
| Entire file | 678 | 880 | +202 |
| .text: machine code | 501 | 626.5 | +125.5 |
| .rdata: constants, tables, C++/exception information | 135.5 | 185 | +49.5 |
| .data: writable file-backed data | 10 | 26 | +16 |
| .pdata: x64 unwind records | 25 | 34 | +9 |
| .reloc | 4 | 6 | +2 |
| .rsrc | 1 | 1 | 0 |

KiB means 1,024 bytes. The table uses the PE sections' on-disk raw
sizes. The file-size difference is 206,848 bytes (202 KiB), or
29.7935%. Header sizes are equal. The product's additional 0.5 KiB
`.fptable` and the original's 0.5 KiB `_RDATA` offset each other in
the total. The five growing sections account for the full
difference. These are file measurements, not private working-set
or RAM measurements.

The product contains two valid PNGs, 4,181 + 3,882 = 8,063 bytes.
The original has 7,488 + 3,882 = 11,370 bytes. Avatar data is
3,307 bytes smaller. The resource section remains 1 KiB; the
avatar is not stored as a PE resource. The product has no debug
directory, COFF symbol table, or trailing overlay. Therefore,
the increase does not arise from avatar data, embedded PDBs,
or additional trailing file data.

## Why this is plausible, and what is established

The source includes explicitly approved additional runtime behavior:
hook 11/SAFE-FREEZE, call gates and retryable cleanup, atomic
file/rollback ownership, bounded worker/session/deadline/event
delivery management, and a listener operating for both address
families. Modern vendor versions, the standard compiler-owned
CRT/STL, template/inlining choices, and C++ exception/RAII code
also yield code generation different from the original binary.
These are concrete source-level differences capable of increasing
code and data size; they do not establish an exact attribution
of the 202 KiB among libraries.

Neither DLL directly imports a dynamic CRT, so merely citing the
product's static CRT does not explain the whole difference.
The current Golden linker profile explicitly chooses `/LTCG:OFF`;
the original's optimization/LTO settings cannot be inferred from
its DLL headers. The product has 2,869 x64 exception-directory
entries versus 2,130 in the original. That is consistent with
more unwind regions, but it is not proof of 739 independent
additional functions.

The Production and Native logs passed real target/linker-map
boundaries; no finding indicates that the 74 reference translation
units entered the product. This does not prove the absence of
all unreachable or dead code. Exact function/library-size
attribution would require linker MAP, OBJ/LIB, or PDB contribution
data from the same accepted native build; those were not uploaded.
Exact byte attribution from the available DLLs is not claimed.

The size difference is plausible given additional behavior and
the different build composition; alone it does not demonstrate a
runtime defect or unnecessary source import. This investigation
did not change runtime code, vendor bytes, linker/optimization
settings, or delivered ZIPs. Only new evidence and the current
acceptance status were recorded.

The Native log contains twelve compiler warnings at locations
belonging to test/fixture sources or their template-instantiation
chains. The Native test result was not a failure. Warning-free
compilation is not claimed; the size investigation did not modify
test code.

[Byte and section dump](../metadata/evidence/OWNER_DLL_SIZE_COMPARISON.json)
· [Final DLL inspection](../metadata/evidence/OWNER_FINAL_DLL_INSPECTION.json)
· [Raw logs and acceptance summary](../metadata/evidence/OWNER_NATIVE_ACCEPTANCE.json)
