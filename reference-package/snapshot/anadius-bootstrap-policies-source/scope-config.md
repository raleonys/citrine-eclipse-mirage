# B7 configuration platform scope

The new `ConfigurationPlatform` binds **six** of the fourteen policies left to
B5's lower provider. B5 remains the outer `Configuration` wrapper, preserving
its seven routes: **13 of 21 selected routes bound, 8 lower policies remain**.
No new virtual provider interface or original native-body closure is claimed.

| Configuration route | Concrete implementation and observed boundary |
|---|---|
| `executable_path_03c6d0` | `GetModuleFileNameW(nullptr, ...)`, retained helper `03c7b0` |
| `program_data_03c920` | `GetEnvironmentVariableW(L"ProgramData", ...)`, retained `039bc0` |
| `local_app_data_03c6f0` | `GetEnvironmentVariableW(L"LocalAppData", ...)`, retained `039bc0` |
| `widen_0145d0` | Explicit UTF-8 codepage65001 through existing `convert_narrow_048ae8`, two passes and first-pass length |
| `narrow_path` | Existing `filesystem_codepage_048ac0` and `narrow_006e60`, matching inline `01c870` filename conversion |
| `resolve_path_0192d0` | UTF-8 conversion then existing `is_absolute_0195b0`, `parent_path_019820` and `join_016b00` when relative resolution is requested |

The environment helpers do **not** call a shell known-folder API. The retained
wrappers pass literal environment names into `039bc0`; replacing those with
known-folder lookup or fallback values would change behavior. Both native
buffer loops start with260 UTF-16 units and grow by260 until the SDK count is
strictly less than capacity. A zero return produces an empty path even if the
SDK records an error, as the retained branches show. No fabricated successful
path or silent substituted default is introduced.

The native buffer is a `std::wstring`; SDK writes never alias `char16_t` objects
as `wchar_t`. The final path copies code units. The selected compatibility
implementation preserves the last SDK call's Win32 LastError across its own
temporary allocation/destruction. This is a rebuilt boundary policy; the
original allocator's incidental LastError and errno side effects are not
claimed. SDK callbacks must obey their normal nonthrowing outcome contracts.
Growth that would exceed DWORD capacity throws `std::length_error` as an
explicit extreme-size policy, rather than wrapping the SDK capacity.

UTF-8 conversion uses explicit lengths, so embedded NUL remains data in these
value-level operations. Empty UTF-8 input does not query a codepage. Narrow path
conversion selects the filesystem codepage even for an empty path, as `01c870`
does before its empty test. Existing encoding failures, substitution rejection
and specific fallback behavior stay in the recovered encoding source. Portable
C++ exceptions, compiler string allocation and the explicit executable-path
argument replace native EH, MSVC string layout and global `0x1800a39e0` access;
none proves original ABI or full runtime equivalence.

`bind_configuration_platform(Resources, ...)` wraps only the owned config
provider. Apply it **before** B5 `bind_configuration_and_console`. Construction
performs no SDK path lookup, file access, callback registration or worker
creation. Existing parser/FilePass code remains unchanged.

Eight policies remain explicit in the lower provider: document fallback,
configuration logging, license subsystem, directory creation, embedded PNG
writing, directory enumeration, removal and achievement persistence. This
stage deliberately makes no storage success or reconstructed license claim.
No actual user CFG/token/achievement contents or blocked version artifact was
read, and no original target or rebuilt DLL was executed or modified.

## Retained static evidence

Target SHA-256:
`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
No new binary session or decompilation is required for these claims. They use
the existing full pseudocode and assembly, whose hashes remain respectively
`26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`
and
`9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

| Address | Pseudocode Evidence ID | Assembly Evidence ID |
|---|---|---|
| `0x1800145d0` | `ev_b7669ec63c588e3e0c3c3ea478118b765778ea8ceeaac18635081dec14844c4d` | `ev_57ba03c192271ff8c347201fe9ae78a51f6632cff0e5a891c609f018940e6aee` |
| `0x1800192d0` | `ev_f97924b6178fd1c17ef606a5e5153562cafc5f04e36eecb08e5cdb93f2a31286` | `ev_aeef0b48642561e9aa088e2e457d40ad95a8159f366b6578e122c959b494e3a0` |
| `0x18003c6d0` | `ev_ba410f99cd0312ac92597ec449496bc2ce3eecfccc66c8f885b9c05f4b586a2d` | `ev_a25bcb6c136d83d915ae0affe5aa508e2c257aaf5193287cbff852b374d11743` |
| `0x18003c6f0` | `ev_cdae4b9b195f2bd0c3b2ecf95a78f67285dadd02ff0b1173c5af0f58698d7f79` | `ev_651ded00d21fdb7e5a9835dcbdc4f5844ee80600929155f268af3d7f708b137d` |
| `0x18003c920` | `ev_79c12e3986d5d1dafc3ee8c3b30a5bfba0c6175c8706b19219fa84deddd096bc` | `ev_037c4e2de199069d759e25ae4d49959902cb9789a34f27c2ca1cb66d01544094` |
| `0x180039bc0` | `ev_72d75fc9fa12d5c38ef034c259adc6d350c8c6000666bab5cd18fc7be06eb529` | `ev_86ba6440a8aa53a23ef4afa027e52749cacaf3379338fdd6370a3d8d0d08c130` |
| `0x18003c7b0` | `ev_17ad912a154a63c866cd96d402845c0371db388a41f12981ec499db6cd784d9f` | `ev_d8977d7a67dd2c948e4482f0547e26f9d9ef13e7e01747bb0f231017608f506c` |
| `0x18001c870` | `ev_78a1faedf06f56d422e6ecc197bbe90ce7f34af0e9b01420eef8b6b8827bf25e` | Existing configuration evidence ledger |

Additional path and encoding callees retain their own module evidence files.
The new adapter's address-suffixed names are composition entries, not new
canonical native function mappings.

Production compile check: `clang++ -std=c++20 -O2 -Wall -Wextra -Werror -static
-c config_platform.cpp` passed. Independent core/integration and common build
results are recorded by the B7 delivery owner, not inferred from this compile.
