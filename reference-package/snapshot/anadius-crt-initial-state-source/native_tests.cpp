#include "initial_state.hpp"
#include "../anadius-crt-initial-locale-source/initial_locale_data.hpp"
#include "../anadius-crt-initial-multibyte-source/initial_mb.hpp"
#include <array>
#include <atomic>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>
#include <vector>
namespace s=anadius::crt_initial_state;
namespace d=anadius::crt::initial_locale_data;
namespace r=anadius::crt_locale_ref_recovered;
namespace mb=anadius::crt_initial_multibyte;
namespace {
unsigned checks=0;
void check(bool yes){++checks;if(!yes)throw std::runtime_error("initial state fixture");}
template<class E,class F>void rejects(F fn){try{fn();}catch(const E&){check(true);return;}check(false);}
void run() {
    s::ImageState image,other;
    const auto& g=image.globals();
    const auto ids=image.reference_identities();
    const auto locale=g.initial_locale,time=image.region_address(d::Time);
    check(g.initial_locale!=other.globals().initial_locale);
    check(g.initial_multibyte!=other.globals().initial_multibyte);
    check(g.current_locale==0&&g.current_multibyte==0&&g.type_table==0&&g.case_table==0);
    check(g.locale_mask==0xfffffffeU&&g.locale_changed==0&&g.system_cp_flag==0&&g.initialized==0);
    check(image.counter_value(locale+0x10)==1);
    check(image.counter_value(time+0x15c)==0);
    check(image.counter_value(g.initial_multibyte)==0);
    check(image.read_address(locale+0x158)==locale);
    check(image.read_address(locale+0xf8)==image.region_address(d::Lconv));
    check(image.read_address(locale+0x120)==time);
    check(image.read_address(locale+0x28)==0);
    for(unsigned i=1;i<6;++i) check(image.read_address(locale+0x28+i*0x20)==ids.category_name_0a1948);
    check(image.region_bytes(d::Category)[0]=='C'&&image.region_bytes(d::Category)[1]==0);
    // Compare all bytes except independently declared DIR64 slots; no assumed
    // padding or scalar bytes may change during materialization.
    const auto regions=d::regions();
    for(std::size_t i=0;i<regions.size();++i) {
        const auto actual=image.region_bytes(i);
        check(actual.size()==regions[i].bytes.size());
        std::vector<bool> pointer(actual.size(),false);
        for(const auto& e:d::relocations()) if(e.source_region==i) {
            const auto expected=image.region_address(e.target_region)+e.target_offset;
            check(image.read_address(image.region_address(i)+e.source_offset)==expected);
            for(std::size_t k=0;k<8;++k) pointer.at(e.source_offset+k)=true;
        }
        for(std::size_t k=0;k<actual.size();++k) if(!pointer[k]) check(actual[k]==regions[i].bytes[k]);
    }
    check(std::memcmp(image.multibyte_bytes().data(),mb::record_bytes().data(),mb::record_size)==0);
    check(std::memcmp(reinterpret_cast<const void*>(image.multibyte_type_table()),mb::initial_type_table().data(),mb::type_size)==0);
    check(std::memcmp(reinterpret_cast<const void*>(image.multibyte_case_table()),mb::initial_case_table().data(),mb::case_size)==0);
    // Actual recovered source accesses the shipped category/time/self graph.
    r::add_locale_ref_070634(locale,ids,image);
    check(image.counter_value(locale+0x10)==2&&image.counter_value(time+0x15c)==0);
    r::release_locale_ref_0708c0(locale,ids,image);
    check(image.counter_value(locale+0x10)==1);
    check(r::add_time_ref_070838(time,ids,image)==std::numeric_limits<std::int32_t>::max());
    check(r::release_time_ref_070898(time,ids,image)==std::numeric_limits<std::int32_t>::max());
    check(image.counter_value(time+0x15c)==0);
    // No byte views or registry mutations during four concurrent workers.
    std::atomic<unsigned> ready{0};
    std::atomic<bool> go{false};
    std::array<std::thread,4> workers;
    for(auto& worker:workers) worker=std::thread([&] {
        r::add_locale_ref_070634(locale,ids,image);
        ready.fetch_add(1);
        while(!go.load()) std::this_thread::yield();
        r::release_locale_ref_0708c0(locale,ids,image);
    });
    while(ready.load()!=4) std::this_thread::yield();
    check(image.counter_value(locale+0x10)==5);
    go.store(true);
    for(auto& worker:workers) worker.join();
    check(image.counter_value(locale+0x10)==1);
    check(other.counter_value(other.globals().initial_locale+0x10)==1);
    check(image.atomic_fetch_add_32(g.initial_multibyte,1)==0);
    check(image.atomic_fetch_add_32(g.initial_multibyte,0xffffffffU)==1);
    check(image.counter_value(g.initial_multibyte)==0);
    rejects<std::out_of_range>([&]{(void)image.region_address(regions.size());});
    rejects<std::out_of_range>([&]{(void)image.read_address(locale+regions[d::Locale].bytes.size()-1);});
    rejects<std::out_of_range>([&]{(void)image.read_address(0x1800a17e0);});
    rejects<std::out_of_range>([&]{(void)image.read_address(0);});
    rejects<std::out_of_range>([&]{(void)image.counter_value(locale);});
}
}
int main(){try{run();std::cout<<"PASS: "<<checks<<" owned initial state checks;116 relocations;actual locale refs;4 own workers\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<" at "<<checks<<"\n";return 1;}}
