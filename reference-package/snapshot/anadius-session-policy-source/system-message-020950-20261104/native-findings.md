# Native system-category message020950

Fresh static REA6.8.0/Ghidra12.1.4 audit, 2026-11-04. binary_session first verified aligned actual server identity and available supported Ghidra. The agent opened its own isolated immutable `C:/Users/Skysect/Downloads/anadius64.dll`, retained full structured replies, exported six successful Evidence records (31261 bytes), and closed that session successfully. SHA256 at open and after close: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

No target execution, production/test source edits, compilation, tests, linking, package update, network, version.dll access, original-byte write, provider reconfiguration, or Ghidra annotation.

## Exact category binding

Fresh028620 pseudo and complete98-byte listing establish return object `0x1800a1110`, with its existing thread-static guard sequence atA38E8. Object read atA1110 gives first QWORD `0x180084248`. The exact96-byte table/literal read at84248 gives virtual slot+0x10 (84258) = **`0x180020950`**. Slot+8 is020940; its name bytes at84278 are `asio.system`. No sibling category is substituted.

This binding uses fresh evidence, not only a historical table assertion. Other QWORDs in the object/table are not inferred to be owned C++ fields or callable HOST pointers.

## FormatMessage call and buffer initialization

Complete020950 listing spans `0x180020950–0x180020a62`,275 bytes. It first zeros the local allocated-buffer pointer (`020964`), sets size0 and argumentsNULL (`020969–020977`), then calls imported **FormatMessageA** at `020989` with:

| Argument | Observed value |
|---|---|
| dwFlags |0x1300 |
| lpSource |NULL |
| dwMessageId |incoming DWORD parameter3/R8D |
| dwLanguageId |0x400 |
| lpBuffer |address of initialized-null local pointer |
| nSize |0 |
| Arguments |NULL |

The local Windows headers identify0x1300 as ALLOCATE_BUFFER0x100 + IGNORE_INSERTS0x200 + FROM_SYSTEM0x1000.0x400 is MAKELANGID(LANG_NEUTRAL0,SUBLANG_DEFAULT1). These symbolic interpretations supplement the observed immediate values, not an assertion about any current OS's localized output.

Exact local declarations: `work/toolchain/extracted/llvm-mingw-20261020-ucrt-x86_64/include/winbase.h:1445` (DWORD WINAPI FormatMessageA signature),`:1133` (HLOCAL WINAPI LocalFree),`:1451,1454,1457` flag constants; `include/winnt.h:959,1102` language constants. Headers were read only; no LLVM tool or target ran.

## Counted trim, then C-string scan

The DWORD FormatMessage return count is **not** the final copied string length.

1. At02099c/02099e, count0 goes directly to fallback. No message-buffer byte is read on that branch.
2. For nonzero count,0209a0/0209a4 inspect buffer[count-1]. If it is LF0x0a,0209ae writes NUL at that position, decrements the effective count, and0209b5 goes to fallback if it is now0.
3.0209bc/0209c0 inspect the last effective character for CR0x0d. If matched,0209c7 writes NUL there;0209cf goes to fallback if that removal empties the counted result.
4. There is no loop repeatedly stripping whitespace or CR/LF. It removes at most one trailing LF, then at most one trailing CR in that order. A single LF, single CR or sole CRLF uses fallback. Repeated LF/CR bytes can remain. A leading/embedded NUL is not itself a fallback condition if neither counted trim empties the result.
5. For the normal copy branch,0209d6–0209e0 zeros all32 bytes of the output string representation.0209e4–0209f8 then scan the buffer from its first byte to the **first NUL** with an incrementing64-bit counter.0209fd calls006b30 with that C-string scan length, not FormatMessage's count. Thus an embedded NUL would shorten the copied string even if the API count is larger; this is static branch semantics, not an observed OS output example.

Nonzero-count output is dereferenced as supplied by FormatMessage; the body does not add a separate null-buffer validation. Do not invent arbitrary invalid provider outputs as supported native behavior.

## Fallback and normal effect order

Fallback020a04–020a43:

- zero output's32-byte representation;
- call04b0bc with allocation request0x20 at020a17;
- publish returned pointer, length0x11 (17), capacity0x1f (31);
- copy exactly `asio.system error` and write its terminating NUL at pointer+0x11.

The exact fallback literal is retained in the84248 table/literal read, at84288–84299. It is not a formatted decimal error code, a compiler generic-category message, or a string based on GetLastError.

At02098f–020997, RBX captures the buffer pointer returned through the local after FormatMessage, before any branch. Both successful-copy and fallback normal paths converge at020a47/020a4a and call **LocalFree(saved pointer)**, then return the output pointer. This call is present even when FormatMessage returned0 and left the initialized pointerNULL. If FormatMessage returns0 but writes a nonnull pointer, the normal fallback branch still frees that saved pointer. The free's return value is ignored.

Output construction/allocation therefore happens while the message buffer is still retained. Copy006b30 precedes LocalFree on success; fallback allocation/publication/literal writes precede LocalFree on failure/empty-trim. There is no GetLastError call in020950, and no evidence here of preserving the caller's LastError around FormatMessage/LocalFree.

## Exception and ABI limits

The normal body's LocalFree is explicit and after output construction. These observations do **not** recover MSVC EH/unwind funclets or establish what happens if006b30/04b0bc throws. Do not infer that an unobserved native unwind either leaks or frees the buffer, and do not claim added HOST RAII cleanup as original evidence. Original std::string object layout/SSO allocator ABI is also distinct from the selected rebuilt typed string policy.006b30 and04b0bc are called source helpers; no new claim of their full exception semantics is made by this audit.

## Complete retained evidence

| Full reply | EID |
|---|---|
| pseudo-180020950.json |ev_0274fa3cd88e934ca61666cd5c1171f80edce8cbc7cd1a7dbe6e851dfdf2f0aa |
| instructions-180020950.json |ev_0c36cbad633abd179eac63d16b40d5c52eabd28764feead934ec5968b478eefc |
| pseudo-180028620.json |ev_6ef7ee9e0cc7a42ee4977f336dc25f81c0f6d4180250698785767507f5185aa7 |
| instructions-180028620.json |ev_7c5b18d757eb4f0424a1535980d1fed4454fe7db4e6d1ec4fff682824f6aa048 |
| bytes-1800a1110.json |ev_eb486a9416f6cbfeed05d5815675c4a981ffb8d4bb337a32016db6ebf9b5f4aa |
| bytes-180084248.json |ev_c1a2d4b167be4a0479021d9a9f8526b20e0227eae9471a9457ad44bef111188a |

session.json/open.json/export.json/close.json retain the full tool replies. `rea-evidence-bundle.json` contains all six Evidence records; its zero recorded Unknown objects do not remove the EH/ABI/runtime limits above. No analysis session remains open for this agent.
