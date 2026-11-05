# Passive default module initialization review

Static portability concern, not an observed project compilation failure. No compiler, linker, tests or DLL were executed.

The first candidate placed a constinit WinsockModuleState containing default imported function pointers at namespace scope. The selected local winsock2.h declares WSAStartup/WSACleanup using WINSOCK_API_LINKAGE (lines1046–1047), which selects DECLSPEC_IMPORT when available (lines34–38).

The official [LLVM constexpr import regression](https://raw.githubusercontent.com/llvm/llvm-project/main/clang/test/SemaCXX/dllimport-constexpr.cpp) rejects constant-expression initialization of imported function pointers for its MS ABI test target. [Microsoft's import rules](https://learn.microsoft.com/en-us/cpp/c-language/rules-and-limitations-for-dllimport-dllexport?view=msvc-170) also distinguish statically initialized import thunks from the function address. These establish a portability/identity concern; they do not by themselves prove the selected LLVM-MinGW23 GNU ABI would reject this candidate or produce mismatched pointers.

The revision removes reliance on that assumption: a nonconstexpr passive imports factory initializes a function-local module state before first use. Its metadata guard finishes before acquire performs any startup, and the getter preserves LastError. No common translation-unit initializer invokes Winsock. The DLL-only compiler-CRT initializer remains the actual first unchecked owner. This preserves typed default-import identity and avoids a cross-translation-unit dynamic-initialization dependency without changing KernelImports defaults, adding a provider registry, or serializing SDK startup completion.

Current compiler acceptance and emitted pointer/callback behavior still require an explicitly authorized rebuild. Earlier source-review hashes describe the first candidate; final review addenda identify the revised source.
