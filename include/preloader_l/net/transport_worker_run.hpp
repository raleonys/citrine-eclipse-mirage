// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_worker_startup.hpp"
#include <limits>

namespace preloader_l::socket {
// Only defined logical error fields are represented. Native error padding and
// an unwritten local error are not initialized or interpreted as ABI bytes.
struct WorkerRunResult {std::optional<Error> error;};
struct WorkerThreadState {
    std::array<std::uintptr_t,10> cache_00;
    std::uint32_t pending_50;
    std::array<std::uintptr_t,2> context_58;
};
struct WorkerTlsContext {
    std::uintptr_t* port_key;
    WorkerThreadState* state;
    void* previous;
};
struct WorkerRunEnvironment {
    // Live original DAT_1800a38f0 projection; reloaded before each TLS API.
    std::uint32_t& tls_index;
    // Logical mapping of constant category object1800a10f8, not its ABI/name VA.
    Category generic_category;
};
class WorkerRunApi {
public:
    virtual ~WorkerRunApi()=default;
    virtual void* tls_get(std::uint32_t index)=0;
    virtual std::int32_t tls_set(std::uint32_t index,void*)=0;
    virtual std::int32_t post_stop(std::uintptr_t port,std::uint32_t bytes,
        std::uintptr_t key,OVERLAPPED*)=0;
    virtual std::uint32_t get_last_error()=0;
};
class WorkerRunEffects {
public:
    virtual ~WorkerRunEffects()=default;
    // Exact low32 interlocked exchanges remain required storage/atomic effects.
    // They must affect the caller's corresponding scheduler fields; no aliasing
    // cast of word_38's uint64 storage or invented 64-bit atomic is performed.
    virtual std::uint32_t exchange_stop_34(IocpSchedulerEntry&,std::uint32_t)=0;
    virtual std::uint32_t exchange_wake_38(IocpSchedulerEntry&,std::uint32_t)=0;
    virtual std::uint64_t dispatch_027c10(IocpSchedulerEntry&,std::uint32_t timeout,
        WorkerThreadState&,WorkerRunResult&)=0;
    // Existing CRT boundaries remain providers; no CRT implementation added.
    virtual void initialize_04a240(std::array<std::uintptr_t,2>&)=0;
    virtual void finish_04a2cc(std::array<std::uintptr_t,2>&)=0;
    virtual void free_cache_04b788(std::uintptr_t)=0;
    virtual void delete_function_04b0f8(IocpWorkerFunction&,std::uint32_t native_bytes)=0;
    [[noreturn]]virtual void raise_028240(Error,std::uintptr_t message_identity)=0;
};
constexpr std::uint64_t increment_worker_count(std::uint64_t count){
    return count==std::numeric_limits<std::uint64_t>::max()?count:count+1;
}
void stop_worker_02a190(IocpSchedulerEntry&,WorkerRunApi&,Api& errors,WorkerRunEffects&);
std::uint64_t run_worker_0298b0(IocpSchedulerEntry&,WorkerRunResult&,
    WorkerRunEnvironment,WorkerRunApi&,Api& errors,WorkerRunEffects&);
void run_worker_function_029890(IocpWorkerFunction&,WorkerRunEnvironment,
    WorkerRunApi&,Api& errors,WorkerRunEffects&);
// Returns an opaque original-this identity, without accessing a deleted object.
std::uintptr_t delete_worker_function_026320(IocpWorkerFunction&,std::uint32_t flags,WorkerRunEffects&);
// Actual source composition of02a000/05e8e0/02a4a0 with the two recovered slots.
// Allocation, CRT thread effects, dispatcher and native lifetime remain required.
class WorkerFunctionEffects:public WorkerStartupEffects {
public:
    WorkerFunctionEffects(WorkerStartupApi& startup,Api& errors,
        threads::Effects& threads,WorkerRunEnvironment environment,
        WorkerRunApi& api,WorkerRunEffects& effects)
        :WorkerStartupEffects(startup,errors,threads),environment_(environment),
         api_(api),errors_(errors),effects_(effects){}
    void delete_function_slot_00(IocpWorkerFunction&,std::uint32_t) final;
    void invoke_function_slot_08(IocpWorkerFunction&) final;
private:
    WorkerRunEnvironment environment_;WorkerRunApi& api_;Api& errors_;WorkerRunEffects& effects_;
};
} // namespace preloader_l::socket
