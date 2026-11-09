// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_worker_startup.hpp"

namespace citrine_eclipse_mirage::socket {
struct WorkerStartupImports {
    decltype(&::CreateEventW) create_event=&::CreateEventW;
    decltype(&::GetLastError) get_last_error=&::GetLastError;
    decltype(&::CloseHandle) close_handle=&::CloseHandle;
    decltype(&::WaitForSingleObject) wait_one=&::WaitForSingleObject;
    decltype(&::SetEvent) set_event=&::SetEvent;
    decltype(&::SleepEx) sleep_ex=&::SleepEx;
};
// SDK imports are injectable. Construction itself creates no event/thread and
// performs no wait; tests supply controlled functions for every import.
class NativeWorkerStartupApi final:public WorkerStartupApi {
public:
    explicit NativeWorkerStartupApi(WorkerStartupImports imports={}):imports_(imports){}
    std::uintptr_t create_event(std::uintptr_t,std::int32_t,std::int32_t,std::uintptr_t) override;
    std::uint32_t get_last_error() override;
    std::int32_t close_handle(std::uintptr_t) override;
    std::uint32_t wait_one(std::uintptr_t,std::uint32_t) override;
    std::int32_t set_event(std::uintptr_t) override;
    std::uint32_t sleep_ex(std::uint32_t,std::int32_t) override;
private:WorkerStartupImports imports_;
};
} // namespace citrine_eclipse_mirage::socket
