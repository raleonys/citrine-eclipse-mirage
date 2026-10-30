#pragma once
#include "preloader_l/net/transport_io_completion.hpp"

namespace preloader_l::socket_recovered {
struct HandlerTaskContext;
class OwnedHandlerTask {
public:
    virtual ~OwnedHandlerTask()=default;
    virtual void call_slot_00(std::uint8_t flag)=0; // native flag0 discard, nonzero invoke
};
struct ReceiveHandlerTask final:OwnedHandlerTask {
    ReceiveHandlerTask(OperationStorage& storage,HandlerTaskContext& context):storage(storage),context(context){}
    OperationStorage& storage;HandlerTaskContext& context;
    ReceiveHandler handler;std::uintptr_t callback_identity_00=0;
    void call_slot_00(std::uint8_t) override;
};
struct SendHandlerTask final:OwnedHandlerTask {
    SendHandlerTask(OperationStorage& storage,HandlerTaskContext& context):storage(storage),context(context){}
    OperationStorage& storage;HandlerTaskContext& context;
    SendHandler handler;std::uintptr_t callback_identity_00=0;
    void call_slot_00(std::uint8_t) override;
};
// Borrowed descriptor is valid only during its scheduler call. Native callback
// identities are data, not executable image pointers; unsupported kinds aren't
// inferred. Handler ownership remains with its outer native caller.
struct BorrowedIoHandler {
    ReceiveHandler* receive=nullptr;SendHandler* send=nullptr;
    std::uintptr_t invoke_identity=0;
    void invoke(IoCompletionEffects&) const;
};
using BorrowedHandlerSchedule=void(*)(IoHandlerEnvironment&,BorrowedIoHandler&);
using OwnedHandlerSchedule=void(*)(IoHandlerEnvironment&,OwnedHandlerTask*&);
struct HandlerSchedule {
    OwnedHandlerSchedule owned_slot_10=nullptr;
    BorrowedHandlerSchedule borrowed_slot_18=nullptr;
};
struct HandlerScheduleBinding {std::uintptr_t identity=0;HandlerSchedule* table=nullptr;};
class HandlerDispatchMemory:public PostedMemoryEffects {
public:
    // Caller supplies coherent typed lifetime and owned bounded byte storage.
    // Projection itself is a selected host representation, not a new native
    // constructor or layout. Context must outlive every projected closure.
    virtual ReceiveHandlerTask& project_receive_task(OperationStorage&,HandlerTaskContext&)=0;
    virtual SendHandlerTask& project_send_task(OperationStorage&,HandlerTaskContext&)=0;
};
struct HandlerTaskContext {
    IoCompletionEnvironment environment;WorkerRunApi& api;HandlerDispatchMemory& memory;
    IoCompletionEffects& effects;std::span<HandlerScheduleBinding> schedules;
};
template<class T>struct HandlerTaskCleanup {
    std::uintptr_t unused_allocator_00=0;OperationStorage* allocation_08=nullptr;T* task_10=nullptr;
};
// RCX allocation tag is unused by native021900, including an uninitialized
// receive-side byte. It is deliberately absent from this defined-field API.
OperationStorage& allocate_handler_cached_021900(WorkerThreadState*,std::uint64_t requested,
    std::uint64_t alignment,PostedEnvironment,PostedMemoryEffects&);
void invoke_borrowed_receive_022640(ReceiveHandler&,IoCompletionEffects&);
void invoke_borrowed_send_022780(SendHandler&,IoCompletionEffects&);
void cleanup_receive_task_0291c0(HandlerTaskCleanup<ReceiveHandlerTask>&,HandlerTaskContext&);
void cleanup_send_task_0292a0(HandlerTaskCleanup<SendHandlerTask>&,HandlerTaskContext&);
void call_receive_task_022670(ReceiveHandlerTask&,std::uint8_t,HandlerTaskContext&);
void call_send_task_0227b0(SendHandlerTask&,std::uint8_t,HandlerTaskContext&);
void schedule_receive_handler_022c60(IoHandlerEnvironment&,ReceiveHandler&,HandlerTaskContext&);
void schedule_send_handler_0229d0(IoHandlerEnvironment&,SendHandler&,HandlerTaskContext&);
} // namespace preloader_l::socket_recovered
