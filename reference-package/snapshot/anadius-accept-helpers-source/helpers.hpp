#pragma once
#include "../anadius-accept-source/accept.hpp"

namespace anadius::socket_recovered {
class SocketFactoryApi {
public:virtual ~SocketFactoryApi()=default;
    virtual Socket open(std::int32_t family,std::int32_t type,std::int32_t protocol)=0;
};
struct SocketFactoryImports {decltype(&::WSASocketW) open=&::WSASocketW;};
class NativeSocketFactoryApi final:public SocketFactoryApi {
public:explicit NativeSocketFactoryApi(SocketFactoryImports imports={}):imports_(imports){}
    Socket open(std::int32_t,std::int32_t,std::int32_t)override;
private:SocketFactoryImports imports_;
};
// Unlike the initial accept adapter, native029660 accepts an explicit per-side
// address length. The buffer must cover both ranges; receive-data length is0.
class RetryAcceptApi {
public:virtual ~RetryAcceptApi()=default;
    virtual AcceptExResult accept(Socket,Socket,std::span<std::byte>,std::uint32_t address_length,OVERLAPPED&)=0;
};
class NativeRetryAcceptApi final:public RetryAcceptApi {
public:explicit NativeRetryAcceptApi(AsyncAcceptImports imports={}):imports_(imports){}
    AcceptExResult accept(Socket,Socket,std::span<std::byte>,std::uint32_t,OVERLAPPED&)override;
private:AsyncAcceptImports imports_;
};
class RetryCancelApi {
public:virtual ~RetryCancelApi()=default;
    virtual std::int32_t cancel(Socket listener,OVERLAPPED&)=0;
};
class NativeRetryCancelApi final:public RetryCancelApi {
public:explicit NativeRetryCancelApi(decltype(&::CancelIoEx) call=&::CancelIoEx):call_(call){}
    std::int32_t cancel(Socket,OVERLAPPED&)override;
private:decltype(&::CancelIoEx) call_;
};
struct RetryEngine {IocpSchedulerEntry* scheduler=nullptr;};
struct AcceptRetryContext {
    SocketFactoryApi& factory;AcceptorApi& sockets;Api& errors;
    RetryAcceptApi& accept;RetryCancelApi& cancel;DispatcherApi& completion;
};
// Defined normal source semantics only: categories are typed identity tokens,
// not mutable native category/vtable objects; padding and EH remain unmodeled.
Socket create_socket_029df0(std::int32_t family,std::int32_t type,std::int32_t protocol,
    Error&,SocketFactoryApi&,AcceptorApi&,Api&);
void reset_socket_holder_029590(Socket&,AcceptorApi&,Api&);
void retry_accept_029660(RetryEngine&,Socket listener,Socket& holder,
    std::int32_t family,std::int32_t type,std::int32_t protocol,std::span<std::byte>,
    std::uint32_t address_length,std::atomic<std::int32_t>* cancellation,
    CancelOperation& selected,DispatcherOperationBinding&,AcceptRetryContext&);
} // namespace anadius::socket_recovered
