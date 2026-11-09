// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_operations.hpp"
#include <ws2tcpip.h>
#include <windows.h>

namespace citrine_eclipse_mirage::socket {
class CategoryProvider {
public:
    virtual ~CategoryProvider()=default;
    virtual Category get_028620()=0;
};
struct Imports {
    decltype(&::WSASetLastError) set_last_error=&::WSASetLastError;
    decltype(&::WSAGetLastError) get_last_error=&::WSAGetLastError;
    decltype(&::MultiByteToWideChar) convert=&::MultiByteToWideChar;
    decltype(&::WSAStringToAddressW) parse=&::WSAStringToAddressW;
    decltype(&::WSASend) send=&::WSASend;
    decltype(&::select) select_write=&::select;
};
// Typed SDK adapter. Imports default to system APIs, but construction invokes
// none. Category factory/lifetime remains mandatory; no hidden WSAStartup.
class NativeApi final:public Api {
public:
    explicit NativeApi(CategoryProvider& categories,Imports imports={})
        :categories_(categories),imports_(imports){}
    void set_last_error(std::int32_t) override;
    Category system_category_028620() override;
    std::int32_t get_last_error() override;
    Conversion convert_acp(const char*,std::int32_t capacity) override;
    AddressCall string_to_address(Conversion&,std::int32_t family,std::int32_t initial_size) override;
    SendCall wsa_send(Socket,std::span<Buffer>,std::uint32_t flags) override;
    std::int32_t select_write(Socket,std::int32_t nfds) override;
private:CategoryProvider& categories_;Imports imports_;
};
} // namespace citrine_eclipse_mirage::socket
