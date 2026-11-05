#pragma once
#include "flow.hpp"

namespace anadius::socket_recovered {
// Composition only: all shared worker-cache allocations/evictions and typed
// projections cross this adapter. No original class/native ABI claim.
// Caller holds existing parent Operation around the whole source chain; accept,
// base IO, constructor, registries and this adapter outlive every pending task.
class CoherentIoMemory final:public HandlerDispatchMemory,public OwnedResponseMemory,
    public AsyncReceiveEffects {
public:
    CoherentIoMemory(OwnedAcceptMemory& a,OwnedIoMemory& b,IoHandlerSentinels s,
        IoEnvironmentConstructorContext& c,std::span<IoEnvironmentSourceBinding> sources)
        :accept_(a),base_(b),sentinels_(s),constructor_(c),sources_(sources){}
    // Serialized Profile registry publication only; existing source objects
    // remain stable while their lookup-array allocation changes.
    void bind_environment_sources(std::span<IoEnvironmentSourceBinding> sources) noexcept {sources_=sources;}
    // The two posted base interfaces intentionally share the same overrides;
    // expose one unambiguous base for async contexts and guarded completion.
    PostedMemoryEffects& posted(){return static_cast<HandlerDispatchMemory&>(*this);}
    OperationStorage& allocate_04b0bc(std::uint64_t)override;
    void free_04b788(OperationStorage&)override;
    PostedOperation& project_posted(OperationStorage&)override;
    ReceiveHandlerTask& project_receive_task(OperationStorage&,HandlerTaskContext&)override;
    SendHandlerTask& project_send_task(OperationStorage&,HandlerTaskContext&)override;
    OwnedResponseTask& project_owned_response(OperationStorage&,OwnedResponseContext&)override;
    AsyncReceiveOperation& project_async_receive(OperationStorage&)override;
    // ComposedWriteEffects also owns finish-write callbacks; its selected
    // source effects delegate this projection rather than inheriting memory.
    AsyncSendOperation& project_async_send(OperationStorage&);
    void construct_environment_023dd0(IoHandlerEnvironment&,std::uintptr_t)override;
    void free_cached_identity(std::uintptr_t);
private:
    OwnedAcceptMemory& accept_;OwnedIoMemory& base_;IoHandlerSentinels sentinels_;
    IoEnvironmentConstructorContext& constructor_;std::span<IoEnvironmentSourceBinding> sources_;
};
// Selected guard/stop/error policies remain explicit; operation routing keeps
// both B36 accept loans and B34 observed receive/send release under admission.
class CoherentIoDispatcher final:public DispatcherEffects {
public:
    CoherentIoDispatcher(OwnedAcceptMemory& a,OwnedIoMemory& b,CoherentIoMemory& m,
        IoCompletionEnvironment e,WorkerRunApi& w,Api& errors,IoCompletionEffects& effects,DispatcherEffects& guard)
        :accept_(a),base_(b),memory_(m),environment_(e),worker_(w),errors_(errors),effects_(effects),guard_(guard){}
    std::uintptr_t capture_tls_slot()override{return guard_.capture_tls_slot();}
    std::int32_t thread_epoch(std::uintptr_t p)override{return guard_.thread_epoch(p);}
    void guard_header_04b838(std::int32_t& n)override{guard_.guard_header_04b838(n);}
    void register_atexit_04b770(std::uintptr_t n)override{guard_.register_atexit_04b770(n);}
    void guard_footer_04b7cc(std::int32_t& n)override{guard_.guard_footer_04b7cc(n);}
    std::uint32_t read_stop_34(IocpSchedulerEntry& e)override{return guard_.read_stop_34(e);}
    std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>& d,const std::array<std::uintptr_t,2>& s)override{return guard_.copy_exception_024100(d,s);}
    [[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>& s)override{guard_.rethrow_029880(s);}
    void complete(IocpSchedulerEntry*,CancelOperation&,const Error&,std::uint32_t)override;
private:
    OwnedAcceptMemory& accept_;OwnedIoMemory& base_;CoherentIoMemory& memory_;
    IoCompletionEnvironment environment_;WorkerRunApi& worker_;Api& errors_;IoCompletionEffects& effects_;DispatcherEffects& guard_;
};
} // namespace anadius::socket_recovered
