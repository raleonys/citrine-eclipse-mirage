// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_environment_constructor.hpp"

namespace preloader_l::socket {
struct ReceiveBufferSlice {std::uintptr_t data=0;std::uint64_t size=0;};
struct AsyncReceiveResult {std::int32_t result=0;std::uint32_t last_error=0,transferred=0;};
class AsyncReceiveApi {
public:
    virtual ~AsyncReceiveApi()=default;
    // One WSARecv followed immediately by unconditional WSAGetLastError.
    // Flags are a private in/out DWORD; their output is not used by021d10.
    virtual AsyncReceiveResult receive(Socket,Buffer&,std::uint32_t flags,OVERLAPPED&)=0;
};
struct AsyncReceiveImports {
    decltype(&::WSARecv) receive=&::WSARecv;
    decltype(&::WSAGetLastError) get_error=&::WSAGetLastError;
};
class NativeAsyncReceiveApi final:public AsyncReceiveApi {
public:
    explicit NativeAsyncReceiveApi(AsyncReceiveImports imports={}):imports_(imports){}
    AsyncReceiveResult receive(Socket,Buffer&,std::uint32_t,OVERLAPPED&) override;
private:AsyncReceiveImports imports_;
};
struct AsyncReceiveService {IocpSchedulerEntry* scheduler_08=nullptr;};
struct ReceiveSocketImplementation {
    Socket socket_00=invalid_socket;std::uint8_t state_08=0;
    std::uintptr_t weak_object_10=0;IoReference* weak_control_18=nullptr;
};
// Independent typed projections, not a native object/OVERLAPPED ABI overlay.
struct AsyncReceiveOperation {
    explicit AsyncReceiveOperation(OperationStorage& storage):io{storage,{},0,nullptr,0,{},{}}{}
    ReceiveIoOperation io;DispatcherOperationBinding* packet=nullptr;
    std::uintptr_t callback_identity_28=0,weak_object_40=0,buffer_data_50=0;
};
class AsyncReceiveEffects {
public:
    virtual ~AsyncReceiveEffects()=default;
    virtual AsyncReceiveOperation& project_async_receive(OperationStorage&)=0;
    virtual void construct_environment_023dd0(IoHandlerEnvironment&,std::uintptr_t allocator_identity)=0;
};
struct AsyncReceiveContext {
    IoCompletionEnvironment environment;WorkerRunApi& worker;PostedMemoryEffects& memory;
    AsyncReceiveApi& receive;DispatcherApi& completion;Api& errors;AsyncReceiveEffects& effects;
};
void start_async_receive_021d10(AsyncReceiveService&,ReceiveSocketImplementation&,ReceiveBufferSlice&,
    std::uint32_t flags,ReceiveHandler&,std::uintptr_t allocator_identity,AsyncReceiveContext&);
// Uses the 023dd0 constructor contract with explicit live source identities.
// Projection, native factory/query/move/destroy and lifetime providers stay required.
class AsyncReceiveEffectsAdapter:public AsyncReceiveEffects {
public:
    AsyncReceiveEffectsAdapter(IoHandlerSentinels sentinels,IoEnvironmentConstructorContext& constructor,
        std::span<IoEnvironmentSourceBinding> sources):sentinels_(sentinels),constructor_(constructor),sources_(sources){}
    void construct_environment_023dd0(IoHandlerEnvironment&,std::uintptr_t allocator_identity) final;
private:
    IoHandlerSentinels sentinels_;IoEnvironmentConstructorContext& constructor_;
    std::span<IoEnvironmentSourceBinding> sources_;
};
} // namespace preloader_l::socket
