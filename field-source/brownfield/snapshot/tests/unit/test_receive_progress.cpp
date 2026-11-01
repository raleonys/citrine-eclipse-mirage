#include "preloader_l/net/socket_transport.hpp"
#include "preloader_l/protocol/socket_exchange.hpp"
#include "../../src/protocol/socket_exchange_internal.hpp"
#include <cstddef>
#include <cstdint>

namespace {
struct ClockReceive {
    uint64_t now_ms = 0;
    unsigned calls = 0;
    bool emit_prefix = false;
    bool emitted_prefix = false;
};

preloader_l::net::TransportStatus receive(void* opaque, uint8_t* output,
                                           size_t capacity, size_t* received) noexcept {
    auto* state = static_cast<ClockReceive*>(opaque);
    if (received) *received = 0;
    if (!state || !output || !received || capacity == 0)
        return preloader_l::net::TransportStatus::invalid_argument;
    ++state->calls;
    state->now_ms += 1000u;
    if (state->emit_prefix && !state->emitted_prefix) {
        output[0] = '<';
        *received = 1u;
        state->emitted_prefix = true;
        return preloader_l::net::TransportStatus::ok;
    }
    return preloader_l::net::TransportStatus::would_block;
}

uint64_t clock_ms(void* opaque) noexcept {
    const auto* state = static_cast<const ClockReceive*>(opaque);
    return state ? state->now_ms : 0u;
}
}

int main() {
    using namespace preloader_l;

    // F323-S0439-001: no receive progress is bounded independently of the
    // worker admission cap.
    ClockReceive idle{};
    net::TransportEndpoint endpoint{
        &idle, &receive, nullptr, nullptr, &clock_ms,
        0xffffffffu, 2500u, 0xffffffffu};
    protocol::ExchangeState state{};
    protocol::detail::WireFrameView view{};
    if (protocol::detail::receive_protocol_frame(endpoint, &state, &view) !=
        protocol::ExchangeStatus::receive_timeout) return 1;
    if (state.wire_size != 0u || idle.calls != 3u) return 2;

    // A first byte starts the cumulative incomplete-frame lifetime. Continued
    // readiness polling cannot renew that deadline indefinitely.
    ClockReceive slow{};
    slow.emit_prefix = true;
    endpoint = net::TransportEndpoint{
        &slow, &receive, nullptr, nullptr, &clock_ms,
        0xffffffffu, 10000u, 2500u};
    state = protocol::ExchangeState{};
    view = protocol::detail::WireFrameView{};
    if (protocol::detail::receive_protocol_frame(endpoint, &state, &view) !=
        protocol::ExchangeStatus::receive_timeout) return 3;
    if (state.wire_size != 1u || state.wire_response[0] != '<' || slow.calls != 4u)
        return 4;
    return 0;
}
