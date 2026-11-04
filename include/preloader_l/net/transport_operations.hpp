// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include "preloader_l/net/server.hpp"
#include <optional>
#include <stdexcept>

namespace preloader_l::socket {
using Socket=std::uintptr_t;
using Buffer=WSABUF; // Actual SDK type; no aliasing cast from a guessed layout.
struct Category {
    // Opaque identity of the category's vtable+8 name function, not its object.
    // Tokens are compared only; they are never called or treated as image VAs.
    std::uintptr_t name_identity=0;
    server::ErrorCategory logical=server::ErrorCategory::system;
};
struct Error {std::int32_t value=0;Category category{};};
struct Conversion {std::int32_t result=0;std::optional<std::wstring> text;};
struct AddressCall {std::int32_t result=-1;std::array<std::uint8_t,16> bytes{};std::uint32_t scope=0;};
struct SendCall {std::int32_t result=0;std::uint32_t transferred=0;};
class UnsupportedInput:public std::invalid_argument {public:using std::invalid_argument::invalid_argument;};
class Api {
public:
    virtual ~Api()=default;
    virtual void set_last_error(std::int32_t)=0;
    // Explicit hook: native028620 is lazy and may affect LastError. Preserve
    // its position before every last-error read and at category comparisons.
    virtual Category system_category_028620()=0;
    virtual std::int32_t get_last_error()=0;
    virtual Conversion convert_acp(const char* text,std::int32_t capacity)=0;
    virtual AddressCall string_to_address(Conversion&,std::int32_t family,std::int32_t initial_size)=0;
    virtual SendCall wsa_send(Socket,std::span<Buffer>,std::uint32_t flags)=0;
    virtual std::int32_t select_write(Socket,std::int32_t nfds)=0;
};
// Logical int32 return and defined outputs only; native RAX extension, error
// padding, dynamic-stack footprint and undefined conversion memory are outside.
std::int32_t address_028690(std::int32_t family,std::string_view,
    std::array<std::uint8_t,16>& output,std::uint32_t* scope,Error&,Api&);
std::int64_t send_02a200(Socket,std::uint8_t state,std::span<Buffer>,
    std::uint32_t flags,bool all_empty,Error&,Api&);
// New partial source bridges. Remaining ASIO/socket ownership methods are
// abstract; these classes do not create a socket, context or listener.
class AddressBackend:public server::ServerBackend {
public:
    explicit AddressBackend(Api& api):api_(api){}
    server::ParsedAddress parse_address_028690(std::uint16_t,std::string_view) final;
private:Api& api_;
};
class SynchronousTransport:public server::Transport {
public:
    SynchronousTransport(const Socket& socket,const std::uint8_t& state,Api& api)
        :socket_(socket),state_(state),api_(api){}
    server::WriteResult synchronous_send(std::span<const char>) final;
private:const Socket& socket_;const std::uint8_t& state_;Api& api_;
};
} // namespace preloader_l::socket
