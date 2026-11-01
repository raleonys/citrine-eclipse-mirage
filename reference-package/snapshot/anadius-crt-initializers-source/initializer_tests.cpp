#include "initializers.hpp"
#include "../anadius-crt-cache-source/cache.hpp"
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
namespace q=anadius::crt_initializers_recovered;
namespace p=anadius::crt_ptd_recovered;
namespace en=anadius::crt_entry_recovered;
namespace ox=anadius::crt_onexit_recovered;
namespace ca=anadius::crt_cache_recovered;
namespace rt=anadius::crt_runtime_recovered;
namespace {
unsigned checks;
void check(bool value){++checks;if(!value)throw std::runtime_error("check "+std::to_string(checks));}
struct Sequencer:q::Effects {
    std::vector<std::string> events;
    std::map<q::Address,std::uint64_t> results;
    std::function<void(q::Address)> on_init,on_cleanup,on_void;
    std::uint64_t invoke_boolean_initializer(q::Address a) override{
        events.push_back("init"+std::to_string(a));if(on_init)on_init(a);return results.contains(a)?results[a]:1;
    }
    void invoke_uninitializer(q::Address a,std::uint32_t argument) override{
        check(argument==0);events.push_back("cleanup"+std::to_string(a));if(on_cleanup)on_cleanup(a);
    }
    void invoke_void_initializer(q::Address a) override{events.push_back("void"+std::to_string(a));if(on_void)on_void(a);}
    std::uint64_t invoke_int_initializer(q::Address a) override{events.push_back("int"+std::to_string(a));return results[a];}
    std::uint32_t common_flush_all(bool value) override{check(value);events.emplace_back("flush");return 0x80000000;}
};
void pair_semantics(){
    std::array<q::Pair,4> pairs{{{1,10},{0,20},{3,0},{4,40}}};Sequencer e;e.results[4]=0x100;
    check(!q::initialize_pairs_0704f4(pairs,e));
    check(e.events==std::vector<std::string>({"init1","init3","init4","cleanup10"}));
    e.events.clear();check(q::uninitialize_pairs_070588(pairs,e));
    check(e.events==std::vector<std::string>({"cleanup40","cleanup20","cleanup10"}));
    e.events.clear();e.on_init=[&](q::Address a){if(a==4){pairs[0].initialize=0;pairs[2].uninitialize=77;}};
    check(!q::initialize_pairs_0704f4(pairs,e));
    check(e.events==std::vector<std::string>({"init1","init3","init4","cleanup77"}));
    e.events.clear();e.on_cleanup=[&](q::Address a){if(a==40)pairs[0].uninitialize=99;};
    check(q::uninitialize_pairs_070588(pairs,e));
    check(e.events==std::vector<std::string>({"cleanup40","cleanup77","cleanup20","cleanup99"}));
    // All nonzero AL values succeed irrespective of high bits; AL zero fails.
    for(unsigned al=0;al<256;++al){
        Sequencer f;std::array<q::Pair,2> two{{{1,11},{2,22}}};f.results[1]=0x123456789abcde00ULL|al;
        const bool success=q::initialize_pairs_0704f4(two,f);
        check(success==(al!=0));check(f.events.size()==(al?2U:1U));
    }
    for(unsigned failure=0;failure<8;++failure){
        std::array<q::Pair,8> eight{};Sequencer f;
        for(unsigned k=0;k<eight.size();++k)eight[k]={k+1,100+k};
        f.results[failure+1]=0;
        check(!q::initialize_pairs_0704f4(eight,f));
        check(f.events.size()==failure+1+failure);
        if(failure)check(f.events.back()=="cleanup100");
    }
    Sequencer empty;check(q::initialize_pairs_0704f4({},empty));check(q::uninitialize_pairs_070588({},empty));check(empty.events.empty());
    Sequencer throwing;std::array<q::Pair,2> two{{{1,11},{2,22}}};
    struct DependencyException{};
    throwing.on_init=[](q::Address a){if(a==2)throw DependencyException{};};
    bool propagated=false;
    try{(void)q::initialize_pairs_0704f4(two,throwing);}catch(const DependencyException&){propagated=true;}
    check(propagated);check(throwing.events==std::vector<std::string>({"init1","init2"}));
}
void c_walkers_and_shutdown(){
    Sequencer e;std::array<q::Address,4> entries{0,1,2,3};
    e.on_void=[&](q::Address a){if(a==1)entries[2]=77;};
    q::initterm_064614(entries,e);check(e.events==std::vector<std::string>({"void1","void77","void3"}));
    entries={0,1,2,3};e.events.clear();e.results[1]=0x100000000ULL;e.results[2]=0xf1234567;
    check(q::initterm_e_064658(entries,e)==0xf1234567);check(e.events==std::vector<std::string>({"int1","int2"}));
    e.events.clear();e.results[2]=0;e.results[3]=0xfeed00000000ULL;
    check(q::initterm_e_064658(entries,e)==0);check(e.events.size()==3);
    std::array<q::Pair,2> pairs{{{1,11},{0,22}}};q::State state;
    for(std::uint64_t flag:{0ULL,1ULL,0x80000000ULL,0x100000000ULL,0x8000000000000000ULL}){
        state.flush_enabled_0a4ef0=flag;e.events.clear();
        check(q::acrt_uninitialize_0645cc(true,state,pairs,e));check(e.events.size()==static_cast<unsigned>(flag!=0));
        e.events.clear();check(q::acrt_uninitialize_0645cc(false,state,pairs,e));
        check(e.events==std::vector<std::string>({"cleanup22","cleanup11"}));
    }
}
struct Ptd final:p::Effects {
    p::Ptd storage; p::Ptd* slot=nullptr;std::uint32_t error=0x12345678;unsigned freed=0,uninit=0;
    p::Ptd* fls_get_value(std::uint32_t)override{return slot;}
    bool fls_set_value(std::uint32_t,p::Ptd* v)override{slot=v;return true;}
    std::uint32_t fls_alloc(p::FlsCallback)override{return 7;}
    bool fls_free(std::uint32_t)override{++uninit;slot=nullptr;return true;}
    std::uint32_t last_error()override{return error;}
    void set_last_error(std::uint32_t v)override{error=v;}
    p::Ptd* calloc_ptd_066848()override{return &storage;}
    void free_0668c0(p::Ptd* v)override{if(v){check(v==&storage);++freed;}}
    void construct_ptd_array_0648fc(p::Ptd* v)override{check(v==&storage);}
    void destroy_ptd_array_0649ec(p::Ptd* v)override{check(v==&storage);}
    [[noreturn]]void abort_05f03c()override{throw std::runtime_error("abort");}
};
struct Onexit final:ox::Effects {
    std::map<ox::Address,std::vector<std::uint64_t>> blocks;
    ox::Address next=0x100000;std::vector<ox::Address> calls;unsigned depth=0;
    void lock_0625f8(std::uint32_t id)override{check(id==2);++depth;}
    void unlock_06264c(std::uint32_t id)override{check(id==2&&depth>0);--depth;}
    ox::Address recalloc_070434(ox::Address old,std::uint64_t count,std::uint64_t size)override{
        check(size==8);std::vector<std::uint64_t> data(static_cast<std::size_t>(count));
        if(old){auto before=blocks.at(old);for(std::size_t k=0;k<before.size()&&k<data.size();++k)data[k]=before[k];blocks.erase(old);}
        const auto address=next;next+=0x100000;blocks.emplace(address,std::move(data));return address;
    }
    void free_0668c0(ox::Address a)override{if(a)check(blocks.erase(a)==1);}
    std::uint64_t& word(ox::Address a){auto it=blocks.upper_bound(a);check(it!=blocks.begin());--it;const auto index=(a-it->first)/8;check((a-it->first)%8==0&&index<it->second.size());return it->second[static_cast<std::size_t>(index)];}
    std::uint64_t read_word(ox::Address a)override{return word(a);}
    void write_word(ox::Address a,std::uint64_t v)override{word(a)=v;}
    void invoke_callback(ox::Address a)override{check(depth>0);calls.push_back(a);}
};
struct Entropy final:ca::EntropyEffects {
    void system_time_as_file_time(std::uint64_t&)override{check(false);}
    std::uint32_t current_thread_id()override{check(false);return 0;}
    std::uint32_t current_process_id()override{check(false);return 0;}
    ca::CounterObservation query_performance_counter()override{check(false);return{false,{}};}
    std::uintptr_t entropy_stack_cell_address()override{check(false);return 0;}
};
struct Outer final:q::AcrtTablesBridge {
    ox::State& onexit;Onexit& exit_effects;ca::CookieState cookie{0x1234,0};Entropy entropy;
    std::vector<std::uint32_t> app_reasons;bool app_failure=false,terminated=false;
    std::uintptr_t tls_slot=0;
    Outer(q::State& s,std::span<q::Pair> pairs,std::span<q::Address> c,std::span<q::Address> cpp,Sequencer& seq,p::State& ptd,Ptd& fx,ox::State& exits,Onexit& ex)
        :AcrtTablesBridge(s,pairs,c,cpp,seq,ptd,fx),onexit(exits),exit_effects(ex){}
    void security_init_cookie_04c134()override{ca::initialize_cookie_04c134(cookie,entropy);}
    void initialize_cpu_04b26c()override{}
    std::uint64_t vcrt_initialize_04d55c()override{return 1;}
    std::uint64_t vcrt_uninitialize_04d5a8(bool)override{return 1;}
    std::uint64_t vcrt_thread_attach_04d584()override{return 1;}
    void vcrt_thread_detach_04d598()override{}
    void initialize_slist_04c1e0()override{}
    void configure_stdio_04c1fc()override{}
    std::uint32_t configure_narrow_argv_0638e8(std::uint32_t mode)override{check(mode==1);return 0;}
    void initialize_narrow_environment_064054()override{}
    std::uint32_t c_termination_complete_05cdb8()override{return terminated;}
    void cexit_05cd9c()override{check(ox::execute_onexit_table_0643b4(&onexit.crt_0a52a8,onexit,exit_effects)==0);terminated=true;}
    void release_slist_04c1f0()override{}
    void vcrt_late_cleanup_04d5c4()override{}
    const std::uintptr_t* tls_callback_slot_04c218()override{return &tls_slot;}
    std::uint64_t slot_in_nonwritable_image_04b64c(const std::uintptr_t*)override{check(false);return 0;}
    void invoke_tls_callback(std::uintptr_t,std::uintptr_t,std::uint32_t,std::uintptr_t)override{check(false);}
    std::uint32_t application_dll_main_038870(std::uintptr_t,std::uint32_t reason,std::uintptr_t)override{
        app_reasons.push_back(reason);
        if(reason==1){check(ox::atexit_04b770(100,onexit,exit_effects)==0);check(ox::atexit_04b770(200,onexit,exit_effects)==0);}
        return reason==1&&app_failure?0:0x80000000;
    }
    [[noreturn]]void fastfail_04bfec(std::uint32_t)override{throw std::runtime_error("fastfail");}
};
void combined_entry_and_exit(){
    for(bool app_failure:{false,true}){
        Sequencer seq;Ptd ptd_effects;p::State ptd;ptd.fls_index=7;
        q::State acrt;rt::State cache(0x1234);ca::EncodedGlobals globals;
        std::array<q::Pair,2> pairs{{{1,11},{2,22}}};
        seq.on_init=[&](q::Address a){if(a==1)(void)ca::initialize_thunks_067538(cache);else check(ca::initialize_encoded_globals_0644b0(globals,cache.cookie));};
        std::array<q::Address,2> c{0,30},cpp{40,0};
        ox::State exits{cache.cookie,{0,0,0},{0,0,0}};Onexit onexit;
        check(ox::initialize_onexit_table_0643f8(&exits.crt_0a52a8,exits)==0);
        en::State state(exits.application_0a47a8);
        Outer effects(acrt,pairs,c,cpp,seq,ptd,ptd_effects,exits,onexit);effects.app_failure=app_failure;
        en::Entry entry(state,effects);
        const auto attached=entry.entry_04bcfc(0x123,1,0);
        check(attached==(app_failure?0U:0x80000000U));
        check(effects.cookie.complement==~effects.cookie.cookie);
        check(&state.onexit_table==&exits.application_0a47a8);
        check(globals.a4f00==cache.cookie&&globals.a5178==cache.cookie);
        for(const auto& cell:cache.procedures)check(cell==cache.cookie);
        if(app_failure){check(state.startup_state==0&&state.attach_count_bits==0);check(effects.app_reasons==std::vector<std::uint32_t>({1,0}));}
        else{
            check(state.startup_state==2&&state.attach_count_bits==1);
            check(entry.dispatch_04bbc8(0x123,2,0)==0x80000000);
            check(ptd_effects.slot==&ptd_effects.storage);
            check(entry.dispatch_04bbc8(0x123,3,0)==1);check(ptd_effects.freed==1);
            check(entry.dispatch_04bbc8(0x123,0,0)==1);check(state.startup_state==0&&state.attach_count_bits==0);
        }
        check(onexit.calls==std::vector<ox::Address>({200,100}));check(onexit.blocks.empty()&&onexit.depth==0);
        check(ptd.fls_index==p::invalid_fls_index&&ptd_effects.uninit==1);
        check(seq.events[0]=="init1"&&seq.events[1]=="init2");
        check(seq.events[seq.events.size()-2]=="cleanup22"&&seq.events.back()=="cleanup11");
    }
}
}
int main(){try{pair_semantics();c_walkers_and_shutdown();combined_entry_and_exit();std::cout<<"PASS "<<checks<<" initializer/outer-CRT/onexit composition checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
