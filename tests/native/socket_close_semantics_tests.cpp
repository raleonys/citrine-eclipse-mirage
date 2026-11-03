// SPDX-License-Identifier: GPL-3.0-only

#include <winsock2.h>
#include <windows.h>
#include <stdexcept>
#include <iostream>
namespace fault {unsigned remaining=0,calls=0;int error=0;}
int WSAAPI TestCloseSocket(SOCKET socket){
    ++fault::calls;
    if(fault::remaining){--fault::remaining;WSASetLastError(fault::error);return SOCKET_ERROR;}
    return ::closesocket(socket);
}
// Header-only Asio uses the controlled Win32 boundary only in this executable.
#define closesocket TestCloseSocket
#include <asio.hpp>
#undef closesocket
#if !defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP)
#error This contract requires the Windows IOCP Asio implementation.
#endif
namespace {
void require(bool b,const char* text){if(!b)throw std::runtime_error(text);}
void scenario(unsigned failures,int error){
    using Tcp=asio::ip::tcp;
    asio::io_context io;Tcp::acceptor listener(io,{asio::ip::address_v4::loopback(),0});
    Tcp::socket peer(io);peer.connect(listener.local_endpoint());auto socket=listener.accept();
    const auto raw=socket.native_handle();
    struct NativeRetirement {SOCKET value;bool owned=false;~NativeRetirement(){if(owned)::closesocket(value);}} retained{raw};
    fault::calls=0;fault::remaining=failures;fault::error=error;
    asio::error_code ec;socket.close(ec);const auto calls=fault::calls;
    // Preserve the raw descriptor in the test before any failing oracle can
    // unwind; Asio has already discarded its implementation handle on error.
    retained.owned=bool(ec);
    require(!socket.is_open(),"IOCP close clears the Asio implementation handle");
    if(!ec){require(calls==(failures?2u:1u),"normal/would-block recovery close attempts");}
    else{
        require(ec.value()==error&&calls==failures,"injected native close error propagated");
        sockaddr_storage address{};int length=sizeof address;
        require(::getpeername(raw,reinterpret_cast<sockaddr*>(&address),&length)==0,"withheld native close retains actual socket");
        asio::error_code retry;socket.close(retry);
        require(!retry&&fault::calls==calls,"repeating Asio close cannot retry discarded native handle");
    }
    fault::remaining=0;
    std::cout<<"PASS: IOCP close semantics failures="<<failures<<" error="<<error<<" retained="<<retained.owned<<'\n';
}
}
int main()try{
    scenario(0,0);scenario(1,WSAEWOULDBLOCK);scenario(2,WSAEWOULDBLOCK);scenario(1,WSAENETDOWN);
    std::cout<<"NOTE: characterizes library semantics; does not claim product close recovery\n";
}catch(const std::exception& e){fault::remaining=0;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
