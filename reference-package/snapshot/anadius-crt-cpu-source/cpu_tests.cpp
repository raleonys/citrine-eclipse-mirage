#include "cpu.hpp"
#include <array>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace c=anadius::crt_cpu_recovered;
namespace {
unsigned checks;
void check(bool value){++checks;if(!value)throw std::runtime_error("check "+std::to_string(checks));}
struct Fixture final:c::Effects {
    c::Registers zero{7,0x756e6547,0x6c65746e,0x49656e69},one{},seven{};
    std::uint64_t xcr0=0;std::vector<unsigned> calls;
    std::function<void()> during_seven;
    c::Registers cpuid(std::uint32_t leaf,std::uint32_t subleaf)override{
        check(subleaf==0);calls.push_back(leaf);
        if(leaf==0)return zero;if(leaf==1)return one;
        check(leaf==7);if(during_seven)during_seven();return seven;
    }
    std::uint64_t xgetbv(std::uint32_t index)override{check(index==0);calls.push_back(100);return xcr0;}
};
void model_and_vendor(){
    constexpr std::array<std::uint32_t,6> recognized{0x106c0,0x20660,0x20670,0x30650,0x30660,0x30670};
    for(const auto model:recognized){c::State s;Fixture e;e.one.eax=model;
        check(c::initialize_cpu_04b26c(s,e)==0);check(s.a1558==0x8000&&s.a1560==~std::uint64_t{0});check((s.a4788&1)==1);
    }
    for(unsigned model=0x30640;model<=0x30680;model+=0x10){c::State s;Fixture e;e.one.eax=model;
        (void)c::initialize_cpu_04b26c(s,e);check((s.a4788&1)==static_cast<unsigned>(model>=0x30650&&model<=0x30670));
    }
    for(unsigned differing=0;differing<3;++differing){c::State s;s.a1558=0x123;s.a1560=0x456;s.a4788=0x80000000;
        Fixture e;if(differing==0)e.zero.ebx^=1;if(differing==1)e.zero.edx^=1;if(differing==2)e.zero.ecx^=1;
        e.one.eax=0x30650;e.seven.ebx=1U<<9;(void)c::initialize_cpu_04b26c(s,e);
        check(s.a1558==0x123&&s.a1560==0x456&&s.a4788==0x80000002);
    }
    for(std::uint32_t max_leaf:{0U,6U,7U,0x7fffffffU,0x80000007U,0xffffffffU}){
        c::State s;Fixture e;e.zero.eax=max_leaf;(void)c::initialize_cpu_04b26c(s,e);
        const bool queried=max_leaf>=7&&max_leaf<0x80000000;
        check(e.calls==(queried?std::vector<unsigned>{0,1,7}:std::vector<unsigned>{0,1}));
    }
}
void feature_matrix(){
    struct Scenario{std::uint32_t leaf1,leaf7;unsigned base_level,base_bits;};
    const auto features=(1U<<20)|(1U<<27)|(1U<<28);
    const std::array<Scenario,6> scenarios{{
        {0,0,1,2},{1U<<20,0,2,6},{features,0,2,6},
        {features,0x20,2,6},{features,0xd0030020U,2,6},{features,0xd0030000U,2,6}}};
    for(unsigned n=0;n<scenarios.size();++n)for(unsigned xcr=0;xcr<256;++xcr){
        const auto scenario=scenarios[n];c::State s;Fixture e;e.one.ecx=scenario.leaf1;e.seven.ebx=scenario.leaf7;e.xcr0=xcr;
        (void)c::initialize_cpu_04b26c(s,e);
        unsigned expected=scenario.base_level,bits=scenario.base_bits;
        if(n>=2&&(xcr&6)==6){expected=3;bits=14;if(n==3||n==4){expected=5;bits=46;if(n==4&&(xcr&0xe0)==0xe0){expected=6;bits=110;}}}
        check(s.a1550==expected&&s.a1554==bits);
        check(e.calls.back()==(n>=2?100U:7U));
    }
    // Removing either guard bit prevents XGETBV; high32 state bits alone do not enable its low-bit tests.
    for(std::uint32_t missing:{1U<<20,1U<<27,1U<<28}){
        c::State s;Fixture e;e.one.ecx=features&~missing;e.xcr0=~std::uint64_t{0};
        (void)c::initialize_cpu_04b26c(s,e);check(e.calls.back()==7);check(s.a1550==((missing==(1U<<20))?1U:2U));
    }
    c::State high;Fixture e;e.one.ecx=features;e.seven.ebx=0xd0030020U;e.xcr0=0xe600000000ULL;
    (void)c::initialize_cpu_04b26c(high,e);check(high.a1550==2&&high.a1554==6);
    c::State s;s.a4788=0x40;Fixture mutation;mutation.seven.ebx=1U<<9;mutation.during_seven=[&]{s.a4788=0x8000;};
    (void)c::initialize_cpu_04b26c(s,mutation);check(s.a4788==0x42);
}
}
int main(){try{model_and_vendor();feature_matrix();std::cout<<"PASS "<<checks<<" synthetic CPU-dispatch checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
