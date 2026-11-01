#include "bridge.hpp"
namespace anadius::crt_locale_resource_bridge {
Resources::Resources(crt_locale_bridge::Globals& g,crt_locale_bridge::Memory& m,
    const crt_locale_ref_recovered::Identities& i,const crt_locale_free_recovered::Defaults& d,
    crt_heap_recovered::State& h,crt_heap_recovered::Effects& hf)
    :globals_(g),memory_(m),identities_(i),defaults_(d),heap_(h),heap_fx_(hf){}
Address Resources::read_address(Address a){return memory_.read_address(a);}
std::uint32_t Resources::read_u32(Address a){return memory_.read_u32(a);}
void Resources::free_0668c0(Address a){crt_heap_recovered::free_0668c0(reinterpret_cast<void*>(a),heap_,heap_fx_);}
Address Resources::current_locale_0a52e0(){return globals_.locale_0a52e0;}
void Resources::release_locale_ref_0708c0(Address a){crt_locale_ref_recovered::release_locale_ref_0708c0(a,identities_,*this);}
std::uint32_t Resources::locale_ref_count_10(Address a){return read_u32(a+0x10);}
void Resources::free_locale_0706c0(Address a){crt_locale_free_recovered::free_locale_0706c0(a,defaults_,*this);}
void Resources::add_locale_ref_070634(Address a){crt_locale_ref_recovered::add_locale_ref_070634(a,identities_,*this);}
}
