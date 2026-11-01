// HF12 IOCP migration Stage01: production wire classifier reused by async ingress.
// Portable, source-linked regression. Not Windows-native IOCP acceptance.
#include "../../src/protocol/socket_exchange_internal.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
namespace p = preloader_l::protocol;
namespace w = preloader_l::protocol::detail;
int main() {
    int checks = 0;
    auto require = [&](bool b) { ++checks; assert(b); };
    p::ExchangeState state{};
    w::WireFrameView frame{};
    require(!w::scan_available_wire_frame(&state, &frame));
    require(frame.consume_length == 0u);
    const std::string one = "<LSX><Request id=\"1\"><GetConfig/></Request></LSX>";
    const std::string two = "<LSX><Request id=\"2\"><GetConfig/></Request></LSX>";
    const std::string three = "<LSX><Request id=\"3\"><GetConfig/></Request></LSX>";
    auto append = [&](const std::string& x) {
        require(state.wire_size + x.size() < sizeof(state.wire_response));
        std::memcpy(state.wire_response + state.wire_size, x.data(), x.size());
        state.wire_size += x.size();
    };
    append(one.substr(0, 3));
    require(!w::scan_available_wire_frame(&state, &frame));
    append(one.substr(3));
    require(w::scan_available_wire_frame(&state, &frame));
    require(frame.payload_length == one.size());
    require(frame.consume_length == one.size());
    w::consume_wire_frame(&state, frame.consume_length);
    require(state.wire_size == 0u);
    append(two + std::string(1,'\0') + three + std::string(1,'\0'));
    require(w::scan_available_wire_frame(&state, &frame));
    require(frame.payload_length == two.size());
    require(frame.consume_length == two.size() + 1u);
    w::consume_wire_frame(&state, frame.consume_length);
    require(w::scan_available_wire_frame(&state, &frame));
    require(frame.payload_length == three.size());
    require(frame.consume_length == three.size() + 1u);
    w::consume_wire_frame(&state, frame.consume_length);
    require(state.wire_size == 0u);
    // Proven malformed input is isolated and consumed, not retained indefinitely.
    append("not-lsx");
    require(w::scan_available_wire_frame(&state, &frame));
    require(frame.payload_length == 7u);
    w::consume_wire_frame(&state, frame.consume_length);
    require(state.wire_size == 0u);
    // Poisoned zero must not create an infinite loop or underflow.
    append(std::string(1,'\0') + one);
    require(w::scan_available_wire_frame(&state, &frame));
    require(frame.consume_length > 0u && frame.consume_length <= state.wire_size);
    w::consume_wire_frame(&state, frame.consume_length);
    require(w::scan_available_wire_frame(&state, &frame));
    require(frame.payload_length == one.size());
    std::cout << "HF12_STAGE01_SOURCE_LINKED_WIRE PASS checks=" << checks << "\n";
}
