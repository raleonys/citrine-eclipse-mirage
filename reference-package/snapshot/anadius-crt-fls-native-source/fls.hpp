#pragma once
#include "../anadius-crt-glue-source/crt_glue.hpp"
#include <atomic>
namespace anadius::crt_fls_native {
// New single-domain callback binding, not original image-global setup.
// All users must be quiescent before source uninitialize/retry_release or
// destruction. Borrowed dependencies stay alive until actual native release.
// Use only this Binding's state(); do not overwrite its index or supply an
// external State/index. No initializer/callback may be in flight at destruction.
class Binding final:public crt_ptd_recovered::Effects {
public:
    Binding(crt_heap_recovered::State&,crt_heap_recovered::Effects&,
        crt_ptd_data_recovered::Identities,crt_ptd_data_recovered::Effects&,
        crt_glue::AbortPolicy&,crt_glue::FlsApi=crt_glue::native_fls_api());
    ~Binding();
    Binding(const Binding&)=delete;
    Binding& operator=(const Binding&)=delete;
    crt_ptd_recovered::State& state() noexcept{return state_;}
    std::uint32_t native_index() const noexcept{return registered_;}
    // Source uninitialize ignores failure and invalidates its own State.
    // Retry retains the distinct native ownership and callback binding.
    bool retry_release();
    crt_ptd_recovered::Ptd* fls_get_value(std::uint32_t) final;
    bool fls_set_value(std::uint32_t,crt_ptd_recovered::Ptd*) final;
    std::uint32_t fls_alloc(crt_ptd_recovered::FlsCallback) final;
    bool fls_free(std::uint32_t) final;
    std::uint32_t last_error() final;
    void set_last_error(std::uint32_t) final;
    crt_ptd_recovered::Ptd* calloc_ptd_066848() final;
    void free_0668c0(crt_ptd_recovered::Ptd*) final;
    void construct_ptd_array_0648fc(crt_ptd_recovered::Ptd*) final;
    void destroy_ptd_array_0649ec(crt_ptd_recovered::Ptd*) final;
    [[noreturn]]void abort_05f03c() final;
private:
    static VOID WINAPI callback(PVOID) noexcept;
    static std::atomic<Binding*> active_;
    crt_ptd_recovered::State state_;
    const crt_ptd_data_recovered::Identities ids_;
    crt_ptd_data_recovered::Effects& data_;
    crt_glue::PtdEffects effects_;
    std::uint32_t registered_=crt_ptd_recovered::invalid_fls_index;
};
}
