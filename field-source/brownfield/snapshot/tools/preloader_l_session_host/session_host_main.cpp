// Deterministic fragmented-session acceptance host for x64dbg.
#if !defined(_WIN32)
#error preloader_l_session_host is Windows-only.
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
constexpr U32 session_need_more = 0x80020001u;
constexpr U32 session_phase_connected = 1u;

struct SessionStatus {
    U32 phase, session_id;
    U64 received_bytes, sent_bytes;
    U32 receive_events, send_events, buffered_bytes, feed_events;
    U32 frames_ready, frames_consumed, ingress_errors, reserved;
};

using RuntimeInitFn = U32 (*)();
using RuntimeShutdownFn = U32 (*)();
using SessionResetFn = U32 (*)(U32);
using SessionFeedFn = U32 (*)(const char*, USize, char*, USize, USize*);
using SessionStatusFn = U32 (*)(SessionStatus*, USize);
using SessionCloseFn = U32 (*)();

namespace {
char g_response[4096];
SessionStatus g_status{};
enum ExitCode : unsigned int {
    success=0, load_failed=10, export_missing=11, init_failed=12, reset_failed=13,
    first_fragment_failed=14, first_status_failed=15, second_fragment_failed=16,
    response_empty=17, final_status_failed=18, close_failed=19,
    shutdown_failed=20, unload_failed=21
};
[[noreturn]] void finish(unsigned int code) noexcept { ExitProcess(code); }
}

int main() noexcept {
    HMODULE module = LoadLibraryW(L"preloader_l_debug.dll");
    if (!module) finish(load_failed);
#define RESOLVE(type, name) reinterpret_cast<type>(GetProcAddress(module, name))
    const auto runtime_init = RESOLVE(RuntimeInitFn, "preloader_l_debug_runtime_init");
    const auto runtime_shutdown = RESOLVE(RuntimeShutdownFn, "preloader_l_debug_runtime_shutdown");
    const auto session_reset = RESOLVE(SessionResetFn, "preloader_l_debug_session_reset");
    const auto session_feed = RESOLVE(SessionFeedFn, "preloader_l_debug_session_feed");
    const auto session_status = RESOLVE(SessionStatusFn, "preloader_l_debug_get_session_status");
    const auto session_close = RESOLVE(SessionCloseFn, "preloader_l_debug_session_close");
#undef RESOLVE
    if (!runtime_init || !runtime_shutdown || !session_reset || !session_feed || !session_status || !session_close) {
        FreeLibrary(module); finish(export_missing);
    }
    if (runtime_init() != 0u) { FreeLibrary(module); finish(init_failed); }
    if (session_reset(45u) != 0u) { FreeLibrary(module); finish(reset_failed); }

    static const char part1[] = "<LSX><Request id=\"45\"><Get";
    static const char part2[] = "Config/></Request></LSX>";
    USize written = 99;
    if (session_feed(part1, sizeof(part1)-1, g_response, sizeof(g_response), &written) != session_need_more || written != 0u) {
        FreeLibrary(module); finish(first_fragment_failed);
    }
    if (session_status(&g_status, sizeof(g_status)) != 0u || g_status.phase != session_phase_connected ||
        g_status.session_id != 45u || g_status.receive_events != 1u || g_status.buffered_bytes == 0u) {
        FreeLibrary(module); finish(first_status_failed);
    }
    written = 0;
    if (session_feed(part2, sizeof(part2)-1, g_response, sizeof(g_response), &written) != 0u) {
        FreeLibrary(module); finish(second_fragment_failed);
    }
    if (written == 0u || g_response[0] == '\0') { FreeLibrary(module); finish(response_empty); }
    if (session_status(&g_status, sizeof(g_status)) != 0u || g_status.buffered_bytes != 0u ||
        g_status.frames_consumed != 1u || g_status.send_events != 1u || g_status.sent_bytes != written) {
        FreeLibrary(module); finish(final_status_failed);
    }
    if (session_close() != 0u) { FreeLibrary(module); finish(close_failed); }
    if (runtime_shutdown() != 0u) { FreeLibrary(module); finish(shutdown_failed); }
    if (!FreeLibrary(module)) finish(unload_failed);
    finish(success);
}
