#include "locale_startup.hpp"
#include "../anadius-crt-initial-state-source/initial_state.hpp"
#include "../anadius-crt-fls-native-source/fls.hpp"
#include "../anadius-crt-heap-native-bridge-source/bridge.hpp"
#include <array>
#include <atomic>
#include <barrier>
#include <iostream>
#include <source_location>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
namespace st=anadius::crt_locale_startup_recovered;
namespace im=anadius::crt_initial_state;
namespace lr=anadius::crt_locale_ref_recovered;
namespace lt=anadius::crt_locale_thread_recovered;
namespace d=anadius::crt_ptd_data_recovered;
namespace p=anadius::crt_ptd_recovered;
namespace fn=anadius::crt_fls_native;
namespace g=anadius::crt_glue;
namespace h=anadius::crt_heap_recovered;
namespace hn=anadius::crt_heap_native;
namespace hb=anadius::crt_heap_native_bridge;
namespace er=anadius::crt_errors_recovered;
namespace {
std::atomic<unsigned> checks{0};
void check(bool yes,std::source_location w=std::source_location::current()) {
    ++checks;if(!yes)throw std::runtime_error("locale startup line "+std::to_string(w.line()));
}
using Address=std::uint64_t;
template<class T>Address address(T* value){return reinterpret_cast<Address>(value);}
struct Trace final:st::Effects {
    Address slot=0x100,initial=0x200,current=0,result=0x345;
    std::uint32_t* change_after=nullptr;
    std::vector<std::string> log;
    Address current_locale_slot_0a52e0()override{log.push_back("slot");return slot;}
    Address initial_locale_0a17e0()override{log.push_back("initial");return initial;}
    Address read_address(Address a)override{check(a==slot);log.push_back("read");return current;}
    void write_address(Address a,Address value)override{check(a==slot);log.push_back("write");current=value;}
    void lock_0625f8(std::uint32_t n)override{log.push_back("lock"+std::to_string(n));}
    void unlock_06264c(std::uint32_t n)override{log.push_back("unlock"+std::to_string(n));}
    Address update_locale_nolock_0709d8(Address a,Address b)override {
        check(a==slot&&b==initial);log.push_back("update");current=0x987;
        if(change_after)*change_after=9;
        return result;
    }
};
void traces() {
    Trace e;
    check(st::initialize_locale_064468(e));
    check(e.current==e.initial);
    check(e.log==std::vector<std::string>({"initial","slot","write"}));
    e.log.clear();st::uninitialize_locale_065338(e);
    check(e.log==std::vector<std::string>({"lock4","slot","initial","read","unlock4"}));
    e.current=0;e.log.clear();std::uint32_t before=3,after=6;e.change_after=&after;
    st::reset_locale_065104(before,after,e);
    check(e.log==std::vector<std::string>({"lock3","slot","initial","read","update","write","unlock9"}));
    check(e.current==e.result&&after==9);
    e.current=0x111;e.log.clear();e.change_after=nullptr;
    st::uninitialize_locale_065338(e);
    check(e.log==std::vector<std::string>({"lock4","slot","initial","read","update","write","unlock4"}));
    check(e.current==e.result);
    e.current=0x567;e.log.clear();check(st::initialize_locale_064468(e));
    check(e.current==e.initial&&e.log==std::vector<std::string>({"initial","slot","write"}));
}
struct Model final:lt::Effects {
    static constexpr Address slot=0x100,initial=0x200,old=0x300;
    Address current=old;std::uint32_t initial_refs=1,old_refs=1;
    unsigned reads=0,writes=0,adds=0,releases=0,frees=0,locks=0,unlocks=0;
    bool mutate_second_read=false;
    Address read_address(Address a)override {
        check(a==slot);++reads;
        if(mutate_second_read&&reads==2)current=initial;
        return current;
    }
    void write_address(Address a,Address value)override{check(a==slot);++writes;current=value;}
    std::uint32_t read_u32(Address a)override{check(a==old+0x10);return old_refs;}
    std::uint32_t locale_mask_0a1c6c()override{throw std::logic_error("unused mask");}
    Address current_locale_0a52e0()override{return current;}
    Address current_multibyte_slot_0a59d0()override{throw std::logic_error("unused MB");}
    Address initial_locale_0a17e0()override{return initial;}
    Address initial_multibyte_0a1c70()override{throw std::logic_error("unused MB");}
    Address required_ptd_064b90()override{throw std::logic_error("unused PTD");}
    void lock_0625f8(std::uint32_t n)override{check(n==4);++locks;}
    void unlock_06264c(std::uint32_t n)override{check(n==4);++unlocks;}
    std::uint32_t atomic_fetch_add_32(Address,std::uint32_t)override{throw std::logic_error("source delegates refs");}
    void free_0668c0(Address)override{throw std::logic_error("unused MB free");}
    void add_locale_ref_070634(Address a)override{check(a==initial&&current==initial);++adds;++initial_refs;}
    void release_locale_ref_0708c0(Address a)override{check(a==old);++releases;--old_refs;}
    void free_locale_0706c0(Address a)override{check(a==old&&old_refs==0);++frees;}
    [[noreturn]]void abort_05f03c()override{throw std::logic_error("unused abort");}
};
void source_models() {
    Model e;st::SourceEffects bridge(e.slot,e.initial,e);
    st::uninitialize_locale_065338(bridge);
    check(e.current==e.initial&&e.initial_refs==2&&e.old_refs==0&&e.frees==1);
    check(e.reads==2&&e.writes==2&&e.adds==1&&e.releases==1&&e.locks==1&&e.unlocks==1);
    st::uninitialize_locale_065338(bridge);
    check(e.reads==3&&e.writes==2&&e.adds==1&&e.releases==1&&e.frees==1);
    Model mutated;mutated.mutate_second_read=true;
    st::SourceEffects second(mutated.slot,mutated.initial,mutated);
    st::uninitialize_locale_065338(second);
    check(mutated.current==mutated.initial&&mutated.reads==2&&mutated.writes==1);
    check(mutated.adds==0&&mutated.releases==0&&mutated.frees==0&&mutated.initial_refs==1);
    Model direct;st::SourceEffects third(direct.slot,direct.initial,direct);
    check(st::initialize_locale_064468(third));
    check(direct.current==direct.initial&&direct.writes==1&&direct.reads==0);
    check(direct.adds==0&&direct.releases==0&&direct.frees==0&&direct.locks==0);
}
struct Locks {
    std::array<CRITICAL_SECTION,6> values{};
    Locks(){for(auto& x:values)::InitializeCriticalSection(&x);}
    ~Locks(){for(auto& x:values)::DeleteCriticalSection(&x);}
    void lock(std::uint32_t n){check(n<values.size());::EnterCriticalSection(&values.at(n));}
    void unlock(std::uint32_t n){check(n<values.size());::LeaveCriticalSection(&values.at(n));}
};
struct Proxy:er::Effects {
    er::Effects* target=nullptr;
    void* optional_ptd_064d08()override{check(target!=nullptr);return target->optional_ptd_064d08();}
};
struct Policy:hb::Policy {
    Locks& locks;
    explicit Policy(Locks& l):locks(l){}
    void lock_0625f8(std::uint32_t i)override{locks.lock(i);}
    void unlock_06264c(std::uint32_t i)override{locks.unlock(i);}
    std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t)override{throw std::logic_error("unexpected new handler");}
    void invalid_parameter_05c954()override{throw std::logic_error("unexpected invalid parameter");}
};
struct Abort:g::AbortPolicy {[[noreturn]]void abort_05f03c()override{throw std::logic_error("unexpected abort");}};
std::atomic<unsigned> live_allocations{0};
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T count) {
    auto* result=::HeapAlloc(heap,flags,count);const auto error=::GetLastError();
    if(result)++live_allocations;
    ::SetLastError(error);return result;
}
BOOL WINAPI free_owned(HANDLE heap,DWORD flags,LPVOID value) {
    const auto result=::HeapFree(heap,flags,value);const auto error=::GetLastError();
    if(result)--live_allocations;
    ::SetLastError(error);return result;
}
struct Data final:d::Effects,lt::Effects {
    im::ImageState& image;Locks& locks;h::State& heap;h::Effects& heap_fx;Address current=0;
    explicit Data(im::ImageState& i,Locks& l,h::State& h,h::Effects& fx):image(i),locks(l),heap(h),heap_fx(fx){}
    void lock_0625f8(std::uint32_t i)override{locks.lock(i);}
    void unlock_06264c(std::uint32_t i)override{locks.unlock(i);}
    std::uint32_t atomic_fetch_add_32(Address a,std::uint32_t n)override{return image.atomic_fetch_add_32(a,n);}
    void free_0668c0(Address a)override{h::free_0668c0(reinterpret_cast<void*>(a),heap,heap_fx);}
    Address current_locale_0a52e0()override{return current;}
    void release_locale_ref_0708c0(Address a)override{lr::release_locale_ref_0708c0(a,image.reference_identities(),image);}
    std::uint32_t locale_ref_count_10(Address a)override{return image.counter_value(a+0x10);}
    void free_locale_0706c0(Address)override{throw std::logic_error("initial locale must not be freed");}
    void add_locale_ref_070634(Address a)override{lr::add_locale_ref_070634(a,image.reference_identities(),image);}
    Address read_address(Address a)override{return a==address(&current)?current:image.read_address(a);}
    void write_address(Address a,Address value)override{check(a==address(&current));current=value;}
    std::uint32_t read_u32(Address a)override{return image.counter_value(a);}
    std::uint32_t locale_mask_0a1c6c()override{return image.globals().locale_mask;}
    Address current_multibyte_slot_0a59d0()override{throw std::logic_error("multibyte startup is excluded");}
    Address initial_locale_0a17e0()override{return image.globals().initial_locale;}
    Address initial_multibyte_0a1c70()override{return image.globals().initial_multibyte;}
    Address required_ptd_064b90()override{throw std::logic_error("unused thread refresh");}
    [[noreturn]]void abort_05f03c()override{throw std::logic_error("unexpected thread abort");}
};
void owned_chain() {
    im::ImageState image;Locks locks;Proxy proxy;er::State errors;Policy policy(locks);
    hn::Calls calls(hn::Api{::GetProcessHeap,allocate,::HeapReAlloc,free_owned,::HeapSize,::GetLastError});
    hb::Effects heap_fx(calls,errors,proxy,policy);h::State heap{0,0,0,0};
    check(live_allocations.load()==0);
    check(h::initialize_heap_0704cc(heap,heap_fx));
    Data data(image,locks,heap,heap_fx);
    const auto locale=image.globals().initial_locale,mb=image.globals().initial_multibyte;
    st::SourceEffects startup(address(&data.current),locale,data);
    check(data.current==0&&image.counter_value(locale+0x10)==1);
    check(st::initialize_locale_064468(startup));
    check(data.current==locale&&image.counter_value(locale+0x10)==1); // direct store, no extra retain
    // Exception-table identity is fixture-only; the initial locale/MB graph is
    // recovered data. This test intentionally excludes MB code-page startup.
    std::uint64_t exception_fixture=0;
    d::Identities ids{address(&exception_fixture),mb,locale};Abort abort;
    fn::Binding binding(heap,heap_fx,ids,data,abort);
    g::ErrorPtd error_ptd(binding.state(),binding);proxy.target=&error_ptd;
    ::SetLastError(0x12345678);check(p::initialize_064e90(binding.state(),binding));
    check(::GetLastError()==0x12345678);
    check(image.counter_value(locale+0x10)==2&&image.counter_value(mb)==1&&live_allocations.load()==1);
    auto* main=p::required_preserve_error_064b90(binding.state(),binding);
    check(d::View(main).load<Address>(0x90)==locale&&d::View(main).load<Address>(0x88)==mb);
    std::barrier ready(5),release(5);std::array<bool,4> okay{};std::array<std::thread,4> workers;
    for(unsigned i=0;i<4;++i)workers[i]=std::thread([&,i]{
        try {
            ::SetLastError(0x11110000+i);
            auto* own=p::required_preserve_error_064b90(binding.state(),binding);
            okay[i]=own&&own!=main&&::GetLastError()==0x11110000+i&&d::View(own).load<Address>(0x90)==locale;
        }catch(...){okay[i]=false;}
        ready.arrive_and_wait();release.arrive_and_wait();
    });
    ready.arrive_and_wait();
    const bool peak=image.counter_value(locale+0x10)==6&&image.counter_value(mb)==5&&live_allocations.load()==5;
    release.arrive_and_wait();for(auto& worker:workers)worker.join();
    check(peak);for(bool value:okay)check(value);
    check(image.counter_value(locale+0x10)==2&&image.counter_value(mb)==1&&live_allocations.load()==1);
    st::uninitialize_locale_065338(startup);
    check(data.current==locale&&image.counter_value(locale+0x10)==2);
    check(p::uninitialize_064ecc(binding.state(),binding));
    check(binding.native_index()==p::invalid_fls_index);
    check(image.counter_value(locale+0x10)==1&&image.counter_value(mb)==0&&live_allocations.load()==0);
    st::uninitialize_locale_065338(startup);
    check(image.counter_value(locale+0x10)==1);
    // Also exercise the actual source helper's null-to-initial route.
    data.current=0;st::uninitialize_locale_065338(startup);
    check(data.current==locale&&image.counter_value(locale+0x10)==2);
    lr::release_locale_ref_0708c0(locale,image.reference_identities(),image);
    check(image.counter_value(locale+0x10)==1);
}
}
int main(){try{traces();source_models();owned_chain();std::cout<<"PASS "<<checks.load()<<" locale startup/reset/owned native FLS checks;4 own workers\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}
