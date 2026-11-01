// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_iocp_constructor.hpp"
#include "preloader_l/runtime/thread_launch.hpp"

namespace preloader_l::socket {
class WorkerStartupApi {
public:
    virtual ~WorkerStartupApi()=default;
    virtual std::uintptr_t create_event(std::uintptr_t security,std::int32_t manual_reset,
        std::int32_t initial_state,std::uintptr_t name)=0;
    virtual std::uint32_t get_last_error()=0;
    virtual std::int32_t close_handle(std::uintptr_t)=0;
    virtual std::uint32_t wait_one(std::uintptr_t,std::uint32_t timeout)=0;
    virtual std::int32_t set_event(std::uintptr_t)=0;
    virtual std::uint32_t sleep_ex(std::uint32_t milliseconds,std::int32_t alertable)=0;
};
class WorkerStartupEffects;
void start_worker_02a000(IocpWorker&,IocpWorkerFunction*,std::uint32_t stack_size,
    WorkerStartupApi&,Api& errors,WorkerStartupEffects&);
std::uint32_t worker_entry_02a4a0(IocpWorkerFunction&,WorkerStartupApi&,WorkerStartupEffects&);
// Selected composition with024990. Function/body/deleting-slot effects and all
// object/storage lifetimes remain explicit. If a delete callback returns and
// error0 permits later reads, its projection must stay live through those reads;
// this does not establish safe native continuation after a deleting destructor.
class WorkerStartupEffects:public IocpConstructionEffects {
public:
    WorkerStartupEffects(WorkerStartupApi& api,Api& errors):api_(api),errors_(errors){}
    void start_worker_02a000(IocpWorker&,IocpWorkerFunction*,std::uint32_t stack_size) final;
    virtual std::uintptr_t begin_thread_05e8e0(std::uintptr_t security,std::uint32_t stack_size,
        std::uintptr_t entry_identity,IocpWorkerFunction*,std::uint32_t flags,std::uint32_t& thread_id)=0;
    virtual void delete_function_slot_00(IocpWorkerFunction&,std::uint32_t flags)=0;
    virtual void invoke_function_slot_08(IocpWorkerFunction&)=0;
private:WorkerStartupApi& api_;Api& errors_;
};
// Reuses the already recovered CRT thread source without changing it. Entry
// identity is data; the supplied threads::Effects must route it to the recovered
// worker_entry_02a4a0 using a valid live argument, never call an image VA.
class WorkerStartupEffects:public WorkerStartupEffects {
public:
    WorkerStartupEffects(WorkerStartupApi& api,Api& errors,threads::Effects& threads)
        :WorkerStartupEffects(api,errors),threads_(threads){}
    std::uintptr_t begin_thread_05e8e0(std::uintptr_t,std::uint32_t,std::uintptr_t,
        IocpWorkerFunction*,std::uint32_t,std::uint32_t&) final;
private:threads::Effects& threads_;
};
} // namespace preloader_l::socket
