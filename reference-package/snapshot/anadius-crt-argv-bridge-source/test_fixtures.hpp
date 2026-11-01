#pragma once
// Shared synthetic argv/CRT fixtures; no native target calls.
#include "argv_bridge.hpp"
#include "../anadius-crt-byteclass-source/byteclass.hpp"
#include "../anadius-crt-startup-bridge-source/test_fixtures.hpp"
#include <functional>
namespace ag=anadius::crt_argv_bridge;
namespace av=anadius::crt_argv_recovered;
namespace bc=anadius::crt_byteclass_recovered;
namespace er=anadius::crt_errors_recovered;
namespace iv=anadius::crt_invalid_recovered;
namespace st=anadius::crt_startup_test;
namespace anadius::crt_argv_test {
using f::check;
struct Heap final:g::HeapErrors {
    f::Memory& memory;bool deny=false;
    Heap(er::State& state,er::Effects& errors,f::Memory& m):HeapErrors(state,errors),memory(m){}
    h::Heap get_process_heap()override{return memory.get_process_heap();}
    void* heap_alloc(h::Heap heap,std::uint32_t flags,std::uint64_t size)override{if(deny)return nullptr;return memory.heap_alloc(heap,flags,size);}
    bool heap_free(h::Heap heap,std::uint32_t flags,void* value)override{return memory.heap_free(heap,flags,value);}
    std::uint32_t last_error()override{return memory.last_error();}
    void lock_0625f8(std::uint32_t index)override{memory.lock_0625f8(index);}
    void unlock_06264c(std::uint32_t index)override{memory.unlock_06264c(index);}
    std::int32_t invoke_new_handler(std::uintptr_t address,std::uint64_t size)override{return memory.invoke_new_handler(address,size);}
};
struct Invalid final:g::InvalidPtd {
    std::uint64_t cookie;unsigned handled=0;
    Invalid(p::State& state,p::Effects& fx,std::uint64_t value):InvalidPtd(state,fx),cookie(value){}
    std::uint32_t locale_changed_0a52d8()override{return 0;}
    std::array<iv::Address,2> locale_pair_0a1938()override{return{0,0};}
    iv::Address encoded_global_handler_0a4f00(std::uint64_t index)override{check(index==0);return r::encode_pointer(0x500,cookie);}
    iv::Address security_cookie_0a1580()override{return cookie;}
    void invoke_handler(iv::Address address,const iv::Arguments& args)override{check(address==0x500&&args==iv::Arguments{});++handled;}
    [[noreturn]]void watson_05c9a4(const iv::Arguments&)override{throw std::runtime_error("unexpected Watson");}
    [[noreturn]]void missing_ptd_05f03c()override{throw std::runtime_error("unexpected missing PTD");}
};
struct Policy final:ag::Policy {
    unsigned initialized=0,filename_calls=0;
    void initialize_multibyte_06f5f8()override{++initialized;}
    std::uint32_t module_filename_06ecdc(std::span<char,260> output)override{++filename_calls;constexpr char filename[]="fallback.exe";std::copy(std::begin(filename),std::end(filename),output.begin());return 0;}
    std::int32_t expand_06ecd4(std::span<char*>,av::Expansion&)override{throw std::runtime_error("mode1 unexpectedly requested wildcard expansion");}
};
struct Locale final:bc::Effects {
    p::State& state;p::Effects& effects;p::Ptd* current=nullptr;unsigned updates=0,clears=0;
    Locale(p::State& s,p::Effects& fx):state(s),effects(fx){}
    bc::LocaleContext locale_update_058b04(bc::Address address)override{
        check(address==0);++updates;current=p::optional_preserve_error_064d08(state,effects);check(current!=nullptr);
        er::Cell32 flags(current->storage.data()+0x3a8);flags.store(flags.load()|2);
        return{reinterpret_cast<bc::Address>(current),0x2000,0x3000,1};
    }
    std::uint8_t read_u8(bc::Address address)override{check(address>=0x3019&&address<0x3119);return address==0x3019+0x81?4:0;}
    std::uint16_t read_u16(bc::Address)override{throw std::runtime_error("lead wrapper read ctype");}
    std::uint32_t read_u32(bc::Address address)override{check(address==reinterpret_cast<bc::Address>(current)+0x3a8);return er::Cell32(current->storage.data()+0x3a8).load();}
    bc::Address read_address(bc::Address)override{throw std::runtime_error("lead wrapper read ctype pointer");}
    void write_u32(bc::Address address,std::uint32_t value)override{check(address==reinterpret_cast<bc::Address>(current)+0x3a8);++clears;er::Cell32(current->storage.data()+0x3a8).store(value);}
};
struct Sequencer final:q::Effects {
    f::Sequencer& base;ag::CleanupState& cleanup;av::Effects& argv_effects;unsigned cleanups=0;
    Sequencer(f::Sequencer& b,ag::CleanupState& c,av::Effects& fx):base(b),cleanup(c),argv_effects(fx){}
    std::uint64_t invoke_boolean_initializer(q::Address address)override{return base.invoke_boolean_initializer(address);}
    void invoke_uninitializer(q::Address address,std::uint32_t argument)override{if(address==13){check(argument==0);++cleanups;check(ag::cleanup_064538(cleanup,argv_effects));}else base.invoke_uninitializer(address,argument);}
    void invoke_void_initializer(q::Address address)override{base.invoke_void_initializer(address);}
    std::uint64_t invoke_int_initializer(q::Address address)override{return base.invoke_int_initializer(address);}
    std::uint32_t common_flush_all(bool value)override{return base.common_flush_all(value);}
};
struct Outer final:ag::ArgvBridge {
    su::State& support;unsigned validation_calls=0,tls_calls=0;std::vector<std::uint32_t> reasons;
    Outer(q::State& state,std::span<q::Pair> pairs,Sequencer& seq,p::State& ps,p::Effects& pf,b::Dependencies vdeps,sb::Dependencies sdeps,ag::Dependencies deps)
        :ArgvBridge(state,pairs,{},{},seq,ps,pf,vdeps,sdeps,deps),support(sdeps.support){}
    void security_init_cookie_04c134()override{} // fixed fixture cookie already selected
    std::uint64_t slot_in_nonwritable_image_04b64c(const std::uintptr_t* slot)override{++validation_calls;check(slot==&support.tls_callback_0a5a70);support.tls_callback_0a5a70=0x202;return 1;}
    void invoke_tls_callback(std::uintptr_t callback,std::uintptr_t module,std::uint32_t reason,std::uintptr_t reserved)override{++tls_calls;check(callback==0x202&&module==0x123&&reason==2&&!reserved);}
    std::uint32_t application_dll_main_038870(std::uintptr_t,std::uint32_t reason,std::uintptr_t)override{reasons.push_back(reason);return 0x100;}
    [[noreturn]]void fastfail_04bfec(std::uint32_t)override{throw std::runtime_error("unexpected fastfail");}
};
inline void combined(bool present,bool fallback){
    f::System system;f::active_system=&system;f::Platform platform;f::Api api_effects(system,platform,present);a::State api;
    r::State runtime(0x1234);f::Runtime runtime_effects(present);l::State locks;
    b::LockEffects lock_effects(runtime,runtime_effects,platform,api,api_effects,platform);
    f::Memory memory;h::State heap{0xaaa,0,0,0};f::Policy thread_policy(system);
    v::Ptd static_ptd;v::State vstate(static_ptd);
    b::PtdEffects vfx(api,api_effects,f::vcrt_callback,heap,memory,locks,lock_effects,thread_policy);
    f::callback_state=&vstate;f::callback_effects=&vfx;
    f::Data data(heap,memory,lock_effects);f::callback_data=&data;f::callback_ids=&data.ids;
    g::FlsApi direct{f::direct_get,f::direct_set,f::direct_alloc,f::direct_free,f::direct_error,f::direct_set_error};
    g::PtdEffects pfx(direct,f::acrt_callback,heap,memory,data.ids,data,thread_policy);p::State pstate;
    c::State cpu{1,2,0x80000,0x2000000,0};f::Cpu cpu_effects;q::State qstate;
    su::State support{0,0,0x201};st::Lists lists;ev::State env;st::Environment environment;
    ox::State tables{runtime.cookie,{},{}};check(ox::initialize_onexit_table_0643f8(&tables.crt_0a52a8,tables)==0);
    anadius::crt_cache_recovered::EncodedGlobals cache;check(anadius::crt_cache_recovered::initialize_encoded_globals_0644b0(cache,runtime.cookie));cache.a4f10=ox::encode_pointer(0x40,runtime.cookie);
    tmn::State term(tables,cache.a4f10);std::array<q::Address,4> first{0,0x301,0x302,0x303};std::array<q::Address,1> second{0};st::Ranges range_effects(first);st::Termination termination(platform,term,range_effects,first,second);
    er::State errors;g::ErrorPtd error_ptd(pstate,pfx);Heap argv_heap(errors,error_ptd,memory);Invalid invalid(pstate,pfx,runtime.cookie);Policy policy;
    ag::ArgvEffects argv_effects(heap,argv_heap,errors,error_ptd,invalid,policy);av::State argv;Locale locale(pstate,pfx);bc::ParserEffects parser(locale);
    std::string command="\"app path.exe\" \"two words\" ";command.push_back(static_cast<char>(0x81));command.push_back(' ');command.push_back('x');command.push_back('\0');
    if(!fallback)argv.command_line=command;
    ag::CleanupState cleanup{argv};f::Sequencer base_seq(locks,lock_effects,pstate,pfx);Sequencer seq(base_seq,cleanup,argv_effects);
    std::array<q::Pair,3> pairs{{{1,11},{2,12},{0,13}}};en::State entry_state(tables.application_0a47a8);
    Outer outer(qstate,pairs,seq,pstate,pfx,{vstate,vfx,cpu,cpu_effects},{support,{7},lists,env,environment,term,termination},{argv,argv_effects,parser});en::Entry entry(entry_state,outer);
    check(entry.entry_04bcfc(0x123,1,0)==0x100);check(outer.validation_calls==1&&outer.tls_calls==1);
    check(argv.program_name==argv.module_filename.data()&&policy.initialized==1&&policy.filename_calls==1);
    if(fallback)check(argv.argc==1&&std::string(argv.argv[0])=="fallback.exe"&&argv.argv[1]==nullptr);
    else{check(argv.argc==3&&std::string(argv.argv[0])=="app path.exe"&&std::string(argv.argv[1])=="two words"&&argv.argv[3]==nullptr);check(static_cast<unsigned char>(argv.argv[2][0])==0x81&&std::string(argv.argv[2]+1)==" x");}
    check(locale.updates>0&&locale.updates==locale.clears&&memory.blocks.size()==2);
    auto* ptd=p::optional_preserve_error_064d08(pstate,pfx);er::Cell32 errno_cell(ptd->storage.data()+0x20),dos_cell(ptd->storage.data()+0x24);
    check((er::Cell32(ptd->storage.data()+0x3a8).load()&2)==0);dos_cell.store(0xabcdef01);errno_cell.store(0);
    auto** old_argv=argv.argv;const auto old_argc=argv.argc;argv_heap.deny=true;
    check(outer.configure_narrow_argv_0638e8(1)==12);check(argv.argv==old_argv&&argv.argc==old_argc&&errno_cell.load()==12&&dos_cell.load()==0xabcdef01);
    argv_heap.deny=false;system.error=0x76543210;check(outer.configure_narrow_argv_0638e8(99)==22);
    check(invalid.handled==1&&errno_cell.load()==22&&dos_cell.load()==0xabcdef01&&system.error==0x76543210);
    check(entry.dispatch_04bbc8(0x123,0,0)==1);check(seq.cleanups==1&&argv.argv==nullptr&&argv.argc==old_argc);
    check(termination.callbacks==std::vector<ox::Address>({0x40})&&range_effects.callbacks==std::vector<q::Address>({0x301,0x304,0x303}));
    check(memory.blocks.empty()&&system.slots.empty()&&platform.deletes==15);
    ev::free_environment_063d80(env.current_0a5288,environment);env={};check(environment.allocations.empty());
}
struct Trace final:av::Effects {
    std::vector<void*> freed;std::function<void(void*)> callback;
    void initialize_multibyte_06f5f8()override{throw std::runtime_error("unexpected init");}
    std::uint32_t module_filename_06ecdc(std::span<char,260>)override{throw std::runtime_error("unexpected path");}
    er::Cell32 errno_05d724()override{throw std::runtime_error("unexpected errno");}
    void invalid_parameter_05c954()override{throw std::runtime_error("unexpected invalid");}
    av::Allocation calloc_066848(std::uint64_t,std::uint64_t)override{throw std::runtime_error("unexpected calloc");}
    void free_0668c0(void* value)override{freed.push_back(value);if(callback)callback(value);}
    std::int32_t expand_06ecd4(std::span<char*>,av::Expansion&)override{throw std::runtime_error("unexpected expand");}
};
inline void cleanup_order(){
    std::array<std::uint64_t,6> tokens{};av::State argv;argv.argc=7;
    std::array<char*,1> third{nullptr},replacement{nullptr};argv.argv=third.data();ag::CleanupState state{argv,&tokens[0],&tokens[1],&tokens[2]};Trace trace;
    trace.callback=[&](void* value){
        if(value==&tokens[0]){check(state.narrow_aux_0a58a8==value);state.wide_aux_0a58b0=&tokens[3];}
        else if(value==&tokens[3]){check(state.narrow_aux_0a58a8==nullptr&&state.wide_aux_0a58b0==value);argv.argv=replacement.data();}
        else if(value==replacement.data()){check(state.wide_aux_0a58b0==nullptr&&argv.argv==replacement.data());state.wide_argv_0a59f8=&tokens[4];}
        else{check(value==&tokens[4]&&argv.argv==nullptr&&state.wide_argv_0a59f8==value);}
    };
    check(ag::cleanup_064538(state,trace));check(trace.freed==std::vector<void*>({&tokens[0],&tokens[3],replacement.data(),&tokens[4]}));
    check(!state.narrow_aux_0a58a8&&!state.wide_aux_0a58b0&&!state.wide_argv_0a59f8&&!argv.argv&&argv.argc==7);
    Trace empty;check(ag::cleanup_064538(state,empty)&&empty.freed==std::vector<void*>(4,nullptr));
    Trace failure;state.narrow_aux_0a58a8=&tokens[0];failure.callback=[](void*){throw std::runtime_error("synthetic free failure");};
    bool caught=false;try{(void)ag::cleanup_064538(state,failure);}catch(const std::runtime_error&){caught=true;}
    check(caught&&state.narrow_aux_0a58a8==&tokens[0]&&failure.freed==std::vector<void*>({&tokens[0]}));
}
}
