// x64dbg-oriented prototype host.
// Freestanding by design: it loads the debug DLL, resolves the four debug-only
// C exports, runs the self-test and one safe GetConfig dispatch, reads stats,
// then unloads. Exit code identifies the first failed gate.

#if !defined(_WIN32)
#error preloader_l_debug_host is Windows-only.
#endif

extern "C" {
using BOOL = int;
using HMODULE = void*;
using FARPROC = void (*)();
__declspec(dllimport) HMODULE __stdcall LoadLibraryW(const wchar_t* file_name);
__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE module, const char* proc_name);
__declspec(dllimport) BOOL __stdcall FreeLibrary(HMODULE module);
__declspec(dllimport) __declspec(noreturn) void __stdcall ExitProcess(unsigned int exit_code);
}

using U32 = unsigned int;
using U64 = unsigned long long;
using USize = unsigned long long;

struct DebugStats {
    U64 self_test_runs;
    U64 self_test_failures;
    U64 dispatch_calls;
    U64 dispatch_ok;
    U64 dispatch_non_ok;
    U64 codec_checks;
    U64 framing_checks;
    U64 route_checks;
};

using SelfTestFn = U32 (*)(U32*, U32*);
using DispatchFn = U32 (*)(const char*, USize, char*, USize, USize*);
using ResetStatsFn = void (*)();
using GetStatsFn = U32 (*)(DebugStats*, USize);

namespace {
char g_response[4096];
DebugStats g_observed;

enum ExitCode : unsigned int {
    success = 0,
    load_failed = 10,
    self_test_export_missing = 11,
    dispatch_export_missing = 12,
    reset_export_missing = 13,
    stats_export_missing = 14,
    self_test_failed = 20,
    dispatch_failed = 21,
    response_empty = 22,
    stats_failed = 23,
    stats_unexpected = 24,
    unload_failed = 30,
};

[[noreturn]] void finish(unsigned int code) noexcept { ExitProcess(code); }
}

int main() noexcept
{
    HMODULE module = LoadLibraryW(L"preloader_l_debug.dll");
    if (!module) finish(load_failed);

    const auto self_test = reinterpret_cast<SelfTestFn>(GetProcAddress(module, "preloader_l_debug_self_test"));
    const auto dispatch = reinterpret_cast<DispatchFn>(GetProcAddress(module, "preloader_l_debug_dispatch"));
    const auto reset = reinterpret_cast<ResetStatsFn>(GetProcAddress(module, "preloader_l_debug_reset_stats"));
    const auto stats = reinterpret_cast<GetStatsFn>(GetProcAddress(module, "preloader_l_debug_get_stats"));
    if (!self_test) { FreeLibrary(module); finish(self_test_export_missing); }
    if (!dispatch) { FreeLibrary(module); finish(dispatch_export_missing); }
    if (!reset) { FreeLibrary(module); finish(reset_export_missing); }
    if (!stats) { FreeLibrary(module); finish(stats_export_missing); }

    reset();
    U32 passed = 0, failed = 0;
    if (self_test(&passed, &failed) != 0 || failed != 0 || passed != 0x0fu) {
        FreeLibrary(module); finish(self_test_failed);
    }

    static const char request[] = "<LSX><Request id=\"35\"><GetConfig/></Request></LSX>";
    USize written = 0;
    if (dispatch(request, sizeof(request) - 1, g_response, sizeof(g_response), &written) != 0u) {
        FreeLibrary(module); finish(dispatch_failed);
    }
    if (written == 0 || g_response[0] == '\0') {
        FreeLibrary(module); finish(response_empty);
    }

    if (stats(&g_observed, sizeof(g_observed)) != 0u) {
        FreeLibrary(module); finish(stats_failed);
    }
    if (g_observed.self_test_runs != 1 || g_observed.self_test_failures != 0 ||
        g_observed.dispatch_calls != 1 || g_observed.dispatch_ok != 1 ||
        g_observed.codec_checks != 1 || g_observed.framing_checks != 1 || g_observed.route_checks != 1) {
        FreeLibrary(module); finish(stats_unexpected);
    }

    if (!FreeLibrary(module)) finish(unload_failed);
    finish(success);
}
