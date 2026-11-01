#include "locale_update.hpp"
#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace l=anadius::crt_locale_update_recovered;
static_assert(offsetof(l::Context,ptd)==0);
static_assert(offsetof(l::Context,locale_info)==8);
static_assert(offsetof(l::Context,multibyte_info)==16);
static_assert(offsetof(l::Context,clear_thread_flag)==24);
namespace {
std::uint64_t checks=0;
void require(bool value,const char* message) {
    ++checks;
    if(!value) throw std::runtime_error(message);
}
struct Fixture final:l::Effects {
    std::map<l::Address,l::Address> addresses{{0x1090,0x2000},{0x1088,0x3000}};
    std::map<l::Address,l::Address*> aliases;
    std::map<l::Address,std::uint32_t> words{{0x13a8,0}};
    std::vector<std::string> trace;
    std::function<void(const std::string&)> observe;
    std::function<l::Address()> locale_update,multibyte_update;
    std::uint32_t changed=1,mask=2;
    l::Address current_locale=0x4000,current_multibyte=0x5000,ptd=0x1000;
    std::array<l::Address,2> global{0x6000,0x7000};
    void event(std::string name) {
        trace.push_back(name);
        if(observe) observe(name);
    }
    l::Address read_address(l::Address address) override {
        event("address:"+std::to_string(address));
        if(aliases.contains(address)) return *aliases.at(address);
        return addresses.at(address);
    }
    std::uint32_t read_u32(l::Address address) override {
        event("read:"+std::to_string(address));
        return words.at(address);
    }
    void write_u32(l::Address address,std::uint32_t value) override {
        event("write:"+std::to_string(address));words.at(address)=value;
    }
    std::uint32_t locale_changed_0a52d8() override {event("changed");return changed;}
    std::array<l::Address,2> global_pair_0a1938() override {event("global");return global;}
    l::Address required_ptd_064b90() override {event("ptd");return ptd;}
    l::Address current_locale_0a52e0() override {event("current-locale");return current_locale;}
    l::Address current_multibyte_0a59d0() override {event("current-multibyte");return current_multibyte;}
    std::uint32_t locale_mask_0a1c6c() override {event("mask");return mask;}
    l::Address update_thread_locale_070968() override {
        event("update-locale");return locale_update?locale_update():0x8000;
    }
    l::Address update_thread_multibyte_06f658() override {
        event("update-multibyte");return multibyte_update?multibyte_update():0x9000;
    }
};
void fast_paths() {
    Fixture f;l::Context out{0xaaaa,0xbbbb,0xcccc,0xff};
    f.aliases.emplace(0xa000,&out.ptd);
    f.aliases.emplace(0xa008,&out.locale_info);
    f.observe=[&](const auto&){require(out.clear_thread_flag==0,"clear flag precedes pair reads");};
    l::locale_update_058b04(out,0xa000,f);
    require(out.ptd==0xaaaa,"explicit locale preserves ptd");
    require(out.locale_info==0xaaaa&&out.multibyte_info==0xbbbb,"overlapping pair snapshots before stores");
    require(f.trace==std::vector<std::string>{"address:40960","address:40968"},"explicit path has no globals or ptd acquisition");
    f.observe={};f.trace.clear();f.changed=0;out={0x1234,0,0,1};
    l::locale_update_058b04(out,0,f);
    require(out.ptd==0x1234&&out.locale_info==0x6000&&out.multibyte_info==0x7000&&out.clear_thread_flag==0,"global fast path preserves ptd");
    require(f.trace==std::vector<std::string>{"changed","global"},"global path does not acquire ptd");
    // The explicit pair can itself be the output pair: no incorrect preclear.
    out={0xabcd,0x3333,0x4444,1};f.trace.clear();
    f.aliases[0xb000]=&out.locale_info;f.aliases[0xb008]=&out.multibyte_info;
    l::locale_update_058b04(out,0xb000,f);
    require(out.ptd==0xabcd&&out.locale_info==0x3333&&out.multibyte_info==0x4444,"same pair alias");
    f=Fixture{};f.changed=0x80000000u;out={0,0,0,9};
    l::locale_update_058b04(out,0,f);
    require(out.ptd==0x1000&&out.clear_thread_flag==1,"full changed DWORD accepts high-bit nonzero");
}
void helper_truth_tables() {
    for(std::uint32_t flags=0;flags<256;++flags) {
        for(std::uint32_t mask=0;mask<256;++mask) {
            for(bool mb:{false,true}) {
                Fixture f;f.words[0x13a8]=flags;f.mask=mask;l::Address field=0x2222;
                if(mb) l::update_multibyte_info_064f5c(0x1000,field,f);
                else l::update_locale_info_064ef0(0x1000,field,f);
                const bool updates=(flags&mask)==0;
                require(field==(updates?(mb?0x9000:0x8000):0x2222),"helper mask matrix");
                std::vector<std::string> expected{mb?"current-multibyte":"current-locale","read:5032","mask"};
                if(updates) expected.push_back(mb?"update-multibyte":"update-locale");
                require(f.trace==expected,"helper exact read and call order");
                f.trace.clear();field=mb?f.current_multibyte:f.current_locale;
                if(mb) l::update_multibyte_info_064f5c(0x1000,field,f);
                else l::update_locale_info_064ef0(0x1000,field,f);
                require(f.trace==std::vector<std::string>{mb?"current-multibyte":"current-locale"},"same global skips flag and mask reads");
            }
        }
    }
    for(const auto bit:{0x100u,0x10000u,0x80000000u}) {
        Fixture f;f.words[0x13a8]=bit;f.mask=bit;l::Address field=17;
        l::update_locale_info_064ef0(0x1000,field,f);
        require(field==17,"full DWORD mask high bits matter");
        f.mask=0;l::update_locale_info_064ef0(0x1000,field,f);
        require(field==0x8000,"zero mask allows high flag bits");
    }
}
void constructor_matrix() {
    for(std::uint32_t flags=0;flags<256;++flags) {
        for(const auto mask:{0u,1u,2u,0xffu,0x80000000u}) {
            Fixture f;f.mask=mask;f.words[0x13a8]=flags;l::Context out{0xdead,0,0,0xff};
            l::locale_update_058b04(out,0,f);
            const bool updates=(flags&mask)==0;
            require(out.ptd==0x1000,"changed path acquires ptd");
            require(out.locale_info==(updates?0x8000:0x2000)&&out.multibyte_info==(updates?0x9000:0x3000),"constructor invokes both real helpers");
            require(out.clear_thread_flag==((flags&2u)?0:1),"only owner marks flag cleanup");
            require(f.words.at(0x13a8)==(flags|2u),"constructor preserves other thread flags");
            std::vector<std::string> expected{"changed","ptd","address:4240","address:4232","current-locale","read:5032","mask"};
            if(updates) expected.push_back("update-locale");
            expected.insert(expected.end(),{"current-multibyte","read:5032","mask"});
            if(updates) expected.push_back("update-multibyte");
            expected.push_back("read:5032");
            if(!(flags&2u)) expected.push_back("write:5032");
            require(f.trace==expected,"constructor order including final guard reload");
        }
    }
}
void mutations() {
    Fixture f;l::Context out{0,0,0,9};
    f.locale_update=[&](){f.words[0x13a8]=0x80000002u;return l::Address{0x8000};};
    l::locale_update_058b04(out,0,f);
    require(out.locale_info==0x8000&&out.multibyte_info==0x3000,"second helper reloads flags after first updater");
    require(out.clear_thread_flag==0&&f.words[0x13a8]==0x80000002u,"final guard reload sees updater flag and preserves high bit");
    require(f.trace.back()=="read:5032","no guard rewrite if bit already set");

    // The constructor reloads its PTD field for the second helper and guard.
    Fixture g;out={0,0,0,1};g.words[0x23a8]=0x20;
    g.locale_update=[&](){out.ptd=0x2000;return l::Address{0x8000};};
    l::locale_update_058b04(out,0,g);
    require(out.ptd==0x2000&&g.words[0x23a8]==0x22&&g.words[0x13a8]==0,"later helper and flag guard use reloaded context ptd");

    // Nested construction acquires the guard. Outer construction must then
    // leave the already-set flag and its own ownership byte untouched at zero.
    Fixture nested;l::Context inner{7,8,9,9};out={0,0,0,1};
    nested.current_multibyte=0x3000;
    nested.locale_update=[&](){
        nested.addresses[0x1090]=nested.current_locale;
        l::locale_update_058b04(inner,0,nested);
        return nested.current_locale;
    };
    l::locale_update_058b04(out,0,nested);
    require(inner.clear_thread_flag==1&&out.clear_thread_flag==0,"nested constructor owns clear flag");
    require(nested.words[0x13a8]==2,"constructor does not clean guard internally");
    require(out.locale_info==nested.current_locale&&out.multibyte_info==0x3000,"nested updated fields remain usable");

    Fixture zero;zero.locale_update=[](){return l::Address{0};};zero.multibyte_update=[](){return l::Address{0};};
    l::locale_update_058b04(out,0,zero);
    require(out.locale_info==0&&out.multibyte_info==0&&out.clear_thread_flag==1,"zero updater result is stored without invented failure handling");

    Fixture changed_field;l::Address field=17;
    changed_field.observe=[&](const auto& event){if(event=="current-locale") field=changed_field.current_locale;};
    l::update_locale_info_064ef0(0x1000,field,changed_field);
    require(changed_field.trace==std::vector<std::string>{"current-locale"},"helper compares field after global read");
    Fixture changed_flags;field=17;changed_flags.words[0x13a8]=0;
    changed_flags.observe=[&](const auto& event){if(event=="mask") changed_flags.words[0x13a8]=2;};
    l::update_locale_info_064ef0(0x1000,field,changed_flags);
    require(field==0x8000,"helper uses flag snapshot preceding mask read");
}
void interrupted_effects() {
    // Throwing fixtures delimit partial normal-flow writes. They do not model
    // original SEH cleanup, which remains outside this source unit.
    Fixture f;l::Context out{1,2,3,4};f.aliases[0xa000]=&out.ptd;f.aliases[0xa008]=&out.locale_info;
    f.observe=[](const auto& event){if(event=="address:40968") throw std::runtime_error("stop");};
    bool stopped=false;try{l::locale_update_058b04(out,0xa000,f);}catch(const std::runtime_error&){stopped=true;}
    require(stopped&&out.ptd==1&&out.locale_info==2&&out.multibyte_info==3&&out.clear_thread_flag==0,"pair read interruption precedes both pair stores");
    Fixture g;out={1,2,3,4};g.observe=[](const auto& event){if(event=="address:4232") throw std::runtime_error("stop");};
    stopped=false;try{l::locale_update_058b04(out,0,g);}catch(const std::runtime_error&){stopped=true;}
    require(stopped&&out.ptd==0x1000&&out.locale_info==0x2000&&out.multibyte_info==3&&out.clear_thread_flag==0,"changed path stores first field before reading second");
    Fixture h;out={1,2,3,4};h.observe=[](const auto& event){if(event=="write:5032") throw std::runtime_error("stop");};
    stopped=false;try{l::locale_update_058b04(out,0,h);}catch(const std::runtime_error&){stopped=true;}
    require(stopped&&out.ptd==0x1000&&out.locale_info==0x8000&&out.multibyte_info==0x9000&&out.clear_thread_flag==0,"ownership byte is set after guard write");
}
}
int main() {
    try {fast_paths();helper_truth_tables();constructor_matrix();mutations();interrupted_effects();
        std::cout<<"PASS locale update: "<<checks<<" checks\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}
}
