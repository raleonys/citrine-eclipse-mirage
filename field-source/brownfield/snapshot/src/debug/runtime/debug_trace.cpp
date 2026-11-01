#if !defined(_WIN32)
#error debug_trace.cpp is Windows-only.
#endif
#include "preloader_l/runtime/trace_format.hpp"
#include "preloader_l/runtime/debug_trace.hpp"
#include "preloader_l/platform/identity_state.hpp"

extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char* text);

namespace preloader_l { namespace runtime {
namespace {
size_t append_text(const char* text, char* out, size_t cap, size_t pos) noexcept {
    if (!text || !out || cap == 0) return 0;
    for (size_t i=0; text[i]; ++i) {
        if (pos + 1 >= cap) return 0;
        out[pos++] = text[i];
    }
    out[pos] = 0; return pos;
}
size_t append_uint(uint32_t value, char* out, size_t cap, size_t pos) noexcept {
    char d[10]; size_t n=0;
    do { d[n++]=static_cast<char>('0'+value%10u); value/=10u; } while(value && n<sizeof(d));
    while(n){ if(pos+1>=cap) return 0; out[pos++]=d[--n]; }
    out[pos]=0; return pos;
}
}
void windows_debug_trace(TraceEvent event, uint32_t value) noexcept {
    char line[96]{};
    if (format_trace_line(event, value, line, sizeof(line)) != 0) OutputDebugStringA(line);
}
void windows_debug_identity() noexcept {
    // Release version identity is intentionally externalized to package/docs.
    // Do not embed semantic release numbers or codenames in DLL debug strings.
    OutputDebugStringA("[preloader_l] release identity externalized\n");
}
void windows_debug_reconstructed_identity() noexcept {
    const auto state = preloader_l::platform::reconstructed_identity_state();
    char line[256]{}; size_t p=0;
    p=append_text("[preloader_l] reconstructed_identity target=",line,sizeof(line),p); if(!p) return;
    p=append_text(state.target_module_ascii,line,sizeof(line),p); if(!p) return;
    p=append_text(" companion=",line,sizeof(line),p); if(!p) return;
    p=append_text(state.companion_module_ascii,line,sizeof(line),p); if(!p) return;
    p=append_text(" identity=",line,sizeof(line),p); if(!p) return;
    p=append_text(state.identity_ascii,line,sizeof(line),p); if(!p) return;
    p=append_text(" branch_hits=",line,sizeof(line),p); if(!p) return;
    p=append_uint(state.branch_hits,line,sizeof(line),p); if(!p) return;
    p=append_text("\n",line,sizeof(line),p); if(!p) return;
    OutputDebugStringA(line);
}

} }
