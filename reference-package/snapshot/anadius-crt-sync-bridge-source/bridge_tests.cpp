#include "bridge.hpp"
#include "../anadius-crt-fls-native-source/fls.hpp"
#include "../anadius-crt-heap-native-bridge-source/bridge.hpp"
#include <barrier>
#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>
#include <source_location>
#include <stdexcept>
#include <thread>
#include <vector>
namespace sb=anadius::crt_sync_bridge;
namespace ln=anadius::crt_lock_native;
namespace an=anadius::crt_atomic_native;
namespace l=anadius::crt_locks_recovered;
namespace r=anadius::crt_runtime_recovered;
namespace va=anadius::vcrt_api_recovered;
namespace vb=anadius::vcrt_bridge;
namespace fn=anadius::crt_fls_native;
namespace p=anadius::crt_ptd_recovered;
namespace d=anadius::crt_ptd_data_recovered;
namespace g=anadius::crt_glue;
namespace hn=anadius::crt_heap_native;
namespace hb=anadius::crt_heap_native_bridge;
namespace h=anadius::crt_heap_recovered;
namespace e=anadius::crt_errors_recovered;
namespace {
std::atomic<unsigned> checks{0};
void check(bool v,std::source_location where=std::source_location::current()){const auto n=checks.fetch_add(1)+1;if(!v)throw std::runtime_error("check "+std::to_string(n)+" at line "+std::to_string(where.line()));}
struct Loader final:r::Effects,sb::VcrtLockCalls {
    std::vector<HMODULE> owned;unsigned lookups=0;bool missing=false;
    Loader(ln::Banks& banks,bool fallback):VcrtLockCalls(banks),missing(fallback){owned.reserve(8);}
    ~Loader(){const auto error=::GetLastError();for(auto value:owned)(void)::FreeLibrary(value);::SetLastError(error);}
    r::Module load_library_ex_w(std::u16string_view name,r::Module file,std::uint32_t flags)override{
        check(!file&&(name==u"api-ms-win-core-synch-l1-2-0"||name==u"kernel32"));
        const auto module=::LoadLibraryExW(reinterpret_cast<LPCWSTR>(name.data()),nullptr,flags);const auto error=::GetLastError();if(module)owned.push_back(module);::SetLastError(error);return reinterpret_cast<r::Module>(module);
    }
    std::uint32_t last_error()override{return ::GetLastError();}
    bool free_library(r::Module value)override{const auto module=reinterpret_cast<HMODULE>(value);const auto it=std::find(owned.begin(),owned.end(),module);check(it!=owned.end());const auto result=::FreeLibrary(module);const auto error=::GetLastError();owned.erase(it);::SetLastError(error);return result!=0;}
    r::Address get_proc_address(r::Module module,std::string_view name)override{check(name=="InitializeCriticalSectionEx");++lookups;if(missing){::SetLastError(ERROR_PROC_NOT_FOUND);return 0;}return reinterpret_cast<r::Address>(::GetProcAddress(reinterpret_cast<HMODULE>(module),name.data()));}
    std::uint32_t process_parameters_flags()override{throw std::logic_error("no PEB policy");}
    std::int32_t call_ro_initialize(r::Address,std::uint32_t)override{throw std::logic_error("no RO call");}
    void call_ro_uninitialize(r::Address)override{throw std::logic_error("no RO call");}
    std::uint32_t call_thread_initialization_policy(r::Address,std::uintptr_t,std::int32_t*)override{throw std::logic_error("no thread policy");}
    std::uint32_t call_fls_alloc(va::Address,va::FlsCallback)override{throw std::logic_error("lock-only VCRT fixture");}
    std::int32_t call_fls_free(va::Address,std::uint32_t)override{throw std::logic_error("lock-only VCRT fixture");}
    void* call_fls_get_value(va::Address,std::uint32_t)override{throw std::logic_error("lock-only VCRT fixture");}
    std::int32_t call_fls_set_value(va::Address,std::uint32_t,void*)override{throw std::logic_error("lock-only VCRT fixture");}
    std::uint32_t tls_alloc()override{throw std::logic_error("lock-only TLS fixture");}
    std::int32_t tls_free(std::uint32_t)override{throw std::logic_error("lock-only TLS fixture");}
    void* tls_get_value(std::uint32_t)override{throw std::logic_error("lock-only TLS fixture");}
    std::int32_t tls_set_value(std::uint32_t,void*)override{throw std::logic_error("lock-only TLS fixture");}
};
struct Cells final:sb::AtomicCells {
    an::Cell32 multibyte{0},locale{0};
    an::Cell32& resolve(d::Address a)override{if(a==multibyte.address())return multibyte;if(a==locale.address())return locale;throw std::domain_error("unknown native atomic identity");}
};
struct Proxy:e::Effects {e::Effects* target=nullptr;void* optional_ptd_064d08()override{check(target);return target->optional_ptd_064d08();}};
struct HeapPolicy:hb::Policy {
    l::Effects& locks;explicit HeapPolicy(l::Effects& fx):locks(fx){}
    void lock_0625f8(std::uint32_t i)override{l::lock_acrt_0625f8(locks,static_cast<std::int32_t>(i));}
    void unlock_06264c(std::uint32_t i)override{l::unlock_acrt_06264c(locks,static_cast<std::int32_t>(i));}
    std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t)override{throw std::logic_error("unexpected handler");}
    void invalid_parameter_05c954()override{throw std::logic_error("unexpected invalid parameter");}
};
struct Observation {
    std::mutex mutex;std::map<void*,std::size_t> blocks;
    std::size_t count(){std::lock_guard guard(mutex);return blocks.size();}
};
Observation* observed=nullptr;
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T n){auto* result=::HeapAlloc(heap,flags,n);const auto error=::GetLastError();if(result){std::lock_guard guard(observed->mutex);observed->blocks[result]=n;}::SetLastError(error);return result;}
BOOL WINAPI release_heap(HANDLE heap,DWORD flags,LPVOID p){BOOL result;DWORD error;{std::lock_guard guard(observed->mutex);check(observed->blocks.contains(p));result=::HeapFree(heap,flags,p);error=::GetLastError();if(result)observed->blocks.erase(p);}::SetLastError(error);return result;}
struct Abort:g::AbortPolicy {[[noreturn]]void abort_05f03c()override{throw std::runtime_error("unexpected abort");}};
struct Data final:sb::PtdSyncEffects {
    Cells& cells;h::State& heap;hb::Effects& heap_fx;std::uint64_t exceptions=0;
    d::Identities ids;
    Data(l::Effects& locks,Cells& c,h::State& s,hb::Effects& fx):PtdSyncEffects(locks,c),cells(c),heap(s),heap_fx(fx),ids{reinterpret_cast<d::Address>(&exceptions),c.multibyte.address(),c.locale.address()}{}
    void free_0668c0(d::Address a)override{h::free_0668c0(reinterpret_cast<void*>(a),heap,heap_fx);}
    d::Address current_locale_0a52e0()override{return ids.initial_locale;}
    void release_locale_ref_0708c0(d::Address a)override{check(a==ids.initial_locale&&cells.locale.fetch_add(UINT32_MAX)>0);}
    std::uint32_t locale_ref_count_10(d::Address a)override{check(a==ids.initial_locale);return cells.locale.load();}
    void free_locale_0706c0(d::Address)override{throw std::logic_error("no dynamic locale in fixture");}
    void add_locale_ref_070634(d::Address a)override{check(a==ids.initial_locale);(void)cells.locale.fetch_add(1);}
};
struct Fixture {
    ln::Banks banks;r::State runtime{0x1234};va::State vapi;Loader loader;
    vb::LockEffects lock_fx{runtime,loader,banks,vapi,loader,banks};l::State locks;
    Cells cells;Proxy proxy;e::State errors;HeapPolicy heap_policy{lock_fx};Observation observation;
    hn::Calls heap_api{hn::Api{::GetProcessHeap,allocate,::HeapReAlloc,release_heap,::HeapSize,::GetLastError}};
    hb::Effects heap_fx{heap_api,errors,proxy,heap_policy};h::State heap{0,0,0,0};Data data{lock_fx,cells,heap,heap_fx};Abort abort;
    fn::Binding binding{heap,heap_fx,data.ids,data,abort};g::ErrorPtd error_ptd{binding.state(),binding};
    explicit Fixture(bool fallback):loader(banks,fallback){
        check(observed==nullptr);observed=&observation;proxy.target=&error_ptd;
        check(l::initialize_acrt_0625b0(locks,lock_fx)&&l::initialize_vcrt_05093c(locks,lock_fx));check(locks.acrt_count==14&&locks.vcrt_count==1&&loader.lookups==2);
        check(h::initialize_heap_0704cc(heap,heap_fx)&&p::initialize_064e90(binding.state(),binding));
    }
    void close(){check(p::uninitialize_064ecc(binding.state(),binding));check(observation.count()==0&&cells.multibyte.load()==0&&cells.locale.load()==0);check(l::uninitialize_vcrt_050984(locks,lock_fx)&&l::uninitialize_acrt_062614(locks,lock_fx));}
    ~Fixture(){check(binding.native_index()==p::invalid_fls_index&&locks.acrt_count==0&&locks.vcrt_count==0);observed=nullptr;}
};
void parallel_ptd(bool fallback){
    Fixture f(fallback);auto* main_ptd=p::required_preserve_error_064b90(f.binding.state(),f.binding);check(main_ptd);
    constexpr unsigned worker_count=4;std::barrier ready(worker_count+1),release(worker_count+1);
    std::array<std::uintptr_t,worker_count> pointers{};std::array<bool,worker_count> success{};std::vector<std::thread> workers;
    for(unsigned i=0;i<worker_count;++i)workers.emplace_back([&,i]{
        try{::SetLastError(0xabc00000+i);auto* value=p::required_preserve_error_064b90(f.binding.state(),f.binding);check(value&&value!=main_ptd&&::GetLastError()==0xabc00000+i);pointers[i]=reinterpret_cast<std::uintptr_t>(value);e::errno_cell_05d724(f.errors,f.error_ptd).store(100+i);check(e::errno_cell_05d724(f.errors,f.error_ptd).load()==100+i);success[i]=true;}catch(...){success[i]=false;}
        ready.arrive_and_wait();release.arrive_and_wait();
    });
    ready.arrive_and_wait();
    const bool at_peak=f.observation.count()==worker_count+1&&f.cells.multibyte.load()==worker_count+1&&f.cells.locale.load()==worker_count+1;
    release.arrive_and_wait();for(auto& thread:workers)thread.join();
    check(at_peak);for(unsigned i=0;i<worker_count;++i){check(success[i]&&pointers[i]);for(unsigned j=0;j<i;++j)check(pointers[i]!=pointers[j]);}
    check(f.observation.count()==1&&f.cells.multibyte.load()==1&&f.cells.locale.load()==1);
    check(p::required_preserve_error_064b90(f.binding.state(),f.binding)==main_ptd&&e::errno_cell_05d724(f.errors,f.error_ptd).load()==0);
    const auto acrt=r::decode_pointer(f.runtime.procedures[15].load(),f.runtime.cookie);const auto vcrt=f.vapi.procedures[4].load();
    check(fallback?acrt==r::missing&&vcrt==va::missing:acrt&&acrt!=r::missing&&vcrt&&vcrt!=va::missing);
    f.close();
}
unsigned init_attempts=0,fail_at=0;
BOOL WINAPI partial_spin(LPCRITICAL_SECTION section,DWORD spin){
    if(++init_attempts==fail_at)return FALSE;
    return ::InitializeCriticalSectionAndSpinCount(section,spin);
}
void partial_bank_failure(){
    auto api=ln::native_api();api.initialize_spin=partial_spin;ln::Banks banks(api);
    r::State runtime{0x1234};va::State vapi;Loader loader(banks,true);
    vb::LockEffects effects(runtime,loader,banks,vapi,loader,banks);l::State state;
    init_attempts=0;fail_at=5;
    check(!l::initialize_acrt_0625b0(state,effects)&&init_attempts==5&&state.acrt_count==0);
    for(unsigned i=0;i<14;++i)check(!banks.initialized({l::Bank::acrt,i}));
    init_attempts=0;fail_at=1;
    check(!l::initialize_vcrt_05093c(state,effects)&&init_attempts==1&&state.vcrt_count==0&&!banks.initialized({l::Bank::vcrt,0}));
}
}
int main(){try{parallel_ptd(false);parallel_ptd(true);partial_bank_failure();std::cout<<"PASS "<<checks.load()<<" native lock/Interlocked/FLS parallel source integration checks\n";}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
