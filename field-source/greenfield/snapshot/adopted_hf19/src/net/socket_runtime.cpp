#include "preloader_l/net/socket_runtime.hpp"
#include "preloader_l/net/winsock_version_policy.hpp"

namespace preloader_l { namespace net {

bool valid_socket_api(const SocketApi& api) noexcept
{
    return api.startup && api.cleanup && api.open_tcp && api.bind_loopback &&
           api.listen && api.accept && api.receive && api.send && api.close &&
           api.last_error;
}

void reset_socket_runtime(SocketRuntime* runtime) noexcept
{
    if (!runtime) return;
    runtime->api = nullptr;
    runtime->started = false;
    runtime->startup_count = 0;
    runtime->cleanup_count = 0;
    runtime->last_error = 0;
}

bool valid_socket_lifecycle_api(const SocketApi& api) noexcept
{
    return api.startup && api.cleanup && api.last_error;
}

namespace {
SocketStatus start_socket_runtime_impl(SocketRuntime* runtime, const SocketApi* api,
                                       bool full_socket_api) noexcept
{
    if (!runtime || !api || !valid_socket_lifecycle_api(*api) ||
        (full_socket_api && !valid_socket_api(*api))) return SocketStatus::invalid_argument;
    if (runtime->started) return SocketStatus::already_started;
    runtime->api = api;
    const int startup_result = api->startup(api->context, winsock_requested_version);
    if (startup_result != 0) {
        // WSAStartup returns its extended error code directly. WSAGetLastError
        // is not the policy for a failed startup call. Preserve the API
        // result exactly so the enclosing service can diagnose the failure.
        runtime->last_error = startup_result;
        if (startup_result == winsock_version_cleanup_retry_required) {
            // HF17 / F1482-001: native WSAStartup succeeded, version negotiation
            // failed, and its immediate WSACleanup failed. Keep the API and a
            // live-started ownership bit so stop/shutdown can retry cleanup.
            runtime->started = true;
            ++runtime->startup_count;
        } else {
            runtime->api = nullptr;
        }
        return SocketStatus::api_error;
    }
    runtime->started = true;
    ++runtime->startup_count;
    runtime->last_error = 0;
    return SocketStatus::ok;
}
}

SocketStatus start_socket_runtime(SocketRuntime* runtime, const SocketApi* api) noexcept
{
    return start_socket_runtime_impl(runtime, api, true);
}

SocketStatus start_socket_runtime_lifecycle(SocketRuntime* runtime, const SocketApi* api) noexcept
{
    return start_socket_runtime_impl(runtime, api, false);
}

SocketStatus stop_socket_runtime(SocketRuntime* runtime) noexcept
{
    if (!runtime) return SocketStatus::invalid_argument;
    if (!runtime->started || !runtime->api) return SocketStatus::not_started;
    const SocketApi* api = runtime->api;
    const int result = api->cleanup(api->context);
    if (result != 0) {
        runtime->last_error = api->last_error(api->context);
        return SocketStatus::api_error;
    }
    runtime->started = false;
    ++runtime->cleanup_count;
    runtime->last_error = 0;
    runtime->api = nullptr;
    return SocketStatus::ok;
}


} }
