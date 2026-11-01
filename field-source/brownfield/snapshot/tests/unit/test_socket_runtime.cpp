#include "preloader_l/net/socket_runtime.hpp"
#include <cassert>

namespace {
struct Fake { int starts=0, stops=0, error=0; };
int startup(void* c, uint16_t v) noexcept { auto* f=static_cast<Fake*>(c); ++f->starts; return v==0x0202u?0:-1; }
int cleanup(void* c) noexcept { ++static_cast<Fake*>(c)->stops; return 0; }
preloader_l::net::SocketHandle open_tcp(void*) noexcept { return 7; }
int bind_loopback(void*, preloader_l::net::SocketHandle, uint16_t p, uint16_t* b) noexcept { if(b)*b=p?p:40000; return 0; }
int listen_fn(void*, preloader_l::net::SocketHandle, int) noexcept { return 0; }
preloader_l::net::SocketHandle accept_fn(void*, preloader_l::net::SocketHandle) noexcept { return 8; }
preloader_l::net::TransportStatus recv_fn(void*, preloader_l::net::SocketHandle, uint8_t*, size_t, size_t* n) noexcept { if(n)*n=0; return preloader_l::net::TransportStatus::would_block; }
preloader_l::net::TransportStatus send_fn(void*, preloader_l::net::SocketHandle, const uint8_t*, size_t n, size_t* s) noexcept { if(s)*s=n; return preloader_l::net::TransportStatus::ok; }
int close_fn(void*, preloader_l::net::SocketHandle) noexcept { return 0;}
int last_error(void* c) noexcept { return static_cast<Fake*>(c)->error; }
}
int main(){
    using namespace preloader_l::net;
    Fake fake{};
    SocketApi api{&fake,startup,cleanup,open_tcp,bind_loopback,listen_fn,accept_fn,recv_fn,send_fn,close_fn,last_error};
    assert(valid_socket_api(api));
    SocketRuntime rt{}; reset_socket_runtime(&rt);
    assert(start_socket_runtime(&rt,&api)==SocketStatus::ok);
    assert(rt.started && rt.startup_count==1 && fake.starts==1);
    assert(start_socket_runtime(&rt,&api)==SocketStatus::already_started);
    assert(stop_socket_runtime(&rt)==SocketStatus::ok);
    assert(!rt.started && rt.cleanup_count==1 && fake.stops==1);
    assert(stop_socket_runtime(&rt)==SocketStatus::not_started);
}
