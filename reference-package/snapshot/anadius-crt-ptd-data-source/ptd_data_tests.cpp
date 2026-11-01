#include "ptd_data.hpp"
#include "../anadius-crt-ptd-source/ptd.hpp"
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
namespace d=anadius::crt_ptd_data_recovered;
namespace p=anadius::crt_ptd_recovered;
namespace t=anadius::threads_recovered;
namespace {
unsigned checks;
void check(bool v) { ++checks; if(!v) throw std::runtime_error("check "+std::to_string(checks)); }
struct Data final:d::Effects {
    d::Identities ids{0x1000,0x2000,0x3000};
    d::Address current=0x4000;
    std::map<d::Address,std::uint32_t> refs;
    std::vector<std::string> events;
    std::vector<d::Address> freed;
    std::function<void(d::Address)> on_release,on_free;
    std::function<void(std::uint32_t)> on_lock;
    void lock_0625f8(std::uint32_t id) override { events.push_back("lock"+std::to_string(id)); if(on_lock)on_lock(id); }
    void unlock_06264c(std::uint32_t id) override { events.push_back("unlock"+std::to_string(id)); }
    std::uint32_t atomic_fetch_add_32(d::Address address,std::uint32_t increment) override {
        events.push_back(increment==1?"increment":"decrement");
        const auto old=refs[address]; refs[address]+=increment; return old;
    }
    void free_0668c0(d::Address address) override {
        events.push_back("free"+std::to_string(address)); freed.push_back(address); if(on_free)on_free(address);
    }
    d::Address current_locale_0a52e0() override { events.emplace_back("current"); return current; }
    void release_locale_ref_0708c0(d::Address address) override {
        events.push_back("release"+std::to_string(address)); if(on_release)on_release(address);
    }
    std::uint32_t locale_ref_count_10(d::Address address) override {
        events.push_back("count"+std::to_string(address)); return refs[address];
    }
    void free_locale_0706c0(d::Address address) override { events.push_back("free_locale"+std::to_string(address)); }
    void add_locale_ref_070634(d::Address address) override { events.push_back("add"+std::to_string(address)); }
};
void construction() {
    // Nonzero prior bytes expose accidental zeroing of unrelated PTD fields.
    for(unsigned seed:{0x00U,0x55U,0xaaU,0xffU}) {
        alignas(8) std::array<std::byte,0x3c8> bytes; bytes.fill(std::byte(seed));
        auto expected=bytes; d::View view(bytes.data()), wanted(expected.data()); Data e;
        wanted.store<std::uint32_t>(0x28,1); wanted.store<d::Address>(0,e.ids.exception_table);
        wanted.store<std::uint32_t>(0x3a8,1); wanted.store<d::Address>(0x88,e.ids.multibyte_info);
        wanted.store<std::uint16_t>(0xbc,0x43); wanted.store<std::uint16_t>(0x1c2,0x43);
        wanted.store<d::Address>(0x3a0,0); wanted.store<d::Address>(0x90,e.current);
        // Constructor's locale update can release a preexisting value. Real
        // calloc PTD is zero, but this bounded probe verifies only touched bytes.
        const auto previous=view.load<d::Address>(0x90); e.refs[previous]=1;
        d::construct_0648fc(bytes.data(),e.ids,e);
        check(bytes==expected); check(e.refs[e.ids.multibyte_info]==1);
        check(e.events[0]=="lock5" && e.events[1]=="increment" && e.events[2]=="unlock5");
        check(e.events[3]=="lock4" && e.events.back()=="unlock4");
    }
    for(std::uint32_t old:{0U,1U,0xffffffffU}) {
        p::Ptd ptd; Data e; e.refs[e.ids.multibyte_info]=old;
        d::construct_0648fc(&ptd,e.ids,e);
        check(e.refs[e.ids.multibyte_info]==old+std::uint32_t{1});
        check(ptd.thread_context==nullptr);
    }
}
void locale_replacement() {
    for(d::Address old:{d::Address{0},0x3000ULL,0x4000ULL,0x5000ULL})
    for(d::Address replacement:{d::Address{0},0x6000ULL})
    for(std::uint32_t count:{0U,1U,0x80000000U}) {
        p::Ptd ptd; d::View view(&ptd); view.store<d::Address>(0x90,old); Data e; e.refs[old]=count;
        d::replace_locale_064ae4(&ptd,replacement,e.ids,e);
        check(view.load<d::Address>(0x90)==replacement);
        const bool free_private=old==0x5000 && count==0;
        check((std::find(e.events.begin(),e.events.end(),"free_locale20480")!=e.events.end())==free_private);
        check(std::count(e.events.begin(),e.events.end(),"current")==static_cast<int>(old!=0));
        check(std::count(e.events.begin(),e.events.end(),"add24576")==static_cast<int>(replacement!=0));
        if(old)check(e.events.front()=="release"+std::to_string(old));
    }
    p::Ptd ptd; d::View view(&ptd); Data e; view.store<d::Address>(0x90,0x5000);
    e.on_release=[&](d::Address old){check(old==0x5000); view.store<d::Address>(0x90,0x7000); e.current=0x8000;};
    d::replace_locale_064ae4(&ptd,0x9000,e.ids,e);
    check(e.events==std::vector<std::string>({"release20480","current","count28672","free_locale28672","add36864"}));
    check(view.load<d::Address>(0x90)==0x9000);
    Data locked; locked.on_lock=[&](std::uint32_t id){check(id==4); locked.current=0x9876;};
    view.store<d::Address>(0x90,0);
    d::current_locale_064854(&ptd,4,7,locked.ids,locked);
    check(view.load<d::Address>(0x90)==0x9876); check(locked.events.back()=="unlock7");
}
void multibyte_and_destruction() {
    for(d::Address info:{d::Address{0},0x2000ULL,0x5000ULL})
    for(std::uint32_t old:{0U,1U,2U,0xffffffffU}) {
        p::Ptd ptd; d::View(&ptd).store<d::Address>(0x88,info); Data e; e.refs[info]=old;
        d::release_multibyte_06489c(&ptd,5,9,e.ids,e);
        check(e.events.front()=="lock5" && e.events.back()=="unlock9");
        check(e.refs[info]==(info?old-std::uint32_t{1}:old));
        check(e.freed.size()==static_cast<std::size_t>(info==0x5000 && old==1));
    }
    p::Ptd ptd; d::View view(&ptd); Data e;
    view.store<d::Address>(0,e.ids.exception_table);
    constexpr std::array<std::size_t,9> offsets{0x70,0x58,0x60,0x68,0x48,0x50,0x78,0x80,0x3c0};
    for(unsigned i=0;i<offsets.size();++i)view.store<d::Address>(offsets[i],0x10000+i);
    view.store<d::Address>(0x88,0x5000); e.refs[0x5000]=1;
    view.store<d::Address>(0x90,e.current);
    e.on_free=[&](d::Address address) { if(address==0x10000)view.store<d::Address>(0x58,0x123456); };
    d::destroy_0649ec(&ptd,e.ids,e);
    check(e.freed==std::vector<d::Address>({0x10000,0x123456,0x10002,0x10003,0x10004,0x10005,0x10006,0x10007,0x10008,0x5000}));
    check(view.load<d::Address>(0x3c0)==0x10008); check(view.load<d::Address>(0x90)==0);
    check(e.events[e.events.size()-2]=="current" && e.events.back()=="unlock4");
    Data zero; p::Ptd empty;
    d::destroy_0649ec(&empty,zero.ids,zero);
    check(zero.freed.size()==10 && std::count(zero.freed.begin(),zero.freed.end(),0)==10);
    zero.events.clear(); zero.freed.clear();
    d::fls_destroy_0649cc(nullptr,zero.ids,zero); check(zero.events.empty());
    d::fls_destroy_0649cc(&empty,zero.ids,zero);
    check(zero.freed.size()==11 && zero.freed.back()==reinterpret_cast<d::Address>(&empty));
}
struct Lifecycle final:p::Effects {
    Data& data; p::Ptd* value=nullptr; std::unique_ptr<p::Ptd> owned;
    std::uint32_t error=0x12345678; std::vector<std::string> trace;
    explicit Lifecycle(Data& d):data(d){}
    p::Ptd* fls_get_value(std::uint32_t) override { trace.emplace_back("get"); return value; }
    bool fls_set_value(std::uint32_t,p::Ptd* v) override { trace.emplace_back("set"); value=v; return true; }
    std::uint32_t fls_alloc(p::FlsCallback cb) override { check(cb==p::FlsCallback::destroy_0649cc); return 7; }
    bool fls_free(std::uint32_t index) override { check(index==7); if(value)d::fls_destroy_0649cc(value,data.ids,data); value=nullptr; owned.reset(); return true; }
    std::uint32_t last_error() override { return error; }
    void set_last_error(std::uint32_t v) override { error=v; }
    p::Ptd* calloc_ptd_066848() override { owned=std::make_unique<p::Ptd>(); return owned.get(); }
    void free_0668c0(p::Ptd* v) override { trace.emplace_back(v?"free_ptd":"free_null"); if(v){check(v==owned.get());owned.reset();} }
    void construct_ptd_array_0648fc(p::Ptd* v) override { check(value==v); d::construct_0648fc(v,data.ids,data); }
    void destroy_ptd_array_0649ec(p::Ptd* v) override { d::destroy_0649ec(v,data.ids,data); }
    [[noreturn]] void abort_05f03c() override { throw std::runtime_error("abort"); }
};
void lifecycle_integration() {
    Data data; Lifecycle lifecycle(data); p::State state;
    check(p::initialize_064e90(state,lifecycle)); check(state.fls_index==7);
    check(lifecycle.error==0x12345678); check(data.refs[data.ids.multibyte_info]==1);
    auto context=std::make_unique<t::Context>(); auto* context_address=context.get();
    auto* ptd=p::required_preserve_error_064b90(state,lifecycle);
    ptd->thread_context=context.get();
    data.on_free=[&](d::Address address){if(address==reinterpret_cast<d::Address>(context_address))context.reset();};
    const auto same=p::optional_preserve_error_064d08(state,lifecycle);
    check(same==ptd && same->thread_context==context_address);
    p::free_ptd_064b4c(state,lifecycle);
    check(!context && !lifecycle.owned && !lifecycle.value);
    check(data.refs[data.ids.multibyte_info]==0);
    check(lifecycle.trace.back()=="free_ptd");
    check(p::uninitialize_064ecc(state,lifecycle)); check(state.fls_index==p::invalid_fls_index);
    // FLS-free callback path destroys children before releasing the PTD itself.
    data.events.clear(); data.freed.clear();
    check(p::initialize_064e90(state,lifecycle));
    auto* second=lifecycle.value;
    check(p::uninitialize_064ecc(state,lifecycle));
    check(data.freed.back()==reinterpret_cast<d::Address>(second));
    check(!lifecycle.value && !lifecycle.owned);
}
}
int main(){try{construction();locale_replacement();multibyte_and_destruction();lifecycle_integration();std::cout<<"PASS "<<checks<<" PTD data/lifetime checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
