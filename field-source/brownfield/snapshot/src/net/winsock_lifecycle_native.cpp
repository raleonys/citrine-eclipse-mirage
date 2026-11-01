// IDS Stage05: the shipping-only Winsock 2.2 lifecycle compatibility provider.
// Asio owns every shipping acceptor/socket. This adapter intentionally cannot
// create or operate legacy raw sockets; the full native implementation remains
// test-only under PRELOADER_L_NATIVE_ACCEPTANCE and in Debug diagnostics.
#if !defined(_WIN32)
#error winsock_lifecycle_native.cpp is Windows-only.
#endif

#include "preloader_l/net/winsock_native.hpp"
#include "preloader_l/net/winsock_version_policy.hpp"
#include <stddef.h>
#include <stdint.h>

#if !defined(PRELOADER_L_NATIVE_ACCEPTANCE)
namespace {
extern "C" {
__declspec(dllimport) int __stdcall WSAStartup(uint16_t version, void* data);
__declspec(dllimport) int __stdcall WSACleanup();
__declspec(dllimport) int __stdcall WSAGetLastError();
}

int startup(void*, uint16_t version) noexcept {
    alignas(void*) uint8_t data[408]{};
    const int result = WSAStartup(version, data);
    if (result != 0) return result;
    const uint16_t negotiated = static_cast<uint16_t>(
        static_cast<uint16_t>(data[0]) |
        static_cast<uint16_t>(static_cast<uint16_t>(data[1]) << 8));
    if (!preloader_l::net::winsock_negotiated_version_supported(negotiated)) {
        if (WSACleanup() != 0)
            return preloader_l::net::winsock_version_cleanup_retry_required;
        return preloader_l::net::winsock_version_not_supported;
    }
    return 0;
}
int cleanup(void*) noexcept { return WSACleanup(); }
int last_error(void*) noexcept { return WSAGetLastError(); }

// There is no dummy raw-socket ABI here. The shipping provider owns only
// WSA startup/cleanup/error reporting; the legacy I/O slots stay null.
// Native fault-injection and Debug still compile the full legacy adapter.
const preloader_l::net::SocketApi kLifecycleOnly{
    nullptr, startup, cleanup, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, last_error
};
}
namespace preloader_l { namespace net {
const SocketApi* native_winsock_api() noexcept { return &kLifecycleOnly; }
} }
#endif
