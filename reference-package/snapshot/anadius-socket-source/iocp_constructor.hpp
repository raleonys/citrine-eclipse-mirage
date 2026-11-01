#pragma once
#include "scheduler_registry.hpp"

namespace anadius::socket_recovered {
struct IocpSchedulerEntry;
struct IocpWorker {
    std::uintptr_t word_00=0,handle_08=0,event_10=0;
};
struct IocpWorkerFunction {
    std::uintptr_t table_identity=0,event_08=0,event_10=0;
    IocpSchedulerEntry* scheduler_18=nullptr;
};
// Live typed projection; not a native0xb0-byte allocation, ASIO overlay or owner.
// Caller supplies a gate proxy for the borrowed section; no enter occurs before
// successful initialization. The gate/section and all projections remain live.
struct IocpSchedulerEntry:ServiceEntry {
    IocpSchedulerEntry(SocketGate& gate,CRITICAL_SECTION& section):completion(gate),section(section){}
    std::uintptr_t table_identity=0,owner_context=0;
    CompletionScheduler completion;
    std::atomic<std::uint32_t> work_count_30{0};
    std::uint32_t high_34=0;
    std::uint64_t word_38=0;
    std::uint32_t timeout_40=0;
    CRITICAL_SECTION& section;
    std::uint64_t word_88=0,word_90=0,word_98=0;
    std::uint32_t concurrency_a0=0;
    IocpWorker* worker_a8=nullptr;
};
class IocpConstructorApi {
public:
    virtual ~IocpConstructorApi()=default;
    virtual std::uint64_t set_version_mask(std::uint64_t mask,std::uint32_t type_mask,std::uint8_t condition)=0;
    virtual std::int32_t verify_version(OSVERSIONINFOEXA&,std::uint32_t type_mask,std::uint64_t condition_mask)=0;
    virtual std::uintptr_t create_completion_port(std::uintptr_t file,std::uintptr_t existing,std::uintptr_t key,std::uint32_t threads)=0;
    virtual std::uint32_t get_last_error()=0;
    virtual std::int32_t close_handle(std::uintptr_t handle)=0;
};
class IocpConstructionEffects {
public:
    virtual ~IocpConstructionEffects()=default;
    // Native byte requests represented by live, exclusively writable projections.
    // Fresh objects and all referenced callback storage remain live through use.
    virtual IocpWorker* allocate_worker_04b0bc(std::uint32_t native_bytes)=0;
    virtual IocpWorkerFunction* allocate_function_04b0bc(std::uint32_t native_bytes)=0;
    // Required native worker/event/thread boundary; function may be null at the
    // observed call site. No successful native null-function execution is claimed.
    virtual void start_worker_02a000(IocpWorker&,IocpWorkerFunction*,std::uint32_t stack_size)=0;
    virtual void delete_worker_04b0f8(IocpWorker&,std::uint32_t native_bytes)=0;
    [[noreturn]] virtual void raise_028240(Error,std::uintptr_t message_identity)=0;
};
IocpSchedulerEntry& construct_iocp_024990(IocpSchedulerEntry&,std::uintptr_t owner_context,
    std::uint32_t concurrency_hint,std::uint8_t own_thread,IocpConstructorApi&,ServiceGateApi&,Api& errors,IocpConstructionEffects&);
// Selected typed bridge; the native allocator and CRT comparator remain abstract.
// Host dynamic-type rejection is not an implementation of original04d2d0 RTTI.
class IocpSchedulerRegistryEffects:public SchedulerRegistryEffects {
public:
    IocpSchedulerRegistryEffects(IocpConstructorApi& api,ServiceGateApi& gates,Api& errors,IocpConstructionEffects& effects)
        :api_(api),gates_(gates),errors_(errors),effects_(effects){}
    ServiceEntry* construct_scheduler_024990(ServiceEntry&,std::uintptr_t owner_context,
        std::uint32_t concurrency_hint,std::uint8_t own_thread) final;
private:IocpConstructorApi& api_;ServiceGateApi& gates_;Api& errors_;IocpConstructionEffects& effects_;
};
} // namespace anadius::socket_recovered
