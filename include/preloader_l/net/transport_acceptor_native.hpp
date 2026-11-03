// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_acceptor.hpp"
#include <ws2tcpip.h>
#include <windows.h>

namespace preloader_l::socket {
struct AcceptorImports {
    decltype(&::WSASocketW) create=&::WSASocketW;
    decltype(&::CreateIoCompletionPort) attach=&::CreateIoCompletionPort;
    decltype(&::GetLastError) last=&::GetLastError;
    decltype(&::setsockopt) option=&::setsockopt;
    decltype(&::bind) bind=&::bind;
    decltype(&::listen) listen=&::listen;
    decltype(&::closesocket) close=&::closesocket;
    decltype(&::ioctlsocket) ioctl=&::ioctlsocket;
};
// SDK signatures with replaceable imports; no WSAStartup or handle ownership.
class NativeAcceptorApi final:public AcceptorApi {
public:
    explicit NativeAcceptorApi(AcceptorImports imports={}):imports_(imports){}
    Socket create_socket(std::int32_t family) override;
    std::uintptr_t attach_completion_port(Socket,std::uintptr_t port) override;
    std::uint32_t get_win32_error() override;
    std::int32_t set_option(Socket,std::int32_t level,std::int32_t option,std::int32_t value) override;
    std::int32_t bind_socket(Socket,std::span<const std::byte>,std::int32_t size) override;
    std::int32_t listen_socket(Socket,std::int32_t backlog) override;
    std::int32_t close_socket(Socket) override;
    std::int32_t set_nonblocking(Socket,std::uint32_t command,std::uint32_t& value) override;
private:AcceptorImports imports_;
};
} // namespace preloader_l::socket
