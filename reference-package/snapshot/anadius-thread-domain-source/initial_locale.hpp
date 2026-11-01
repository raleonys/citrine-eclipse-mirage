#pragma once
#include "../anadius-crt-initial-state-source/initial_state.hpp"
#include "../anadius-crt-ptd-data-source/ptd_data.hpp"
#include "../anadius-crt-heap-source/heap.hpp"
#include "../anadius-crt-lock-native-source/native.hpp"
#include <array>

namespace anadius::thread_domain {
// Restricted source composition: owns the relocated image-initial locale graph
// and selects its initial locale as this domain's current locale. ImageState is
// still initial DATA: its original current/initialized globals are not changed,
// and no recovered outer CRT initializer is invoked here.
//
// The exception identity is a caller-supplied live identity whose backing owner
// must outlive this object and all PTDs. It is never dereferenced as a table or
// made callable by this provider. Heap allocations passed to free_0668c0 must
// belong to the same caller-owned heap domain (or be null), as required by PTD
// source effects. No preferred original VA is supplied by this class.
//
// Banks, heap state and heap effects outlive this object. The owner initializes
// ACRT locks4/5 before any PTD operation and closes all FLS/worker users before
// destroying this object or inspecting image byte spans. Counter observations
// use ImageState's actual typed LONG/Interlocked registry.
class InitialLocale final:public crt_ptd_data_recovered::Effects {
public:
    InitialLocale(crt_heap_recovered::State&,crt_heap_recovered::Effects&,
        crt_lock_native::Banks&,std::uintptr_t exception_identity);
    InitialLocale(const InitialLocale&)=delete;
    InitialLocale& operator=(const InitialLocale&)=delete;
    InitialLocale(InitialLocale&&)=delete;
    InitialLocale& operator=(InitialLocale&&)=delete;
    crt_ptd_data_recovered::Identities ids() const noexcept;
    std::array<std::uintptr_t,2> pair() const noexcept;
    const crt_initial_state::ImageState& image() const noexcept{return image_;}
    void lock_0625f8(std::uint32_t) override;
    void unlock_06264c(std::uint32_t) override;
    std::uint32_t atomic_fetch_add_32(crt_ptd_data_recovered::Address,std::uint32_t) override;
    void free_0668c0(crt_ptd_data_recovered::Address) override;
    crt_ptd_data_recovered::Address current_locale_0a52e0() override;
    void release_locale_ref_0708c0(crt_ptd_data_recovered::Address) override;
    std::uint32_t locale_ref_count_10(crt_ptd_data_recovered::Address) override;
    void free_locale_0706c0(crt_ptd_data_recovered::Address) override;
    void add_locale_ref_070634(crt_ptd_data_recovered::Address) override;
private:
    void require_initial(crt_ptd_data_recovered::Address) const;
    static crt_locks_recovered::LockRef lock_ref(std::uint32_t);
    crt_heap_recovered::State& heap_;
    crt_heap_recovered::Effects& heap_effects_;
    crt_lock_native::Banks& banks_;
    crt_initial_state::ImageState image_;
    const std::uintptr_t exception_identity_;
};
} // namespace anadius::thread_domain
