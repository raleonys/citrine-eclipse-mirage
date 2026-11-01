#pragma once
// Shared synthetic startup fixtures; no native environment or target execution.
#include "startup_bridge.hpp"
#include "../anadius-vcrt-bridge-source/test_fixtures.hpp"
#include "../anadius-crt-cache-source/cache.hpp"
#include <algorithm>
#include <span>
namespace sb=anadius::crt_startup_bridge;
namespace su=anadius::crt_support_recovered;
namespace ev=anadius::crt_environment_recovered;
namespace tmn=anadius::crt_termination_recovered;
namespace ox=anadius::crt_onexit_recovered;
namespace f=anadius::vcrt_test;
namespace anadius::crt_startup_test {
using f::check;
struct Lists final:su::Effects {
    unsigned initialized=0,flushed=0;su::Address first=0;
    std::map<su::Address,su::Address> nodes;std::vector<su::Address> freed;
    void initialize_slist_head(su::Head head)override{check(head.token==7);++initialized;first=0;}
    su::Address interlocked_flush_slist(su::Head head)override{check(head.token==7);++flushed;const auto result=first;first=0;return result;}
    su::Address next(su::Address value)override{check(nodes.contains(value));return nodes.at(value);}
    void free_05caec(su::Address value)override{check(nodes.erase(value)==1);freed.push_back(value);}
};
struct Environment final:ev::Effects {
    static constexpr char16_t raw_block[]=u"A=1\0=C:=hidden\0D=two\0";
    const std::u16string block{raw_block,std::size(raw_block)};
    struct Allocation{std::size_t count;bool pointers;};
    std::map<void*,Allocation> allocations;
    bool fail_query=false;unsigned multibyte=0,wide_releases=0;
    ~Environment(){for(auto [value,a]:allocations){if(a.pointers)delete[] static_cast<char**>(value);else delete[] static_cast<char*>(value);}}
    void initialize_multibyte_06f5f8()override{++multibyte;}
    std::span<const char16_t> get_environment_strings_w()override{return block;}
    void free_environment_strings_w(std::span<const char16_t> source)override{check(source.data()==block.data());++wide_releases;}
    std::int32_t convert_06cd58(std::span<const char16_t> source,std::int32_t count,ev::Buffer target,std::int32_t capacity)override{
        check(source.data()==block.data()&&count==22);
        if(fail_query)return 0;
        if(!target.data){check(capacity==0);return count;}
        check(capacity==count&&target.capacity>=static_cast<std::size_t>(count));
        for(std::int32_t i=0;i<count;++i)target.data[i]=static_cast<char>(source[static_cast<std::size_t>(i)]);
        return count;
    }
    ev::Buffer malloc_06a308(std::uint64_t bytes)override{check(bytes<=1024);auto* value=new char[static_cast<std::size_t>(bytes)]{};allocations[value]={static_cast<std::size_t>(bytes),false};return{value,static_cast<std::size_t>(bytes)};}
    ev::Environment calloc_pointers_066848(std::uint64_t count,std::uint64_t size)override{check(size==8&&count<=64);auto* value=new char*[static_cast<std::size_t>(count)]{};allocations[value]={static_cast<std::size_t>(count),true};return{value,static_cast<std::size_t>(count)};}
    ev::Buffer calloc_chars_066848(std::uint64_t count,std::uint64_t size)override{check(size==1);return malloc_06a308(count);}
    void free_0668c0(void* value)override{if(!value)return;check(allocations.contains(value));const auto a=allocations.at(value);allocations.erase(value);if(a.pointers)delete[] static_cast<char**>(value);else delete[] static_cast<char*>(value);}
    std::int32_t strcpy_s_05cff0(ev::Buffer destination,std::span<const char> source)override{check(source.size()==destination.capacity&&source.back()==0);std::memcpy(destination.data,source.data(),source.size());return 0;}
    [[noreturn]]void invoke_watson_05c9a4()override{throw std::runtime_error("unexpected Watson");}
};
struct Ranges final:q::Effects {
    std::array<q::Address,4>& first;std::vector<q::Address> callbacks;
    explicit Ranges(std::array<q::Address,4>& values):first(values){}
    std::uint64_t invoke_boolean_initializer(q::Address)override{throw std::runtime_error("boolean terminator");}
    void invoke_uninitializer(q::Address,std::uint32_t)override{throw std::runtime_error("uninitializer terminator");}
    void invoke_void_initializer(q::Address address)override{check(address==0x301||address==0x304||address==0x303);callbacks.push_back(address);if(address==0x301)first[2]=0x304;}
    std::uint64_t invoke_int_initializer(q::Address)override{throw std::runtime_error("int terminator");}
    std::uint32_t common_flush_all(bool)override{throw std::runtime_error("flush terminator");}
};
struct Termination final:sb::TerminationTablesBridge {
    f::Platform& platform;tmn::State& state;
    std::map<ox::Address,std::vector<std::uint64_t>> buffers;ox::Address serial=0x100000;
    std::vector<ox::Address> callbacks;
    Termination(f::Platform& p,tmn::State& s,Ranges& fx,std::span<q::Address> first,std::span<q::Address> second)
        :TerminationTablesBridge(fx,first,second),platform(p),state(s){}
    void lock_0625f8(std::uint32_t index)override{check(index==2);platform.enter({l::Bank::acrt,index});}
    void unlock_06264c(std::uint32_t index)override{check(index==2);platform.leave({l::Bank::acrt,index});}
    ox::Address recalloc_070434(ox::Address old,std::uint64_t count,std::uint64_t width)override{
        check(width==8&&count<=1024);const auto address=serial;serial+=0x10000;
        auto& buffer=buffers[address];buffer.resize(static_cast<std::size_t>(count));
        if(old){check(buffers.contains(old));const auto& previous=buffers.at(old);std::copy_n(previous.begin(),std::min(previous.size(),buffer.size()),buffer.begin());buffers.erase(old);}
        return address;
    }
    void free_0668c0(ox::Address address)override{if(address)check(buffers.erase(address)==1);}
    std::uint64_t& word(ox::Address address){for(auto& [begin,words]:buffers)if(address>=begin&&address-begin<words.size()*8&&(address-begin)%8==0)return words[static_cast<std::size_t>((address-begin)/8)];throw std::runtime_error("onexit fixture bounds");}
    std::uint64_t read_word(ox::Address address)override{return word(address);}
    void write_word(ox::Address address,std::uint64_t value)override{word(address)=value;}
    void invoke_callback(ox::Address address)override{check(address==0x101||address==0x102);callbacks.push_back(address);}
    void invoke_termination_callback(tmn::Address address,std::uint32_t x,std::uint32_t y,std::uint32_t z)override{check(address==0x40&&!x&&!y&&!z);callbacks.push_back(address);}
    tmn::Address get_module_handle_w_null()override{throw std::runtime_error("native module query on cexit");}
    std::uint16_t read_image_u16(tmn::Address)override{throw std::runtime_error("native image query on cexit");}
    std::uint32_t read_image_u32(tmn::Address)override{throw std::runtime_error("native image query on cexit");}
    std::int32_t get_module_handle_ex_w(std::uint32_t,std::u16string_view,tmn::Address&)override{throw std::runtime_error("module load on cexit");}
    tmn::Address get_proc_address(tmn::Address,std::string_view)override{throw std::runtime_error("module proc on cexit");}
    void invoke_cor_exit_process(tmn::Address,std::uint32_t)override{throw std::runtime_error("managed exit on cexit");}
    bool free_library(tmn::Address)override{throw std::runtime_error("module free on cexit");}
    std::uint32_t policy_06c110()override{throw std::runtime_error("native policy on cexit");}
    std::uint32_t policy_06c09c()override{throw std::runtime_error("native policy on cexit");}
    tmn::Address get_current_process()override{throw std::runtime_error("process query on cexit");}
    std::int32_t terminate_process(tmn::Address,std::uint32_t)override{throw std::runtime_error("process termination on cexit");}
    [[noreturn]]void exit_process(std::uint32_t)override{throw std::runtime_error("process exit on cexit");}
};
struct Outer final:sb::StartupBridge {
    su::State& support;std::vector<std::uint32_t> reasons;bool mutate_tls;
    unsigned validation_calls=0,tls_calls=0;
    Outer(q::State& state,std::span<q::Pair> pairs,f::Sequencer& seq,p::State& ps,p::Effects& pf,b::Dependencies vdeps,sb::Dependencies deps,bool mutate)
        :StartupBridge(state,pairs,{},{},seq,ps,pf,vdeps,deps),support(deps.support),mutate_tls(mutate){}
    void security_init_cookie_04c134()override{} // selected fixture cookie already initialized
    std::uint32_t configure_narrow_argv_0638e8(std::uint32_t mode)override{check(mode==1);return 0;}
    std::uint64_t slot_in_nonwritable_image_04b64c(const std::uintptr_t* slot)override{++validation_calls;check(slot==&support.tls_callback_0a5a70);if(mutate_tls)support.tls_callback_0a5a70=0x202;return 0x101;}
    void invoke_tls_callback(std::uintptr_t callback,std::uintptr_t module,std::uint32_t reason,std::uintptr_t reserved)override{++tls_calls;check(callback==(mutate_tls?0x202:0x201)&&module==0x123&&reason==2&&!reserved);}
    std::uint32_t application_dll_main_038870(std::uintptr_t,std::uint32_t reason,std::uintptr_t)override{reasons.push_back(reason);return 0x100;}
    [[noreturn]]void fastfail_04bfec(std::uint32_t)override{throw std::runtime_error("unexpected fastfail");}
};
inline void combined(bool present,bool env_failure,bool mutate_tls){
    f::System system;f::active_system=&system;f::Platform platform;f::Api api_effects(system,platform,present);a::State api;
    r::State runtime(0x1234);f::Runtime runtime_effects(present);l::State locks;
    b::LockEffects lock_effects(runtime,runtime_effects,platform,api,api_effects,platform);
    f::Memory memory;h::State heap{0xaaa,0,0,0};f::Policy policy(system);
    v::Ptd static_ptd;v::State vstate(static_ptd);
    b::PtdEffects vfx(api,api_effects,f::vcrt_callback,heap,memory,locks,lock_effects,policy);
    f::callback_state=&vstate;f::callback_effects=&vfx;
    f::Data data(heap,memory,lock_effects);f::callback_data=&data;f::callback_ids=&data.ids;
    g::FlsApi direct{f::direct_get,f::direct_set,f::direct_alloc,f::direct_free,f::direct_error,f::direct_set_error};
    g::PtdEffects pfx(direct,f::acrt_callback,heap,memory,data.ids,data,policy);p::State pstate;
    c::State cpu{1,2,0x80000,0x2000000,0};f::Cpu cpu_effects;q::State qstate;
    std::array<q::Pair,2> pairs{{{1,11},{2,12}}};f::Sequencer seq(locks,lock_effects,pstate,pfx);
    su::State support{0x8000000000000000ULL,0x4000000000000000ULL,0x201};Lists lists;
    ev::State env;Environment environment;environment.fail_query=env_failure;
    ox::State tables{runtime.cookie,{},{}};
    check(ox::initialize_onexit_table_0643f8(&tables.crt_0a52a8,tables)==0);
    anadius::crt_cache_recovered::EncodedGlobals cache;
    check(anadius::crt_cache_recovered::initialize_encoded_globals_0644b0(cache,runtime.cookie));
    cache.a4f10=ox::encode_pointer(0x40,runtime.cookie);
    tmn::State term(tables,cache.a4f10);
    std::array<q::Address,4> first{0,0x301,0x302,0x303};std::array<q::Address,1> second{0};Ranges range_effects(first);
    Termination termination(platform,term,range_effects,first,second);
    check(&term.encoded_callback_a4f10==&cache.a4f10);
    en::State entry_state(tables.application_0a47a8);
    Outer outer(qstate,pairs,seq,pstate,pfx,{vstate,vfx,cpu,cpu_effects},{support,{7},lists,env,environment,term,termination},mutate_tls);en::Entry entry(entry_state,outer);
    check(entry.entry_04bcfc(0x123,1,0)==0x100);
    check(outer.validation_calls==1&&outer.tls_calls==1);
    check(support.printf_options_0a37d0==0x8000000000000024ULL&&support.scanf_options_0a37d8==0x4000000000000002ULL);
    check(lists.initialized==1&&lists.flushed==0&&environment.multibyte==1&&environment.wide_releases==1);
    if(env_failure)check(!env.current_0a5288.data&&!env.initial_0a52a0.data&&environment.allocations.empty());
    else{check(env.current_0a5288.data==env.initial_0a52a0.data);check(std::string(env.current_0a5288.data[0])=="A=1"&&std::string(env.current_0a5288.data[1])=="D=two"&&env.current_0a5288.data[2]==nullptr);}
    check(outer.c_termination_complete_05cdb8()==0);
    check(ox::atexit_04b770(0x101,tables,termination)==0&&ox::atexit_04b770(0x102,tables,termination)==0);
    lists.first=0x100;lists.nodes={{0x100,0x200},{0x200,0}};
    check(entry.dispatch_04bbc8(0x123,0,0)==1);
    check(termination.callbacks==std::vector<ox::Address>({0x40,0x102,0x101}));
    check(range_effects.callbacks==std::vector<q::Address>({0x301,0x304,0x303}));
    check(outer.c_termination_complete_05cdb8()==1&&term.completed_a4f18==0);
    check(lists.flushed==1&&lists.freed==std::vector<su::Address>({0x100,0x200})&&lists.nodes.empty());
    check(locks.acrt_count==0&&locks.vcrt_count==0&&platform.deletes==15&&memory.blocks.empty()&&system.slots.empty());
    check(termination.buffers.empty()&&outer.reasons==std::vector<std::uint32_t>({1,0}));
    ev::free_environment_063d80(env.current_0a5288,environment);env={};check(environment.allocations.empty());
    // A second process detach is rejected before app/cleanup work.
    check(entry.dispatch_04bbc8(0x123,0,0)==0&&lists.flushed==1&&termination.callbacks.size()==3);
    try{Termination bad(platform,term,range_effects,std::span<q::Address>(first).first(3),second);check(false);}catch(const std::invalid_argument&){check(true);}
    try{termination.initterm_range_064614(1,2);check(false);}catch(const std::domain_error&){check(true);}
}
}
