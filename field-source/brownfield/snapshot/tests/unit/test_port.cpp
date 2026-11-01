#include "preloader_l/net/port_mapping.hpp"

#include <cstddef>
#include <cstdint>

using namespace preloader_l::net;

struct ProbeContext {
    int available;
    uint16_t calls[16];
    size_t count;
};

bool probe(uint16_t port, void* opaque) noexcept
{
    auto* ctx = static_cast<ProbeContext*>(opaque);
    ctx->calls[ctx->count++] = port;
    return ctx->available == static_cast<int>(port);
}

int main()
{
    for (int available = 3226; available <= 3236; ++available) {
        ProbeContext ctx{available, {}, 0};
        const uint16_t selected = select_port(probe, &ctx);
        if (selected != available) return 1;
        const size_t expected_calls = static_cast<size_t>(available - 3226 + 1);
        if (ctx.count != expected_calls) return 2;
        for (size_t i = 0; i < ctx.count; ++i) {
            if (ctx.calls[i] != static_cast<uint16_t>(3226 + i)) return 3;
        }
    }

    ProbeContext none{-1, {}, 0};
    if (select_port(probe, &none) != 3227 || none.count != 11) return 4;
    for (size_t i = 0; i < none.count; ++i) if (none.calls[i] != static_cast<uint16_t>(3226 + i)) return 5;
    if (select_port(nullptr, nullptr) != 0) return 6;

    const uint16_t selected_values[] = {3226, 3236};
    const uint16_t requested_values[] = {0, 3216, 3226, 65535};
    for (uint16_t selected : selected_values) {
        for (uint16_t requested : requested_values) {
            const uint16_t expected = requested == 3216 ? selected : requested;
            if (map_requested_port(requested, selected) != expected) return 7;
        }
    }
    return 0;
}
