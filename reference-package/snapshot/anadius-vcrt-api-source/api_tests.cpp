#include "api.hpp"
#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace anadius::vcrt_api_recovered;
namespace {
unsigned checks{};
void check(bool condition) { ++checks; if (!condition) throw std::runtime_error("check " + std::to_string(checks)); }
void callback(void*) {}
struct Load { std::u16string name; Module file; std::uint32_t flags; };
struct Fake : Effects {
    std::vector<Load> loads;
    std::vector<Module> frees, queries;
    std::vector<std::string> names;
    std::vector<Module> results{0x1234};
    std::size_t cursor{};
    std::uint32_t error{126}, errors{};
    Address result{0x5678}, called{};
    std::string method;
    FlsCallback received_callback{};
    std::uint32_t index{}, spin{}, flags{};
    void* pointer{};
    std::function<void()> on_load, on_query, on_free;
    Module load_library_ex_w(std::u16string_view name, Module file, std::uint32_t flags_) override {
        loads.push_back({std::u16string(name),file,flags_});
        if (on_load) on_load();
        return cursor < results.size() ? results[cursor++] : 0;
    }
    std::uint32_t last_error() override { ++errors; return error; }
    bool free_library(Module module) override { frees.push_back(module); if(on_free) on_free(); return false; }
    Address get_proc_address(Module module, std::string_view name) override {
        queries.push_back(module); names.emplace_back(name); if(on_query) on_query(); return result;
    }
    std::uint32_t call_fls_alloc(Address a, FlsCallback cb) override { called=a; method="fls_alloc";received_callback=cb;return 0xffffffff; }
    std::int32_t call_fls_free(Address a,std::uint32_t i) override {called=a;method="fls_free";index=i;return -37;}
    void* call_fls_get_value(Address a,std::uint32_t i) override {called=a;method="fls_get";index=i;return &result;}
    std::int32_t call_fls_set_value(Address a,std::uint32_t i,void* p) override {called=a;method="fls_set";index=i;pointer=p;return 0x12345678;}
    std::int32_t call_initialize_critical_section_ex(Address a,void* p,std::uint32_t s,std::uint32_t f) override {
        called=a;method="cs_ex";pointer=p;spin=s;flags=f;return -71;
    }
    std::uint32_t tls_alloc() override {method="tls_alloc";return 0xfffffffe;}
    std::int32_t tls_free(std::uint32_t i) override {method="tls_free";index=i;return -99;}
    void* tls_get_value(std::uint32_t i) override {method="tls_get";index=i;return &error;}
    std::int32_t tls_set_value(std::uint32_t i,void* p) override {method="tls_set";index=i;pointer=p;return -101;}
    std::int32_t initialize_critical_section_and_spin_count(void* p,std::uint32_t s) override {
        method="cs_spin";pointer=p;spin=s;return 0x13579;
    }
};
constexpr std::array<std::uint32_t,2> candidates{0,2};
void resolver_tests() {
    {
        State s; Fake f;
        check(s.modules.size()==3 && s.procedures.size()==5);
        check(s.module_names[0]==u"api-ms-win-core-fibers-l1-1-1");
        check(s.module_names[1]==u"api-ms-win-core-synch-l1-2-0");
        check(s.module_names[2]==u"kernel32");
        s.procedures[0]=0x88776655;
        check(resolve_0509bc(s,f,0,"cached",candidates)==0x88776655);
        check(f.loads.empty() && f.queries.empty() && f.errors==0);
        s.procedures[0]=missing;
        check(resolve_0509bc(s,f,0,"cached",candidates)==0);
        check(f.loads.empty() && f.queries.empty());
    }
    {
        State s; Fake f;
        check(resolve_0509bc(s,f,1,"test",candidates)==f.result);
        check(f.loads.size()==1 && f.loads[0].flags==0x800 && f.loads[0].file==0);
        check(f.loads[0].name==s.module_names[0]);
        check(f.queries==std::vector<Module>{0x1234} && f.names==std::vector<std::string>{"test"});
        check(s.modules[0]==0x1234 && s.procedures[1]==f.result && f.errors==0);
        check(resolve_0509bc(s,f,1,"unused",candidates)==f.result);
        check(f.loads.size()==1 && f.queries.size()==1);
        s.procedures[1]=0;
        check(resolve_0509bc(s,f,1,"again",candidates)==f.result);
        check(f.loads.size()==1 && f.queries.size()==2);
    }
    {
        State s; Fake f; f.result=0;
        check(resolve_0509bc(s,f,0,"absent",candidates)==0);
        check(f.loads.size()==1 && f.queries.size()==1); // no kernel32 retry
        check(s.procedures[0]==missing && s.modules[0]==0x1234 && s.modules[2]==0);
    }
    {
        State s; Fake f; f.results={0,0x9999};
        check(resolve_0509bc(s,f,0,"fallback",candidates)==f.result);
        check(s.modules[0]==missing && s.modules[2]==0x9999);
        check(f.loads.size()==2 && f.loads[1].name==u"kernel32");
        check(f.loads[0].flags==0x800 && f.loads[1].flags==0x800 && f.errors==1);
    }
    for(auto name : {u"api-ms-case",u"API-MS-case",u"ext-ms-case",u"kernel32",u"api-m",u""}) {
        for(auto error : {87u,126u,0u}) {
            State s; Fake f; s.module_names[0]=name; f.results={0,0x9999};f.error=error;
            const std::array<std::uint32_t,1> one{0};
            const bool retry=error==87 && !s.module_names[0].starts_with(u"api-ms-");
            check(resolve_0509bc(s,f,0,"x",one)==(retry ? f.result:0));
            check(f.loads.size()==(retry?2u:1u) && f.errors==1);
            check(f.loads[0].flags==0x800);
            if(retry) check(f.loads[1].flags==0 && f.loads[1].file==0 && f.loads[1].name==name);
            check(s.modules[0]==(retry?0x9999:missing));
        }
    }
    {
        State s;Fake f;f.results={0,0};f.error=87;
        check(resolve_0509bc(s,f,0,"all-missing",candidates)==0);
        check(f.loads.size()==3 && f.errors==2 && f.queries.empty());
        check(s.modules[0]==missing && s.modules[2]==missing && s.procedures[0]==missing);
        s.procedures[0]=0;
        check(resolve_0509bc(s,f,0,"cached-modules",candidates)==0);
        check(f.loads.size()==3 && f.errors==2);
    }
    {
        State s;Fake f;const std::span<const std::uint32_t> empty;
        check(resolve_0509bc(s,f,0,"none",empty)==0);
        check(s.procedures[0]==missing && f.loads.empty());
    }
    for(Module existing : {Module{0x4444}, missing}) {
        State s;Fake f;
        f.on_load=[&]{s.modules[0]=existing;};
        f.on_free=[&]{check(s.modules[0]==0x1234);};
        check(resolve_0509bc(s,f,0,"racing",candidates)==f.result);
        check(f.frees==std::vector<Module>{0x1234});
        check(f.queries==std::vector<Module>{0x1234}); // same local after free
        check(s.modules[0]==0x1234);
    }
    {
        State s;Fake f;f.results={0};
        f.on_load=[&]{s.modules[0]=0x7777;};
        const std::array<std::uint32_t,1> one{0};
        check(resolve_0509bc(s,f,0,"racing-failure",one)==0);
        check(s.modules[0]==missing && f.frees.empty()); // unconditional XCHG
    }
    {
        State s;Fake f;f.on_query=[&]{s.procedures[0]=0x9999;};
        check(resolve_0509bc(s,f,0,"racing-procedure",candidates)==f.result);
        check(s.procedures[0]==f.result);
    }
    {
        State s;Fake f;s.modules[0]=0x3333;
        f.on_query=[] {throw std::runtime_error("provider exception");};
        bool caught=false;try{(void)resolve_0509bc(s,f,0,"throw",candidates);}catch(...){caught=true;}
        check(caught && s.modules[0]==0x3333 && s.procedures[0]==0);
    }
}
void wrapper_tests() {
    for(bool available : {false,true}) {
        State s;Fake f;
        for(auto& p:s.procedures)p=available?0x87654321:missing;
        check(fls_alloc_050b0c(s,f,callback)==(available?0xffffffff:0xfffffffe));
        check(f.method==(available?"fls_alloc":"tls_alloc"));
        check(f.received_callback==(available?callback:nullptr));
        check(fls_free_050b54(s,f,0xffffffff)==(available?-37:-99));
        check(f.index==0xffffffff && f.method==(available?"fls_free":"tls_free"));
        check(fls_get_value_050b9c(s,f,0xfedcba98)==(available?static_cast<void*>(&f.result):static_cast<void*>(&f.error)));
        check(f.index==0xfedcba98 && f.method==(available?"fls_get":"tls_get"));
        check(fls_set_value_050be4(s,f,0xdeadbeef,&s)==(available?0x12345678:-101));
        check(f.index==0xdeadbeef && f.pointer==&s && f.method==(available?"fls_set":"tls_set"));
        check(initialize_critical_section_ex_050c38(s,f,&s,0xfedcba98,0x89abcdef)==(available?-71:0x13579));
        check(f.pointer==&s && f.spin==0xfedcba98 && f.method==(available?"cs_ex":"cs_spin"));
        check(f.flags==(available?0x89abcdef:0));
        check(f.loads.empty() && f.queries.empty());
        if(available)check(f.called==0x87654321);
    }
    const std::array<std::string_view,5> names{"FlsAlloc","FlsFree","FlsGetValue","FlsSetValue","InitializeCriticalSectionEx"};
    for(unsigned i=0;i<5;++i) {
        State s;Fake f;
        switch(i){case 0:(void)fls_alloc_050b0c(s,f,nullptr);break;case 1:(void)fls_free_050b54(s,f,0);break;
        case 2:(void)fls_get_value_050b9c(s,f,0);break;case 3:(void)fls_set_value_050be4(s,f,0,nullptr);break;
        case 4:(void)initialize_critical_section_ex_050c38(s,f,nullptr,0,0);break;}
        check(f.names.size()==1 && f.names[0]==names[i]);
        check(f.loads[0].name==s.module_names[i==4?1:0]);
        check(s.procedures[i]==f.result);
        for(unsigned j=0;j<5;++j)if(j!=i)check(s.procedures[j]==0);
    }
}
}
int main(){try{resolver_tests();wrapper_tests();std::cout<<checks<<" VCRT API fallback checks passed\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return EXIT_FAILURE;}}
