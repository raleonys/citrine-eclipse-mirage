// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_completion_dispatcher_native.hpp"

namespace preloader_l::socket {
void NativeDispatcherApi::set_last_error(std::uint32_t value){imports_.set_last_error(value);}
std::uint32_t NativeDispatcherApi::get_last_error(){return imports_.get_last_error();}
std::int32_t NativeDispatcherApi::get(std::uintptr_t port,std::uint32_t& bytes,std::uintptr_t& key,
    OVERLAPPED*& operation,std::uint32_t timeout){
    DWORD native_bytes=bytes;ULONG_PTR native_key=key;
    const auto result=imports_.get(reinterpret_cast<HANDLE>(port),&native_bytes,&native_key,&operation,timeout);
    bytes=native_bytes;key=native_key;return result;
}
std::int32_t NativeDispatcherApi::post(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* operation){
    return imports_.post(reinterpret_cast<HANDLE>(port),bytes,key,operation);
}
std::int32_t NativeDispatcherApi::set_timer(std::uintptr_t timer,LARGE_INTEGER& due,std::int32_t period,
    PTIMERAPCROUTINE callback,void* argument,std::int32_t resume){
    return imports_.set_timer(reinterpret_cast<HANDLE>(timer),&due,period,callback,argument,resume);
}
} // namespace preloader_l::socket
