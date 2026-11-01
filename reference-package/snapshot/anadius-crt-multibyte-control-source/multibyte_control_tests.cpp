#include "multibyte_control.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace mc=anadius::crt_multibyte_control_recovered;
using Address=mc::Address;
namespace {
std::size_t checks;
void check(bool condition) {++checks;if(!condition) throw std::runtime_error("check "+std::to_string(checks));}
struct Fixture final:mc::Effects {
    static constexpr Address ptd=0x1000,slot=0x2000,old=0x3000,fresh=0x4000,initial=0x5000,type=0x6000,cases=0x7000,other=0x8000;
    std::array<std::byte,0x10000> mem{};
    std::vector<std::string> calls;
    std::vector<std::pair<char,Address>> traffic;
    std::vector<Address> frees;
    std::uint32_t error=77,status=0,mask=0,changed=0,system=9,acp=932;
    std::uint8_t initialized=0;
    Address allocation=fresh,typetable=type,casetable=cases,initialpair=0;
    std::function<void()> malloc_hook,build_hook,atomic_hook,free_hook;
    Fixture() {
        put<Address>(ptd+0x88,old);put<Address>(slot,old);
        for(unsigned n=0;n<0x228;++n) mem[old+n]=std::byte((n*73+11)&255);
        put<std::uint32_t>(old,2);put<std::uint32_t>(old+4,1252);
        for(unsigned n=0;n<0x228;++n) mem[initial+n]=mem[old+n];
    }
    template<class T> T get(Address a) const {if(a>mem.size()-sizeof(T)) throw std::runtime_error("read bounds");T v;std::memcpy(&v,mem.data()+a,sizeof v);return v;}
    template<class T> void put(Address a,T v) {if(a>mem.size()-sizeof(T)) throw std::runtime_error("write bounds");std::memcpy(mem.data()+a,&v,sizeof v);}
    anadius::crt_byteclass_recovered::LocaleContext locale_update_058b04(Address a) override {check(a==0);calls.push_back("locale");return {};}
    Address read_address(Address a) override {traffic.emplace_back('r',a);return get<Address>(a);}
    void write_address(Address a,Address v) override {traffic.emplace_back('w',a);put(a,v);}
    std::uint8_t read_u8(Address a) override {return get<std::uint8_t>(a);}
    void write_u8(Address a,std::uint8_t v) override {put(a,v);}
    std::uint32_t read_u32(Address a) override {return get<std::uint32_t>(a);}
    void write_u32(Address a,std::uint32_t v) override {put(a,v);}
    std::uint32_t get_acp() override {calls.push_back("acp");return acp;}
    std::uint32_t get_oemcp() override {calls.push_back("oem");return 437;}
    void write_system_cp_flag_0a59d8(std::uint32_t v) override {system=v;}
    Address type_table_0a59c0() override {return typetable;}
    Address case_table_0a59c8() override {return casetable;}
    Address initial_multibyte_0a1c70() override {return initial;}
    void lock_0625f8(std::uint32_t n) override {check(n==5);calls.push_back("lock");}
    void unlock_06264c(std::uint32_t n) override {check(n==5);calls.push_back("unlock");}
    std::uint32_t atomic_fetch_add_32(Address a,std::uint32_t n) override {auto v=get<std::uint32_t>(a);put(a,v+n);calls.push_back("atomic");if(atomic_hook) {auto h=std::move(atomic_hook);atomic_hook={};h();}return v;}
    void free_0668c0(Address a) override {frees.push_back(a);calls.push_back("free");if(free_hook) free_hook();}
    anadius::crt_errors_recovered::Cell32 errno_cell_05d724() override {calls.push_back("errno");return anadius::crt_errors_recovered::Cell32(&error);}
    void invalid_parameter_05c954() override {calls.push_back("invalid");}
    std::uint32_t exchange_locale_changed_0a52d8(std::uint32_t v) override {auto prev=changed;changed=v;calls.push_back("mark");return prev;}
    Address malloc_06a308(std::uint64_t n) override {check(n==0x228);calls.push_back("malloc");if(malloc_hook) malloc_hook();return allocation;}
    std::uint32_t build_multibyte_06f674(std::int32_t cp,Address record) override {check(cp==static_cast<std::int32_t>(acp));check(record==fresh);check(get<std::uint32_t>(fresh)==0);calls.push_back("build");if(build_hook) build_hook();return status;}
    Address update_thread_multibyte_06f540(Address p,Address s) override {check(p==ptd&&s==slot);calls.push_back("update");return 0xdeadbeef;}
    Address required_ptd_raw_064c64() override {calls.push_back("ptd");return ptd;}
    std::uint8_t initialized_0a59dc() override {return initialized;}
    void set_initialized_0a59dc(std::uint8_t v) override {calls.push_back("initialized");initialized=v;}
    void set_type_table_0a59c0(Address v) override {calls.push_back("type");typetable=v;}
    void set_case_table_0a59c8(Address v) override {calls.push_back("case");casetable=v;}
    Address current_multibyte_slot_0a59d0() override {return slot;}
    void set_initial_pair_multibyte_0a1940(Address v) override {calls.push_back("pair");initialpair=v;}
    std::uint32_t locale_mask_0a1c6c() override {return mask;}
    std::uint32_t run(std::uint8_t init=0) {return mc::set_multibyte_06f2d0(static_cast<std::int32_t>(acp),init,ptd,slot,*this);}
    unsigned count(const std::string& s) const {return static_cast<unsigned>(std::count(calls.begin(),calls.end(),s));}
};
void early_paths() {
    Fixture same;same.acp=1252;check(same.run()==0);check(same.calls==std::vector<std::string>({"update","locale"}));check(same.frees.empty());check(same.system==0);
    Fixture denied;denied.allocation=0;check(denied.run()==UINT32_MAX);check(denied.frees==std::vector<Address>{0});check(denied.error==77);check(denied.count("build")==0);check(denied.changed==0);
    Fixture invalid;invalid.status=UINT32_MAX;check(invalid.run()==UINT32_MAX);check(invalid.error==22);check(invalid.frees==std::vector<Address>{Fixture::fresh});check(invalid.get<Address>(Fixture::ptd+0x88)==Fixture::old);check(invalid.get<std::uint32_t>(Fixture::old)==2);check(invalid.changed==0);check(invalid.count("lock")==0);
}
void matrix() {
    for(auto status:std::array<std::uint32_t,4>{0,1,0x80000000U,0xfffffffeU})
    for(auto init:std::array<std::uint8_t,3>{0,1,128})
    for(auto skip:std::array<bool,2>{false,true})
    for(auto refs:std::array<std::uint32_t,3>{0,1,2})
    for(auto initial_old:std::array<bool,2>{false,true}) {
        Fixture f;f.status=status;f.mask=0x20;f.put<std::uint32_t>(Fixture::ptd+0x3a8,skip?0x20:0);
        const Address old=initial_old?Fixture::initial:Fixture::old;
        f.put<Address>(Fixture::ptd+0x88,old);f.put<Address>(Fixture::slot,old);f.put<std::uint32_t>(old,refs);
        std::array<std::byte,0x228> expected;std::copy_n(f.mem.data()+old,expected.size(),expected.data());
        f.build_hook=[&] {for(unsigned i=4;i<0x228;++i) check(f.mem[Fixture::fresh+i]==expected[i]);};
        check(f.run(init)==status);check(f.error==77);check(f.changed==(init==0?1U:0U));
        check(f.get<Address>(Fixture::ptd+0x88)==Fixture::fresh);
        check(f.get<std::uint32_t>(Fixture::fresh)==(skip?1U:2U));
        check(f.get<Address>(Fixture::slot)==(skip?old:Fixture::fresh));
        check(f.get<std::uint32_t>(old)==refs-(skip?1U:2U));
        check(f.count("lock")==(!skip?1U:0U));check(f.count("unlock")==(!skip?1U:0U));
        check(f.initialpair==((!skip&&init!=0)?Fixture::fresh:0));check(f.frees.back()==0);
        check(std::count(f.frees.begin(),f.frees.end(),old)==((!initial_old&&(refs==1||(!skip&&refs==2)))?1:0));
        if(!skip) {
            for(unsigned i=0;i<257;++i) check(f.mem[Fixture::type+i]==expected[0x18+i]);
            for(unsigned i=0;i<256;++i) check(f.mem[Fixture::cases+i]==expected[0x119+i]);
        }
        auto first_write=std::find(f.traffic.begin(),f.traffic.end(),std::pair<char,Address>{'w',Fixture::fresh});
        check(first_write!=f.traffic.end());
        check(std::count_if(f.traffic.begin(),first_write,[old](auto x){return x.first=='r'&&x.second>=old&&x.second<old+0x228;})==69);
        for(unsigned i=0;i<69;++i) check((first_write+i)->first=='w'&&(first_write+i)->second==Fixture::fresh+i*8);
    }
}
void mutations() {
    Fixture f;f.mask=1;f.put<std::uint32_t>(Fixture::ptd+0x3a8,1);
    std::copy_n(f.mem.data()+Fixture::old,0x228,f.mem.data()+Fixture::other);f.put<std::uint32_t>(Fixture::other,1);
    f.malloc_hook=[&] {f.put<Address>(Fixture::ptd+0x88,Fixture::other);f.mem[Fixture::other+0x227]=std::byte{0xc7};};
    f.atomic_hook=[&] {f.put<Address>(Fixture::ptd+0x88,Fixture::old);};
    f.free_hook=[&] {f.error=99;};
    check(f.run()==0);check(f.mem[Fixture::fresh+0x227]==std::byte{0xc7});
    check(f.frees==std::vector<Address>({Fixture::old,0}));check(f.get<std::uint32_t>(Fixture::other)==0);check(f.error==99);
    Fixture status;status.status=0x80000000U;status.free_hook=[&]{status.status=7;};check(status.run()==0x80000000U);
}
void initialization() {
    const mc::Identities ids{Fixture::initial,Fixture::type,Fixture::cases};
    for(auto fail:std::array<unsigned,3>{0,1,2}) {
        Fixture f;f.typetable=0;f.casetable=0;
        if(fail==1) f.allocation=0;
        if(fail==2) f.status=UINT32_MAX;
        check(mc::initialize_multibyte_06f5f8(ids,f));check(f.initialized==1);check(f.system==1);check(f.changed==0);
        check(f.typetable==Fixture::type&&f.casetable==Fixture::cases);
        check(f.calls.front()=="case");check(f.calls[1]=="type");check(f.calls[2]=="ptd");
        check(f.calls.back()=="initialized");check(f.count("acp")==1);
        const auto calls=f.calls;const auto traffic=f.traffic;const auto frees=f.frees;
        check(mc::initialize_multibyte_06f5f8(ids,f));check(f.calls==calls);check(f.traffic==traffic);check(f.frees==frees);
    }
    Fixture f;f.initialized=0xff;check(mc::initialize_multibyte_06f5f8(ids,f));check(f.calls.empty());
}
}
int main() {
    try {early_paths();matrix();mutations();initialization();std::printf("multibyte control: %zu checks PASS\n",checks);return 0;}
    catch(const std::exception& ex) {std::fprintf(stderr,"FAIL: %s\n",ex.what());return 1;}
}
