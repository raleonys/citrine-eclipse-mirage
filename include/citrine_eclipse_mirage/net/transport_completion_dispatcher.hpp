// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_worker_run.hpp"

namespace citrine_eclipse_mirage::socket {
struct DispatcherQueue {CancelOperation* head=nullptr;CancelOperation* tail=nullptr;};
class DispatcherService {
public:
    virtual ~DispatcherService()=default;
    DispatcherService* next=nullptr;
    virtual void collect_slot_20(DispatcherQueue&)=0;
    virtual std::int32_t wait_slot_18(std::int32_t previous)=0;
};
// Packet metadata at native0/+10/+14 is distinct from cancellation reason+38.
// No OVERLAPPED fields are reinterpreted as native ASIO metadata or Error ABI.
struct DispatcherOperationBinding {
    CancelOperation* operation=nullptr;
    Error packet_error;
    std::uint32_t transferred_14=0;
    // Capture while the operation is live. Later unrelated lookups must not
    // access an operation whose completion callback ended its lifetime.
    // Rebinding requires replacing this identity together with operation.
    OVERLAPPED* overlapped_identity=operation?&operation->overlapped:nullptr;
};
struct DispatcherState {
    IocpSchedulerEntry& entry;
    DispatcherService* services_88=nullptr;
    std::uintptr_t timer_enabled_48=0,timer_50=0;
    std::span<DispatcherOperationBinding> operations;
};
struct DispatcherEnvironment {
    std::int32_t& guard_epoch_a38e8;
    Category generic_category; // logical mapping of constant objectA10F8
    Category system_category;  // logical mapping of constant objectA1110
};
class DispatcherApi {
public:
    virtual ~DispatcherApi()=default;
    virtual void set_last_error(std::uint32_t)=0;
    virtual std::uint32_t get_last_error()=0;
    virtual std::int32_t get(std::uintptr_t port,std::uint32_t& bytes,
        std::uintptr_t& key,OVERLAPPED*& operation,std::uint32_t timeout)=0;
    virtual std::int32_t post(std::uintptr_t port,std::uint32_t bytes,
        std::uintptr_t key,OVERLAPPED*)=0;
    virtual std::int32_t set_timer(std::uintptr_t timer,LARGE_INTEGER& due,
        std::int32_t period,PTIMERAPCROUTINE,void* argument,std::int32_t resume)=0;
};
class DispatcherEffects {
public:
    virtual ~DispatcherEffects()=default;
    // Native GS TLS-slot address captured once; pointee/epoch read live later.
    virtual std::uintptr_t capture_tls_slot()=0;
    virtual std::int32_t thread_epoch(std::uintptr_t slot)=0;
    virtual void guard_header_04b838(std::int32_t&)=0;
    virtual void register_atexit_04b770(std::uintptr_t function_identity)=0;
    virtual void guard_footer_04b7cc(std::int32_t&)=0;
    virtual std::uint32_t read_stop_34(IocpSchedulerEntry&)=0;
    // Provider includes024100's zero-pair/exception-copy/return-destination.
    virtual std::array<std::uintptr_t,2>& copy_exception_024100(
        std::array<std::uintptr_t,2>& destination,const std::array<std::uintptr_t,2>& source)=0;
    // Native029880 calls rethrow04a2d4 followed by INT3, not normal fallthrough.
    [[noreturn]]virtual void rethrow_029880(std::array<std::uintptr_t,2>&)=0;
    // Operation may end its lifetime here; caller never accesses it afterwards.
    virtual void complete(IocpSchedulerEntry* owner,CancelOperation&,const Error&,std::uint32_t bytes)=0;
};
std::int64_t dispatcher_timer_due(std::int32_t wait);
std::uint64_t dispatch_completion_027c10(DispatcherState&,std::uint32_t requested_timeout,
    WorkerThreadState&,WorkerRunResult&,DispatcherEnvironment,DispatcherApi&,
    DispatcherEffects&,WorkerRunApi&,Api& errors,WorkerRunEffects&);
// Actual bridge replacing prior required027c10. Remaining methods/CRT/atomic
// storage stay mandatory; caller supplies state for the exact entry identity.
class DispatcherWorkerEffects:public WorkerRunEffects {
public:
    DispatcherWorkerEffects(DispatcherState& state,DispatcherEnvironment environment,
        DispatcherApi& api,DispatcherEffects& effects,WorkerRunApi& worker_api,Api& errors)
        :state_(state),environment_(environment),api_(api),effects_(effects),worker_api_(worker_api),errors_(errors){}
    std::uint64_t dispatch_027c10(IocpSchedulerEntry&,std::uint32_t,
        WorkerThreadState&,WorkerRunResult&) final;
private:
    DispatcherState& state_;DispatcherEnvironment environment_;DispatcherApi& api_;
    DispatcherEffects& effects_;WorkerRunApi& worker_api_;Api& errors_;
};
} // namespace citrine_eclipse_mirage::socket
