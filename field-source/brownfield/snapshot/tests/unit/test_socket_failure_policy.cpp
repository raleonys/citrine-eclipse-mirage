#include "preloader_l/net/socket_runtime.hpp"
#include "preloader_l/net/winsock_version_policy.hpp"
#include <cstdint>

namespace {
struct Fake {
    int startup_result = 0;
    int cleanup_failures = 0;
    int last_error_value = 0;
    unsigned starts = 0;
    unsigned stops = 0;
};
int startup(void* c, uint16_t version) noexcept {
    auto* f = static_cast<Fake*>(c);
    ++f->starts;
    if (version != 0x0202u) return 10022;
    return f->startup_result;
}
int cleanup(void* c) noexcept {
    auto* f = static_cast<Fake*>(c);
    ++f->stops;
    if (f->cleanup_failures > 0) {
        --f->cleanup_failures;
        return -1;
    }
    return 0;
}
preloader_l::net::SocketHandle open_tcp(void*) noexcept { return 7; }
int bind_loopback(void*, preloader_l::net::SocketHandle, uint16_t p, uint16_t* b) noexcept { if (b) *b = p; return 0; }
int listen_fn(void*, preloader_l::net::SocketHandle, int) noexcept { return 0; }
preloader_l::net::SocketHandle accept_fn(void*, preloader_l::net::SocketHandle) noexcept { return 8; }
preloader_l::net::TransportStatus recv_fn(void*, preloader_l::net::SocketHandle, uint8_t*, size_t, size_t* n) noexcept {
    if (n) *n = 0;
    return preloader_l::net::TransportStatus::would_block;
}
preloader_l::net::TransportStatus send_fn(void*, preloader_l::net::SocketHandle, const uint8_t*, size_t n, size_t* sent) noexcept {
    if (sent) *sent = n;
    return preloader_l::net::TransportStatus::ok;
}
int close_fn(void*, preloader_l::net::SocketHandle) noexcept { return 0;}
int last_error(void* c) noexcept { return static_cast<Fake*>(c)->last_error_value; }

preloader_l::net::SocketApi make_api(Fake* f) noexcept {
    return preloader_l::net::SocketApi{
        f, startup, cleanup, open_tcp, bind_loopback, listen_fn, accept_fn,
        recv_fn, send_fn, close_fn, last_error
    };
}
}

int main() {
    using namespace preloader_l::net;

    // F1421-001: WSAStartup's direct return code is the failure policy.
    Fake startup_fail{};
    startup_fail.startup_result = 10093;
    startup_fail.last_error_value = 777;
    const auto startup_api = make_api(&startup_fail);
    SocketRuntime startup_runtime{};
    reset_socket_runtime(&startup_runtime);
    if (start_socket_runtime(&startup_runtime, &startup_api) != SocketStatus::api_error) return 1;
    if (startup_runtime.last_error != 10093) return 2;
    if (startup_runtime.started || startup_runtime.api != nullptr || startup_runtime.startup_count != 0) return 3;
    if (startup_fail.starts != 1) return 4;

    // F1421-003 prerequisite: a failed cleanup retains the live registration
    // and API policy so an enclosing owner can retry it.
    Fake cleanup_retry{};
    cleanup_retry.cleanup_failures = 1;
    cleanup_retry.last_error_value = 10014;
    const auto cleanup_api = make_api(&cleanup_retry);
    SocketRuntime cleanup_runtime{};
    reset_socket_runtime(&cleanup_runtime);
    if (start_socket_runtime(&cleanup_runtime, &cleanup_api) != SocketStatus::ok) return 5;
    if (stop_socket_runtime(&cleanup_runtime) != SocketStatus::api_error) return 6;
    if (!cleanup_runtime.started || cleanup_runtime.api != &cleanup_api) return 7;
    if (cleanup_runtime.cleanup_count != 0 || cleanup_runtime.last_error != 10014) return 8;
    if (stop_socket_runtime(&cleanup_runtime) != SocketStatus::ok) return 9;
    if (cleanup_runtime.started || cleanup_runtime.api != nullptr) return 10;
    if (cleanup_runtime.cleanup_count != 1 || cleanup_retry.stops != 2) return 11;

    // HF17 / F1482-001: a native mismatch whose immediate WSACleanup failed
    // reports the internal retained-ownership sentinel. SocketRuntime must keep
    // api/started state so the enclosing stop/shutdown path can balance it.
    Fake mismatch_cleanup{};
    mismatch_cleanup.startup_result = winsock_version_cleanup_retry_required;
    mismatch_cleanup.cleanup_failures = 1;
    mismatch_cleanup.last_error_value = 10014;
    const auto mismatch_api = make_api(&mismatch_cleanup);
    SocketRuntime mismatch_runtime{};
    reset_socket_runtime(&mismatch_runtime);
    if (start_socket_runtime(&mismatch_runtime, &mismatch_api) != SocketStatus::api_error) return 12;
    if (!mismatch_runtime.started || mismatch_runtime.api != &mismatch_api) return 13;
    if (mismatch_runtime.startup_count != 1u || mismatch_runtime.last_error != winsock_version_cleanup_retry_required) return 14;
    if (stop_socket_runtime(&mismatch_runtime) != SocketStatus::api_error) return 15;
    if (!mismatch_runtime.started || mismatch_runtime.api != &mismatch_api) return 16;
    if (stop_socket_runtime(&mismatch_runtime) != SocketStatus::ok) return 17;
    if (mismatch_runtime.started || mismatch_runtime.api != nullptr) return 18;
    if (mismatch_runtime.cleanup_count != 1u || mismatch_cleanup.stops != 2u) return 19;
    return 0;
}
