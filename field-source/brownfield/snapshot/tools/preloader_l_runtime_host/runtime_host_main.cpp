// Loader/runtime lifecycle acceptance host for x64dbg.
#if !defined(_WIN32)
#error preloader_l_runtime_host is Windows-only.
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
constexpr U32 runtime_not_initialized = 0x80010001u;
constexpr U32 phase_loader_attached = 1u;
constexpr U32 phase_initialized = 2u;
constexpr U32 phase_shutdown = 3u;

struct DebugStats {
    U64 self_test_runs, self_test_failures, dispatch_calls, dispatch_ok, dispatch_non_ok;
    U64 codec_checks, framing_checks, route_checks;
};
struct RuntimeStatus {
    U32 phase, generation;
    U64 module_base;
    U32 initialize_count, shutdown_count, process_attach_count, process_detach_count, surge_call_count, reserved;
};

using SelfTestFn = U32 (*)(U32*, U32*);
using DispatchFn = U32 (*)(const char*, USize, char*, USize, USize*);
using ResetStatsFn = void (*)();
using GetStatsFn = U32 (*)(DebugStats*, USize);
using RuntimeInitFn = U32 (*)();
using RuntimeShutdownFn = U32 (*)();
using RuntimeStatusFn = U32 (*)(RuntimeStatus*, USize);
using SurgeFn = void (*)();

namespace {
char g_response[4096];
DebugStats g_stats{};
RuntimeStatus g_runtime{};

enum ExitCode : unsigned int {
    success=0, load_failed=10, export_missing=11, loader_status_failed=20,
    preinit_dispatch_not_blocked=21, init_failed=22, initialized_status_failed=23,
    self_test_failed=24, dispatch_failed=25, response_empty=26, surge_status_failed=27,
    stats_failed=28, shutdown_failed=29, shutdown_status_failed=30, unload_failed=31
};
[[noreturn]] void finish(unsigned int code) noexcept { ExitProcess(code); }
}

int main() noexcept {
    HMODULE module = LoadLibraryW(L"preloader_l_debug.dll");
    if (!module) finish(load_failed);
#define RESOLVE(type, name) reinterpret_cast<type>(GetProcAddress(module, name))
    const auto self_test = RESOLVE(SelfTestFn, "preloader_l_debug_self_test");
    const auto dispatch = RESOLVE(DispatchFn, "preloader_l_debug_dispatch");
    const auto reset = RESOLVE(ResetStatsFn, "preloader_l_debug_reset_stats");
    const auto stats = RESOLVE(GetStatsFn, "preloader_l_debug_get_stats");
    const auto runtime_init = RESOLVE(RuntimeInitFn, "preloader_l_debug_runtime_init");
    const auto runtime_shutdown = RESOLVE(RuntimeShutdownFn, "preloader_l_debug_runtime_shutdown");
    const auto runtime_status = RESOLVE(RuntimeStatusFn, "preloader_l_debug_get_runtime_status");
    const auto surge = RESOLVE(SurgeFn, "SURGE");
#undef RESOLVE
    if (!self_test || !dispatch || !reset || !stats || !runtime_init || !runtime_shutdown || !runtime_status || !surge) {
        FreeLibrary(module); finish(export_missing);
    }

    if (runtime_status(&g_runtime, sizeof(g_runtime)) != 0u ||
        g_runtime.phase != phase_loader_attached || g_runtime.process_attach_count != 1u ||
        g_runtime.generation != 1u || g_runtime.module_base == 0u) {
        FreeLibrary(module); finish(loader_status_failed);
    }

    static const char request[] = "<LSX><Request id=\"40\"><GetConfig/></Request></LSX>";
    USize written = 99;
    if (dispatch(request, sizeof(request)-1, g_response, sizeof(g_response), &written) != runtime_not_initialized || written != 0u) {
        FreeLibrary(module); finish(preinit_dispatch_not_blocked);
    }

    if (runtime_init() != 0u) { FreeLibrary(module); finish(init_failed); }
    if (runtime_status(&g_runtime, sizeof(g_runtime)) != 0u || g_runtime.phase != phase_initialized || g_runtime.initialize_count != 1u) {
        FreeLibrary(module); finish(initialized_status_failed);
    }

    reset();
    U32 passed=0, failed=0;
    if (self_test(&passed, &failed) != 0u || failed != 0u || passed != 0x0fu) {
        FreeLibrary(module); finish(self_test_failed);
    }
    written = 0;
    if (dispatch(request, sizeof(request)-1, g_response, sizeof(g_response), &written) != 0u) {
        FreeLibrary(module); finish(dispatch_failed);
    }
    if (written == 0u || g_response[0] == '\0') { FreeLibrary(module); finish(response_empty); }

    surge();
    if (runtime_status(&g_runtime, sizeof(g_runtime)) != 0u || g_runtime.surge_call_count != 1u) {
        FreeLibrary(module); finish(surge_status_failed);
    }
    if (stats(&g_stats, sizeof(g_stats)) != 0u || g_stats.self_test_runs != 1u || g_stats.self_test_failures != 0u ||
        g_stats.dispatch_calls != 1u || g_stats.dispatch_ok != 1u || g_stats.codec_checks != 1u ||
        g_stats.framing_checks != 1u || g_stats.route_checks != 1u) {
        FreeLibrary(module); finish(stats_failed);
    }

    if (runtime_shutdown() != 0u) { FreeLibrary(module); finish(shutdown_failed); }
    if (runtime_status(&g_runtime, sizeof(g_runtime)) != 0u || g_runtime.phase != phase_shutdown || g_runtime.shutdown_count != 1u) {
        FreeLibrary(module); finish(shutdown_status_failed);
    }
    if (!FreeLibrary(module)) finish(unload_failed);
    finish(success);
}
