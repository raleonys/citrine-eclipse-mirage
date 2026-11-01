#include "preloader_l/runtime/trace_format.hpp"

namespace preloader_l { namespace runtime {
namespace {
bool append_char(char c, char* out, size_t cap, size_t* pos) noexcept {
    if (!out || !pos || *pos + 1 >= cap) return false;
    out[(*pos)++] = c; return true;
}
bool append_text(const char* text, char* out, size_t cap, size_t* pos) noexcept {
    if (!text) return false;
    for (size_t i=0; text[i]; ++i) if (!append_char(text[i], out, cap, pos)) return false;
    return true;
}
bool append_uint(uint32_t value, char* out, size_t cap, size_t* pos) noexcept {
    char digits[10]; size_t n=0;
    do { digits[n++] = static_cast<char>('0' + (value % 10u)); value /= 10u; } while (value && n < sizeof(digits));
    while (n) if (!append_char(digits[--n], out, cap, pos)) return false;
    return true;
}
}

size_t format_trace_line(TraceEvent event, uint32_t value,
                         char* output, size_t capacity) noexcept
{
    if (!output || capacity == 0) return 0;
    output[0] = 0;
    size_t pos = 0;
    if (!append_text("[preloader_l] ", output, capacity, &pos) ||
        !append_text(trace_event_name(event), output, capacity, &pos) ||
        !append_text(" value=", output, capacity, &pos) ||
        !append_uint(value, output, capacity, &pos) ||
        !append_char('\n', output, capacity, &pos)) {
        output[0] = 0; return 0;
    }
    output[pos] = 0;
    return pos;
}

} }
