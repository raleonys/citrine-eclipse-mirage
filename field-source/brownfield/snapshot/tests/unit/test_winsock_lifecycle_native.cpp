// IDS Stage05 source-linked lifecycle acceptance; works with the shipping
// fail-closed provider and the retained complete native/debug provider.
#include "preloader_l/net/winsock_native.hpp"
#include "preloader_l/net/winsock_version_policy.hpp"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
using namespace preloader_l::net;
namespace {
int startup_rc=0,cleanup_rc=0,last_error_value=10022;
uint16_t negotiated=0x0202;
unsigned startup_calls=0,cleanup_calls=0,raw_calls=0;
void reset() noexcept {
    startup_rc=0;cleanup_rc=0;negotiated=0x0202;startup_calls=0;cleanup_calls=0;raw_calls=0;
}
}
extern "C" int WSAStartup(uint16_t requested,void* data) {
    ++startup_calls;
    assert(requested==winsock_requested_version);
    if (startup_rc) return startup_rc;
    auto* p=static_cast<uint8_t*>(data);
    p[0]=static_cast<uint8_t>(negotiated&0xffu);
    p[1]=static_cast<uint8_t>(negotiated>>8);
    return 0;
}
extern "C" int WSACleanup() { ++cleanup_calls; return cleanup_rc; }
extern "C" int WSAGetLastError() { return last_error_value; }
#if defined(IDS_TEST_FULL_NATIVE)
extern "C" uintptr_t socket(int,int,int) { ++raw_calls; return 42u; }
extern "C" int bind(uintptr_t,const void*,int) { ++raw_calls; return 0; }
extern "C" int setsockopt(uintptr_t,int,int,const char*,int) { ++raw_calls; return 0; }
extern "C" int listen(uintptr_t,int) { ++raw_calls; return 0; }
extern "C" uintptr_t accept(uintptr_t,void*,int*) { ++raw_calls; return 44u; }
extern "C" int recv(uintptr_t,char*,int,int) { ++raw_calls; return 0; }
extern "C" int send(uintptr_t,const char*,int,int) { ++raw_calls; return 0; }
extern "C" int closesocket(uintptr_t) { ++raw_calls; return 0; }
#endif
SocketStatus start_checked(SocketRuntime* runtime, const SocketApi* api) noexcept {
#if defined(IDS_TEST_FULL_NATIVE)
    return start_socket_runtime(runtime,api);
#else
    return start_socket_runtime_lifecycle(runtime,api);
#endif
}
int main() {
    const auto* api=native_winsock_api();
    assert(api && valid_socket_lifecycle_api(*api));
#if defined(IDS_TEST_FULL_NATIVE)
    assert(valid_socket_api(*api));
#else
    assert(!valid_socket_api(*api));
    assert(api->open_tcp==nullptr && api->bind_loopback==nullptr &&
           api->listen==nullptr && api->accept==nullptr &&
           api->receive==nullptr && api->send==nullptr && api->close==nullptr);
#endif
    assert(api==native_winsock_api());
    assert(api->last_error(nullptr)==10022);
    SocketRuntime runtime{};
    // Correct negotiation creates precisely one balanced startup reference.
    reset();reset_socket_runtime(&runtime);
    assert(start_checked(&runtime,api)==SocketStatus::ok);
    assert(runtime.started && runtime.startup_count==1 && runtime.cleanup_count==0);
    assert(startup_calls==1 && cleanup_calls==0);
    assert(start_checked(&runtime,api)==SocketStatus::already_started);
    assert(startup_calls==1);
    assert(stop_socket_runtime(&runtime)==SocketStatus::ok);
    assert(!runtime.started && runtime.cleanup_count==1 && cleanup_calls==1);
    assert(stop_socket_runtime(&runtime)==SocketStatus::not_started);
    // Startup failure returns the direct WSAStartup error and owns no ref.
    reset();reset_socket_runtime(&runtime);startup_rc=10093;
    assert(start_checked(&runtime,api)==SocketStatus::api_error);
    assert(runtime.last_error==10093 && !runtime.started && !runtime.api);
    assert(cleanup_calls==0);
    // Unsupported negotiation with successful compensating WSACleanup.
    reset();reset_socket_runtime(&runtime);negotiated=0x0101;
    assert(start_checked(&runtime,api)==SocketStatus::api_error);
    assert(runtime.last_error==winsock_version_not_supported);
    assert(!runtime.started && !runtime.api && cleanup_calls==1);
    // If that cleanup fails, the successful startup reference survives until retry.
    reset();reset_socket_runtime(&runtime);negotiated=0x0101;cleanup_rc=-1;
    assert(start_checked(&runtime,api)==SocketStatus::api_error);
    assert(runtime.last_error==winsock_version_cleanup_retry_required);
    assert(runtime.started && runtime.api==api && runtime.startup_count==1);
    assert(stop_socket_runtime(&runtime)==SocketStatus::api_error);
    assert(runtime.started && runtime.cleanup_count==0);
    cleanup_rc=0;
    assert(stop_socket_runtime(&runtime)==SocketStatus::ok);
    assert(!runtime.started && runtime.cleanup_count==1 && cleanup_calls==3);
    // Ordinary shutdown error must also hold ownership for retry.
    reset();reset_socket_runtime(&runtime);
    assert(start_checked(&runtime,api)==SocketStatus::ok);
    cleanup_rc=-1;
    assert(stop_socket_runtime(&runtime)==SocketStatus::api_error);
    assert(runtime.started && runtime.api==api && runtime.cleanup_count==0);
    cleanup_rc=0;
    assert(stop_socket_runtime(&runtime)==SocketStatus::ok);
    assert(runtime.cleanup_count==1 && cleanup_calls==2);
#if !defined(IDS_TEST_FULL_NATIVE)
    // The original strict legacy API entrypoint must reject this provider.
    reset();reset_socket_runtime(&runtime);
    assert(start_socket_runtime(&runtime,api)==SocketStatus::invalid_argument);
    assert(!runtime.started && !runtime.api);
    assert(raw_calls==0 && startup_calls==0 && cleanup_calls==0);
#else
    reset();
    assert(api->open_tcp(nullptr)==42u);
    assert(raw_calls==1);
#endif
    puts("WINSOCK_LIFECYCLE_SOURCE_LINKED: PASS");
}
