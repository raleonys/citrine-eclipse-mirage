#include "preloader_l/net/port_mapping.hpp"

namespace preloader_l { namespace net {

uint16_t select_port(PortProbe probe, void* context) noexcept
{
    if (!probe) return 0; // clean API guard; no claim about original null-callback behavior
    if (probe(3226, context)) return 3226;
    for (uint16_t port = 3227; port <= 3236; ++port) {
        if (probe(port, context)) return port;
    }
    return 3227;
}

uint16_t map_requested_port(uint16_t requested, uint16_t selected) noexcept
{
    return requested == 3216 ? selected : requested;
}

} } // namespace preloader_l::net
