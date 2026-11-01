#include "bindings.hpp"
#include "../anadius-crt-argv-bridge-source/test_fixtures.hpp"
#include <functional>
#include <limits>
namespace pb=anadius::crt_pair_bindings;
namespace pl=anadius::crt_pair_leaves_recovered;
namespace lo=anadius::crt_lowio_lifecycle_recovered;
namespace ec=anadius::crt_environment_cleanup_recovered;
namespace lc=anadius::crt_lifecycle_callbacks_recovered;
namespace ls=anadius::crt_locale_startup_recovered;
namespace cs=anadius::crt_cache_recovered;
namespace mc=anadius::crt_multibyte_control_recovered;
namespace at=anadius::crt_argv_test;
namespace {
using f::check;
using Address=std::uint64_t;
using Id=pb::Id;
Address id(Id value){return pb::token(value);}
template<class T>Address address(T* p){return reinterpret_cast<Address>(p);}
template<class F>void rejects(F fn){bool caught=false;try{fn();}catch(const std::invalid_argument&){caught=true;}check(caught);}
bool fail_acrt_allocate=false;
DWORD WINAPI allocate_acrt(PFLS_CALLBACK_FUNCTION cb){return fail_acrt_allocate?p::invalid_fls_index:f::direct_alloc(cb);}
struct HeapInit final:h::Effects {
    f::Memory& base;bool fail=false;
    explicit HeapInit(f::Memory& b):base(b){}
    h::Heap get_process_heap()override{return fail?0:base.get_process_heap();}
    void* heap_alloc(h::Heap x,std::uint32_t flags,std::uint64_t n)override{return base.heap_alloc(x,flags,n);}
    bool heap_free(h::Heap x,std::uint32_t flags,void* value)override{return base.heap_free(x,flags,value);}
    std::uint32_t last_error()override{return base.last_error();}
    std::uint32_t errno_from_os_error_05d650(std::uint32_t n)override{return base.errno_from_os_error_05d650(n);}
    void write_error_slot_05d724(std::uint32_t n)override{base.write_error_slot_05d724(n);}
    void lock_0625f8(std::uint32_t n)override{base.lock_0625f8(n);}
    void unlock_06264c(std::uint32_t n)override{base.unlock_06264c(n);}
    std::int32_t invoke_new_handler(std::uintptr_t a,std::uint64_t n)override{return base.invoke_new_handler(a,n);}
};
struct Locale final:ls::Effects {
    Address current=0,initial=0x333;
    Address current_locale_slot_0a52e0()override{return address(&current);}
    Address initial_locale_0a17e0()override{return initial;}
    Address read_address(Address a)override{check(a==address(&current));return current;}
    void write_address(Address a,Address b)override{check(a==address(&current));current=b;}
    void lock_0625f8(std::uint32_t)override{throw std::logic_error("unused locale reset");}
    void unlock_06264c(std::uint32_t)override{throw std::logic_error("unused locale reset");}
    Address update_locale_nolock_0709d8(Address,Address)override{throw std::logic_error("unused locale reset");}
};
struct Lowio final:lo::ArrayEffects {
    l::Effects& locks;bool fail=false;unsigned ensure=0,inherited=0,standard=0;
    explicit Lowio(l::Effects& l):locks(l){}
    void lock_0625f8(std::uint32_t n)override{l::lock_acrt_0625f8(locks,static_cast<std::int32_t>(n));}
    void unlock_06264c(std::uint32_t n)override{l::unlock_acrt_06264c(locks,static_cast<std::int32_t>(n));}
    std::uint64_t ensure_fh_exists_0732a8(std::int32_t n)override{check(n==0);++ensure;return fail?1:0;}
    void initialize_inherited_067698()override{++inherited;}
    void initialize_standard_067788()override{++standard;}
    void delete_critical_section(Address)override{throw std::logic_error("empty fixture lowio table");}
    void free_0668c0(Address)override{throw std::logic_error("empty fixture lowio table");}
};
struct Commands final:pl::CommandLineEffects {
    av::State& argv;std::span<const char16_t>& wide;
    std::string text=std::string("own.exe alpha \"two words\"")+char(0);
    std::u16string wtext=std::u16string(u"own.exe")+char16_t(0);
    unsigned a=0,w=0;std::function<void()> on_w;
    Commands(av::State& s,std::span<const char16_t>& v):argv(s),wide(v){}
    std::span<const char> get_command_line_a()override{++a;return {text.data(),text.size()};}
    std::span<const char16_t> get_command_line_w()override{
        ++w;check(argv.command_line.data()==text.data()&&wide.empty());
        if(on_w)on_w();
        return {wtext.data(),wtext.size()};
    }
};
struct Multibyte final:mc::Effects,lc::MultibyteEffects {
    p::State& pstate;p::Effects& pfx;f::Data& data;
    Address current=0,types=0,cases=0,pair=0;
    std::uint32_t system_flag=0,error=0;std::uint8_t initialized=0;
    unsigned malloc_calls=0,shutdown_atomic=0;
    Multibyte(p::State& s,p::Effects& f,f::Data& d):pstate(s),pfx(f),data(d){}
    Address required_ptd_raw_064c64()override{return address(p::required_raw_064c64(pstate,pfx));}
    Address current_multibyte_slot_0a59d0()override{return address(&current);}
    Address current_multibyte_0a59d0()override{return current;}
    void set_current_multibyte_0a59d0(Address a)override{current=a;}
    Address initial_multibyte_0a1c70()override{return data.ids.multibyte_info;}
    std::uint8_t initialized_0a59dc()override{return initialized;}
    void set_initialized_0a59dc(std::uint8_t n)override{initialized=n;}
    void set_type_table_0a59c0(Address n)override{types=n;}
    void set_case_table_0a59c8(Address n)override{cases=n;}
    Address type_table_0a59c0()override{return types;}
    Address case_table_0a59c8()override{return cases;}
    void set_initial_pair_multibyte_0a1940(Address n)override{pair=n;}
    std::uint32_t locale_mask_0a1c6c()override{return 0xfffffffeU;}
    anadius::crt_byteclass_recovered::LocaleContext locale_update_058b04(Address n)override{check(n==0);return {};}
    void write_system_cp_flag_0a59d8(std::uint32_t n)override{system_flag=n;}
    std::uint32_t get_acp()override{return 65001;}
    std::uint32_t get_oemcp()override{throw std::logic_error("unexpected OEMCP");}
    Address read_address(Address a)override{
        if(a==address(&current))return current;
        auto* value=p::required_raw_064c64(pstate,pfx);
        check(a==address(value)+0x88);
        return d::View(value).load<Address>(0x88);
    }
    void write_address(Address a,Address value)override{check(a==address(&current));current=value;}
    std::uint32_t read_u32(Address a)override{check(a==data.ids.multibyte_info+4);return 0;}
    void write_u32(Address,std::uint32_t)override{throw std::logic_error("unused MB write");}
    std::uint8_t read_u8(Address)override{throw std::logic_error("unused MB read");}
    void write_u8(Address,std::uint8_t)override{throw std::logic_error("unused MB write");}
    Address update_thread_multibyte_06f540(Address a,Address b)override{
        check(a==required_ptd_raw_064c64()&&b==address(&current));return data.ids.multibyte_info;
    }
    Address malloc_06a308(std::uint64_t n)override{check(n==0x228);++malloc_calls;error=12;return 0;} // controlled lower failure
    std::uint32_t build_multibyte_06f674(std::int32_t,Address)override{throw std::logic_error("allocation denied before builder");}
    void lock_0625f8(std::uint32_t n)override{data.lock_0625f8(n);}
    void unlock_06264c(std::uint32_t n)override{data.unlock_06264c(n);}
    std::uint32_t atomic_fetch_add_32(Address a,std::uint32_t n)override{++shutdown_atomic;return data.atomic_fetch_add_32(a,n);}
    void free_0668c0(Address a)override{data.free_0668c0(a);}
    er::Cell32 errno_cell_05d724()override{return er::Cell32(&error);}
    void invalid_parameter_05c954()override{throw std::logic_error("unexpected invalid MB");}
    std::uint32_t exchange_locale_changed_0a52d8(std::uint32_t)override{throw std::logic_error("unused changed");}
};
struct Args final:av::Effects {
    h::State& heap;f::Memory& memory;Multibyte& mb;const mc::Identities& ids;std::uint32_t error=0;
    Args(h::State& h,f::Memory& m,Multibyte& b,const mc::Identities& i):heap(h),memory(m),mb(b),ids(i){}
    void initialize_multibyte_06f5f8()override{check(mc::initialize_multibyte_06f5f8(ids,mb));}
    std::uint32_t module_filename_06ecdc(std::span<char,260> out)override{const char name[]="fallback.exe";std::copy(std::begin(name),std::end(name),out.begin());return 0;}
    er::Cell32 errno_05d724()override{return er::Cell32(&error);}
    void invalid_parameter_05c954()override{throw std::logic_error("unexpected invalid argv");}
    av::Allocation calloc_066848(std::uint64_t n,std::uint64_t size)override{
        auto* value=h::calloc_base_066848(n,size,heap,memory);return {value,{static_cast<std::byte*>(value),static_cast<std::size_t>(n*size)}};
    }
    void free_0668c0(void* value)override{h::free_0668c0(value,heap,memory);}
    std::int32_t expand_06ecd4(std::span<char*>,av::Expansion&)override{throw std::logic_error("no wildcard fixture");}
};
struct Parser final:anadius::crt_argv_parse_recovered::Effects {
    std::int32_t is_mb_lead_06f9a8(std::int32_t)override{return 0;}
};
struct Cache final:cs::CacheEffects {std::vector<r::Module> freed;bool free_library(r::Module n)override{freed.push_back(n);return true;}};
struct Dispatcher final:pb::Effects {
    unsigned flushes=0;
    explicit Dispatcher(pb::Dependencies d):Effects(d){}
    void invoke_void_initializer(q::Address)override{throw std::invalid_argument("non-pair void policy");}
    std::uint64_t invoke_int_initializer(q::Address)override{throw std::invalid_argument("non-pair int policy");}
    std::uint32_t common_flush_all(bool n)override{check(n);++flushes;return 0;}
};
struct Recorder final:q::Effects {
    Dispatcher& dispatch;std::vector<Address> init,uninit;std::function<void(Address)> after;
    explicit Recorder(Dispatcher& d):dispatch(d){}
    std::uint64_t invoke_boolean_initializer(q::Address n)override{init.push_back(n);return dispatch.invoke_boolean_initializer(n);}
    void invoke_uninitializer(q::Address n,std::uint32_t arg)override{check(arg==0);uninit.push_back(n);dispatch.invoke_uninitializer(n,arg);if(after)after(n);}
    void invoke_void_initializer(q::Address n)override{dispatch.invoke_void_initializer(n);}
    std::uint64_t invoke_int_initializer(q::Address n)override{return dispatch.invoke_int_initializer(n);}
    std::uint32_t common_flush_all(bool n)override{return dispatch.common_flush_all(n);}
};
struct Harness {
    f::System system;f::Platform platform;f::Api api_fx{system,platform,true};a::State api;
    r::State runtime{0x1234};f::Runtime runtime_fx{true};l::State locks;
    b::LockEffects lock_fx{runtime,runtime_fx,platform,api,api_fx,platform};
    f::Memory memory;h::State heap{0xaaa,0,runtime.cookie,0};HeapInit heap_init{memory};
    f::Policy policy{system};v::Ptd static_ptd;v::State vstate{static_ptd};
    b::PtdEffects vfx{api,api_fx,f::vcrt_callback,heap,memory,locks,lock_fx,policy};
    f::Data data{heap,memory,lock_fx};
    g::PtdEffects pfx{g::FlsApi{f::direct_get,f::direct_set,allocate_acrt,f::direct_free,f::direct_error,f::direct_set_error},f::acrt_callback,heap,memory,data.ids,data,policy};
    p::State pstate;
    Locale locale;cs::EncodedGlobals encoded;Cache cache;
    lo::State lowio;Lowio lowio_fx{lock_fx};
    av::State argv;std::span<const char16_t> wide_command;
    pl::CommandLines commands{argv,wide_command};Commands command_fx{argv,wide_command};
    Multibyte mb{pstate,pfx,data};mc::Identities mb_ids{data.ids.multibyte_info,0x5000,0x6000};
    Args argv_fx{heap,memory,mb,mb_ids};ag::CleanupState cleanup{argv};
    ev::State environment;ev::Environment current_wide,initial_wide;
    ec::State env_cleanup{environment,current_wide,initial_wide};st::Environment env_fx;
    ox::State onexit{runtime.cookie,{},{}};tmn::State termination{onexit,encoded.a4f10};
    std::unique_ptr<Dispatcher> dispatch;std::unique_ptr<Recorder> recorder;q::State outer;
    pb::Dependencies deps(tmn::State* t=nullptr,ag::CleanupState* c=nullptr){
        return {locale,encoded,runtime,cache,locks,lock_fx,heap,heap_init,vstate,vfx,pstate,pfx,lowio,lowio_fx,
            commands,command_fx,mb_ids,mb,c?*c:cleanup,argv_fx,env_cleanup,env_fx,t?*t:termination,mb};
    }
    Harness(){
        f::active_system=&system;f::callback_state=&vstate;f::callback_effects=&vfx;f::callback_data=&data;f::callback_ids=&data.ids;fail_acrt_allocate=false;
        // Original outer VCRT entry precedes the ACRT table; prepared explicitly
        // with controlled providers here, not recovered native DLL startup.
        check(v::initialize_04d55c(vstate,vfx));dispatch=std::make_unique<Dispatcher>(deps());recorder=std::make_unique<Recorder>(*dispatch);
    }
    ~Harness(){
        fail_acrt_allocate=false;
        if(pstate.fls_index!=p::invalid_fls_index)check(p::uninitialize_064ecc(pstate,pfx));
        if(vstate.fls_index!=v::invalid_fls_index)check(v::uninitialize_04d5a8(vstate,vfx,0));
        if(locks.acrt_count)check(l::uninitialize_acrt_062614(locks,lock_fx));
        check(memory.blocks.empty()&&system.slots.empty()&&env_fx.allocations.empty());
        f::active_system=nullptr;f::callback_state=nullptr;f::callback_effects=nullptr;f::callback_data=nullptr;f::callback_ids=nullptr;
    }
    bool start(){return q::acrt_initialize_064594(dispatch->pairs(),*recorder);}
    void close(){check(q::acrt_uninitialize_0645cc(false,outer,dispatch->pairs(),*recorder));}
    void seed_consumers(){
        Parser parser;check(av::configure_0638e8(argv,1,argv_fx,parser)==0);
        check(argv.argc==3&&std::string(argv.argv[1])=="alpha"&&std::string(argv.argv[2])=="two words");
        const char block[]="A=own\0B=two\0";const std::span<const char> span(block,sizeof(block));
        environment.initial_0a52a0=ev::build_array_063b54(span,env_fx);environment.current_0a5288=environment.initial_0a52a0;
        initial_wide=ev::build_array_063b54(span,env_fx);current_wide=initial_wide;
        check(env_fx.allocations.size()==6);
    }
};
void full_walk(){
    Harness h;check(h.start());
    const std::vector<Address> expected{id(Id::locale_initialize),id(Id::encoded_initialize),id(Id::thunks_initialize),id(Id::success),
        id(Id::locks_initialize),id(Id::heap_initialize),id(Id::ptd_initialize),id(Id::lowio_initialize),
        id(Id::command_lines_initialize),id(Id::multibyte_initialize),id(Id::success),id(Id::onexit_initialize)};
    check(h.recorder->init==expected&&h.locale.current==h.locale.initial);
    check(h.heap.encoded_handler_0a5178==h.runtime.cookie&&h.encoded.a5178==h.runtime.cookie);
    check(h.onexit.crt_0a52a8.begin==h.runtime.cookie&&h.termination.quick_table_a52c0.begin==h.runtime.cookie);
    check(h.mb.initialized==1&&h.mb.malloc_calls==1&&h.mb.error==12&&h.mb.system_flag==1);
    check(h.data.refs==1&&h.lowio_fx.ensure==1&&h.lowio_fx.inherited==1&&h.lowio_fx.standard==1);
    h.seed_consumers();
    h.close();
    const std::vector<Address> reverse{id(Id::success),id(Id::environment_uninitialize),id(Id::multibyte_uninitialize),id(Id::argv_cleanup),
        id(Id::success),id(Id::success),id(Id::lowio_uninitialize),id(Id::ptd_uninitialize),id(Id::vcrt_uninitialize),
        id(Id::heap_uninitialize),id(Id::locks_uninitialize),id(Id::success),id(Id::thunks_uninitialize)};
    check(h.recorder->uninit==reverse);
    check(h.heap.heap_0a5a10==0&&h.locks.acrt_count==0&&h.locks.vcrt_count==0&&h.argv.argv==nullptr);
    check(h.pstate.fls_index==p::invalid_fls_index&&h.vstate.fls_index==v::invalid_fls_index);
    check(h.data.refs==0xffffffffU&&h.mb.shutdown_atomic==1); // controlled failed-MB-startup branch; not normal native global proof
    check(h.memory.blocks.empty()&&h.env_fx.allocations.empty());
}
void failures(){
    for(unsigned failed:{4U,5U,7U,8U}){
        Harness h;
        if(failed==4)h.platform.result=0;
        if(failed==5)h.heap_init.fail=true;
        if(failed==7)fail_acrt_allocate=true;
        if(failed==8)h.lowio_fx.fail=true;
        check(!h.start());
        std::vector<Address> rollback;
        if(failed>=8)rollback.push_back(id(Id::ptd_uninitialize));
        if(failed>=7)rollback.push_back(id(Id::heap_uninitialize));
        if(failed>=5)rollback.push_back(id(Id::locks_uninitialize));
        rollback.push_back(id(Id::success));rollback.push_back(id(Id::thunks_uninitialize));
        check(h.recorder->uninit==rollback);
        check(std::find(rollback.begin(),rollback.end(),id(Id::vcrt_uninitialize))==rollback.end());
        check(h.vstate.fls_index!=v::invalid_fls_index&&h.locks.vcrt_count==1);
        check(h.pstate.fls_index==p::invalid_fls_index&&h.locks.acrt_count==0&&h.memory.blocks.empty());
        check(h.command_fx.a==0&&h.mb.initialized==0);
    }
}
void domains_arguments_aliases(){
    Harness h;
    const auto thunk=h.dispatch->invoke_boolean_initializer(id(Id::thunks_initialize));
    check((thunk&255)==1&&thunk!=1);
    h.runtime.modules[0].store(0xabcd);h.runtime.modules[1].store(r::missing);
    h.dispatch->invoke_uninitializer(id(Id::thunks_uninitialize),0x101);
    check(h.runtime.modules[0].load()==0xabcd&&h.cache.freed.empty());
    h.dispatch->invoke_uninitializer(id(Id::thunks_uninitialize),0x100);
    check(h.runtime.modules[0].load()==0&&h.runtime.modules[1].load()==0&&h.cache.freed==std::vector<r::Module>{0xabcd});
    rejects([&]{(void)h.dispatch->invoke_boolean_initializer(id(Id::argv_cleanup));});
    rejects([&]{h.dispatch->invoke_uninitializer(id(Id::locale_initialize),0);});
    rejects([&]{(void)h.dispatch->invoke_boolean_initializer(0x180064468ULL);});
    rejects([&]{h.dispatch->invoke_uninitializer(0,0);});
    rejects([&]{h.dispatch->invoke_void_initializer(id(Id::success));});
    rejects([&]{(void)h.dispatch->invoke_int_initializer(id(Id::success));});
    const auto calls=h.platform.enters;
    const auto slots=h.system.slots.size();
    h.heap.cookie_0a1580^=1;rejects([&]{Dispatcher bad(h.deps());});h.heap.cookie_0a1580^=1;
    h.onexit.cookie_0a1580^=1;rejects([&]{Dispatcher bad(h.deps());});h.onexit.cookie_0a1580^=1;
    std::uint64_t other=0;tmn::State badterm(h.onexit,other);rejects([&]{Dispatcher bad(h.deps(&badterm));});
    av::State otherargv;ag::CleanupState badcleanup{otherargv};rejects([&]{Dispatcher bad(h.deps(nullptr,&badcleanup));});
    check(h.platform.enters==calls&&h.system.slots.size()==slots);
    h.dispatch->invoke_uninitializer(id(Id::vcrt_uninitialize),0x101);
    check(h.vstate.fls_index==v::invalid_fls_index&&h.locks.vcrt_count==0);
}
void mutable_and_terminating(){
    {
        Harness h;
        h.command_fx.on_w=[&]{h.dispatch->pairs()[10].initialize=id(Id::success);h.mb.current=h.data.ids.multibyte_info;};
        check(h.start());check(h.mb.initialized==0&&h.mb.malloc_calls==0);
        check(std::find(h.recorder->init.begin(),h.recorder->init.end(),id(Id::multibyte_initialize))==h.recorder->init.end());
        h.recorder->after=[&](Address value){if(value==id(Id::environment_uninitialize))h.dispatch->pairs()[13].uninitialize=id(Id::success);};
        h.close();check(h.mb.shutdown_atomic==0&&h.data.refs==0);
    }
    {
        Harness h;check(h.start());
        h.outer.flush_enabled_0a4ef0=1;const auto slots=h.system.slots.size();
        check(q::acrt_uninitialize_0645cc(true,h.outer,h.dispatch->pairs(),*h.recorder));
        check(h.dispatch->flushes==1&&h.recorder->uninit.empty()&&h.system.slots.size()==slots&&h.locks.acrt_count==14);
        h.close();check(h.heap.heap_0a5a10==0);
    }
}
}
int main(int argc,char** argv){
    if(argc==2&&std::string(argv[1])=="--dump"){
        const auto pairs=pb::rebuilt_pairs();std::cout<<"[";
        for(std::size_t i=0;i<pairs.size();++i){if(i)std::cout<<",";std::cout<<"{\"initialize\":"<<pairs[i].initialize<<",\"uninitialize\":"<<pairs[i].uninitialize<<"}";}
        std::cout<<"]\n";return 0;
    }
    try{full_walk();failures();domains_arguments_aliases();mutable_and_terminating();
        std::cout<<"PASS "<<f::checks<<" typed16-pair source composition checks;4 rollback points;normal reverse/mutable/low8/domain/alias\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}
}
