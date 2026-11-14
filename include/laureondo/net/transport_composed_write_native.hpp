// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/net/transport_composed_write.hpp"
namespace laureondo::socket {
struct AsyncSendImports {
    decltype(&::WSASend) send=&::WSASend;
    decltype(&::WSAGetLastError) get_error=&::WSAGetLastError;
};
// Construction invokes no imports. No WSAStartup, socket ownership or network
// lifecycle is supplied. Tests inject both exact SDK-typed import pointers.
class NativeAsyncSendApi final:public AsyncSendApi {
public:
    explicit NativeAsyncSendApi(AsyncSendImports imports={}):imports_(imports){}
    AsyncSendResult send(Socket,Buffer&,std::uint32_t,OVERLAPPED&) override;
private:AsyncSendImports imports_;
};
} // namespace laureondo::socket
