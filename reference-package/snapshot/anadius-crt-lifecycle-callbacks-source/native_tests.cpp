#include "callbacks.hpp"
#include "../anadius-crt-counter-native-source/counters.hpp"
#include "../anadius-crt-initial-state-source/initial_state.hpp"
#include "../anadius-crt-heap-native-bridge-source/bridge.hpp"
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>
#include <vector>
namespace cb=anadius::crt_lifecycle_callbacks_recovered;
namespace o=anadius::crt_onexit_recovered;
namespace t=anadius::crt_termination_recovered;
namespace cn=anadius::crt_counter_native;
namespace im=anadius::crt_initial_state;
namespace h=anadius::crt_heap_recovered;
namespace hn=anadius::crt_heap_native;
namespace hb=anadius::crt_heap_native_bridge;
namespace rc=anadius::crt_recalloc_recovered;
namespace er=anadius::crt_errors_recovered;
namespace {
using Address=std::uint64_t;
unsigned checks=0;
void check(bool yes,std::source_location w=std::source_location::current()){
    ++checks;if(!yes)throw std::runtime_error("lifecycle callback line "+std::to_string(w.line()));
}
template<class T>Address address(T* p){return reinterpret_cast<Address>(p);}
struct Trace final:cb::MultibyteEffects {
    Address current=0x100,initial=0x200,atomic_redirect=0,free_redirect=0,freed=0;
    std::uint32_t previous=0;
    std::vector<std::string> log;
    Address current_multibyte_0a59d0()override{log.push_back("current");return current;}
    Address initial_multibyte_0a1c70()override{log.push_back("initial");return initial;}
    std::uint32_t atomic_fetch_add_32(Address a,std::uint32_t n)override{
        check(a==0x100&&n==0xffffffffU);log.push_back("atomic");
        if(atomic_redirect)current=atomic_redirect;
        return previous;
    }
    void free_0668c0(Address a)override{freed=a;log.push_back("free");if(free_redirect)current=free_redirect;}
    void set_current_multibyte_0a59d0(Address a)override{log.push_back("set");current=a;}
};
void traces(){
    for(auto n:{0U,2U,0xffffffffU}){
        Trace e;e.previous=n;check(cb::uninitialize_multibyte_0644f8(e));
        check(e.log==std::vector<std::string>({"current","atomic"}));check(e.current==0x100&&!e.freed);
    }
    Trace initial;initial.previous=1;initial.atomic_redirect=initial.initial;
    check(cb::uninitialize_multibyte_0644f8(initial));
    check(initial.log==std::vector<std::string>({"current","atomic","current","initial"}));
    check(!initial.freed&&initial.current==initial.initial);
    Trace dynamic;dynamic.previous=1;dynamic.atomic_redirect=0x300;dynamic.free_redirect=0x400;
    check(cb::uninitialize_multibyte_0644f8(dynamic));
    check(dynamic.log==std::vector<std::string>({"current","atomic","current","initial","free","set"}));
    check(dynamic.freed==0x300&&dynamic.current==dynamic.initial);
}
struct Memory {
    struct Block{std::size_t extent;LONG* counter=nullptr;};
    cn::Counters counters;std::map<void*,Block> blocks;bool fail_free=false;unsigned frees=0;
    void* resolve(Address a,std::size_t n){
        for(auto& [p,b]:blocks){const auto begin=address(p);
            if(a>=begin&&n<=b.extent&&a-begin<=b.extent-n)return static_cast<std::byte*>(p)+(a-begin);
        }
        throw std::out_of_range("owned callback memory");
    }
};
Memory* active=nullptr;
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T n) {
    auto* value=::HeapAlloc(heap,flags,n);const auto error=::GetLastError();
    if(value)active->blocks.emplace(value,Memory::Block{n,nullptr});
    ::SetLastError(error);return value;
}
LPVOID WINAPI reallocate(HANDLE heap,DWORD flags,LPVOID old,SIZE_T n){
    auto it=active->blocks.find(old);check(it!=active->blocks.end()&&!it->second.counter);
    auto* value=::HeapReAlloc(heap,flags,old,n);const auto error=::GetLastError();
    if(value){active->blocks.erase(it);active->blocks.emplace(value,Memory::Block{n,nullptr});}
    ::SetLastError(error);return value;
}
BOOL WINAPI release(HANDLE heap,DWORD flags,LPVOID old){
    auto it=active->blocks.find(old);check(it!=active->blocks.end());
    if(active->fail_free){active->fail_free=false;::SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    auto* counter=it->second.counter;if(counter)active->counters.unbind(*counter);
    const auto result=::HeapFree(heap,flags,old);const auto error=::GetLastError();
    if(result){active->blocks.erase(it);++active->frees;}
    else if(counter)active->counters.bind(*counter);
    ::SetLastError(error);return result;
}
struct ErrorPtd:er::Effects{void* optional_ptd_064d08()override{return nullptr;}};
struct Policy:hb::Policy {
    CRITICAL_SECTION lock;
    Policy(){::InitializeCriticalSection(&lock);}
    ~Policy(){::DeleteCriticalSection(&lock);}
    void lock_0625f8(std::uint32_t n)override{check(n==0||n==2);::EnterCriticalSection(&lock);}
    void unlock_06264c(std::uint32_t n)override{check(n==0||n==2);::LeaveCriticalSection(&lock);}
    std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t)override{throw std::logic_error("unexpected handler");}
    void invalid_parameter_05c954()override{throw std::logic_error("unexpected invalid");}
};
struct Native final:cb::MultibyteEffects,o::Effects {
    im::ImageState image;Memory memory;ErrorPtd error_ptd;er::State errors;Policy policy;
    hn::Calls calls{hn::Api{::GetProcessHeap,allocate,reallocate,release,::HeapSize,::GetLastError}};
    hb::Effects heap_fx{calls,errors,error_ptd,policy};h::State heap{0,0,0,0};
    Address current=image.globals().initial_multibyte;
    o::State tables{0x12345678abcdef01ULL,{0,0,0},{0,0,0}};
    Address encoded_callback=0;t::State termination{tables,encoded_callback};
    std::vector<Address> invoked;
    Native(){check(!active);active=&memory;check(h::initialize_heap_0704cc(heap,heap_fx));}
    ~Native(){check(memory.blocks.empty());active=nullptr;}
    Address current_multibyte_0a59d0()override{return current;}
    Address initial_multibyte_0a1c70()override{return image.globals().initial_multibyte;}
    void set_current_multibyte_0a59d0(Address a)override{current=a;}
    std::uint32_t atomic_fetch_add_32(Address a,std::uint32_t n)override {
        return a==initial_multibyte_0a1c70()?image.atomic_fetch_add_32(a,n):memory.counters.fetch_add(a,n);
    }
    void free_0668c0(Address a)override{h::free_0668c0(reinterpret_cast<void*>(a),heap,heap_fx);}
    void lock_0625f8(std::uint32_t n)override{policy.lock_0625f8(n);}
    void unlock_06264c(std::uint32_t n)override{policy.unlock_06264c(n);}
    Address recalloc_070434(Address a,std::uint64_t n,std::uint64_t size)override{
        return address(rc::recalloc_base_070434(reinterpret_cast<void*>(a),n,size,heap,heap_fx));
    }
    std::uint64_t read_word(Address a)override{std::uint64_t value;std::memcpy(&value,memory.resolve(a,8),8);return value;}
    void write_word(Address a,std::uint64_t value)override{std::memcpy(memory.resolve(a,8),&value,8);}
    void invoke_callback(Address a)override {
        check(a==11||a==12||a==13||a==21);invoked.push_back(a);
        if(a==12)check(o::register_onexit_function_064420(&tables.crt_0a52a8,13,tables,*this)==0);
    }
    Address record(std::uint32_t refs){
        auto* storage=h::calloc_base_066848(1,0x228,heap,heap_fx);check(storage!=nullptr);
        auto* counter=std::construct_at(static_cast<LONG*>(storage),static_cast<LONG>(refs));
        memory.counters.bind(*counter);memory.blocks.at(storage).counter=counter;
        return address(counter);
    }
};
bool empty(const o::Table& a,Address cookie){return a.begin==cookie&&a.end==cookie&&a.capacity_end==cookie;}
void onexit_native(Native& e){
    const auto cookie=e.tables.cookie_0a1580;
    check(cb::initialize_onexit_tables_06447c(e.termination));
    check(empty(e.tables.crt_0a52a8,cookie)&&empty(e.termination.quick_table_a52c0,cookie));
    check(e.tables.application_0a47a8.begin==0&&e.encoded_callback==0);
    check(o::register_onexit_function_064420(&e.tables.crt_0a52a8,11,e.tables,e)==0);
    check(o::register_onexit_function_064420(&e.tables.crt_0a52a8,12,e.tables,e)==0);
    check(o::register_onexit_function_064420(&e.termination.quick_table_a52c0,21,e.tables,e)==0);
    check(e.memory.blocks.size()==2);
    const auto before_normal=e.tables.crt_0a52a8,before_quick=e.termination.quick_table_a52c0;
    check(cb::initialize_onexit_tables_06447c(e.termination));
    check(e.tables.crt_0a52a8.begin==before_normal.begin&&e.tables.crt_0a52a8.end==before_normal.end);
    check(e.termination.quick_table_a52c0.begin==before_quick.begin&&e.termination.quick_table_a52c0.end==before_quick.end);
    check(o::execute_onexit_table_0643b4(&e.tables.crt_0a52a8,e.tables,e)==0);
    check(e.invoked==std::vector<Address>({12,13,11}));
    check(empty(e.tables.crt_0a52a8,cookie)&&e.memory.blocks.size()==1);
    check(e.termination.quick_table_a52c0.begin==before_quick.begin);
    check(o::execute_onexit_table_0643b4(&e.termination.quick_table_a52c0,e.tables,e)==0);
    check(e.invoked==std::vector<Address>({12,13,11,21}));
    check(empty(e.termination.quick_table_a52c0,cookie)&&e.memory.blocks.empty());
}
void multibyte_native(Native& e){
    const auto initial=e.initial_multibyte_0a1c70();
    check(e.image.counter_value(initial)==0);
    check(cb::uninitialize_multibyte_0644f8(e));
    check(e.current==initial&&e.image.counter_value(initial)==0xffffffffU);
    (void)e.image.atomic_fetch_add_32(initial,2);
    check(cb::uninitialize_multibyte_0644f8(e));
    check(e.current==initial&&e.image.counter_value(initial)==0);
    const auto record=e.record(2);e.current=record;
    check(cb::uninitialize_multibyte_0644f8(e));
    check(e.current==record&&e.memory.counters.load(record)==1);
    check(cb::uninitialize_multibyte_0644f8(e));
    check(e.current==initial&&!e.memory.counters.contains(record)&&e.memory.blocks.empty());
    // A normal-returning HeapFree failure still reaches the original reset.
    const auto failed=e.record(1);e.current=failed;e.memory.fail_free=true;
    check(cb::uninitialize_multibyte_0644f8(e));
    check(e.current==initial&&e.memory.counters.load(failed)==0&&e.memory.blocks.size()==1);
    check(e.errors.errno_fallback==er::errno_from_os_error_05d650(ERROR_ACCESS_DENIED));
    e.free_0668c0(failed);check(e.memory.blocks.empty()&&!e.memory.counters.contains(failed));
    check(e.image.globals().initialized==0&&e.image.globals().type_table==0&&e.image.globals().case_table==0);
}
void table_conditions(){
    for(Address cookie:{0ULL,1ULL,0xffffffffffffffffULL,0x123456789abcdef0ULL}){
        o::State tables{cookie,{4,5,6},{0,123,0}};Address callback=9;t::State s(tables,callback);
        s.quick_table_a52c0={10,11,12};
        check(cb::initialize_onexit_tables_06447c(s));
        check(empty(tables.crt_0a52a8,cookie));
        check(s.quick_table_a52c0.begin==10&&s.quick_table_a52c0.end==11&&s.quick_table_a52c0.capacity_end==12);
        check(tables.application_0a47a8.begin==4&&callback==9);
        tables.crt_0a52a8={20,21,22};s.quick_table_a52c0={30,31,30};
        check(cb::initialize_onexit_tables_06447c(s));
        check(tables.crt_0a52a8.begin==20&&tables.crt_0a52a8.end==21&&tables.crt_0a52a8.capacity_end==22);
        check(empty(s.quick_table_a52c0,cookie));
    }
}
}
int main(){try{traces();table_conditions();Native e;onexit_native(e);multibyte_native(e);std::cout<<"PASS "<<checks<<" lifecycle onexit/multibyte callback checks;owned native heap/counters\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}
