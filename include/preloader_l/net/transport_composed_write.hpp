// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_handler_dispatch_bridge.hpp"
#include "preloader_l/net/transport_io_completion_dispatch.hpp"

namespace preloader_l::socket {
struct SendBufferSlice {std::uintptr_t data=0;std::uint64_t size=0;};
struct AsyncSendResult {std::int32_t result=0;std::uint32_t last_error=0,transferred=0;};
class AsyncSendApi {
public:
    virtual ~AsyncSendApi()=default;
    // Exactly one WSASend then unconditional immediate WSAGetLastError. The
    // SDK buffer uses low32 length; the original full64 size is kept separately.
    virtual AsyncSendResult send(Socket,Buffer&,std::uint32_t flags,OVERLAPPED&)=0;
};
struct AsyncSendService {IocpSchedulerEntry* scheduler_08=nullptr;};
struct SendSocketImplementation {
    Socket socket_00=invalid_socket;std::uint8_t state_08=0;
    std::uintptr_t weak_object_10=0;IoReference* weak_control_18=nullptr;
};
struct SendStreamBinding {
    std::uintptr_t identity=0;AsyncSendService* service=nullptr;
    SendSocketImplementation* implementation=nullptr;
    std::uintptr_t allocator_identity_68=0; // original member ADDRESS token, not contents
};
// Raw bytes, real SDK OVERLAPPED and logical ASIO projections are separate.
// Packet metadata belongs to the dispatcher binding, never cancellation.error.
struct AsyncSendOperation {
    explicit AsyncSendOperation(OperationStorage& storage):io{storage,{},nullptr,{},{}}{}
    SendIoOperation io;DispatcherOperationBinding* packet=nullptr;
    std::uintptr_t callback_identity_28=0,weak_object_38=0;
    SendBufferSlice buffer_48;
};
class ComposedWriteEffects;
struct ComposedWriteContext {
    IoCompletionEnvironment environment;WorkerRunApi& worker;PostedMemoryEffects& memory;
    AsyncSendApi& send;DispatcherApi& completion;Api& errors;ComposedWriteEffects& effects;
    std::span<SendStreamBinding> streams;
};
class ComposedWriteEffects {
public:
    virtual ~ComposedWriteEffects()=default;
    // Projection supplies a coherent, live operation and its existing
    // DispatcherOperationBinding before native initialization starts.
    virtual AsyncSendOperation& project_async_send(OperationStorage&)=0;
    virtual void construct_environment_023dd0(IoHandlerEnvironment&,std::uintptr_t allocator_identity)=0;
    // Native tail view begins at handler+30; this typed provider sees its
    // defined fields in SendHandler. The application/deque/log body stays open.
    virtual void finish_write_025a70(SendHandler&,Error& private_error)=0;
};
OperationStorage& allocate_io_storage_0269b0(std::uint64_t requested,PostedEnvironment,WorkerRunApi&,PostedMemoryEffects&);
void publish_send_status_028b50(IocpSchedulerEntry&,CancelOperation&,DispatcherOperationBinding&,
    std::uint32_t status,std::uint32_t bytes,DispatcherApi&,Api& errors);
void publish_send_started_028c00(IocpSchedulerEntry&,CancelOperation&,DispatcherApi&);
void start_async_send_021fd0(AsyncSendService&,SendSocketImplementation&,SendBufferSlice&,
    std::uint32_t flags,SendHandler&,std::uintptr_t allocator_identity,ComposedWriteContext&);
void composed_write_0261d0(SendHandler&,const Error&,std::uint64_t bytes,std::uint32_t flag,ComposedWriteContext&);
// Closes the previous0261d0 provider using the actual recovered source. Native
// receive body/factory/025a70 still remain mandatory. Context and dependencies
// must outlive all operations; no native object/OVERLAPPED ABI overlay is used.
class ComposedWriteEffects:public HandlerDispatchEffects,public ComposedWriteEffects {
public:
    ComposedWriteEffects(IoCompletionEnvironment environment,WorkerRunApi& worker,
        HandlerDispatchMemory& memory,std::span<HandlerScheduleBinding> schedules,
        AsyncSendApi& send,DispatcherApi& completion,Api& errors,std::span<SendStreamBinding> streams)
        :HandlerDispatchEffects(environment,worker,memory,schedules),
         context_{environment,worker,memory,send,completion,errors,*this,streams}{}
    void send_body_0261d0(SendHandler&,Error&,std::uint64_t,std::uint32_t) final;
    ComposedWriteContext& write_context(){return context_;}
private:ComposedWriteContext context_;
};
} // namespace preloader_l::socket
