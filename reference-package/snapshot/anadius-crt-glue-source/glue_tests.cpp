#include "crt_glue.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
namespace g=anadius::crt_glue;
namespace h=anadius::crt_heap_recovered;
namespace p=anadius::crt_ptd_recovered;
namespace d=anadius::crt_ptd_data_recovered;
namespace e=anadius::crt_errors_recovered;
namespace i=anadius::crt_invalid_recovered;
namespace r=anadius::crt_runtime_recovered;
namespace t=anadius::threads_recovered;
namespace {
unsigned checks;
void check(bool v){++checks;if(!v)throw std::runtime_error("check "+std::to_string(checks));}
struct Exit{DWORD code;bool module;};
struct Terminal:std::runtime_error{using std::runtime_error::runtime_error;};
struct Proxy final:e::Effects {
    e::Effects* target=nullptr;
    void* optional_ptd_064d08() override {check(target!=nullptr);return target->optional_ptd_064d08();}
};
struct Memory final:g::HeapErrors {
    std::map<void*,std::size_t> blocks;
    std::vector<std::size_t> requests;
    std::vector<void*> freed;
    bool fail_alloc=false,fail_free=false;
    DWORD error=0xdeadbeef;
    Memory(e::State& s,e::Effects& fx):HeapErrors(s,fx){}
    ~Memory(){for(auto [ptr,size]:blocks){(void)size;::operator delete(ptr);}}
    h::Heap get_process_heap() override{return 0xabc;}
    void* heap_alloc(h::Heap heap,std::uint32_t flags,std::uint64_t bytes) override{
        check(heap==0xabc && flags==8);requests.push_back(static_cast<std::size_t>(bytes));
        if(fail_alloc){error=8;return nullptr;}
        auto* ptr=::operator new(static_cast<std::size_t>(bytes));std::memset(ptr,0,static_cast<std::size_t>(bytes));
        blocks.emplace(ptr,static_cast<std::size_t>(bytes));return ptr;
    }
    bool heap_free(h::Heap heap,std::uint32_t flags,void* ptr) override{
        check(heap==0xabc && flags==0);check(blocks.contains(ptr));
        if(fail_free){error=5;return false;}
        freed.push_back(ptr);blocks.erase(ptr);::operator delete(ptr);return true;
    }
    std::uint32_t last_error() override{return error;}
    void lock_0625f8(std::uint32_t) override{check(false);}
    void unlock_06264c(std::uint32_t) override{check(false);}
    std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t) override{check(false);return 0;}
};
struct Data final:d::Effects {
    h::State& heap;Memory& memory;const d::Identities ids{0x1110,0x2220,0x3330};
    std::uint32_t mb_refs=0;
    Data(h::State& s,Memory& m):heap(s),memory(m){}
    void lock_0625f8(std::uint32_t id) override{check(id==4||id==5);}
    void unlock_06264c(std::uint32_t id) override{check(id==4||id==5);}
    std::uint32_t atomic_fetch_add_32(d::Address a,std::uint32_t n) override{
        check(a==ids.multibyte_info);const auto old=mb_refs;mb_refs+=n;return old;
    }
    void free_0668c0(d::Address a) override{h::free_0668c0(reinterpret_cast<void*>(a),heap,memory);}
    d::Address current_locale_0a52e0() override{return ids.initial_locale;}
    void release_locale_ref_0708c0(d::Address a) override{check(a==ids.initial_locale);}
    std::uint32_t locale_ref_count_10(d::Address) override{check(false);return 0;}
    void free_locale_0706c0(d::Address) override{check(false);}
    void add_locale_ref_070634(d::Address a) override{check(a==ids.initial_locale);}
};
struct Abort final:g::AbortPolicy {[[noreturn]]void abort_05f03c() override{throw Terminal("abort");}};
struct FakeSystem {
    Memory* memory;Data* data;
    PVOID fls_value=nullptr;PFLS_CALLBACK_FUNCTION callback=nullptr;
    DWORD error=0x12345678,index=9,id=0x87654321;
    bool create_fails=false,run_inline=true;
    DWORD result=0;unsigned module_exit=0,plain_exit=0,module_release=0,close=0;
    void* created_context=nullptr;
};
FakeSystem* system_fixture;
g::ThreadEffects* active_thread;
VOID WINAPI fls_destructor(PVOID value){d::fls_destroy_0649cc(value,system_fixture->data->ids,*system_fixture->data);}
PVOID WINAPI fls_get(DWORD index){check(index==system_fixture->index);return system_fixture->fls_value;}
BOOL WINAPI fls_set(DWORD index,PVOID value){check(index==system_fixture->index);system_fixture->fls_value=value;system_fixture->error=0xaaaa;return TRUE;}
DWORD WINAPI fls_alloc(PFLS_CALLBACK_FUNCTION callback){system_fixture->callback=callback;return system_fixture->index;}
void thread_cleanup(){auto* value=system_fixture->fls_value;system_fixture->fls_value=nullptr;if(value)system_fixture->callback(value);}
BOOL WINAPI fls_free(DWORD index){check(index==system_fixture->index);thread_cleanup();return TRUE;}
DWORD WINAPI get_error(){return system_fixture->error;}
VOID WINAPI set_error(DWORD value){system_fixture->error=value;}
g::FlsApi fls_api(){return{fls_get,fls_set,fls_alloc,fls_free,get_error,set_error};}
DWORD WINAPI thread_wrapper(PVOID context){t::thread_entry_05e7a8(static_cast<t::Context*>(context),*active_thread);}
unsigned __stdcall application(void* payload){check(payload==reinterpret_cast<void*>(0x777));return 0xf1234567;}
BOOL WINAPI get_module(DWORD flags,LPCWSTR address,HMODULE* out){
    check(flags==4 && reinterpret_cast<std::uintptr_t>(address)==reinterpret_cast<std::uintptr_t>(application));
    *out=reinterpret_cast<HMODULE>(0x555);return TRUE;
}
HANDLE WINAPI create_thread(LPSECURITY_ATTRIBUTES security,SIZE_T stack,LPTHREAD_START_ROUTINE start,PVOID payload,DWORD flags,LPDWORD id){
    check(!security && stack==0 && flags==0 && start==thread_wrapper);
    system_fixture->created_context=payload;
    if(system_fixture->create_fails){system_fixture->error=5;return nullptr;}
    *id=system_fixture->id;
    if(system_fixture->run_inline){try{(void)start(payload);check(false);}catch(const Exit& end){system_fixture->result=end.code;thread_cleanup();}}
    return reinterpret_cast<HANDLE>(0x999);
}
BOOL WINAPI close_handle(HANDLE handle){check(handle==reinterpret_cast<HANDLE>(0x999));++system_fixture->close;return TRUE;}
BOOL WINAPI release_module(HMODULE handle){check(handle==reinterpret_cast<HMODULE>(0x555));++system_fixture->module_release;return TRUE;}
DECLSPEC_NORETURN VOID WINAPI release_exit(HMODULE handle,DWORD code){check(handle==reinterpret_cast<HMODULE>(0x555));++system_fixture->module_exit;throw Exit{code,true};}
DECLSPEC_NORETURN VOID WINAPI exit_thread(DWORD code){++system_fixture->plain_exit;throw Exit{code,false};}
anadius::thread_native::Api thread_api(){return{get_module,create_thread,get_error,close_handle,release_module,release_exit,exit_thread};}
struct Runtime final:r::Effects{
    unsigned init=0,uninit=0;
    r::Module load_library_ex_w(std::u16string_view,r::Module file,std::uint32_t flags) override{check(!file&&flags==0x800);return 0x9999;}
    std::uint32_t last_error() override{check(false);return 0;}
    bool free_library(r::Module) override{check(false);return false;}
    r::Address get_proc_address(r::Module m,std::string_view name) override{check(m==0x9999);return name=="RoInitialize"?1:name=="RoUninitialize"?2:3;}
    std::uint32_t process_parameters_flags() override{return 0;}
    std::int32_t call_ro_initialize(r::Address a,std::uint32_t mode) override{check(a==1&&mode==1);++init;return 0;}
    void call_ro_uninitialize(r::Address a) override{check(a==2);++uninit;}
    std::uint32_t call_thread_initialization_policy(r::Address a,std::uintptr_t token,std::int32_t* out) override{check(a==3&&token==std::uintptr_t(-6));*out=1;return 0;}
};
struct Invalid final:g::InvalidPtd{
    unsigned calls=0;
    Invalid(p::State& s,p::Effects& fx):InvalidPtd(s,fx){}
    std::uint32_t locale_changed_0a52d8() override{return 0;}
    std::array<i::Address,2> locale_pair_0a1938() override{return {0x111,0x222};}
    i::Address encoded_global_handler_0a4f00(std::uint64_t index) override{check(!index);return 0x1111;}
    i::Address security_cookie_0a1580() override{return 0;}
    void invoke_handler(i::Address handler,const i::Arguments& args) override{check(handler==0x1111&&args==i::Arguments{});++calls;}
    [[noreturn]]void watson_05c9a4(const i::Arguments&) override{throw Terminal("Watson");}
    [[noreturn]]void missing_ptd_05f03c() override{throw Terminal("missing PTD");}
};
struct Environment {
    e::State errors;Proxy proxy;Memory memory{errors,proxy};h::State heap{0,0,0,0};Data data{heap,memory};Abort abort;
    FakeSystem system{&memory,&data};p::State ptd;
    g::PtdEffects ptd_effects{fls_api(),fls_destructor,heap,memory,data.ids,data,abort};
    g::ErrorPtd error_ptd{ptd,ptd_effects};Invalid invalid{ptd,ptd_effects};
    r::State runtime_state{0x1234567812345678};Runtime runtime;
    g::ThreadEffects thread{thread_wrapper,thread_api(),runtime_state,runtime,errors,error_ptd,{ptd,ptd_effects,heap,memory,invalid}};
    Environment(){system_fixture=&system;active_thread=&thread;proxy.target=&error_ptd;check(h::initialize_heap_0704cc(heap,memory));}
    ~Environment(){system_fixture=&system;if(ptd.fls_index!=p::invalid_fls_index)(void)p::uninitialize_064ecc(ptd,ptd_effects);}
    void initialize(){check(p::initialize_064e90(ptd,ptd_effects));check(system.callback==fls_destructor);}
};
void successful_worker(){
    Environment env;env.initialize();check(env.system.error==0x12345678);check(env.data.mb_refs==1);
    p::free_ptd_064b4c(env.ptd,env.ptd_effects);check(env.memory.blocks.empty()&&env.data.mb_refs==0);
    for(unsigned n=0;n<3;++n){std::uint32_t id=0;
        const auto handle=t::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(application),0x777,0,&id,env.thread);
        check(handle==0x999&&id==env.system.id&&env.system.result==0xf1234567);
        check(env.memory.blocks.empty()&&!env.system.fls_value&&env.data.mb_refs==0);
        check(std::find(env.memory.freed.begin(),env.memory.freed.end(),env.system.created_context)!=env.memory.freed.end());
    }
    check(env.runtime.init==3&&env.runtime.uninit==3&&env.system.module_exit==3&&env.system.plain_exit==0);
    check(std::count(env.memory.requests.begin(),env.memory.requests.end(),sizeof(t::Context))==3);
    check(std::count(env.memory.requests.begin(),env.memory.requests.end(),sizeof(p::Ptd))==4);
}
void failure_paths(){
    Environment env;env.initialize();env.system.create_fails=true;std::uint32_t id=0x44444444;
    check(t::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(application),0x777,0,&id,env.thread)==0);
    check(id==0x44444444&&env.system.module_release==1);
    auto* ptd=static_cast<p::Ptd*>(env.system.fls_value);d::View view(ptd);
    check(view.load<std::uint32_t>(0x20)==13&&view.load<std::uint32_t>(0x24)==5);
    check(t::begin_thread_05e8e0(nullptr,0,0,0,0,nullptr,env.thread)==0);
    check(env.invalid.calls==1&&view.load<std::uint32_t>(0x20)==22&&view.load<std::uint32_t>(0x24)==5);
    std::uint32_t saved=0x11223344;
    check(env.invalid.optional_ptd_064dd0(&saved,0)==ptd&&saved==0x11223344);
    try{(void)env.invalid.optional_ptd_064dd0(&saved,1);check(false);}catch(const std::out_of_range&){check(true);}
    auto* block=h::calloc_base_066848(1,16,env.heap,env.memory);env.memory.fail_free=true;
    h::free_0668c0(block,env.heap,env.memory);check(view.load<std::uint32_t>(0x20)==13&&view.load<std::uint32_t>(0x24)==5);
    env.memory.fail_free=false;h::free_0668c0(block,env.heap,env.memory);
    p::free_ptd_064b4c(env.ptd,env.ptd_effects);
    // Allocation failure recursively requests errno/PTD once, then the -1 FLS
    // construction sentinel forces fallback instead of unbounded recursion.
    env.memory.fail_alloc=true;const auto before=env.memory.requests.size();
    check(env.thread.calloc_context_066848()==nullptr);
    check(env.memory.requests.size()==before+2&&!env.system.fls_value);
    check(env.errors.errno_fallback==12&&env.errors.dos_fallback==8);
}
void composition_guards(){
    Environment env;
    const auto native=g::native_fls_api();check(native.get==&::FlsGetValue&&native.allocate==&::FlsAlloc&&native.set_error==&::SetLastError);
    for(unsigned n=0;n<7;++n){auto api=fls_api();auto cb=fls_destructor;
        switch(n){case 0:api.get=nullptr;break;case 1:api.set=nullptr;break;case 2:api.allocate=nullptr;break;case 3:api.release=nullptr;break;case 4:api.error=nullptr;break;case 5:api.set_error=nullptr;break;default:cb=nullptr;}
        try{g::PtdEffects bad(api,cb,env.heap,env.memory,env.data.ids,env.data,env.abort);check(false);}catch(const std::invalid_argument&){check(true);}
    }
    try{(void)env.ptd_effects.fls_alloc(static_cast<p::FlsCallback>(99));check(false);}catch(const std::invalid_argument&){check(true);}
}
}
int main(){try{successful_worker();failure_paths();composition_guards();std::cout<<"PASS "<<checks<<" integrated CRT/PTD/FLS/heap/thread checks\n";}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
