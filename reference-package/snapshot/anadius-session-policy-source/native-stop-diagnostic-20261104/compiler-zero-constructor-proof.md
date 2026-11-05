# Exact error_code plus counted-string constructor proof

Read-only follow-up from existing selected LLVM-MinGW23.1.4 COFF evidence. No new compiler/LLVM invocation, native query, build, test, link, execution or production edit. The frozen compiler-provider-review report/manifest and original full dump were not changed.

The actual selected libc++.a member system_error.cpp.obj SHA256 is31dfd7fc526dc8106710319b9c66cc4bf89bfac99c1614e254a774a9559452d7. Existing complete system-error-disassembly.txt SHA256 is7e3d9a665e034337bfc00d5d0af88bc687a5643a5871daea6235c2d860f39187. Its lines420–501 were copied here as compiler-zero-string-constructor.txt, SHA25689a39fd763d05f074ae40729f4a67aa1060416f23b6950a80dbe96221a87edf0. This is the complete82-line function section including its exception tail, not a newly disassembled object.

The exact overload is **std::__1::system_error::system_error(std::__1::error_code, std::__1::basic_string<char,...> const&)**, section `_ZNSt3__112system_errorC2ENS_10error_codeERKNS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE`. This is not inferred from the different int/category/char* overload.

Observed normal body, function-relative offsets:

- +0x6 saves incoming error_code address from RDX in RDI. +0xc onward copies the supplied counted std::string using its short/long representation; long copy calls __init_copy_ctor_external with stored length at+0x22, rather than formatting a category.
- +0x3b/+0x40 prepare output and copied string. +0x45 passes that same error_code address in RDX. **+0x48 calls make_error_str**, with relocation+0x49 to `_ZNSt3__112_GLOBAL__N_114make_error_strERKNS_10error_codeENS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE`.
- The already retained complete make_error_str body loads code.value at+0xd, tests it at+0x10 and jumps directly to+0xa4 on zero. That branch moves the supplied string to output without the nonzero prefix/separator/category.message path. Thus a supplied precomposed zero-code string is not formatted a second time.
- Constructor+0x55 calls runtime_error(const std::string&) with the returned string. +0x95 sets its system_error vtable. **+0x98/+0x9b copy the original16-byte error_code into the exception**. Formatting text does not replace or synthesize the stored zero value/category.

Consequently, a bounded helper may explicitly obtain category.message(0) once, compose the observed context + optional `: ` + message in a std::string, and throw plain std::system_error(std::error_code(0,category), counted_text). The exact selected constructor's zero branch preserves that text without another category-message/SDK call. It also preserves raw code0 and the original stable category reference. Nonzero can retain its existing single category-message constructor path.

This validates selected compiler value assembly only. It does not certify arbitrary future compiler implementations, native exception/RTTI/unwind ABI, allocation equivalence or native exceptional cleanup. Ordinary02a190 with failed-post GetLastError0 still does not raise; the direct code0 constructor/value regression is a different declared input path.
