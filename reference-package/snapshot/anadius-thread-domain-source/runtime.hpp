#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "../anadius-crt-runtime-source/runtime.hpp"
#include <memory>

namespace anadius::thread_domain {
struct NativeRuntimeApi {
    decltype(&::LoadLibraryExW) load;
    decltype(&::GetProcAddress) procedure;
    decltype(&::FreeLibrary) release;
    decltype(&::GetLastError) error;
    std::uint32_t (*process_flags)();
};
NativeRuntimeApi native_runtime_api() noexcept;
// New OS boundary and module-reference owner, not another recovered native body.
// The recovered runtime resolver/state still decides caches, candidates and RO
// policy. Module names/procedures contain no embedded NUL. Procedure addresses
// must be rebuilt callbacks or real typed SDK exports, never original VAs.
// API.error and bookkeeping use the same real Win32 LastError domain.
//
// Loads/resolution/RO calls may overlap on workers. close requires WHOLE caller
// quiescence, including every matching RoUninitialize, and closes all admission.
// active_calls==0 alone is not that proof. It must run outside loader lock after
// worker/FLS cleanup. Failed release retains its reference for retry. An injected
// API exception with unknown native outcome poisons safe release permanently.
// During workers, free_library is restricted to an EXTRA duplicate reference
// while an independently retained owned cache reference keeps the module/code
// mapped. Releasing a LAST reference must be serialized with every lookup, RO
// call, load and other free. Only quiescent close performs final domain release;
// the ownership check alone is not a per-procedure/module lifetime lease. The
// recovered resolver's missing-cache/concurrent-unload edge remains unproven.
class NativeRuntime final:public crt_runtime_recovered::Effects {
public:
    explicit NativeRuntime(NativeRuntimeApi=native_runtime_api());
    ~NativeRuntime() noexcept;
    NativeRuntime(const NativeRuntime&)=delete;NativeRuntime& operator=(const NativeRuntime&)=delete;
    NativeRuntime(NativeRuntime&&)=delete;NativeRuntime& operator=(NativeRuntime&&)=delete;
    bool close();
    std::size_t outstanding_modules() const;
    bool closed() const;
    bool tracking_poisoned() const;
    crt_runtime_recovered::Module load_library_ex_w(std::u16string_view,crt_runtime_recovered::Module,std::uint32_t) override;
    std::uint32_t last_error() override;
    bool free_library(crt_runtime_recovered::Module) override;
    crt_runtime_recovered::Address get_proc_address(crt_runtime_recovered::Module,std::string_view) override;
    std::uint32_t process_parameters_flags() override;
    std::int32_t call_ro_initialize(crt_runtime_recovered::Address,std::uint32_t) override;
    void call_ro_uninitialize(crt_runtime_recovered::Address) override;
    std::uint32_t call_thread_initialization_policy(crt_runtime_recovered::Address,std::uintptr_t,std::int32_t*) override;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
} // namespace anadius::thread_domain
