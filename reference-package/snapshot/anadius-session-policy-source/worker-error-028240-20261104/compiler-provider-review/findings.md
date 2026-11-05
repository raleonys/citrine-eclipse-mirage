# Selected compiler system-message provider

Result: the selected local LLVM-MinGW libc++ system-category message implementation is **not value-equivalent** to native020950 across the admitted message domain. Correcting the terminal context literal from post to pqcs alone does not close that difference. No production change is made by this audit.

This is a bounded local primary-artifact review. No web access/download, compiler invocation, build, linking, test run, reconstructed/original DLL execution, original-binary access or REA query occurred. Read-only LLVM archive/symbol/disassembly tools were selected by dot-sourcing work/use-tools.ps1. Extracted COFF members and tool output were written only to this evidence directory; toolchain/archive and frozen build files were not changed.

## Provider selection and identity

The historical successful production validation at outputs/dll-link-gate-source/build-current-full/validation.json records clang23.1.4 commit7a941dfb2877a0b8cf30323fefa85ad019e0b0f5 with -shared -static. Its retained link.log first line resolves the static C++ provider to:

`C:/Users/Skysect/Documents/Codex/2026-10-16/mcp/work/toolchain/extracted/llvm-mingw-20261020-ucrt-x86_64/x86_64-w64-mingw32/lib/libc++.a`

link.log:2579 reads that exact archive, and2821 loads system_error.cpp.obj. The focused historical excerpt is saved here without modifying the original log. Current selected archive SHA256 is **7951a18341870b622e67bea17bed9cd953d827cc7fbd0b521f01944f1ac7d22c** (3,715,512 bytes).

No shipped system_error.cpp implementation source was found in the selected toolchain inventory. The implementation was inspected in actual coff-x86-64 archive members, not inferred from a different libc++ version or generic documentation. Archive/member listing, full focused member disassembly/relocations/sections, exact tool paths/versions and hashes are retained in manifest.json. All three read-only tools report LLVM23.1.4.

| Extracted member | SHA256 | Scope |
|---|---|---|
| system_error.cpp.obj |31dfd7fc526dc8106710319b9c66cc4bf89bfac99c1614e254a774a9559452d7 | Actual system-category message, make_error_str and system_error constructors/vtable |
| stdlib_stdexcept.cpp.obj |f702a3ff014fe41754270f6a042afdd0e987bd4b06e49450a5aa46dc021412a1 | Inherited runtime_error::what pointer-return body only |
| error_category.cpp.obj |fa76ab9b44d1e856fffeb8460592ba702e3d83dfad4fc3e8a5447049a7e780ab | Base category candidate; concrete message lives in system_error member |

Historical validation/log hashes are in manifest.json. The historical record proves selected path/member names; it did not record a libc++.a SHA256. The archive/member hashes here identify **current local bytes at that selected path**, not a retroactive hash of unrecorded historical inputs. No frozen build validation claim is rewritten.

## Concrete message comparison

Offsets below are relative to COFF function sections, not original/image virtual addresses. The complete function body is retained in system-error-disassembly.txt and focused-system-category-message.txt. Imported symbols are resolved by COFF relocations, not guessed from call shapes.

| Behavior | Current selected libc++ system_category.message | Native020950 supplement |
|---|---|---|
| SDK encoding | FormatMessageA; no W call or conversion in this body | FormatMessageA |
| Flags |0x1300 at function+0x40 |0x1300 at020989 |
| Language |0, xor r9d at+0x47 |0x400 |
| Other arguments | Null source, incoming DWORD code, initialized null output pointer, size0, null args | Same values |
| Trailing handling | Repeatedly strips bytes LF10, CR13 and space32, until a different byte or empty; mask0x100002400 at+0x5d and loop+0x70–0x89 | At most one LF then at most one CR; no general/repeated whitespace trim |
| Nonempty copy length | Remaining returned count, memmove at+0x10a then NUL at+0x10f | Subsequent first-NUL scan0209e4–0209f8, then006b30 copy |
| Failure/empty fallback | Exact `Unknown error`,13 bytes, instructions+0x8b–0xaa and retained .rdata literal | Exact `asio.system error`,17 bytes |
| Normal release | LocalFree(output pointer) at+0xb3, including null failure; return ignored | Saved-pointer LocalFree at020a47/020a4a, including null failure; return ignored |
| Category name | `system`, local .str.2 literal | `asio.system` via020940/table bytes |

The compiler mask has bits10,13,32 set and its branch loops while remaining count is nonzero. It does not trim tab9. Successful copy uses the remaining count as r8 to memmove, preserving embedded-NUL bytes in the resulting std::string length. It performs no post-trim strlen of the SDK buffer. Both normal branches construct the result before LocalFree. Neither visible body explicitly reads/restores LastError.

Bounded static counterexamples, **not executed SDK observations**: a valid counted `x ` buffer retains its space natively but compiler strips it; `x\n\n` loses one LF natively and both in compiler; positive count3 buffer `A\0B` produces native first-NUL length1 versus compiler counted length3; count0 produces different fallback strings. The language argument difference may also select different localized text, but no current OS result is assumed. Positive count is interpreted under the same real SDK requirement of valid accessible allocated output; no malicious invalid buffer behavior is invented.

## system_error diagnostic composition

The same system_error.cpp.obj contains make_error_str, complete output retained in focused-make-error-str.txt:

- +0xd–0x13 tests code.value. Zero returns the supplied prefix without calling category.message; this zero case is outside the selected nonzero failed-stop raise.
- For nonzero code, +0x1c–0x4a checks prefix length and appends the literal `: ` (two bytes) only for a nonempty prefix. The actual literal bytes3a2000 are retained in system-error-sections.txt.
- +0x4d–0x63 calls the supplied compiler category's message virtual. +0x63–0x85 appends its pointer and stored count. No category name, decimal number or bracket suffix is appended by this function.
- system_error(int, category const&, char const*) copies its supplied C prefix, calls make_error_str at function+0x97, then passes that composed string to runtime_error at+0xa4. It stores code/category afterward. Thus the selected nonzero input `pqcs` composes `pqcs: ` plus **compiler category.message(code)**, with the concrete category differences above still present.
- system-error-relocations.txt records the system_error vtable what entry at table+0x20 as `_ZNKSt13runtime_error4whatEv`. That selected stdlib_stdexcept member's complete what body is `mov 0x8(%rcx),%rax; ret`. It returns the stored diagnostic pointer; it does not format another suffix on each what call.

Fresh native028240/0248b0/00a270 evidence in this stage independently proves the selected native literal pqcs and nonempty-prefix `: ` plus supplied category message. Prior fresh020950 evidence is supplementary native authority and was not queried again. For this nonzero selected context, the compiler separator/prefix composition agrees at value level; its default **system message provider does not**.

## Scope and next decision

The minimal remaining comparison target is the selected terminal84530 Policy raise path's supplied category-message provider, in both original and child work-stop projections. The existing concrete020950 SDK adapter already models the native normal message value, but the compiler exception constructors still resolve their own category virtual. This audit does not authorize a global category migration or prescribe exception ownership changes. A production follow-up must connect exact selected native message behavior while preserving the existing logical code/category and failure prefix; merely changing names or claiming system_category equivalence is insufficient.

No MSVC exception layout, vtable slot ABI, RTTI, throw helper023990, unwind/catch compatibility, allocation/SSO capacity equivalence, actual OS message output or runtime parity is established. COFF member boundaries and compiler EH metadata remain compiler artifacts. Generic-category/default-condition semantics and broader CRT were deliberately not audited. No completion percentage is inferred from this bounded result.
