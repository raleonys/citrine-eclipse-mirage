#include "preloader_l/net/transport_worker_startup_native.hpp"

namespace preloader_l::socket_recovered {
std::uintptr_t NativeWorkerStartupApi::create_event(std::uintptr_t security,std::int32_t manual,std::int32_t initial,std::uintptr_t name){return reinterpret_cast<std::uintptr_t>(imports_.create_event(reinterpret_cast<LPSECURITY_ATTRIBUTES>(security),manual,initial,reinterpret_cast<LPCWSTR>(name)));}
std::uint32_t NativeWorkerStartupApi::get_last_error(){return imports_.get_last_error();}
std::int32_t NativeWorkerStartupApi::close_handle(std::uintptr_t handle){return imports_.close_handle(reinterpret_cast<HANDLE>(handle));}
std::uint32_t NativeWorkerStartupApi::wait_one(std::uintptr_t handle,std::uint32_t timeout){return imports_.wait_one(reinterpret_cast<HANDLE>(handle),timeout);}
std::int32_t NativeWorkerStartupApi::set_event(std::uintptr_t handle){return imports_.set_event(reinterpret_cast<HANDLE>(handle));}
std::uint32_t NativeWorkerStartupApi::sleep_ex(std::uint32_t milliseconds,std::int32_t alertable){return imports_.sleep_ex(milliseconds,alertable);}
} // namespace preloader_l::socket_recovered
