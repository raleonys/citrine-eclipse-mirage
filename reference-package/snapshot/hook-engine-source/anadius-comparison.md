# anadius64.dll hook engine: bounded static comparison

The 21 retained functions below strongly correspond to the selected official MinHook source. **No behavior-changing divergence was identified in this bounded manual review.** The DLL's original exact MinHook release/commit remains unproven. Vendored v1.3.4 is a selected compatibility dependency, not recovered original source.

Target SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`; preferred base `0x180000000`. Function prototypes, Evidence IDs, source line references and retained pseudo/assembly hashes are in [anadius-comparison.json](anadius-comparison.json). No target DLL, detour, thread suspension or network operation was executed. Source is the [official pinned commit](https://github.com/TsudaKageyu/minhook/tree/c3fcafdc10146beb5919319d0683e44e3c30d537); exact local file digests and license are retained in [upstream-provenance.json](upstream-provenance.json). JSON line ranges refer to local verbatim files; web rendering removes blank lines.

| Original address | Selected upstream correspondence |
|---|---|
| `0x1800460e0` | `EnableAllHooksLL` |
| `0x180046230` | `EnableHook` |
| `0x180046350` | `EnableHookLL` |
| `0x180046460` | `EnumerateThreads` |
| `0x1800465d0` | `Freeze` |
| `0x1800466e0` | `MH_CreateHook` |
| `0x180046970` | `MH_CreateHookApiEx` |
| `0x180046a00` | `MH_DisableHook` |
| `0x180046a10` | `MH_EnableHook` |
| `0x180046a20` | `MH_Initialize` |
| `0x180046ac0` | `MH_Uninitialize` |
| `0x180046b80` | `ProcessThreadIPs (+ FindOldIP/FindNewIP inlined)` |
| `0x180046d30` | `Unfreeze` |
| `0x180046db0` | `AllocateBuffer` |
| `0x180046de0` | `FreeBuffer` |
| `0x180046e60` | `GetMemoryBlock (+ FindPrevFreeRegion/FindNextFreeRegion inlined)` |
| `0x1800470c0` | `IsExecutableAddress` |
| `0x180047100` | `UninitializeBuffer` |
| `0x180047150` | `CreateTrampolineFunction` |
| `0x180047500` | `IsCodePadding` |
| `0x180047540` | `hde64_disasm` |

## Layout and statuses

Entry stride is 56 bytes: pointers target/relay/trampoline at 0/8/16, backup[8] at 24, flag bits patchAbove/isEnabled/queueEnable at byte 32 bits 0/1/2, nIP low nibble at 36, oldIPs[8]/newIPs[8] at 40/48. This matches the observed MSVC layout, not automatically every compiler's bitfields. At creation, detour means the generated relay.

Globals are lock 1800a3f50, heap 1800a3f58, block list 1800a5a50, entries 1800a5a60, capacity 1800a5a68, count 1800a5a6c. Creation allocates 32 entries (0x700 bytes), doubles capacity, stores disabled/unqueued flags, and backs up 5 or 7 bytes. Status values 0..12 align with selected MH_STATUS: success, already initialized, uninitialized, already created, absent hook, enabled, disabled, nonexecutable, unsupported, allocation, protection, absent module, absent procedure. Original symbol names were not recovered.

## Critical branch matches

* **Single target disable still passes Freeze action 1.** ASM 1800462ef copies R14D=1 to R8D before the call 1800462f9, irrespective of requested enable. Selected local hook.c:742 does the same. All-target operation1800460e0 uses action 0/1 from enable. Changing this apparent quirk would diverge from both. Assembly Evidence `ev_58ec25b3623b400a9a6229dc204b2316245fb88510d6875095865b6748224dc5`.
* **Enumeration failure is status 9; individual thread failure is not.** 180046460 selects same-process threads excluding current, checks record size covers owner PID, allocates 128 DWORD IDs and doubles. Thread32Next termination needs last error 18; other errors free the list and fail. 1800465d0 opens with access 0x5a, zeroes IDs whose OpenThread/SuspendThread fails, and continues successfully. Get/SetThreadContext failure does not alter status. 180046d30 resumes only nonzero IDs and frees the list. Evidence IDs are recorded per function in JSON.
* **IP mapping uses exact instruction boundaries.** 180046b80 uses CONTEXT_CONTROL=0x100001; enable maps target+oldIPs to trampoline+newIPs. Disable reverses this and recognizes target-5 hotpatch and relay entries. Already matching state is skipped; other action values use queueEnable.
* **Init/cleanup preserve failure order.** 180046a20 spins with Sleep(0) for 32 retries then Sleep(1), calls HeapCreate(0,0,0), returns 9 on failure or 1 if initialized. Its buffer-init no-op is folded to sharedRET180017a30. 180046ac0 disables all first, then only on success frees pages, entries and heap and clears globals. Absent heap returns 2.
* **Patch restoration failure does not override success.** 180046350 returns 10 only for failed initial VirtualProtect. It writes E9 rel32 plus optional EB F9 or restores 5/7 bytes; subsequent protection restore/cache-flush results are ignored before flags update.
* **ApiEx does not load a module.** 180046970 calls GetModuleHandleW then GetProcAddress, returns 11/12 respectively on failure, stores ppTarget if supplied, and calls CreateHook.

## Buffer and trampoline limits

ASM 180046e92/ea7 proves origin **plus/minus 0x40000000 bytes (1 GiB)**, clamped to system bounds, with maximum reduced by 0xfff. Pseudocode plus/minus 0x8000000 on 8-byte pointers describes the same range. It reuses in-range blocks with a free slot, then searches granularity steps downward before upward. New pages use VirtualAlloc(4096,0x3000,0x40). There are 63 slots of 64 bytes beginning at page+0x40; last-slot release unlinks/frees the page. Assembly Evidence `ev_e3d393c9e8e4afd15dc6f438d135712742122ecc05a0fd642eed597b80fbff1f`.

1800470c0 requires MEM_COMMIT and protection mask 0xf0, but does not check VirtualQuery's return before inspecting its output. Selected upstream shares that invalid-input/uninitialized-output corner; a deterministic safe failure would need an explicit domain restriction.

180047150 rejects decoder F_ERROR 0x1000; handles RIP-relative displacement, direct CALL(16-byte absolute sequence), JMP(14 bytes), external Jcc(16-byte inverted condition), internal branches into the first 5 bytes and RET termination. External LOOP-family branches are unsupported. It rejects changing length within a pending internal branch, relocated size>**50 bytes** at 1800473c7, and **nIP>=8** at 1800473d4. A 14-byte relay follows within the 64-byte slot. Short bodies require equal-byte padding 00/90/CC, or sufficient two-byte entry plus executable five-byte padding above. These match selected trampoline.c. Evidence `ev_0a525d7badb74c559af7e9edb527c614ab9543e37412f464811df6724beb334b`.

## Exact decoder table corroboration

A read-only PE section conversion, after verifying the complete target digest, maps VA 1800a1140 to fileoffset**0x9f740**. All**529 bytes** equal the pinned table64.h array. Both table SHA-256 values are:

`a35a7be4500f9100d6e014c666513cbb7ba52e4f7010eb230baefb638eac5033`

The 37-byte packed decoder output,16-prefix loop, ModR/M/SIB/immediate/displacement/error flags and length>15 clamping to 15 match selected HDE64 structurally. The unused raw rex byte remains zero while rex_w/r/x/b are filled, also upstream behavior. Table equality does not prove exhaustive opcode equivalence or the exact MinHook release.

## Differences and remaining boundaries

No concrete behavior-changing divergence was found in the selected paths. **Source surface differs:** RemoveHook, queued enable/disable/apply, StatusToString and four-argument CreateHookApi bodies were not identified in the bounded retained block. Full upstream definitions add compatibility APIs; they are not recovered bodies.

Ghidra's void guesses for Enable/Disable tail wrappers and AllocateBuffer are type artifacts: assembly preserves return status and returns the slot in RAX. Likewise its decoder byte return does not establish the source prototype. Inlining/shared no-ops prevent one-source-symbol-per-address claims.

Compiler flags, complete original ABI, exact release, exhaustive opcode behavior and real concurrency remain unproven. This comparison neither ran native hooks nor establishes a complete working rebuilt DLL.

