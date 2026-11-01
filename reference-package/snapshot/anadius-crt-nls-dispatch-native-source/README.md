# NLS dispatch: Windows SDK boundary

`native.cpp` provides the typed host calls required by the recovered `0671d4` / `0672c0` dispatch chain. It is new reconstruction glue, not another recovered function body or a claim of original class/ABI equality. `Calls` implements `crt_nls_dispatch_bridge::NativeCalls`; `LCMapStringW` is a direct SDK import and the Ex/LocaleNameToLCID function pointers come from the caller's recovered resolver. An original image virtual address is never a callable host pointer.

All Ex arguments survive intact: locale pointer, full DWORD flags, source pointer, signed source/destination counts, version pointer, reserved pointer and pointer-width `LPARAM` sort handle. The return remains full `int` or `LCID`. With empty tracked output the native buffer pointer is forwarded unchanged.

Nonempty tracked cells are supported for `LCMAP_SORTKEY` bytes only. A positive capacity with a nonnull destination uses an aligned `WCHAR` allocation as raw byte storage; cchDest remains a byte count. On success exactly the returned byte count becomes defined, including any terminating zero. On failure every potentially written cell becomes undefined, rather than treating zero-initialized staging or partial native output as known evidence. Null destinations remain null for native validation. Capacity-zero tracked queries pass a null native destination and materialize no bytes. Other mapping flags with tracked cells, negative tracked capacities, spans smaller than capacity, and impossible successful output lengths raise explicit reconstruction boundaries. Impossible successes invalidate the potentially written cells before throwing. Raw ordinary mapping uses the caller's genuine wide buffer.

The incoming LastError is saved before allocation, restored immediately before the native call, captured immediately after it, and restored after staging destruction. Allocation failure is not translated into an invented original CRT failure path.

`build.ps1` compiles one production object with C++20, O2 and warnings as errors, then runs the standalone fixture executable. Its 92 checks cover full argument forwarding, nontrivial signed counts and high bits, byte output lengths, odd capacities, unknown failure output, null destination, capacity queries, full result values and LastError. Real Windows LCMapStringW/Ex and LocaleNameToLCID calls use only the test's `AbCd` / `en-US` literals. Ex/Locale pointers are found in the already loaded kernel32 module. No original DLL, game, rebuilt target DLL, configuration or user data is loaded or executed.

Primary SDK semantics: [LCMapStringEx](https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-lcmapstringex), [LCMapStringW](https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-lcmapstringw). These document byte counts for sort keys, character counts for ordinary mapping, capacity-zero queries and zero on API failure.

Remaining boundaries include genuine original startup/resolver wiring, original exception/SEH behavior, concurrency and arbitrary aliasing involving caller-managed scratch. The adapter accepts live host pointers; it does not map recovered image addresses or validate arbitrary memory.


