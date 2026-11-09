// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_completion_dispatcher.hpp"

namespace citrine_eclipse_mirage::socket {
struct DispatcherImports {
    decltype(&::SetLastError) set_last_error=&::SetLastError;
    decltype(&::GetLastError) get_last_error=&::GetLastError;
    decltype(&::GetQueuedCompletionStatus) get=&::GetQueuedCompletionStatus;
    decltype(&::PostQueuedCompletionStatus) post=&::PostQueuedCompletionStatus;
    decltype(&::SetWaitableTimer) set_timer=&::SetWaitableTimer;
};
class NativeDispatcherApi final:public DispatcherApi {
public:
    explicit NativeDispatcherApi(DispatcherImports imports={}):imports_(imports){}
    void set_last_error(std::uint32_t) override;
    std::uint32_t get_last_error() override;
    std::int32_t get(std::uintptr_t,std::uint32_t&,std::uintptr_t&,OVERLAPPED*&,std::uint32_t) override;
    std::int32_t post(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*) override;
    std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t) override;
private:DispatcherImports imports_;
};
} // namespace citrine_eclipse_mirage::socket
