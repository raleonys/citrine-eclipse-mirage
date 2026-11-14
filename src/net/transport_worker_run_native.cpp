// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_worker_run_native.hpp"

namespace laureondo::socket {
void* NativeWorkerRunApi::tls_get(std::uint32_t index){return imports_.tls_get(index);}
std::int32_t NativeWorkerRunApi::tls_set(std::uint32_t index,void* value){return imports_.tls_set(index,value);}
std::int32_t NativeWorkerRunApi::post_stop(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* operation){
    return imports_.post_stop(reinterpret_cast<HANDLE>(port),bytes,key,operation);
}
std::uint32_t NativeWorkerRunApi::get_last_error(){return imports_.get_last_error();}
} // namespace laureondo::socket
