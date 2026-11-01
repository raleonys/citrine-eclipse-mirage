#include "multibyte_support.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace m=anadius::crt_multibyte_support_recovered;
namespace {
std::uint64_t checks;
void check(bool b) { ++checks; if(!b) throw std::runtime_error("check "+std::to_string(checks)); }
struct Event { std::string operation; m::Address address,value; };
class Fixture final:public m::Effects {
public:
    std::unordered_map<m::Address,std::uint8_t> bytes;
    std::vector<Event> events;
    std::function<void(const Event&)> hook;
    anadius::crt_byteclass_recovered::LocaleContext locale{0x1000,0x2000,0x3000,1};
    m::Address type=0x5000,fold=0x6000,initial=0x7000;
    std::uint32_t error=0,flag=7,changed=0,acp=1252,oem=437;
    Fixture() {
        region(0x1000,0x400);region(0x2000,0x400);region(0x3000,0x400);
        region(0x4000,0x400);region(type,257);region(fold,256);region(initial,0x400);
        region(0x8000,8);region(0x9000,0x400);region(0xa000,8);
        put(0x1088,m::Address{0x3000});put(0x2088,m::Address{0x9000});
        put(0x8000,m::Address{0x4000});put(0xa000,m::Address{0x7000});
        put(0x4000,std::uint32_t{1});put(0x3000,std::uint32_t{2});put(0x7000,std::uint32_t{1});
        put(0x200c,std::uint32_t{932});put(0x300c,std::uint32_t{936});put(0x13a8,std::uint32_t{0xdeadbeef});
        for(unsigned i=0;i<513;++i) bytes[0x3018+i]=static_cast<std::uint8_t>(i*7+3);
        for(unsigned i=0;i<513;++i) bytes[0x9018+i]=static_cast<std::uint8_t>(i*11+9);
    }
    void region(m::Address a,unsigned n) { for(unsigned i=0;i<n;++i) bytes[a+i]=0; }
    template<class T> T get(m::Address a) const {
        T value{}; auto* p=reinterpret_cast<std::uint8_t*>(&value);
        for(unsigned i=0;i<sizeof(T);++i) p[i]=bytes.at(a+i);
        return value;
    }
    template<class T> void put(m::Address a,T value) {
        const auto* p=reinterpret_cast<const std::uint8_t*>(&value);
        for(unsigned i=0;i<sizeof(T);++i) bytes.at(a+i)=p[i];
    }
    void event(std::string op,m::Address a=0,m::Address v=0) {
        const Event e{std::move(op),a,v}; events.push_back(e);if(hook)hook(e);
    }
    anadius::crt_byteclass_recovered::LocaleContext locale_update_058b04(m::Address a) override {
        const auto result=locale;event("locale",a);return result;
    }
    m::Address read_address(m::Address a) override { auto x=get<m::Address>(a);event("r64",a,x);return x; }
    void write_address(m::Address a,m::Address x) override { put(a,x);event("w64",a,x); }
    std::uint8_t read_u8(m::Address a) override { auto x=get<std::uint8_t>(a);event("r8",a,x);return x; }
    void write_u8(m::Address a,std::uint8_t x) override { put(a,x);event("w8",a,x); }
    std::uint32_t read_u32(m::Address a) override { auto x=get<std::uint32_t>(a);event("r32",a,x);return x; }
    void write_u32(m::Address a,std::uint32_t x) override { put(a,x);event("w32",a,x); }
    std::uint32_t get_acp() override { event("acp");return acp; }
    std::uint32_t get_oemcp() override { event("oem");return oem; }
    void write_system_cp_flag_0a59d8(std::uint32_t x) override {flag=x;event("flag",0,x);}
    m::Address type_table_0a59c0() override {event("type");return type;}
    m::Address case_table_0a59c8() override {event("case");return fold;}
    m::Address initial_multibyte_0a1c70() override {return initial;}
    void lock_0625f8(std::uint32_t x) override {event("lock",x);}
    void unlock_06264c(std::uint32_t x) override {event("unlock",x);}
    std::uint32_t atomic_fetch_add_32(m::Address a,std::uint32_t delta) override {
        const auto old=get<std::uint32_t>(a);put(a,old+delta);event("atomic",a,delta);return old;
    }
    void free_0668c0(m::Address a) override {event("free",a);}
    anadius::crt_errors_recovered::Cell32 errno_cell_05d724() override {event("errno");return anadius::crt_errors_recovered::Cell32(&error);}
    void invalid_parameter_05c954() override {check(error==22);event("invalid");}
    std::uint32_t exchange_locale_changed_0a52d8(std::uint32_t x) override {const auto old=changed;changed=x;event("exchange",0,x);return old;}
    unsigned count(const std::string& op) const {return static_cast<unsigned>(std::count_if(events.begin(),events.end(),[&](const auto& e){return e.operation==op;}));}
};
void cp_tests() {
    for(const auto request:{0,1,65001,-1,-2,-3,-4,INT32_MIN,INT32_MAX}) {
        for(unsigned clear=0;clear<2;++clear) {
            Fixture f;f.locale.clear_thread_flag=static_cast<std::uint8_t>(clear);
            const auto result=m::system_cp_06efd0(request,f);
            const auto expected=request==-2?437:request==-3?1252:request==-4?932:request;
            check(result==expected);check(f.flag==static_cast<unsigned>(request<=-2&&request>=-4));
            check(f.events[0].operation=="locale");check(f.events[1].operation=="flag"&&f.events[1].value==0);
            check(f.get<std::uint32_t>(0x13a8)==(clear?0xdeadbeedU:0xdeadbeefU));
            check(f.count("acp")==static_cast<unsigned>(request==-3));check(f.count("oem")==static_cast<unsigned>(request==-2));
        }
    }
    Fixture f;f.acp=0xffffffffU;check(m::system_cp_06efd0(-3,f)==-1);
    f.hook=[](const Event& e){if(e.operation=="oem")throw std::runtime_error("api");};
    f.put(0x13a8,std::uint32_t{2});bool threw=false;
    try{(void)m::system_cp_06efd0(-2,f);}catch(const std::runtime_error&){threw=true;}
    check(threw);check(f.get<std::uint32_t>(0x13a8)==2);check(f.flag==1);
}
void basic_publish() {
    for(const auto oldcount:{0U,1U,2U,0xffffffffU}) {
        Fixture f;f.put(0x4000,oldcount);m::Address ptd=0x1000,slot=0x8000;
        m::publish_06ee00(ptd,slot,5,7,f);
        check(f.events.front().operation=="lock"&&f.events.front().address==5);
        check(f.events.back().operation=="unlock"&&f.events.back().address==7);
        for(unsigned i=0;i<257;++i)check(f.bytes.at(0x5000+i)==f.bytes.at(0x3018+i));
        for(unsigned i=0;i<256;++i)check(f.bytes.at(0x6000+i)==f.bytes.at(0x3119+i));
        check(f.get<std::uint32_t>(0x4000)==oldcount-1);check(f.get<std::uint32_t>(0x3000)==3);
        check(f.get<m::Address>(slot)==0x3000);check(f.count("free")==static_cast<unsigned>(oldcount==1));
        // First 16-byte vector source read completes before either qword write.
        check(f.events[3].operation=="r64"&&f.events[3].address==0x3018);
        check(f.events[4].operation=="r64"&&f.events[4].address==0x3020);
        check(f.events[5].operation=="w64"&&f.events[5].address==0x5000);
        check(f.events[6].operation=="w64"&&f.events[6].address==0x5008);
    }
    Fixture f;m::Address ptd=0x1000,slot=0xa000;
    m::publish_06ee00(ptd,slot,1,1,f);check(f.count("free")==0);check(f.get<std::uint32_t>(0x7000)==0);
    Fixture same;same.put(0x8000,m::Address{0x3000});same.put(0x3000,std::uint32_t{1});
    slot=0x8000;m::publish_06ee00(ptd,slot,1,1,same);
    check(same.count("free")==1);check(same.get<std::uint32_t>(0x3000)==1);
}
void mutation_publish() {
    Fixture f;m::Address ptd=0x1000,slot=0x8000;
    f.hook=[&](const Event& e){
        if(e.operation=="case")ptd=0x2000;
        if(e.operation=="atomic"&&e.value==0xffffffffU){slot=0xa000;f.put(0xa000,m::Address{0x9000});}
        if(e.operation=="free"){check(e.address==0x9000);ptd=0x1000;}
        if(e.operation=="w64"&&e.address==0xa000){ptd=0x2000;}
    };
    m::publish_06ee00(ptd,slot,2,3,f);
    check(f.get<m::Address>(0xa000)==0x3000);check(f.get<std::uint32_t>(0x3000)==2);
    check(f.get<std::uint32_t>(0x9000)==1);check(f.count("free")==1);
    Fixture fresh;ptd=0x1000;slot=0x8000;unsigned reads=0;
    fresh.hook=[&](const Event& e){
        if(e.operation=="r64"&&e.address==0x8000&&++reads==2)
            fresh.put(0x8000,m::Address{0x9000});
        if(e.operation=="free")check(e.address==0x9000);
    };
    m::publish_06ee00(ptd,slot,2,3,fresh);
    check(reads==3);check(fresh.count("free")==1);
    Fixture aliased;ptd=0x1000;aliased.put(0x1000,m::Address{0x4000});
    m::publish_06ee00(ptd,ptd,2,3,aliased);
    check(aliased.get<m::Address>(0x1000)==0x3000);
    check(aliased.get<std::uint32_t>(0x3000)==3);
    Fixture second;ptd=0x1000;slot=0x8000;
    second.hook=[&](const Event& e){if(e.operation=="w8"&&e.address==0x5100)ptd=0x2000;};
    m::publish_06ee00(ptd,slot,2,3,second);
    for(unsigned i=0;i<256;++i)check(second.bytes.at(0x6000+i)==second.bytes.at(0x9119+i));
    // memcpy follows forward 16-byte chunks rather than snapshotting 256 bytes.
    Fixture overlap;overlap.type=0x3020;std::vector<std::uint8_t> expected(0x400);
    for(unsigned i=0;i<expected.size();++i)expected[i]=overlap.bytes.at(0x3000+i);
    for(unsigned k=0;k<16;++k){std::uint8_t chunk[16];std::memcpy(chunk,expected.data()+0x18+k*16,16);std::memcpy(expected.data()+0x20+k*16,chunk,16);}
    expected[0x120]=expected[0x118];ptd=0x1000;slot=0x8000;
    m::publish_06ee00(ptd,slot,4,4,overlap);
    for(unsigned i=0;i<257;++i)check(overlap.bytes.at(0x3020+i)==expected[0x20+i]);
}
void error_publish() {
    for(unsigned mode=0;mode<4;++mode) {
        Fixture f;m::Address ptd=0x1000,slot=0x8000;
        if(mode==0){f.type=0;f.fold=0;}
        if(mode==1){f.type=0;f.hook=[&](const Event&e){if(e.operation=="invalid")ptd=0x2000;};}
        if(mode==2){f.put(0x1088,m::Address{0xffffffffffffffe8ULL});f.hook=[&](const Event&e){if(e.operation=="invalid")f.put(0x1088,m::Address{0x3000});};}
        if(mode==3){f.put(0x1088,m::Address{0xfffffffffffffee7ULL});f.type=0;f.hook=[&](const Event&e){if(e.operation=="invalid"&&f.count("invalid")==2)f.put(0x1088,m::Address{0x3000});};}
        m::publish_06ee00(ptd,slot,6,6,f);
        check(f.error==22);check(f.count("invalid")==((mode==0||mode==3)?2U:1U));check(f.count("unlock")==1);
        if(mode==1)for(unsigned i=0;i<256;++i)check(f.bytes.at(0x6000+i)==f.bytes.at(0x9119+i));
        if(mode==2)for(unsigned i=0;i<257;++i)check(f.bytes.at(0x5000+i)==0);
        if(mode==3)for(unsigned i=0;i<256;++i)check(f.bytes.at(0x6000+i)==0);
    }
    Fixture f;f.type=0;m::Address ptd=0x1000,slot=0x8000;
    f.hook=[](const Event& e){if(e.operation=="invalid")throw std::runtime_error("handler");};
    bool threw=false;try{m::publish_06ee00(ptd,slot,2,2,f);}catch(const std::runtime_error&){threw=true;}
    check(threw);check(f.count("unlock")==0);check(f.count("atomic")==0);check(f.count("case")==0);
}
}
int main(){try{cp_tests();basic_publish();mutation_publish();error_publish();Fixture f;f.changed=0xffffffffU;check(m::mark_locale_changed_06532c(f)==0xffffffffU);check(m::mark_locale_changed_06532c(f)==1);check(f.changed==1);std::cout<<checks<<" multibyte support checks passed\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
