#include "preloader_l/net/winsock_version_policy.hpp"

int main() {
    using namespace preloader_l::net;
    static_assert(winsock_requested_version == 0x0202u, "Winsock request drift");
    static_assert(winsock_version_not_supported == 10092, "WSAVERNOTSUPPORTED drift");
    if (!winsock_negotiated_version_supported(0x0202u)) return 1;
    if (winsock_negotiated_version_supported(0x0201u)) return 2;
    if (winsock_negotiated_version_supported(0x0101u)) return 3;
    if (winsock_negotiated_version_supported(0x0000u)) return 4;
    return 0;
}
