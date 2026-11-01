#pragma once
#include "../anadius-socket-source/async_receive.hpp"
#include "../anadius-socket-source/handler_dispatch.hpp"
#include <mswsock.h>

namespace anadius::socket_recovered {
struct AcceptEndpoint {std::array<std::byte,28> bytes{};};
struct AcceptPending {SocketObject& object;IocpSchedulerEntry* scheduler=nullptr;};
struct AsyncAcceptService {IocpSchedulerEntry* scheduler=nullptr;std::uintptr_t retry_engine_identity=0;};
struct AcceptAlternate {CancelOperation& operation;DispatcherOperationBinding& packet;};
struct AsyncAcceptOperation {
    explicit AsyncAcceptOperation(OperationStorage& storage):storage(storage){}
    OperationStorage& storage;CancelOperation operation;DispatcherOperationBinding* packet=nullptr;
    std::uintptr_t callback_identity=0,retry_engine_identity=0;
    Socket listener=invalid_socket,accepted=invalid_socket;AcceptPending* pending=nullptr;
    std::uint32_t protocol=0;AcceptEndpoint* endpoint_output=nullptr;
    // Original+68..187. Caller supplies this actual bounded raw allocation range.
    std::span<std::byte> address_buffer;
    std::uint8_t no_retry=0;AcceptAlternate* alternate=nullptr;std::uintptr_t retry_context=0;
    std::uintptr_t server_this=0;IoHandlerEnvironment environment;
};
struct AcceptExResult {std::int32_t result=0;std::uint32_t last_error=0;};
struct AcceptAddresses {std::span<const std::byte> remote;std::int32_t remote_length=0;};
class AsyncAcceptApi {
public:virtual ~AsyncAcceptApi()=default;
    virtual AcceptExResult accept(Socket listener,Socket accepted,std::span<std::byte>,OVERLAPPED&)=0;
    virtual AcceptAddresses addresses(std::span<std::byte>)=0;
    virtual std::int32_t update_context(Socket accepted,Socket& listener_value)=0;
};
struct AsyncAcceptImports {
    decltype(&::AcceptEx) accept=&::AcceptEx;
    decltype(&::GetAcceptExSockaddrs) addresses=&::GetAcceptExSockaddrs;
    decltype(&::setsockopt) option=&::setsockopt;
    decltype(&::WSAGetLastError) error=&::WSAGetLastError;
};
class NativeAsyncAcceptApi final:public AsyncAcceptApi {
public:explicit NativeAsyncAcceptApi(AsyncAcceptImports imports={}):imports_(imports){}
    AcceptExResult accept(Socket,Socket,std::span<std::byte>,OVERLAPPED&)override;
    AcceptAddresses addresses(std::span<std::byte>)override;
    std::int32_t update_context(Socket,Socket&)override;
private:AsyncAcceptImports imports_;
};
struct AcceptHandler {std::uintptr_t server_this=0;Error error;};
struct AcceptBorrowed {AcceptHandler& handler;std::uintptr_t invoke_identity=0x180022500ULL;};
using BorrowedAcceptSchedule=void(*)(IoHandlerEnvironment&,AcceptBorrowed&);
struct AcceptSchedule {OwnedHandlerSchedule owned_slot_10=nullptr;BorrowedAcceptSchedule borrowed_slot_18=nullptr;};
struct AcceptScheduleBinding {std::uintptr_t identity=0;AcceptSchedule* table=nullptr;};
struct AcceptTask:OwnedHandlerTask {
    AcceptTask(OperationStorage& storage):storage(storage){}
    OperationStorage& storage;AcceptHandler handler;std::uintptr_t callback_identity=0;
};
class AcceptLower {
public:virtual ~AcceptLower()=default;
    virtual void construct_environment(IoHandlerEnvironment&,std::uintptr_t allocator_identity)=0; //023dd0
    virtual Socket create_socket(std::uint32_t family,std::int32_t type,std::int32_t protocol,Error&)=0; //029df0
    virtual void reset_owned_socket(Socket&)=0; //029590; including invalidating captured holder
    virtual void reset_pending_owner(AcceptPending&)=0; //0237f0
    virtual Category misc_category()=0; //0285b0
    virtual Category success_category()=0; //logicalA10F8, no callback in native path
    virtual void retry(AsyncAcceptOperation&,CancelOperation& selected)=0; //29660 REQUIRED, no native reconstruction claim
    virtual void invoke(AcceptHandler&)=0; //021a10→025720 REQUIRED, raw server pointer not IoReference
};
class AsyncAcceptEffects:public AcceptLower {
public:virtual AsyncAcceptOperation& project(OperationStorage&)=0;
    virtual AcceptTask& project_task(OperationStorage&)=0;
};
struct AsyncAcceptContext {
    IoCompletionEnvironment environment;WorkerRunApi& worker;PostedMemoryEffects& memory;
    AsyncAcceptApi& api;DispatcherApi& completion;Api& errors;AcceptorApi& sockets;AsyncAcceptEffects& effects;
    std::span<AcceptScheduleBinding> schedules;
};
// Normal source bodies only. SDK/address/category/factory/retry/handler/context
// lifetime providers remain explicit; padding, native layout and EH not modeled.
void start_async_accept_021b00(AsyncAcceptService&,AcceptorState&,AcceptPending&,
    AcceptEndpoint* optional_endpoint,const std::uintptr_t& server_this,std::uintptr_t allocator_identity,AsyncAcceptContext&);
void complete_async_accept_0271a0(std::uintptr_t owner,AsyncAcceptOperation&,const Error&,AsyncAcceptContext&);
} // namespace anadius::socket_recovered
