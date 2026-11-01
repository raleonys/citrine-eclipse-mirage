#include "native.hpp"
#include "../anadius-crt-sync-bridge-source/bridge.hpp"
#include <memory>
#include <cstring>
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
namespace nl=anadius::crt_locale_native;
namespace cn=anadius::crt_counter_native;
namespace lb=anadius::crt_locale_bridge;
namespace lt=anadius::crt_locale_thread_recovered;
namespace lr=anadius::crt_locale_ref_recovered;
namespace lf=anadius::crt_locale_free_recovered;
using Address=std::uint64_t;
template<class T>Address address(T* p){return reinterpret_cast<Address>(p);}
struct Category {Address narrow{},name{},narrow_ref{},name_ref{};};
struct Locale {std::array<std::byte,0x10> prefix{};LONG refs{};std::uint32_t pad{};Address reserved{};std::array<Category,6> categories{};Address lconv_ref{},numeric_ref{},monetary_ref{},lconv{},ctype_ref{},ctype{},lower{},upper{},time{};std::array<Address,6> extras{};std::array<std::byte,8> tail{};};
struct Time {std::array<Address,43> before{};std::uint32_t field{};LONG refs{};std::array<Address,44> after{};};
struct Lconv {std::array<Address,19> pointers{};};
static_assert(sizeof(Category)==0x20&&sizeof(Locale)==0x160&&offsetof(Locale,refs)==0x10&&offsetof(Locale,categories)==0x20&&offsetof(Locale,lconv_ref)==0xe0&&offsetof(Locale,time)==0x120&&offsetof(Locale,extras)==0x128);
static_assert(sizeof(Time)==0x2c0&&offsetof(Time,refs)==0x15c&&offsetof(Time,after)==0x160&&sizeof(Lconv)==0x98);
struct Memory final:lb::Memory {
    struct Block {std::size_t extent;std::vector<LONG*> refs;};
    cn::Counters& counters;std::mutex mutex;std::map<void*,Block> blocks;std::map<Address,std::size_t> borrowed;unsigned freed=0;
    explicit Memory(cn::Counters& c):counters(c){}
    template<class T>void borrow(T& value){borrowed[address(&value)]=sizeof(T);}
    void bind(LONG& value){counters.bind(value);const auto a=address(&value);for(auto& [p,b]:blocks){const auto begin=address(p);if(b.extent>=sizeof(LONG)&&a>=begin&&a-begin<=b.extent-sizeof(LONG)){b.refs.push_back(&value);return;}}}
    void* resolve(Address a,std::size_t n){for(auto& [p,b]:blocks){const auto begin=address(p);if(a>=begin&&n<=b.extent&&a-begin<=b.extent-n)return reinterpret_cast<std::byte*>(p)+a-begin;}for(auto [begin,size]:borrowed)if(a>=begin&&n<=size&&a-begin<=size-n)return reinterpret_cast<std::byte*>(begin)+a-begin;throw std::out_of_range("native locale live memory domain");}
    Address read_address(Address a)override{std::lock_guard guard(mutex);Address value;std::memcpy(&value,resolve(a,8),8);return value;}
    std::uint32_t read_u32(Address a)override{if(counters.contains(a))return counters.load(a);std::lock_guard guard(mutex);std::uint32_t value;std::memcpy(&value,resolve(a,4),4);return value;}
    void write_address(Address a,Address value)override{std::lock_guard guard(mutex);std::memcpy(resolve(a,8),&value,8);}
    void write_u32(Address a,std::uint32_t value)override{if(counters.contains(a)){(void)counters.exchange(a,value);return;}std::lock_guard guard(mutex);std::memcpy(resolve(a,4),&value,4);}
    std::size_t count(){std::lock_guard guard(mutex);return blocks.size();}
};
Memory* observed=nullptr;
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T n){auto* p=::HeapAlloc(heap,flags,n);const auto error=::GetLastError();if(p){std::lock_guard guard(observed->mutex);observed->blocks.emplace(p,Memory::Block{n,{}});}::SetLastError(error);return p;}
BOOL WINAPI release_heap(HANDLE heap,DWORD flags,LPVOID p){
    BOOL result;DWORD error;{std::lock_guard guard(observed->mutex);const auto it=observed->blocks.find(p);check(it!=observed->blocks.end());
        // Only quiescent tree disposal has registered counters. Worker PTD
        // blocks have none, so the registry remains immutable during workers.
        for(auto* counter:it->second.refs)observed->counters.unbind(*counter);
        result=::HeapFree(heap,flags,p);error=::GetLastError();
        if(result){observed->blocks.erase(it);++observed->freed;}else for(auto* counter:it->second.refs)observed->counters.bind(*counter);
    }::SetLastError(error);return result;
}
struct Proxy:e::Effects {e::Effects* target=nullptr;void* optional_ptd_064d08()override{check(target);return target->optional_ptd_064d08();}};
struct HeapPolicy:hb::Policy {l::Effects& locks;explicit HeapPolicy(l::Effects& fx):locks(fx){}void lock_0625f8(std::uint32_t i)override{l::lock_acrt_0625f8(locks,i);}void unlock_06264c(std::uint32_t i)override{l::unlock_acrt_06264c(locks,i);}std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t)override{throw std::logic_error("unexpected handler");}void invalid_parameter_05c954()override{throw std::logic_error("unexpected invalid");}};
struct Abort:g::AbortPolicy {[[noreturn]]void abort_05f03c()override{throw std::runtime_error("unexpected abort");}};
struct Fixture {
    ln::Banks banks;r::State runtime{0x1234};va::State vapi;Loader loader;
    vb::LockEffects lock_fx{runtime,loader,banks,vapi,loader,banks};l::State locks;
    cn::Counters counters;Memory memory{counters};Proxy proxy;e::State errors;HeapPolicy heap_policy{lock_fx};
    hn::Calls heap_api{hn::Api{::GetProcessHeap,allocate,::HeapReAlloc,release_heap,::HeapSize,::GetLastError}};
    hb::Effects heap_fx{heap_api,errors,proxy,heap_policy};h::State heap{0,0,0,0};
    Locale initial;Time initial_time;Lconv initial_lconv;LONG multibyte=0;std::array<char16_t,2> category{u'C',0};std::uint64_t exceptions=0;
    lb::Globals globals;lr::Identities ids{address(category.data()),address(&initial_time)};lf::Defaults defaults;
    d::Identities ptd_ids{address(&exceptions),address(&multibyte),address(&initial)};
    nl::Resources resources{globals,memory,ids,defaults,heap,heap_fx,lock_fx,counters};Abort abort;
    fn::Binding binding{heap,heap_fx,ptd_ids,resources,abort};g::ErrorPtd error_ptd{binding.state(),binding};
    lb::ThreadEffects thread{globals,memory,binding.state(),binding,resources,heap,heap_fx};
    Locale* tree=nullptr;std::vector<Address> dynamic_counters;
    explicit Fixture(bool fallback):loader(banks,fallback){
        check(observed==nullptr);observed=&memory;proxy.target=&error_ptd;
        check(l::initialize_acrt_0625b0(locks,lock_fx)&&l::initialize_vcrt_05093c(locks,lock_fx)&&h::initialize_heap_0704cc(heap,heap_fx));
        memory.borrow(initial);memory.borrow(initial_time);memory.borrow(initial_lconv);memory.borrow(multibyte);memory.borrow(globals);
        memory.bind(initial.refs);memory.bind(multibyte);
        for(auto& c:initial.categories)c.name=ids.category_name_0a1948;initial.time=ids.initial_time_08ad20;
        defaults.category_name_0a1948=ids.category_name_0a1948;defaults.initial_time_08ad20=ids.initial_time_08ad20;defaults.lconv_0a1700=address(&initial_lconv);
        globals.mask_0a1c6c=1;globals.initial_locale_0a17e0=address(&initial);globals.initial_multibyte_0a1c70=globals.multibyte_0a59d0=address(&multibyte);
        tree=make<Locale>();tree->refs=1;bind_dynamic(tree->refs);
        auto* lc=make<Lconv>();tree->lconv=address(lc);
        tree->lconv_ref=counter();tree->numeric_ref=counter();tree->monetary_ref=counter();tree->ctype_ref=counter();
        for(unsigned i=0;i<19;++i)if(i!=10)lc->pointers[i]=bytes(8);
        tree->ctype=bytes(258)+0xfe;tree->lower=bytes(256)+0x80;tree->upper=bytes(256)+0x80;
        auto* time=make<Time>();time->refs=1;bind_dynamic(time->refs);tree->time=address(time);
        for(auto& p:time->before)p=bytes(8);for(auto& p:time->after)p=bytes(8);
        for(unsigned i=0;i<6;++i){auto text=bytes(8);tree->categories[i]={ids.category_name_0a1948,text,counter(),counter()};tree->extras[i]=text;}
        globals.locale_0a52e0=address(tree);
        check(memory.count()==133&&dynamic_counters.size()==18);for(auto a:dynamic_counters)check(counters.load(a)==1);
        check(p::initialize_064e90(binding.state(),binding));check(memory.count()==134);for(auto a:dynamic_counters)check(counters.load(a)==2);
    }
    template<class T>T* make(){auto* storage=h::calloc_base_066848(1,sizeof(T),heap,heap_fx);check(storage);return std::construct_at(static_cast<T*>(storage));}
    Address bytes(std::size_t n){auto* storage=h::calloc_base_066848(1,n,heap,heap_fx);check(storage);return address(storage);}
    void bind_dynamic(LONG& value){memory.bind(value);dynamic_counters.push_back(address(&value));}
    Address counter(){auto* value=make<LONG>();*value=1;bind_dynamic(*value);return address(value);}
    void replace_and_close(){
        check(counters.load(address(&tree->refs))==2);const auto freed_before=memory.freed;
        resources.lock_0625f8(4);check(lt::update_locale_nolock_0709d8(address(&globals.locale_0a52e0),address(&initial),thread)==address(&initial));resources.unlock_06264c(4);
        check(counters.load(address(&tree->refs))==1&&counters.load(address(&initial.refs))==1&&memory.count()==134);
        auto* main=p::required_preserve_error_064b90(binding.state(),binding);memory.write_u32(address(main)+0x3a8,0);
        check(lt::update_thread_locale_070968(thread)==address(&initial));
        check(memory.freed==freed_before+133&&memory.count()==1&&counters.load(address(&initial.refs))==2);
        for(auto a:dynamic_counters)check(!counters.contains(a));tree=nullptr;
        check(p::uninitialize_064ecc(binding.state(),binding)&&memory.count()==0&&counters.load(address(&multibyte))==0&&counters.load(address(&initial.refs))==1);
        resources.lock_0625f8(4);resources.release_locale_ref_0708c0(address(&initial));globals.locale_0a52e0=0;resources.unlock_06264c(4);
        check(counters.load(address(&initial.refs))==0);counters.unbind(initial.refs);counters.unbind(multibyte);
        check(l::uninitialize_vcrt_050984(locks,lock_fx)&&l::uninitialize_acrt_062614(locks,lock_fx));
    }
    ~Fixture(){check(binding.native_index()==p::invalid_fls_index&&locks.acrt_count==0&&locks.vcrt_count==0&&memory.count()==0);observed=nullptr;}
};
void native_resources(bool fallback){
    Fixture f(fallback);constexpr unsigned count=4;std::barrier ready(count+1),release(count+1);std::array<bool,count> success{};std::vector<std::thread> workers;
    for(unsigned i=0;i<count;++i)workers.emplace_back([&,i]{try{::SetLastError(0x65430000+i);auto* own=p::required_preserve_error_064b90(f.binding.state(),f.binding);check(own&&::GetLastError()==0x65430000+i);e::errno_cell_05d724(f.errors,f.error_ptd).store(20+i);check(e::errno_cell_05d724(f.errors,f.error_ptd).load()==20+i);success[i]=true;}catch(...){success[i]=false;}ready.arrive_and_wait();release.arrive_and_wait();});
    ready.arrive_and_wait();bool peak=f.memory.count()==138&&f.counters.load(address(&f.multibyte))==5;for(auto a:f.dynamic_counters)peak=peak&&f.counters.load(a)==6;
    release.arrive_and_wait();for(auto& worker:workers)worker.join();check(peak);for(bool ok:success)check(ok);
    check(f.memory.count()==134&&f.counters.load(address(&f.multibyte))==1);for(auto a:f.dynamic_counters)check(f.counters.load(a)==2);
    f.replace_and_close();
}
}
int main(){try{native_resources(false);native_resources(true);std::cout<<"PASS "<<checks.load()<<" native locale/ref/deep-free/FLS source integration checks\n";}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}