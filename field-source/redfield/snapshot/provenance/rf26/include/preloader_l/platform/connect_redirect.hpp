// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdint.h>

namespace preloader_l { namespace platform {

// Consumer-confirmed LSX transport policy: only an IPv4 connection whose
// destination is exactly 127.0.0.1:3216 may be redirected to the listener's
// selected local port. All other connect destinations remain untouched.
void set_connect_selected_port(uint16_t port) noexcept;
uint16_t connect_selected_port() noexcept;

// `address_bytes` is the four bytes of sockaddr_in::sin_addr in network order.
// `port_network` and `mapped_port_network` use sockaddr_in::sin_port encoding.
// Returns true only when an actual 127.0.0.1:3216 -> selected-port rewrite is
// required. A selected port of zero disables the redirect.
bool map_ipv4_connect_destination(uint16_t family,
                                  const uint8_t* address_bytes,
                                  uint16_t port_network,
                                  uint16_t* mapped_port_network) noexcept;

#if defined(_WIN32)
enum class ConnectRedirectInstallResult : uint32_t {
    installed = 1,
    already_installed = 2,
    module_unavailable = 3,
    export_unavailable = 4,
    near_allocation_failed = 5,
    protect_failed = 6,
    patch_failed = 7,
    release_failed = 8,
    plan_error_base = 100,
};
ConnectRedirectInstallResult install_connect_redirect() noexcept;
bool uninstall_connect_redirect() noexcept;
uint32_t connect_redirect_call_count() noexcept;
uint32_t connect_redirect_mapped_count() noexcept;
uintptr_t connect_redirect_target_address() noexcept;
uintptr_t connect_redirect_trampoline_address() noexcept;
#endif

} } // namespace preloader_l::platform
