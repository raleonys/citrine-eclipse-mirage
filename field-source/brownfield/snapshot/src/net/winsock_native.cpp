#if !defined(_WIN32)
#error winsock_native.cpp is Windows-only.
#endif

#include "preloader_l/net/winsock_native.hpp"
#include "preloader_l/net/winsock_version_policy.hpp"
#include <stddef.h>
#include <stdint.h>

namespace {
using NativeSocket = uintptr_t;
constexpr NativeSocket kInvalidSocket = static_cast<NativeSocket>(~static_cast<NativeSocket>(0));
constexpr int kSocketError = -1;
constexpr int kAfInet = 2;
constexpr int kSockStream = 1;
constexpr int kIpProtoTcp = 6;
constexpr int kSolSocket = 0xffff;
constexpr int kSoReuseAddr = 0x0004;
constexpr int kWsaWouldBlock = 10035;

struct SockAddr { uint16_t family; char data[14]; };
struct SockAddrIn {
    uint16_t family;
    uint16_t port;
    uint32_t address;
    uint8_t zero[8];
};
static_assert(sizeof(SockAddrIn) == 16, "IPv4 sockaddr size mismatch");

extern "C" {
__declspec(dllimport) int __stdcall WSAStartup(uint16_t version, void* data);
__declspec(dllimport) int __stdcall WSACleanup();
__declspec(dllimport) int __stdcall WSAGetLastError();
__declspec(dllimport) NativeSocket __stdcall socket(int af, int type, int protocol);
__declspec(dllimport) int __stdcall bind(NativeSocket s, const SockAddr* name, int name_len);
__declspec(dllimport) int __stdcall setsockopt(NativeSocket s, int level, int optname, const char* optval, int optlen);
__declspec(dllimport) int __stdcall listen(NativeSocket s, int backlog);
__declspec(dllimport) NativeSocket __stdcall accept(NativeSocket s, SockAddr* addr, int* addr_len);
__declspec(dllimport) int __stdcall recv(NativeSocket s, char* buffer, int length, int flags);
__declspec(dllimport) int __stdcall send(NativeSocket s, const char* buffer, int length, int flags);
__declspec(dllimport) int __stdcall closesocket(NativeSocket s);
}

constexpr uint16_t swap16(uint16_t value) noexcept
{
    return static_cast<uint16_t>((value << 8) | (value >> 8));
}

int startup(void*, uint16_t version) noexcept
{
    alignas(void*) uint8_t data[408]{};
    const int result = WSAStartup(version, data);
    if (result != 0) return result;

    // F1421-002: a successful WSAStartup registration is not sufficient by
    // itself. The provider must negotiate the exact Winsock 2.2 contract used
    // by the shipping socket layer. A mismatch owns one successful startup
    // reference, so clean it up before returning fail-closed to SocketRuntime.
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
preloader_l::net::SocketHandle open_tcp(void*) noexcept
{
    const NativeSocket s = socket(kAfInet, kSockStream, kIpProtoTcp);
    return static_cast<preloader_l::net::SocketHandle>(s);
}
int bind_loopback(void*, preloader_l::net::SocketHandle handle,
                  uint16_t requested_port, uint16_t* bound_port) noexcept
{
    if (!bound_port) return kSocketError;
    *bound_port = 0;
    SockAddrIn address{};
    address.family = static_cast<uint16_t>(kAfInet);
    address.port = swap16(requested_port);
    // Consumer/Donor1-converged local LSX listener. Reuse remains a required
    // compatibility option, but a failed option application is not silently
    // ignored: startup must not claim a listener policy it did not establish.
    const int reuse = 1;
    if (setsockopt(static_cast<NativeSocket>(handle), kSolSocket, kSoReuseAddr,
                   reinterpret_cast<const char*>(&reuse), static_cast<int>(sizeof(reuse))) == kSocketError)
        return kSocketError;
    // sockaddr_in::sin_addr bytes 7F 00 00 01 on the supported little-endian
    // Windows target: bind the LSX service to IPv4 loopback only.
    address.address = 0x0100007fu;
    if (bind(static_cast<NativeSocket>(handle), reinterpret_cast<const SockAddr*>(&address),
             static_cast<int>(sizeof(address))) == kSocketError) return kSocketError;

    // Active listener selection binds an explicit 3226..3236 candidate; no
    // active bound-port query import is not present in the curio surface. A
    // successful bind therefore publishes the requested candidate directly.
    *bound_port = requested_port;
    return *bound_port == 0 ? kSocketError : 0;
}
int listen_fn(void*, preloader_l::net::SocketHandle handle, int backlog) noexcept
{
    return listen(static_cast<NativeSocket>(handle), backlog);
}
preloader_l::net::SocketHandle accept_fn(void*, preloader_l::net::SocketHandle listener) noexcept
{
    return static_cast<preloader_l::net::SocketHandle>(
        accept(static_cast<NativeSocket>(listener), nullptr, nullptr));
}
preloader_l::net::TransportStatus receive_fn(void*, preloader_l::net::SocketHandle handle,
                                         uint8_t* output, size_t capacity,
                                         size_t* received) noexcept
{
    if (received) *received = 0;
    if (!output || !received || capacity == 0 || capacity > 0x7FFFFFFFu)
        return preloader_l::net::TransportStatus::invalid_argument;
    const int result = recv(static_cast<NativeSocket>(handle), reinterpret_cast<char*>(output),
                            static_cast<int>(capacity), 0);
    if (result > 0) { *received = static_cast<size_t>(result); return preloader_l::net::TransportStatus::ok; }
    if (result == 0) return preloader_l::net::TransportStatus::closed;
    return WSAGetLastError() == kWsaWouldBlock ? preloader_l::net::TransportStatus::would_block
                                               : preloader_l::net::TransportStatus::io_error;
}
preloader_l::net::TransportStatus send_fn(void*, preloader_l::net::SocketHandle handle,
                                      const uint8_t* input, size_t length,
                                      size_t* sent) noexcept
{
    if (sent) *sent = 0;
    if (!input || !sent || length == 0 || length > 0x7FFFFFFFu)
        return preloader_l::net::TransportStatus::invalid_argument;
    const int result = send(static_cast<NativeSocket>(handle), reinterpret_cast<const char*>(input),
                            static_cast<int>(length), 0);
    if (result > 0) { *sent = static_cast<size_t>(result); return preloader_l::net::TransportStatus::ok; }
    if (result == 0) return preloader_l::net::TransportStatus::closed;
    return WSAGetLastError() == kWsaWouldBlock ? preloader_l::net::TransportStatus::would_block
                                               : preloader_l::net::TransportStatus::io_error;
}
int close_fn(void*, preloader_l::net::SocketHandle handle) noexcept
{
    if (handle == preloader_l::net::invalid_socket_handle || handle == kInvalidSocket) return 0;
    return closesocket(static_cast<NativeSocket>(handle));
}
int last_error(void*) noexcept { return WSAGetLastError(); }

const preloader_l::net::SocketApi kApi{
    nullptr, startup, cleanup, open_tcp, bind_loopback, listen_fn, accept_fn,
    receive_fn, send_fn, close_fn, last_error
};
}

namespace preloader_l { namespace net {
const SocketApi* native_winsock_api() noexcept { return &kApi; }
} }
