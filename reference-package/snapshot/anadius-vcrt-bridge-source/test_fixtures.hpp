#pragma once
// Shared synthetic fixtures; no native OS calls. Extracted from bridge_tests.cpp.
#include "bridge.hpp"
#include "../anadius-crt-glue-source/crt_glue.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <source_location>
#include <string>
#include <vector>
namespace b=anadius::vcrt_bridge;
namespace v=anadius::vcrt_ptd_recovered;
namespace a=anadius::vcrt_api_recovered;
namespace l=anadius::crt_locks_recovered;
namespace q=anadius::crt_initializers_recovered;
namespace p=anadius::crt_ptd_recovered;
namespace d=anadius::crt_ptd_data_recovered;
namespace h=anadius::crt_heap_recovered;
namespace g=anadius::crt_glue;
namespace r=anadius::crt_runtime_recovered;
namespace c=anadius::crt_cpu_recovered;
namespace en=anadius::crt_entry_recovered;
namespace anadius::vcrt_test {
inline unsigned checks;
inline void check(bool value,const std::source_location where=std::source_location::current()){
    ++checks;if(!value)throw std::runtime_error("check "+std::to_string(checks)+" at "+where.file_name()+":"+std::to_string(where.line()));
}
struct Memory final:h::Effects {
    std::map<void*,std::size_t> blocks;std::vector<std::size_t> requested;
    ~Memory(){for(auto [address,size]:blocks){(void)size;::operator delete(address);}}
    h::Heap get_process_heap()override{return 0xaaa;}
    void* heap_alloc(h::Heap heap,std::uint32_t flags,std::uint64_t bytes)override{
        check(heap==0xaaa&&flags==8);auto* storage=::operator new(static_cast<std::size_t>(bytes));
        std::memset(storage,0,static_cast<std::size_t>(bytes));blocks[storage]=static_cast<std::size_t>(bytes);requested.push_back(static_cast<std::size_t>(bytes));return storage;
    }
    bool heap_free(h::Heap heap,std::uint32_t flags,void* storage)override{check(heap==0xaaa&&!flags&&blocks.erase(storage)==1);::operator delete(storage);return true;}
    std::uint32_t last_error()override{check(false);return 0;}
    std::uint32_t errno_from_os_error_05d650(std::uint32_t)override{check(false);return 0;}
    void write_error_slot_05d724(std::uint32_t)override{check(false);}
    void lock_0625f8(std::uint32_t)override{check(false);}
    void unlock_06264c(std::uint32_t)override{check(false);}
    std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t)override{check(false);return 0;}
};
struct Platform final:b::LockPlatform,l::InitializeEffects {
    std::array<bool,14> acrt{};std::array<bool,1> vcrt{};
    std::array<std::uint64_t,14> acrt_tokens{};std::uint64_t vcrt_token{};
    std::int32_t result=1;unsigned deletes=0,enters=0;
    bool& active(l::LockRef ref){check(ref.index>=0&&ref.index<(ref.bank==l::Bank::acrt?14:1));return ref.bank==l::Bank::acrt?acrt[static_cast<std::size_t>(ref.index)]:vcrt[0];}
    void* storage(l::LockRef ref)override{(void)active(ref);return ref.bank==l::Bank::acrt?static_cast<void*>(&acrt_tokens[static_cast<std::size_t>(ref.index)]):&vcrt_token;}
    std::uint64_t initialize(l::LockRef ref,std::uint32_t spin,std::uint32_t flags){check(spin==4000&&!flags&&!active(ref));if(result)active(ref)=true;return static_cast<std::uint32_t>(result);}
    std::uint64_t call_initialize_ex(r::Address address,l::LockRef ref,std::uint32_t spin,std::uint32_t flags)override{check(address==0x555);return initialize(ref,spin,flags);}
    std::uint64_t initialize_and_spin(l::LockRef ref,std::uint32_t spin)override{return initialize(ref,spin,0);}
    std::int32_t initialize_vcrt(void* token,std::uint32_t spin,std::uint32_t flags){check(token==&vcrt_token);return static_cast<std::int32_t>(initialize({l::Bank::vcrt,0},spin,flags));}
    void destroy(l::LockRef ref)override{check(active(ref));active(ref)=false;++deletes;}
    void enter(l::LockRef ref)override{check(active(ref));++enters;}
    void leave(l::LockRef ref)override{check(active(ref));}
};
struct System {
    struct Slot{void* value;void(*callback)(void*);};
    std::map<std::uint32_t,Slot> slots;std::uint32_t next=10,error=0xcafe;
    std::uint32_t allocate(void(*callback)(void*)){const auto index=++next;slots.emplace(index,Slot{nullptr,callback});return index;}
    void* get(std::uint32_t index){check(slots.contains(index));error=0x1234;return slots.at(index).value;}
    std::int32_t set(std::uint32_t index,void* value){check(slots.contains(index));slots.at(index).value=value;error=0x7777;return -1;}
    std::int32_t release(std::uint32_t index){check(slots.contains(index));const auto value=slots.at(index);slots.erase(index);if(value.callback&&value.value)value.callback(value.value);return -1;}
};
inline System* active_system;
inline v::State* callback_state;
inline v::Effects* callback_effects;
inline void vcrt_callback(void* value){v::free_callback_04d5d4(*callback_state,*callback_effects,value);}
inline d::Effects* callback_data;
inline const d::Identities* callback_ids;
inline VOID WINAPI acrt_callback(PVOID value){d::fls_destroy_0649cc(value,*callback_ids,*callback_data);}
inline PVOID WINAPI direct_get(DWORD index){return active_system->get(index);}
inline BOOL WINAPI direct_set(DWORD index,PVOID value){return active_system->set(index,value);}
inline DWORD WINAPI direct_alloc(PFLS_CALLBACK_FUNCTION callback){return active_system->allocate(callback);}
inline BOOL WINAPI direct_free(DWORD index){return active_system->release(index);}
inline DWORD WINAPI direct_error(){return active_system->error;}
inline VOID WINAPI direct_set_error(DWORD error){active_system->error=error;}
struct Api final:a::Effects {
    System& system;Platform& platform;bool available;
    unsigned native_calls=0,tls_calls=0;
    Api(System& s,Platform& p,bool present):system(s),platform(p),available(present){}
    a::Module load_library_ex_w(std::u16string_view,a::Module file,std::uint32_t flags)override{check(!file&&flags==0x800);system.error=0x9876;return 0x1000;}
    std::uint32_t last_error()override{return system.error;}
    bool free_library(a::Module)override{check(false);return false;}
    a::Address get_proc_address(a::Module module,std::string_view name)override{
        check(module==0x1000);if(!available)return 0;
        if(name=="FlsAlloc")return 1;if(name=="FlsFree")return 2;if(name=="FlsGetValue")return 3;if(name=="FlsSetValue")return 4;check(name=="InitializeCriticalSectionEx");return 5;
    }
    std::uint32_t call_fls_alloc(a::Address address,a::FlsCallback cb)override{check(address==1&&cb==vcrt_callback);++native_calls;return system.allocate(cb);}
    std::int32_t call_fls_free(a::Address address,std::uint32_t index)override{check(address==2);++native_calls;return system.release(index);}
    void* call_fls_get_value(a::Address address,std::uint32_t index)override{check(address==3);++native_calls;return system.get(index);}
    std::int32_t call_fls_set_value(a::Address address,std::uint32_t index,void* value)override{check(address==4);++native_calls;return system.set(index,value);}
    std::int32_t call_initialize_critical_section_ex(a::Address address,void* token,std::uint32_t spin,std::uint32_t flags)override{check(address==5);++native_calls;return platform.initialize_vcrt(token,spin,flags);}
    std::uint32_t tls_alloc()override{++tls_calls;return system.allocate(nullptr);}
    std::int32_t tls_free(std::uint32_t index)override{++tls_calls;return system.release(index);}
    void* tls_get_value(std::uint32_t index)override{++tls_calls;return system.get(index);}
    std::int32_t tls_set_value(std::uint32_t index,void* value)override{++tls_calls;return system.set(index,value);}
    std::int32_t initialize_critical_section_and_spin_count(void* token,std::uint32_t spin)override{++tls_calls;return platform.initialize_vcrt(token,spin,0);}
};
struct Runtime final:r::Effects {
    bool available;
    explicit Runtime(bool present):available(present){}
    r::Module load_library_ex_w(std::u16string_view,r::Module file,std::uint32_t flags)override{check(!file&&flags==0x800);return 0x999;}
    std::uint32_t last_error()override{check(false);return 0;}
    bool free_library(r::Module)override{check(false);return false;}
    r::Address get_proc_address(r::Module module,std::string_view name)override{check(module==0x999&&name=="InitializeCriticalSectionEx");return available?0x555:0;}
    std::uint32_t process_parameters_flags()override{check(false);return 0;}
    std::int32_t call_ro_initialize(r::Address,std::uint32_t)override{check(false);return 0;}
    void call_ro_uninitialize(r::Address)override{check(false);}
    std::uint32_t call_thread_initialization_policy(r::Address,std::uintptr_t,std::int32_t*)override{check(false);return 0;}
};
struct Policy final:b::ThreadPolicy,g::AbortPolicy {
    System& system;explicit Policy(System& s):system(s){}
    void set_last_error(std::uint32_t value)override{system.error=value;}
    [[noreturn]]void abort_05f03c()override{throw std::runtime_error("abort");}
};
struct Data final:d::Effects {
    const d::Identities ids{0x111,0x222,0x333};
    h::State& heap;Memory& memory;l::Effects& locks;std::uint32_t refs=0;
    Data(h::State& h,Memory& m,l::Effects& l):heap(h),memory(m),locks(l){}
    void lock_0625f8(std::uint32_t index)override{l::lock_acrt_0625f8(locks,static_cast<std::int32_t>(index));}
    void unlock_06264c(std::uint32_t index)override{l::unlock_acrt_06264c(locks,static_cast<std::int32_t>(index));}
    std::uint32_t atomic_fetch_add_32(d::Address address,std::uint32_t increment)override{check(address==ids.multibyte_info);const auto old=refs;refs+=increment;return old;}
    void free_0668c0(d::Address address)override{h::free_0668c0(reinterpret_cast<void*>(address),heap,memory);}
    d::Address current_locale_0a52e0()override{return ids.initial_locale;}
    void release_locale_ref_0708c0(d::Address address)override{check(address==ids.initial_locale);}
    std::uint32_t locale_ref_count_10(d::Address)override{check(false);return 0;}
    void free_locale_0706c0(d::Address)override{check(false);}
    void add_locale_ref_070634(d::Address address)override{check(address==ids.initial_locale);}
};
struct Cpu final:c::Effects {
    c::Registers cpuid(std::uint32_t leaf,std::uint32_t subleaf)override{check(subleaf==0);return leaf==0?c::Registers{7,0,0,0}:c::Registers{};}
    std::uint64_t xgetbv(std::uint32_t)override{check(false);return 0;}
};
struct Sequencer final:q::Effects {
    l::State& locks;l::Effects& lock_effects;p::State& ptd;p::Effects& ptd_effects;
    Sequencer(l::State& l,l::Effects& fx,p::State& p,p::Effects& px):locks(l),lock_effects(fx),ptd(p),ptd_effects(px){}
    std::uint64_t invoke_boolean_initializer(q::Address address)override{
        if(address==1)return l::initialize_acrt_0625b0(locks,lock_effects);
        check(address==2);return p::initialize_064e90(ptd,ptd_effects);
    }
    void invoke_uninitializer(q::Address address,std::uint32_t value)override{
        check(value==0);if(address==12)(void)p::uninitialize_064ecc(ptd,ptd_effects);
        else{check(address==11);(void)l::uninitialize_acrt_062614(locks,lock_effects);}
    }
    void invoke_void_initializer(q::Address)override{check(false);}
    std::uint64_t invoke_int_initializer(q::Address)override{check(false);return 0;}
    std::uint32_t common_flush_all(bool)override{check(false);return 0;}
};
struct Outer final:b::LifecycleBridge {
    std::uintptr_t tls_slot=0;unsigned cexits=0;std::vector<std::uint32_t> reasons;
    Outer(q::State& qstate,std::span<q::Pair> pairs,Sequencer& seq,p::State& ps,p::Effects& pf,b::Dependencies deps)
        :LifecycleBridge(qstate,pairs,{},{},seq,ps,pf,deps){}
    void security_init_cookie_04c134()override{}
    void initialize_slist_04c1e0()override{}
    void configure_stdio_04c1fc()override{}
    std::uint32_t configure_narrow_argv_0638e8(std::uint32_t mode)override{check(mode==1);return 0;}
    void initialize_narrow_environment_064054()override{}
    std::uint32_t c_termination_complete_05cdb8()override{return cexits;}
    void cexit_05cd9c()override{++cexits;}
    void release_slist_04c1f0()override{}
    const std::uintptr_t* tls_callback_slot_04c218()override{return &tls_slot;}
    std::uint64_t slot_in_nonwritable_image_04b64c(const std::uintptr_t*)override{check(false);return 0;}
    void invoke_tls_callback(std::uintptr_t,std::uintptr_t,std::uint32_t,std::uintptr_t)override{check(false);}
    std::uint32_t application_dll_main_038870(std::uintptr_t,std::uint32_t reason,std::uintptr_t)override{reasons.push_back(reason);return 0x100;}
    [[noreturn]]void fastfail_04bfec(std::uint32_t)override{throw std::runtime_error("fastfail");}
};
inline void combined_lifetime(bool present){
    System system;active_system=&system;Platform platform;Api api_effects(system,platform,present);a::State api;
    r::State runtime(0x1234);Runtime runtime_effects(present);l::State locks;
    b::LockEffects lock_effects(runtime,runtime_effects,platform,api,api_effects,platform);
    Memory memory;h::State heap{0xaaa,0,0,0};Policy policy(system);
    v::Ptd static_ptd;v::State vstate(static_ptd);
    b::PtdEffects vfx(api,api_effects,vcrt_callback,heap,memory,locks,lock_effects,policy);
    callback_state=&vstate;callback_effects=&vfx;
    Data data(heap,memory,lock_effects);callback_data=&data;callback_ids=&data.ids;
    g::FlsApi direct{direct_get,direct_set,direct_alloc,direct_free,direct_error,direct_set_error};
    g::PtdEffects pfx(direct,acrt_callback,heap,memory,data.ids,data,policy);p::State pstate;
    c::State cpu_state;Cpu cpu_effects;q::State qstate;
    std::array<q::Pair,2> pairs{{{1,11},{2,12}}};Sequencer seq(locks,lock_effects,pstate,pfx);
    anadius::crt_onexit_recovered::Table exit_table{};en::State entry_state(exit_table);
    Outer outer(qstate,pairs,seq,pstate,pfx,{vstate,vfx,cpu_state,cpu_effects});en::Entry entry(entry_state,outer);
    check(entry.entry_04bcfc(0x123,1,0)==0x100);check(cpu_state.a1550==1&&cpu_state.a1554==2);
    check(locks.acrt_count==14&&locks.vcrt_count==1);
    check(vstate.fls_index!=pstate.fls_index&&system.slots.size()==2);
    check(system.slots.at(vstate.fls_index).value==&static_ptd&&static_ptd.frame_state==-2);
    check(system.slots.at(vstate.fls_index).callback==(present?vcrt_callback:nullptr));
    check(system.slots.at(pstate.fls_index).callback==acrt_callback);
    check(memory.blocks.size()==1&&data.refs==1);
    std::map<std::uint32_t,void*> saved;
    for(auto& [index,slot]:system.slots){saved[index]=slot.value;slot.value=nullptr;}
    system.error=0xaabbccdd;
    check(entry.dispatch_04bbc8(0x123,2,0)==0x100);check(system.error==0xaabbccdd);
    auto* dynamic=static_cast<v::Ptd*>(system.slots.at(vstate.fls_index).value);
    check(dynamic!=&static_ptd&&dynamic->frame_state==-2);
    check(memory.blocks.size()==3&&memory.blocks.at(dynamic)==128&&data.refs==2);
    check(entry.dispatch_04bbc8(0x123,3,0)==1);check(memory.blocks.size()==1&&data.refs==1);
    check(system.slots.at(vstate.fls_index).value==nullptr&&system.slots.at(pstate.fls_index).value==nullptr);
    for(auto& [index,slot]:system.slots)slot.value=saved[index];
    check(entry.dispatch_04bbc8(0x123,0,0)==1);
    check(locks.acrt_count==0&&locks.vcrt_count==0&&platform.deletes==15);
    check(memory.blocks.empty()&&system.slots.empty()&&data.refs==0&&outer.cexits==1);
    check(vstate.fls_index==0xffffffff&&pstate.fls_index==0xffffffff);
    check(outer.reasons==std::vector<std::uint32_t>({1,2,3,0}));
    check(present?(api_effects.native_calls>0&&api_effects.tls_calls==0):(api_effects.native_calls==0&&api_effects.tls_calls>0));
    for(std::size_t index=0;index<api.procedures.size();++index)
        check(api.procedures[index].load()==(present?index+1:a::missing));
    // Constructor and allocation domain checks occur before provider work.
    try{b::PtdEffects bad(api,api_effects,nullptr,heap,memory,locks,lock_effects,policy);check(false);}catch(const std::invalid_argument&){check(true);}
    try{(void)vfx.calloc_05cae4(2,64);check(false);}catch(const std::invalid_argument&){check(true);}
    try{(void)vfx.fls_alloc_050b0c(static_cast<v::FlsCallback>(99));check(false);}catch(const std::invalid_argument&){check(true);}
    // Raw BOOL32 return bits are zero-extended to the lock-provider uint64 API.
    platform.result=static_cast<std::int32_t>(0x80000000U);
    check(lock_effects.initialize_vcrt_050c38({l::Bank::vcrt,0},4000,0)==0x80000000ULL);
    lock_effects.delete_critical_section({l::Bank::vcrt,0});
}
}
