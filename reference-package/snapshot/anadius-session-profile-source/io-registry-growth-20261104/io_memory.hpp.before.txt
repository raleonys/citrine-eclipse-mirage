#pragma once
#include "../anadius-session-memory-source/memory.hpp"
#include "../anadius-socket-source/async_receive.hpp"
#include "../anadius-socket-source/owned_response.hpp"
#include "../anadius-socket-source/io_completion_dispatch.hpp"
#include <vector>

namespace anadius::socket_recovered {
struct IoMemoryHandle {std::size_t slot=0;std::uint64_t generation=0;bool operator==(const IoMemoryHandle&)const=default;};
enum class IoMemoryKind {None,Posted,Receive,Send,ReceiveTask,SendTask,Response};
enum class IoSubmission {NotSubmitted,Unclassified,Issuing,Kernel,Software,Unknown};
struct IoLoan {
    IoMemoryHandle handle;IoMemoryKind kind;IoSubmission state;std::uint64_t submission;
    Socket socket;OVERLAPPED* overlapped;CancelOperation* operation;
    IoReference* source_control;std::uintptr_t buffer;std::uint64_t bytes;
    bool pending,completed;
    // Private projection remains authoritative. Posted may be an intentional
    // accept reservation; the Stop coordinator must resolve a mismatching
    // public binding against its actual OwnedAcceptMemory before ignoring it.
    bool binding_valid=true;CancelOperation* binding_operation=nullptr;OVERLAPPED* binding_overlapped=nullptr;
};
enum class IoMemoryPhase {Open,Retiring,Closed,Poisoned};
struct IoMemorySnapshot {IoMemoryPhase phase=IoMemoryPhase::Open;std::size_t allocations=0,pending=0,cancel_requested=0,active_calls=0;};
// Concrete storage/projection composition, NOT a native ASIO layout overlay.
// Hold parent Operation around the entire source call. Parent and all supplied
// handler/dispatcher/environment contexts outlive pending work and cache reuse.
// Fixed-capacity binding arrays are never resized while contexts borrow spans.
class OwnedIoMemory final:public HandlerDispatchMemory {
public:
    OwnedIoMemory(std::shared_ptr<SessionMemoryDomain>,std::size_t maximum_allocations);
    ~OwnedIoMemory()noexcept;
    OwnedIoMemory(const OwnedIoMemory&)=delete;OwnedIoMemory& operator=(const OwnedIoMemory&)=delete;
    OperationStorage& allocate_04b0bc(std::uint64_t native_bytes)override;
    void free_04b788(OperationStorage&)override;
    void free_cached_identity(std::uintptr_t); // worker source owns terminal cache ordering; no hidden slot clear
    PostedOperation& project_posted(OperationStorage&)override;
    ReceiveHandlerTask& project_receive_task(OperationStorage&,HandlerTaskContext&)override;
    SendHandlerTask& project_send_task(OperationStorage&,HandlerTaskContext&)override;
    AsyncReceiveOperation& project_async_receive(OperationStorage&);
    AsyncSendOperation& project_async_send(OperationStorage&);
    OwnedResponseMemory& response_memory();
    std::span<OperationStorageBinding> storage_bindings();
    std::span<DispatcherOperationBinding> dispatcher_bindings();
    IoMemoryHandle handle_for(const OperationStorage&)const;
    IoMemoryHandle handle_for(const OVERLAPPED*)const;
    IoMemoryHandle handle_for(const CancelOperation*)const;
    IoMemoryKind kind(IoMemoryHandle)const;
    AsyncReceiveOperation& receive_operation(IoMemoryHandle);
    AsyncSendOperation& send_operation(IoMemoryHandle);
    // SDK wrappers arm a buffer-range lease plus an EXTRA source-control strong
    // reference. Cancel request never ends these loans. Only an observed final
    // completion may release them; this is an explicit compatibility pin policy.
    IoMemoryHandle arm_receive(OVERLAPPED&,const Buffer&);
    IoMemoryHandle arm_send(OVERLAPPED&,const Buffer&);
    // Typed SDK entry points record exact socket/range/owner before the effect.
    // Legacy two-argument arm remains unclassified, never guessed as Kernel by
    // SessionIo stop. Successful/failed SDK result is recorded AFTER return.
    IoMemoryHandle arm_receive(Socket,OVERLAPPED&,const Buffer&);
    IoMemoryHandle arm_send(Socket,OVERLAPPED&,const Buffer&);
    void note_submission(IoMemoryHandle,std::int32_t result,std::uint32_t last_error);
    // Copies are nonowning metadata. Only pending loans pin their source owner
    // and byte range; never dereference captured old tokens after source ack.
    IoLoan loan(IoMemoryHandle)const;
    std::vector<IoLoan> pending_loans()const;
    std::vector<IoLoan> active_loans()const; // live packet bindings, including Posted software work
    void note_cancel_requested(IoMemoryHandle);
    void note_unknown_submission(IoMemoryHandle)noexcept;
    void complete_receive(IoMemoryHandle,std::uintptr_t owner,const Error&,std::uint64_t,
        IoCompletionEnvironment,WorkerRunApi&,Api&,IoCompletionEffects&);
    void complete_send(IoMemoryHandle,std::uintptr_t owner,const Error&,std::uint64_t,
        IoCompletionEnvironment,WorkerRunApi&,Api&,IoCompletionEffects&);
    void complete_posted(IoMemoryHandle,std::uintptr_t owner,PostedEnvironment,WorkerRunApi&);
    // Additive coherent-cache variants: preserve observed-loan/generation and
    // failure guards while routing source allocation/free through one adapter.
    void complete_receive_with_memory(IoMemoryHandle,std::uintptr_t,const Error&,std::uint64_t,
        IoCompletionEnvironment,WorkerRunApi&,Api&,IoCompletionEffects&,PostedMemoryEffects&);
    void complete_send_with_memory(IoMemoryHandle,std::uintptr_t,const Error&,std::uint64_t,
        IoCompletionEnvironment,WorkerRunApi&,Api&,IoCompletionEffects&,PostedMemoryEffects&);
    void complete_posted_with_memory(IoMemoryHandle,std::uintptr_t,PostedEnvironment,WorkerRunApi&,PostedMemoryEffects&);
    void invoke_task(IoMemoryHandle,std::uint8_t flag);
    IoMemorySnapshot snapshot()const;
    bool close(); // external stop+drain+worker-cache cleanup; mandatory before destructor
private:struct Impl;std::unique_ptr<Impl> impl_;
};
class PinnedAsyncReceiveApi final:public AsyncReceiveApi {
public:PinnedAsyncReceiveApi(OwnedIoMemory& memory,AsyncReceiveApi& lower):memory_(memory),lower_(lower){}
    AsyncReceiveResult receive(Socket,Buffer&,std::uint32_t,OVERLAPPED&)override;
private:OwnedIoMemory& memory_;AsyncReceiveApi& lower_;
};
class PinnedAsyncSendApi final:public AsyncSendApi {
public:PinnedAsyncSendApi(OwnedIoMemory& memory,AsyncSendApi& lower):memory_(memory),lower_(lower){}
    AsyncSendResult send(Socket,Buffer&,std::uint32_t,OVERLAPPED&)override;
private:OwnedIoMemory& memory_;AsyncSendApi& lower_;
};
// Replaces dispatcher operation routing only; guard/stop/exception/OS policies
// remain explicit lower providers. Do not use the old IoDispatcherEffects route
// to bypass observed-completion loan release.
class OwnedIoDispatcherEffects:public DispatcherEffects {
public:OwnedIoDispatcherEffects(OwnedIoMemory& m,IoCompletionEnvironment e,WorkerRunApi& a,Api& errors,IoCompletionEffects& effects)
        :memory_(m),environment_(e),api_(a),errors_(errors),effects_(effects){}
    void complete(IocpSchedulerEntry*,CancelOperation&,const Error&,std::uint32_t)final;
private:OwnedIoMemory& memory_;IoCompletionEnvironment environment_;WorkerRunApi& api_;Api& errors_;IoCompletionEffects& effects_;
};
} // namespace anadius::socket_recovered
