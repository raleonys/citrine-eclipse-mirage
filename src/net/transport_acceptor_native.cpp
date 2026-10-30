#include "preloader_l/net/transport_acceptor_native.hpp"
#include <bit>

namespace preloader_l::socket_recovered {
static_assert(sizeof(Socket)==8 && sizeof(SOCKET)==8 && sizeof(HANDLE)==8);
static_assert(sizeof(int)==4 && sizeof(u_long)==4 && sizeof(long)==4);
static_assert(FIONBIO==0x8004667eU && AF_INET==2 && AF_INET6==23);
Socket NativeAcceptorApi::create_socket(std::int32_t family){
    return static_cast<Socket>(imports_.create(family,1,6,nullptr,0,1));
}
std::uintptr_t NativeAcceptorApi::attach_completion_port(Socket socket,std::uintptr_t port){
    return reinterpret_cast<std::uintptr_t>(imports_.attach(
        reinterpret_cast<HANDLE>(socket),reinterpret_cast<HANDLE>(port),0,0));
}
std::uint32_t NativeAcceptorApi::get_win32_error(){return imports_.last();}
std::int32_t NativeAcceptorApi::set_option(Socket socket,std::int32_t level,std::int32_t option,std::int32_t value){
    return imports_.option(static_cast<SOCKET>(socket),level,option,
        reinterpret_cast<const char*>(&value),sizeof(value));
}
std::int32_t NativeAcceptorApi::bind_socket(Socket socket,std::span<const std::byte> address,std::int32_t size){
    if(size<0 || address.size()<static_cast<std::size_t>(size))
        throw UnsupportedInput("Native bind length exceeds initialized storage");
    return imports_.bind(static_cast<SOCKET>(socket),reinterpret_cast<const sockaddr*>(address.data()),size);
}
std::int32_t NativeAcceptorApi::listen_socket(Socket socket,std::int32_t backlog){
    return imports_.listen(static_cast<SOCKET>(socket),backlog);
}
std::int32_t NativeAcceptorApi::close_socket(Socket socket){return imports_.close(static_cast<SOCKET>(socket));}
std::int32_t NativeAcceptorApi::set_nonblocking(Socket socket,std::uint32_t command,std::uint32_t& value){
    u_long native=value;
    const auto result=imports_.ioctl(static_cast<SOCKET>(socket),std::bit_cast<long>(command),&native);
    value=native;return result;
}
} // namespace preloader_l::socket_recovered
