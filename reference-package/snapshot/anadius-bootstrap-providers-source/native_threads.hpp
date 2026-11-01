#pragma once
#include "../anadius-crt-glue-source/crt_glue.hpp"
#include <memory>

namespace anadius::bootstrap_providers {
struct ThreadDomain;
// No per-image route and no raw provider global: each launch envelope carries
// one provider/context pair. Envelope is deleted BEFORE invoking recovered
// thread_entry, because ExitThread bypasses C++ stack destruction.
// B3 TrackingThreads (or caller) MUST join every worker before destroying this
// provider, including pending/suspended and FLS exit cleanup. No implicit join.
class NativeThreads final:public threads_recovered::Effects {
public:
    NativeThreads(std::unique_ptr<threads_recovered::Effects>,thread_native::Api=thread_native::native_api(),std::shared_ptr<void> domain_lifetime={});
    ~NativeThreads() noexcept;
    NativeThreads(const NativeThreads&)=delete;NativeThreads& operator=(const NativeThreads&)=delete;
    threads_recovered::Context* calloc_context_066848() override;
    void free_0668c0(threads_recovered::Context*) override;
    bool get_module_handle_ex_w(std::uint32_t,std::uintptr_t,threads_recovered::Handle*) override;
    threads_recovered::Handle create_thread(void*,std::uint64_t,threads_recovered::Context*,std::uint32_t,std::uint32_t*) override;
    std::uint32_t last_error() override;
    void write_error_slot_05d724(std::uint32_t) override;
    void invalid_parameter_05c954() override;
    void map_os_error_05d698(std::uint32_t) override;
    bool close_handle(threads_recovered::Handle) override;
    bool free_library(threads_recovered::Handle) override;
    threads_recovered::Context*& required_thread_slot_064b90() override;
    threads_recovered::Context* optional_thread_context_064d08() override;
    int runtime_policy_06c0d0() override;
    int initialize_runtime_067320(int) override;
    void uninitialize_runtime_06736c() override;
    std::uint32_t invoke_entry(std::uintptr_t,std::uintptr_t) override;
    [[noreturn]] void free_library_and_exit_thread(threads_recovered::Handle,std::uint32_t) override;
    [[noreturn]] void exit_thread(std::uint32_t) override;
private:
    friend std::unique_ptr<NativeThreads> make_recovered_threads(ThreadDomain,thread_native::Api);
    struct Launch;
    static DWORD WINAPI start(void*) noexcept;
    std::shared_ptr<void> lifetime_; // destroyed after lower provider
    std::unique_ptr<threads_recovered::Effects> lower_;thread_native::Api api_;
};
// Existing concrete recovered CRT graph, explicitly supplied, never invented.
// lifetime owns all referenced state/providers and outlives worker FLS teardown.
struct ThreadDomain {
    std::shared_ptr<void> lifetime;
    crt_runtime_recovered::State& runtime;crt_runtime_recovered::Effects& runtime_effects;
    crt_errors_recovered::State& errors;crt_errors_recovered::Effects& error_effects;
    crt_glue::Dependencies dependencies;
};
std::unique_ptr<NativeThreads> make_recovered_threads(ThreadDomain,thread_native::Api=thread_native::native_api());
} // namespace anadius::bootstrap_providers
