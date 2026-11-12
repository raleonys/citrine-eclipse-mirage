// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstdint>

namespace citrine_eclipse_mirage::net::listener_policy {
// Product-owned automatic listener range. The selected live port remains state;
// configuration cannot request a different range or an arbitrary explicit port.
inline constexpr std::uint16_t first_port=3226;
inline constexpr std::uint16_t last_port=3236;
}
