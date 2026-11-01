#include "preloader_l/net/socket_runtime.hpp"
#include <cstring>

namespace {
struct Fake { int closes=0; int receives=0; int sends=0; };
int startup(void*, uint16_t) noexcept { return 0; }
int cleanup(void*) noexcept { return 0; }
preloader_l::net::SocketHandle open_tcp(void*) noexcept { return 20; }
int bind_loopback(void*, preloader_l::net::SocketHandle, uint16_t, uint16_t* p) noexcept { if(p)*p=44000; return 0; }
int listen_fn(void*, preloader_l::net::SocketHandle, int) noexcept { return 0; }
preloader_l::net::SocketHandle accept_fn(void*, preloader_l::net::SocketHandle s) noexcept { return s==20?21:preloader_l::net::invalid_socket_handle; }
preloader_l::net::TransportStatus recv_fn(void* c, preloader_l::net::SocketHandle s, uint8_t* out, size_t cap, size_t* n) noexcept {
    auto* f=static_cast<Fake*>(c); ++f->receives; if(s!=21) return preloader_l::net::TransportStatus::io_error; static const char msg[]="abc"; if(cap<3) return preloader_l::net::TransportStatus::io_error; std::memcpy(out,msg,3); if(n)*n=3; return preloader_l::net::TransportStatus::ok;
}
preloader_l::net::TransportStatus send_fn(void* c, preloader_l::net::SocketHandle s, const uint8_t* in, size_t n, size_t* sent) noexcept {
    auto* f=static_cast<Fake*>(c); ++f->sends; if(s!=21||n!=3||!in||in[0]!='x') return preloader_l::net::TransportStatus::io_error; if(sent)*sent=n; return preloader_l::net::TransportStatus::ok;
}
int close_fn(void* c, preloader_l::net::SocketHandle) noexcept { ++static_cast<Fake*>(c)->closes;  return 0;}
int last_error(void*) noexcept { return 0; }
}
int main(){ using namespace preloader_l::net;
    Fake f{}; SocketApi api{&f,startup,cleanup,open_tcp,bind_loopback,listen_fn,accept_fn,recv_fn,send_fn,close_fn,last_error};
    SocketRuntime rt{}; reset_socket_runtime(&rt); if(start_socket_runtime(&rt,&api)!=SocketStatus::ok) return 1;
    SocketListenerState listener{}; reset_socket_listener(&listener);
    if(create_loopback_listener(&rt,0,1,&listener)!=SocketStatus::ok) return 2;
    SocketEndpointState ep{}; reset_socket_endpoint(&ep); TransportEndpoint transport{};
    if(accept_socket_endpoint(&listener,&ep,&transport)!=SocketStatus::ok) return 3;
    if(!ep.open || ep.socket!=21 || !valid_endpoint(transport)) return 4;
    uint8_t buf[8]{}; size_t n=0; if(transport.receive(transport.context,buf,sizeof(buf),&n)!=TransportStatus::ok) return 5;
    if(n!=3 || buf[0]!='a' || f.receives!=1) return 6;
    const uint8_t out[3]={'x','y','z'}; size_t sent=0; if(transport.send(transport.context,out,3,&sent)!=TransportStatus::ok) return 7;
    if(sent!=3 || f.sends!=1) return 8;
    close_endpoint(&transport); if(ep.open || f.closes!=1) return 9;
    if(close_socket_listener(&listener)!=SocketStatus::ok || f.closes!=2) return 10;
    if(stop_socket_runtime(&rt)!=SocketStatus::ok) return 11;
    return 0;
}
