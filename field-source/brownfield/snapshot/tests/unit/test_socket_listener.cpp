#include "preloader_l/net/socket_runtime.hpp"
#include <cassert>

namespace {
struct Fake { int closes=0; int listens=0; int error=0; bool fail_bind=false; bool fail_listen=false; };
int startup(void*, uint16_t) noexcept { return 0; }
int cleanup(void*) noexcept { return 0; }
preloader_l::net::SocketHandle open_tcp(void*) noexcept { return 10; }
int bind_loopback(void* c, preloader_l::net::SocketHandle s, uint16_t requested, uint16_t* bound) noexcept {
    auto* f=static_cast<Fake*>(c); if(f->fail_bind) return -1; assert(s==10); if(bound)*bound=requested?requested:43210; return 0;
}
int listen_fn(void* c, preloader_l::net::SocketHandle s, int backlog) noexcept {
    auto* f=static_cast<Fake*>(c); ++f->listens; assert(s==10 && backlog==1); return f->fail_listen?-1:0;
}
preloader_l::net::SocketHandle accept_fn(void*, preloader_l::net::SocketHandle) noexcept { return 11; }
preloader_l::net::TransportStatus recv_fn(void*, preloader_l::net::SocketHandle, uint8_t*, size_t, size_t* n) noexcept { if(n)*n=0; return preloader_l::net::TransportStatus::would_block; }
preloader_l::net::TransportStatus send_fn(void*, preloader_l::net::SocketHandle, const uint8_t*, size_t n, size_t* s) noexcept { if(s)*s=n; return preloader_l::net::TransportStatus::ok; }
int close_fn(void* c, preloader_l::net::SocketHandle) noexcept { ++static_cast<Fake*>(c)->closes;  return 0;}
int last_error(void* c) noexcept { return static_cast<Fake*>(c)->error; }
}
int main(){ using namespace preloader_l::net;
    Fake f{}; SocketApi api{&f,startup,cleanup,open_tcp,bind_loopback,listen_fn,accept_fn,recv_fn,send_fn,close_fn,last_error};
    SocketRuntime rt{}; reset_socket_runtime(&rt); assert(start_socket_runtime(&rt,&api)==SocketStatus::ok);
    SocketListenerState listener{}; reset_socket_listener(&listener);
    assert(create_loopback_listener(&rt,0,1,&listener)==SocketStatus::ok);
    assert(listener.listening && listener.socket==10 && listener.port==43210 && f.listens==1);
    close_socket_listener(&listener); assert(!listener.listening && f.closes==1);
    f.fail_listen=true; assert(create_loopback_listener(&rt,3226,1,&listener)==SocketStatus::api_error); assert(f.closes==2);
    assert(stop_socket_runtime(&rt)==SocketStatus::ok);
}
