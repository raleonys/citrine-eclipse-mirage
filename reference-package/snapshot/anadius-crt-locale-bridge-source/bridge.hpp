#pragma once
#include "../anadius-crt-locale-update-source/locale_update.hpp"
#include "../anadius-crt-locale-thread-source/locale_thread.hpp"
#include "../anadius-crt-find-bridge-source/bridge.hpp"
#include "../anadius-crt-ptd-source/ptd.hpp"
#include "../anadius-crt-ptd-data-source/ptd_data.hpp"
namespace anadius::crt_locale_bridge {
using Address=std::uint64_t;
struct Globals {
    std::uint32_t changed_0a52d8{},mask_0a1c6c{};
    std::array<Address,2> pair_0a1938{};
    Address locale_0a52e0{},multibyte_0a59d0{};
    Address initial_locale_0a17e0{},initial_multibyte_0a1c70{};
};
class Memory {
public:
    virtual ~Memory()=default;
    virtual Address read_address(Address)=0;
    virtual void write_address(Address,Address)=0;
    virtual std::uint32_t read_u32(Address)=0;
    virtual void write_u32(Address,std::uint32_t)=0;
};
// Caller-owned globals/memory must share the PTD/resource provider address
// domain and remain synchronized. This is not original image initialization.
class ThreadEffects final:public crt_locale_thread_recovered::Effects {
public:
    ThreadEffects(Globals&,Memory&,crt_ptd_recovered::State&,crt_ptd_recovered::Effects&,
        crt_ptd_data_recovered::Effects&,crt_heap_recovered::State&,crt_heap_recovered::Effects&);
    Address read_address(Address) final;
    void write_address(Address,Address) final;
    std::uint32_t read_u32(Address) final;
    std::uint32_t locale_mask_0a1c6c() final;
    Address current_locale_0a52e0() final;
    Address current_multibyte_slot_0a59d0() final;
    Address initial_locale_0a17e0() final;
    Address initial_multibyte_0a1c70() final;
    Address required_ptd_064b90() final;
    void lock_0625f8(std::uint32_t) final;
    void unlock_06264c(std::uint32_t) final;
    std::uint32_t atomic_fetch_add_32(Address,std::uint32_t) final;
    void free_0668c0(Address) final;
    void add_locale_ref_070634(Address) final;
    void release_locale_ref_0708c0(Address) final;
    void free_locale_0706c0(Address) final;
    [[noreturn]]void abort_05f03c() final;
private:
    Globals& globals_;Memory& memory_;crt_ptd_recovered::State& ptd_;
    crt_ptd_recovered::Effects& ptd_fx_;crt_ptd_data_recovered::Effects& data_;
    crt_heap_recovered::State& heap_;crt_heap_recovered::Effects& heap_fx_;
};
class UpdateEffects final:public crt_locale_update_recovered::Effects {
public:
    UpdateEffects(Globals&,Memory&,ThreadEffects&);
    Address read_address(Address) final;
    std::uint32_t read_u32(Address) final;
    void write_u32(Address,std::uint32_t) final;
    std::uint32_t locale_changed_0a52d8() final;
    std::array<Address,2> global_pair_0a1938() final;
    Address required_ptd_064b90() final;
    Address current_locale_0a52e0() final;
    Address current_multibyte_0a59d0() final;
    std::uint32_t locale_mask_0a1c6c() final;
    Address update_thread_locale_070968() final;
    Address update_thread_multibyte_06f658() final;
private:
    Globals& globals_;Memory& memory_;ThreadEffects& thread_;
};
class WildcardEffects:public crt_find_bridge::WildcardEffects {
public:
    WildcardEffects(crt_recalloc_bridge::Allocator&,crt_conversion_recovered::Effects&,
        crt_runtime_recovered::State&,crt_runtime_recovered::Effects&,
        crt_conversion_api::FileApiEffects&,crt_find_native::Session&,
        crt_locale_update_recovered::Effects&);
    crt_byteclass_recovered::LocaleContext locale_update_058b04(Address) final;
private:
    crt_locale_update_recovered::Effects& locale_;
};
}
