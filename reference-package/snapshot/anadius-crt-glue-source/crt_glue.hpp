#pragma once
#include "../anadius-thread-native-source/win32_effects.hpp"
#include "../anadius-crt-ptd-source/ptd.hpp"
#include "../anadius-crt-ptd-data-source/ptd_data.hpp"
#include "../anadius-crt-heap-source/heap.hpp"
#include "../anadius-crt-invalid-source/invalid.hpp"

namespace anadius::crt_glue {
struct FlsApi {
    decltype(&::FlsGetValue) get;
    decltype(&::FlsSetValue) set;
    decltype(&::FlsAlloc) allocate;
    decltype(&::FlsFree) release;
    decltype(&::GetLastError) error;
    decltype(&::SetLastError) set_error;
};
FlsApi native_fls_api() noexcept;
class AbortPolicy {
public:
    virtual ~AbortPolicy()=default;
    [[noreturn]] virtual void abort_05f03c()=0;
};
// New composition. Native FLS callback address is a rebuilt function supplied
// by caller and must stay bound for the whole FLS/worker lifetime. This class
// never manufactures a global context or installs the caller's callback itself.
class PtdEffects final:public crt_ptd_recovered::Effects {
public:
    PtdEffects(FlsApi,PFLS_CALLBACK_FUNCTION,
        crt_heap_recovered::State&,crt_heap_recovered::Effects&,
        const crt_ptd_data_recovered::Identities&,crt_ptd_data_recovered::Effects&,
        AbortPolicy&);
    crt_ptd_recovered::Ptd* fls_get_value(std::uint32_t) override;
    bool fls_set_value(std::uint32_t,crt_ptd_recovered::Ptd*) override;
    std::uint32_t fls_alloc(crt_ptd_recovered::FlsCallback) override;
    bool fls_free(std::uint32_t) override;
    std::uint32_t last_error() override;
    void set_last_error(std::uint32_t) override;
    crt_ptd_recovered::Ptd* calloc_ptd_066848() override;
    void free_0668c0(crt_ptd_recovered::Ptd*) override;
    void construct_ptd_array_0648fc(crt_ptd_recovered::Ptd*) override;
    void destroy_ptd_array_0649ec(crt_ptd_recovered::Ptd*) override;
    [[noreturn]] void abort_05f03c() override;
private:
    const FlsApi api_;
    const PFLS_CALLBACK_FUNCTION callback_;
    crt_heap_recovered::State& heap_;
    crt_heap_recovered::Effects& heap_effects_;
    const crt_ptd_data_recovered::Identities& ids_;
    crt_ptd_data_recovered::Effects& data_;
    AbortPolicy& abort_;
};
class ErrorPtd final:public crt_errors_recovered::Effects {
public:
    ErrorPtd(crt_ptd_recovered::State&,crt_ptd_recovered::Effects&);
    void* optional_ptd_064d08() override;
private:
    crt_ptd_recovered::State& state_;
    crt_ptd_recovered::Effects& effects_;
};
// Fill only the two error methods; real heap/lock/new-handler effects stay pure.
class HeapErrors:public crt_heap_recovered::Effects {
public:
    HeapErrors(crt_errors_recovered::State&,crt_errors_recovered::Effects&);
    std::uint32_t errno_from_os_error_05d650(std::uint32_t) final;
    void write_error_slot_05d724(std::uint32_t) final;
private:
    crt_errors_recovered::State& state_;
    crt_errors_recovered::Effects& effects_;
};
// State-index0 is the only recovered PTD-array domain here. Locale/global
// handler/Watson policies remain mandatory; no native terminal fallback exists.
class InvalidPtd:public crt_invalid_recovered::Effects {
public:
    InvalidPtd(crt_ptd_recovered::State&,crt_ptd_recovered::Effects&);
    std::uint32_t last_error() final;
    void set_last_error(std::uint32_t) final;
    void* optional_ptd_064dd0(std::uint32_t*,std::uint64_t) final;
private:
    crt_ptd_recovered::State& state_;
    crt_ptd_recovered::Effects& effects_;
};
struct Dependencies {
    crt_ptd_recovered::State& ptd;
    crt_ptd_recovered::Effects& ptd_effects;
    crt_heap_recovered::State& heap;
    crt_heap_recovered::Effects& heap_effects;
    crt_invalid_recovered::Effects& invalid;
};
// All recovered thread Effects methods now have source implementations. OS,
// locale, heap, runtime provider and terminal policies are still supplied.
// start is the CRT wrapper, not Context::entry. All entries must be rebuilt.
class ThreadEffects final:public thread_native::RecoveredWin32Effects {
public:
    ThreadEffects(LPTHREAD_START_ROUTINE,thread_native::Api,
        crt_runtime_recovered::State&,crt_runtime_recovered::Effects&,
        crt_errors_recovered::State&,crt_errors_recovered::Effects&,Dependencies);
    threads_recovered::Context* calloc_context_066848() override;
    void free_0668c0(threads_recovered::Context*) override;
    void invalid_parameter_05c954() override;
    threads_recovered::Context*& required_thread_slot_064b90() override;
    threads_recovered::Context* optional_thread_context_064d08() override;
    std::uint32_t invoke_entry(std::uintptr_t,std::uintptr_t) override;
private:
    const Dependencies deps_;
};
}
