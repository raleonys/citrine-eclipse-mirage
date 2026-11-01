#pragma once
#include "../anadius-crt-locale-ref-source/locale_ref.hpp"
#include "../anadius-crt-locale-free-source/locale_free.hpp"
#include "../anadius-crt-locale-bridge-source/bridge.hpp"
namespace anadius::crt_locale_resource_bridge {
using Address=std::uint64_t;
// New composition of recovered locale-resource bodies. Native lock/atomic
// implementations remain mandatory. Callers own synchronized globals, live
// memory, identities/defaults and the same heap domain used for allocations.
class Resources:public crt_ptd_data_recovered::Effects,
    public crt_locale_ref_recovered::Effects,public crt_locale_free_recovered::Effects {
public:
    Resources(crt_locale_bridge::Globals&,crt_locale_bridge::Memory&,
        const crt_locale_ref_recovered::Identities&,const crt_locale_free_recovered::Defaults&,
        crt_heap_recovered::State&,crt_heap_recovered::Effects&);
    Address read_address(Address) final;
    std::uint32_t read_u32(Address) final;
    void free_0668c0(Address) final;
    Address current_locale_0a52e0() final;
    void release_locale_ref_0708c0(Address) final;
    std::uint32_t locale_ref_count_10(Address) final;
    void free_locale_0706c0(Address) final;
    void add_locale_ref_070634(Address) final;
private:
    crt_locale_bridge::Globals& globals_;
    crt_locale_bridge::Memory& memory_;
    const crt_locale_ref_recovered::Identities& identities_;
    const crt_locale_free_recovered::Defaults& defaults_;
    crt_heap_recovered::State& heap_;
    crt_heap_recovered::Effects& heap_fx_;
};
}
