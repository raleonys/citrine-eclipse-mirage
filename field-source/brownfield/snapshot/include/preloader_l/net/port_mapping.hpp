#pragma once

#include <stdint.h>

namespace preloader_l { namespace net {

using PortProbe = bool (*)(uint16_t port, void* context) noexcept;

// Observed selection order: 3226, then 3227..3236. If every probe fails the
// model path still returns 3227; that value is not proof of reservation.
uint16_t select_port(PortProbe probe, void* context) noexcept;

// Model adapter: a requested 3216 aliases the selected server port.
uint16_t map_requested_port(uint16_t requested, uint16_t selected) noexcept;

} } // namespace preloader_l::net
