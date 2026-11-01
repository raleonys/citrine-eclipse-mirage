#include "preloader_l/platform/connect_redirect.hpp"
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace preloader_l { namespace platform {
namespace {
volatile long g_selected_port = 0;
constexpr uint16_t kAfInet = 2u;
constexpr uint16_t kLegacyLsxPort = 3216u;

constexpr uint16_t swap16(uint16_t value) noexcept
{
    return static_cast<uint16_t>((value << 8) | (value >> 8));
}
}

void set_connect_selected_port(uint16_t port) noexcept
{
#if defined(_MSC_VER) && !defined(__clang__)
    (void)_InterlockedExchange(&g_selected_port, static_cast<long>(port));
#else
    __atomic_store_n(&g_selected_port, static_cast<long>(port), __ATOMIC_RELEASE);
#endif
}

uint16_t connect_selected_port() noexcept
{
#if defined(_MSC_VER) && !defined(__clang__)
    const long value = _InterlockedCompareExchange(&g_selected_port, 0, 0);
#else
    const long value = __atomic_load_n(&g_selected_port, __ATOMIC_ACQUIRE);
#endif
    return static_cast<uint16_t>(value & 0xFFFFL);
}

bool map_ipv4_connect_destination(uint16_t family,
                                  const uint8_t* address_bytes,
                                  uint16_t port_network,
                                  uint16_t* mapped_port_network) noexcept
{
    if (mapped_port_network) *mapped_port_network = port_network;
    if (!address_bytes || !mapped_port_network || family != kAfInet) return false;
    if (address_bytes[0] != 127u || address_bytes[1] != 0u ||
        address_bytes[2] != 0u || address_bytes[3] != 1u) return false;
    if (swap16(port_network) != kLegacyLsxPort) return false;

    const uint16_t selected = connect_selected_port();
    if (selected == 0u || selected == kLegacyLsxPort) return false;
    *mapped_port_network = swap16(selected);
    return true;
}

} }
