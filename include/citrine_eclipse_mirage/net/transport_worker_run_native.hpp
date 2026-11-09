// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_worker_run.hpp"

namespace citrine_eclipse_mirage::socket {
struct WorkerRunImports {
    decltype(&::TlsGetValue) tls_get=&::TlsGetValue;
    decltype(&::TlsSetValue) tls_set=&::TlsSetValue;
    decltype(&::PostQueuedCompletionStatus) post_stop=&::PostQueuedCompletionStatus;
    decltype(&::GetLastError) get_last_error=&::GetLastError;
};
class NativeWorkerRunApi final:public WorkerRunApi {
public:
    explicit NativeWorkerRunApi(WorkerRunImports imports={}):imports_(imports){}
    void* tls_get(std::uint32_t) override;
    std::int32_t tls_set(std::uint32_t,void*) override;
    std::int32_t post_stop(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*) override;
    std::uint32_t get_last_error() override;
private:WorkerRunImports imports_;
};
} // namespace citrine_eclipse_mirage::socket
