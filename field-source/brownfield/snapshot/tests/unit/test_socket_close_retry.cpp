#include "preloader_l/net/socket_runtime.hpp"
#include <cassert>
using namespace preloader_l::net;
namespace {
struct F{int closes=0;int accepts=0;bool fail=true;int error=10054;SocketHandle next_accept=11;SocketHandle last_closed=invalid_socket_handle;};
int startup(void*,uint16_t) noexcept{return 0;} int cleanup(void*) noexcept{return 0;}
SocketHandle open_tcp(void*) noexcept{return 10;} int bind_loopback(void*,SocketHandle,uint16_t p,uint16_t*b) noexcept{*b=p?p:40000;return 0;}
int listen_fn(void*,SocketHandle,int) noexcept{return 0;} SocketHandle accept_fn(void*c,SocketHandle) noexcept{auto*f=static_cast<F*>(c);++f->accepts;return f->next_accept++;}
TransportStatus recv_fn(void*,SocketHandle,uint8_t*,size_t,size_t*n) noexcept{if(n)*n=0;return TransportStatus::closed;}
TransportStatus send_fn(void*,SocketHandle,const uint8_t*,size_t,size_t*n) noexcept{if(n)*n=0;return TransportStatus::closed;}
int close_fn(void*c,SocketHandle s) noexcept{auto*f=static_cast<F*>(c);++f->closes;f->last_closed=s;return f->fail?-1:0;} int last(void*c) noexcept{return static_cast<F*>(c)->error;}
}
int main(){F f{};SocketApi api{&f,startup,cleanup,open_tcp,bind_loopback,listen_fn,accept_fn,recv_fn,send_fn,close_fn,last};SocketRuntime rt{};reset_socket_runtime(&rt);assert(start_socket_runtime(&rt,&api)==SocketStatus::ok);
 SocketListenerState l{};reset_socket_listener(&l);assert(create_loopback_listener(&rt,3226,1,&l)==SocketStatus::ok);assert(close_socket_listener(&l)==SocketStatus::api_error);assert(l.socket==10&&l.runtime==&rt&&l.listening&&rt.last_error==10054);f.fail=false;assert(close_socket_listener(&l)==SocketStatus::ok);assert(l.socket==invalid_socket_handle&&!l.listening);
 f.fail=true;assert(create_loopback_listener(&rt,3226,1,&l)==SocketStatus::ok);SocketEndpointState e{};TransportEndpoint t{};assert(accept_socket_endpoint(&l,&e,&t)==SocketStatus::ok);assert(f.accepts==1&&e.socket==11);assert(close_socket_endpoint(&e)==SocketStatus::api_error);assert(e.open&&e.socket==11&&f.last_closed==11);TransportEndpoint blocked{};assert(accept_socket_endpoint(&l,&e,&blocked)==SocketStatus::already_started);assert(f.accepts==1&&e.open&&e.socket==11&&blocked.context==nullptr&&blocked.receive==nullptr&&blocked.send==nullptr&&blocked.close==nullptr);f.fail=false;assert(close_socket_endpoint(&e)==SocketStatus::ok);assert(f.last_closed==11&&!e.open&&e.socket==invalid_socket_handle);assert(accept_socket_endpoint(&l,&e,&t)==SocketStatus::ok);assert(f.accepts==2&&e.socket==12);assert(close_socket_endpoint(&e)==SocketStatus::ok&&f.last_closed==12);assert(close_socket_listener(&l)==SocketStatus::ok);return 0;}
