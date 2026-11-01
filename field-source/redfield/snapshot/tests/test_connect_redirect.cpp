// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/platform/connect_redirect.hpp"
#include <stdint.h>

namespace {
constexpr uint16_t net16(uint16_t value) noexcept {
    return static_cast<uint16_t>((value << 8) | (value >> 8));
}
}

int main() {
    using namespace preloader_l::platform;
    const uint8_t loopback[4]{127u,0u,0u,1u};
    const uint8_t remote[4]{192u,168u,1u,20u};
    uint16_t mapped = 0u;

    set_connect_selected_port(0u);
    if (map_ipv4_connect_destination(2u, loopback, net16(3216u), &mapped)) return 1;
    if (mapped != net16(3216u)) return 2;

    set_connect_selected_port(3226u);
    if (connect_selected_port() != 3226u) return 3;
    if (!map_ipv4_connect_destination(2u, loopback, net16(3216u), &mapped)) return 4;
    if (mapped != net16(3226u)) return 5;

    if (map_ipv4_connect_destination(2u, remote, net16(3216u), &mapped)) return 6;
    if (mapped != net16(3216u)) return 7;
    if (map_ipv4_connect_destination(2u, loopback, net16(443u), &mapped)) return 8;
    if (mapped != net16(443u)) return 9;
    if (map_ipv4_connect_destination(23u, loopback, net16(3216u), &mapped)) return 10;
    if (map_ipv4_connect_destination(2u, nullptr, net16(3216u), &mapped)) return 11;
    if (map_ipv4_connect_destination(2u, loopback, net16(3216u), nullptr)) return 12;

    set_connect_selected_port(3216u);
    if (map_ipv4_connect_destination(2u, loopback, net16(3216u), &mapped)) return 13;
    if (mapped != net16(3216u)) return 14;

    set_connect_selected_port(3236u);
    if (!map_ipv4_connect_destination(2u, loopback, net16(3216u), &mapped)) return 15;
    if (mapped != net16(3236u)) return 16;
    return 0;
}
