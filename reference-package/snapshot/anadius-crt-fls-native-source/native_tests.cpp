#include "fls.hpp"
#include "../anadius-crt-heap-native-bridge-source/bridge.hpp"
#include <functional>
#include <iostream>
#include <map>
#include <thread>
#include <stdexcept>
#include <source_location>
#include <cstdlib>
namespace fn=anadius::crt_fls_native;
namespace g=anadius::crt_glue;
namespace p=anadius::crt_ptd_recovered;
namespace d=anadius::crt_ptd_data_recovered;
namespace h=anadius::crt_heap_recovered;
namespace hn=anadius::crt_heap_native;
namespace hb=anadius::crt_heap_native_bridge;
namespace e=anadius::crt_errors_recovered;
namespace {
unsigned checks;
void check(bool v,std::source_location where=std::source_location::current()){++checks;if(!v)throw std::runtime_error("check "+std::to_string(checks)+" at line "+std::to_string(where.line()));}
struct Proxy:e::Effects {e::Effects* target=nullptr;void* optional_ptd_064d08()override{check(target);return target->optional_ptd_064d08();}};
struct HeapPolicy:hb::Policy {
    void lock_0625f8(std::uint32_t)override{throw std::logic_error("unexpected handler lock");}
    void unlock_06264c(std::uint32_t)override{throw std::logic_error("unexpected handler unlock");}
    std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t)override{throw std::logic_error("unexpected handler");}
    void invalid_parameter_05c954()override{throw std::logic_error("unexpected invalid parameter");}
};
struct Abort:g::AbortPolicy{[[noreturn]]void abort_05f03c()override{throw std::runtime_error("unexpected abort");}};
struct Observation {
    std::map<void*,std::size_t> blocks;bool deny_alloc=false,deny_release=false,deny_slot=false,deny_publish=false;
    unsigned frees=0,releases=0;std::function<void()> allocation_hook;
};
Observation* observed=nullptr;
PFLS_CALLBACK_FUNCTION supplied_callback=nullptr;
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T n){
    auto& o=*observed;if(o.allocation_hook){auto call=std::move(o.allocation_hook);call();}
    if(o.deny_alloc)return nullptr;
    auto* value=::HeapAlloc(heap,flags,n);const auto error=::GetLastError();if(value)o.blocks[value]=n;::SetLastError(error);return value;
}
BOOL WINAPI release_heap(HANDLE heap,DWORD flags,LPVOID value){
    auto& o=*observed;check(o.blocks.contains(value));
    const auto result=::HeapFree(heap,flags,value);const auto error=::GetLastError();if(result){o.blocks.erase(value);++o.frees;}::SetLastError(error);return result;
}
DWORD WINAPI allocate_slot(PFLS_CALLBACK_FUNCTION cb){supplied_callback=cb;if(observed->deny_slot)return FLS_OUT_OF_INDEXES;return ::FlsAlloc(cb);}
BOOL WINAPI release_slot(DWORD slot){++observed->releases;if(observed->deny_release){::SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return ::FlsFree(slot);}
BOOL WINAPI set_slot(DWORD slot,PVOID value){if(observed->deny_publish&&value&&value!=p::construction_sentinel()){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}return ::FlsSetValue(slot,value);}
struct Data:d::Effects {
    hb::Effects& heap_fx;h::State& heap;std::uint32_t mb=0,locale=0;std::uint64_t exceptions=0;
    d::Identities ids{reinterpret_cast<d::Address>(&exceptions),reinterpret_cast<d::Address>(&mb),reinterpret_cast<d::Address>(&locale)};
    std::function<void()> construct_hook;unsigned null_frees=0;bool locked=false,throw_cleanup=false;
    Data(hb::Effects& fx,h::State& state):heap_fx(fx),heap(state){}
    void lock_0625f8(std::uint32_t i)override{check((i==4||i==5)&&!locked);locked=true;}
    void unlock_06264c(std::uint32_t i)override{check((i==4||i==5)&&locked);locked=false;}
    std::uint32_t atomic_fetch_add_32(d::Address a,std::uint32_t n)override{check(a==ids.multibyte_info&&locked);auto old=mb;mb+=n;return old;}
    void free_0668c0(d::Address a)override{if(throw_cleanup)throw std::runtime_error("injected cleanup throw");if(!a)++null_frees;h::free_0668c0(reinterpret_cast<void*>(a),heap,heap_fx);}
    d::Address current_locale_0a52e0()override{return ids.initial_locale;}
    void release_locale_ref_0708c0(d::Address a)override{check(locked&&a==ids.initial_locale&&locale);--locale;}
    std::uint32_t locale_ref_count_10(d::Address)override{throw std::logic_error("unexpected locale count");}
    void free_locale_0706c0(d::Address)override{throw std::logic_error("unexpected deep locale free");}
    void add_locale_ref_070634(d::Address a)override{check(locked&&a==ids.initial_locale);++locale;if(construct_hook){auto call=std::move(construct_hook);call();}}
};
struct Fixture {
    bool skip_exit_checks=false;
    Observation observation;Proxy proxy;e::State errors;HeapPolicy heap_policy;
    hn::Calls api{hn::Api{::GetProcessHeap,allocate,::HeapReAlloc,release_heap,::HeapSize,::GetLastError}};
    hb::Effects heap_fx{api,errors,proxy,heap_policy};h::State heap{0,0,0,0};Data data{heap_fx,heap};Abort abort;
    fn::Binding binding{heap,heap_fx,data.ids,data,abort,g::FlsApi{::FlsGetValue,set_slot,allocate_slot,release_slot,::GetLastError,::SetLastError}};
    g::ErrorPtd error_ptd{binding.state(),binding};
    Fixture(){check(observed==nullptr);observed=&observation;proxy.target=&error_ptd;check(h::initialize_heap_0704cc(heap,heap_fx));}
    ~Fixture(){if(!skip_exit_checks)check(observation.blocks.empty()&&data.mb==0&&data.locale==0&&!data.locked);observed=nullptr;}
};
void lifecycle(){
    Fixture f;auto& state=f.binding.state();
    bool duplicate=false;try{fn::Binding second(f.heap,f.heap_fx,f.data.ids,f.data,f.abort);}catch(const std::logic_error&){duplicate=true;}check(duplicate);
    f.data.construct_hook=[&]{auto* published=p::optional_preserve_error_064d08(state,f.binding);check(published&&d::View(published).load<std::uint32_t>(0x28)==1);};
    check(p::initialize_064e90(state,f.binding));::SetLastError(0xa1234567);
    auto* first=p::required_preserve_error_064b90(state,f.binding);check(::GetLastError()==0xa1234567);check(first&&f.data.mb==1&&f.data.locale==1&&f.observation.blocks.size()==1);
    bool rejected=false;try{(void)p::initialize_064e90(state,f.binding);}catch(const std::logic_error&){rejected=true;}check(rejected&&state.fls_index==f.binding.native_index());
    rejected=false;try{(void)f.binding.retry_release();}catch(const std::logic_error&){rejected=true;}check(rejected);
    const auto releases=f.observation.releases;rejected=false;try{(void)f.binding.fls_free(p::invalid_fls_index);}catch(const std::logic_error&){rejected=true;}check(rejected&&f.observation.releases==releases);
    check(p::required_preserve_error_064b90(state,f.binding)==first&&::FlsGetValue(state.fls_index)==first);
    const d::View view(first);check(view.load<d::Address>(0)==f.data.ids.exception_table&&view.load<std::uint32_t>(0x3a8)==1&&view.load<std::uint16_t>(0xbc)==0x43);
    auto* context=h::calloc_base_066848(1,sizeof(anadius::threads_recovered::Context),f.heap,f.heap_fx);check(context);
    first->thread_context=std::construct_at(static_cast<anadius::threads_recovered::Context*>(context));
    p::free_ptd_064b4c(state,f.binding);check(f.observation.blocks.empty()&&::FlsGetValue(state.fls_index)==nullptr&&f.data.mb==0&&f.data.locale==0);
    auto* again=p::required_preserve_error_064b90(state,f.binding);check(again&&f.observation.blocks.size()==1);
    check(p::uninitialize_064ecc(state,f.binding)&&state.fls_index==p::invalid_fls_index&&f.binding.native_index()==p::invalid_fls_index);
    check(f.observation.blocks.empty()&&f.data.null_frees==17);
}
void failure_paths(){
    Fixture f;auto& state=f.binding.state();f.observation.deny_slot=true;check(!p::initialize_064e90(state,f.binding)&&f.binding.native_index()==p::invalid_fls_index);f.observation.deny_slot=false;
    f.observation.deny_alloc=true;f.observation.allocation_hook=[&]{check(p::optional_preserve_error_064d08(state,f.binding)==nullptr);};
    ::SetLastError(0x98765432);check(!p::initialize_064e90(state,f.binding)&&::GetLastError()==0x98765432&&f.errors.errno_fallback==12&&f.observation.blocks.empty());
    f.observation.deny_alloc=false;f.observation.deny_publish=true;check(!p::initialize_064e90(state,f.binding)&&f.observation.blocks.empty());f.observation.deny_publish=false;
    check(p::initialize_064e90(state,f.binding));const auto owned=f.binding.native_index();
    f.observation.deny_release=true;check(p::uninitialize_064ecc(state,f.binding)&&state.fls_index==p::invalid_fls_index&&f.binding.native_index()==owned&&f.observation.blocks.size()==1);
    bool blocked=false;try{(void)p::initialize_064e90(state,f.binding);}catch(const std::logic_error&){blocked=true;}check(blocked&&f.binding.native_index()==owned);
    check(!f.binding.retry_release()&&f.observation.blocks.size()==1);f.observation.deny_release=false;check(f.binding.retry_release()&&f.binding.native_index()==p::invalid_fls_index&&f.observation.blocks.empty());
    check(p::initialize_064e90(state,f.binding));check(p::uninitialize_064ecc(state,f.binding));
}
void thread_exit(){
    Fixture f;check(p::initialize_064e90(f.binding.state(),f.binding));auto* main_ptd=p::required_preserve_error_064b90(f.binding.state(),f.binding);
    bool success=false;
    std::thread worker([&]{try{::SetLastError(0x1234abcd);auto* own=p::required_preserve_error_064b90(f.binding.state(),f.binding);check(own&&own!=main_ptd&&::GetLastError()==0x1234abcd);check(f.observation.blocks.size()==2&&f.data.mb==2);e::errno_cell_05d724(f.errors,f.error_ptd).store(77);check(e::errno_cell_05d724(f.errors,f.error_ptd).load()==77);success=true;}catch(...){success=false;}});
    worker.join();check(success&&f.observation.blocks.size()==1&&f.data.mb==1&&f.data.locale==1);
    check(p::required_preserve_error_064b90(f.binding.state(),f.binding)==main_ptd&&e::errno_cell_05d724(f.errors,f.error_ptd).load()==0);
    check(p::uninitialize_064ecc(f.binding.state(),f.binding)&&f.observation.blocks.empty());
}
}
int main(int argc,char** argv){try{
    // Hard exit of this own test process avoids ExitProcess/FLS rundown
    // reentering the deliberately invalid callback/dependency lifetime.
    if(argc==2){std::set_terminate([]{::TerminateProcess(::GetCurrentProcess(),88);std::abort();});Fixture f;f.skip_exit_checks=true;check(p::initialize_064e90(f.binding.state(),f.binding));const std::string mode=argv[1];
        if(mode=="--open-binding")return 0;
        if(mode=="--callback-sentinel"){supplied_callback(p::construction_sentinel());return 2;}
        if(mode=="--callback-throw"){f.data.throw_cleanup=true;(void)p::uninitialize_064ecc(f.binding.state(),f.binding);return 2;}
        return 3;
    }
    lifecycle();failure_paths();thread_exit();std::cout<<"PASS "<<checks<<" native FLS/PTD/heap lifecycle checks\n";
}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
