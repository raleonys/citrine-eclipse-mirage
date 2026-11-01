#include "lowio_lifecycle.hpp"
#include "../anadius-crt-heap-native-bridge-source/bridge.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>
#include <vector>
namespace lo=anadius::crt_lowio_lifecycle_recovered;
namespace h=anadius::crt_heap_recovered;
namespace hn=anadius::crt_heap_native;
namespace hb=anadius::crt_heap_native_bridge;
namespace er=anadius::crt_errors_recovered;
namespace {
using Address=std::uint64_t;
unsigned checks=0;
void check(bool value,std::source_location w=std::source_location::current()){
    ++checks;if(!value)throw std::runtime_error("lowio lifecycle line "+std::to_string(w.line()));
}
template<class T>Address address(T* p){return reinterpret_cast<Address>(p);}
struct Trace final:lo::ArrayEffects {
    lo::State state;
    std::uint64_t result=0;bool mutation=false;
    std::vector<std::string> calls;
    std::vector<Address> deleted,freed;
    void lock_0625f8(std::uint32_t n)override{check(n==7);calls.push_back("lock");}
    void unlock_06264c(std::uint32_t n)override{check(n==7);calls.push_back("unlock");}
    std::uint64_t ensure_fh_exists_0732a8(std::int32_t n)override{check(n==0);calls.push_back("ensure");return result;}
    void initialize_inherited_067698()override{calls.push_back("inherited");}
    void initialize_standard_067788()override{calls.push_back("standard");}
    void delete_critical_section(Address a)override{deleted.push_back(a);}
    void free_0668c0(Address a)override{
        freed.push_back(a);
        if(mutation&&a==0x10000){state.arrays_0a54a0[0]=0x40000;state.arrays_0a54a0[127]=0x30000;}
    }
};
void trace_paths(){
    for(std::uint64_t result:{0ULL,1ULL,0x100ULL,0xffffffffULL,0x100000000ULL,0x1234567800000000ULL}){
        Trace e;e.result=result;
        const bool success=static_cast<std::uint32_t>(result)==0;
        check(lo::initialize_lowio_067890(e)==success);
        check(e.calls==(success?std::vector<std::string>{"lock","ensure","inherited","standard","unlock"}:std::vector<std::string>{"lock","ensure","unlock"}));
    }
    Trace e;lo::destroy_handle_array_073258(0,e);check(e.deleted.empty()&&e.freed.empty());
    e.state.capacity_0a58a0=777;e.state.arrays_0a54a0[0]=0x10000;e.state.arrays_0a54a0[64]=0x20000;e.mutation=true;
    check(lo::uninitialize_lowio_0678cc(e.state,e));
    check(e.freed==std::vector<Address>({0x10000,0x20000,0x30000}));
    check(e.deleted.size()==192);
    for(unsigned group=0;group<3;++group)for(unsigned i=0;i<64;++i)
        check(e.deleted[group*64+i]==Address(0x10000)*(group+1)+Address(i)*0x48);
    check(std::all_of(e.state.arrays_0a54a0.begin(),e.state.arrays_0a54a0.end(),[](auto a){return a==0;}));
    check(e.state.capacity_0a58a0==777&&e.calls.empty());
    check(lo::uninitialize_lowio_0678cc(e.state,e)&&e.deleted.size()==192&&e.freed.size()==3);
}
struct Record {CRITICAL_SECTION lock;std::array<std::byte,0x20> remaining;};
using Array=std::array<Record,64>;
static_assert(sizeof(CRITICAL_SECTION)==40&&offsetof(Record,lock)==0&&sizeof(Record)==0x48&&sizeof(Array)==0x1200);
struct ErrorPtd:er::Effects{void* optional_ptd_064d08()override{return nullptr;}};
struct Policy:hb::Policy{
    CRITICAL_SECTION lock;
    Policy(){::InitializeCriticalSection(&lock);}
    ~Policy(){::DeleteCriticalSection(&lock);}
    void lock_0625f8(std::uint32_t n)override{check(n==0);::EnterCriticalSection(&lock);}
    void unlock_06264c(std::uint32_t n)override{check(n==0);::LeaveCriticalSection(&lock);}
    std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t)override{throw std::logic_error("unexpected handler");}
    void invalid_parameter_05c954()override{throw std::logic_error("unexpected invalid");}
};
unsigned native_frees=0;
BOOL WINAPI release(HANDLE heap,DWORD flags,LPVOID p){
    const auto okay=::HeapFree(heap,flags,p);const auto error=::GetLastError();
    if(okay)++native_frees;::SetLastError(error);return okay;
}
struct Native final:lo::ArrayEffects {
    struct Owned{Array* data;std::array<bool,64> alive;};
    std::map<Address,Owned> arrays;lo::State state;
    ErrorPtd error_ptd;er::State errors;Policy policy;
    hn::Calls calls{hn::Api{::GetProcessHeap,::HeapAlloc,::HeapReAlloc,release,::HeapSize,::GetLastError}};
    hb::Effects heap_fx{calls,errors,error_ptd,policy};h::State heap{0,0,0,0};
    CRITICAL_SECTION lifecycle;
    unsigned deleted=0,held=0,recursive=0,inherited=0,standard=0;
    Native(){::InitializeCriticalSection(&lifecycle);check(h::initialize_heap_0704cc(heap,heap_fx));}
    ~Native(){check(arrays.empty()&&held==0);::DeleteCriticalSection(&lifecycle);}
    Address create(){
        auto* raw=h::calloc_base_066848(1,sizeof(Array),heap,heap_fx);check(raw!=nullptr);
        auto* p=std::construct_at(static_cast<Array*>(raw));
        for(auto& record:*p)::InitializeCriticalSection(&record.lock);
        Owned owned{p,{}};owned.alive.fill(true);arrays.emplace(address(p),owned);return address(p);
    }
    void lock_0625f8(std::uint32_t n)override{check(n==7);::EnterCriticalSection(&lifecycle);++held;}
    void unlock_06264c(std::uint32_t n)override{check(n==7&&held==1);--held;::LeaveCriticalSection(&lifecycle);}
    std::uint64_t ensure_fh_exists_0732a8(std::int32_t n)override{
        check(n==0&&held==1);::EnterCriticalSection(&lifecycle);++recursive;
        state.arrays_0a54a0[0]=create();state.capacity_0a58a0=64;
        ::LeaveCriticalSection(&lifecycle);return 0;
    }
    void initialize_inherited_067698()override{check(held==1);++inherited;}
    void initialize_standard_067788()override{check(held==1);++standard;}
    void delete_critical_section(Address a)override{
        for(auto& [base,owned]:arrays)if(a>=base&&a-base<sizeof(Array)){
            const auto delta=a-base;check(delta%sizeof(Record)==0);
            const auto i=static_cast<std::size_t>(delta/sizeof(Record));
            check(owned.alive.at(i)&&address(&(*owned.data)[i].lock)==a);
            ::DeleteCriticalSection(&(*owned.data)[i].lock);owned.alive[i]=false;++deleted;return;
        }
        throw std::out_of_range("typed native lowio CS");
    }
    void free_0668c0(Address a)override{
        auto it=arrays.find(a);check(it!=arrays.end());
        check(std::none_of(it->second.alive.begin(),it->second.alive.end(),[](bool live){return live;}));
        std::destroy_at(it->second.data);
        const auto before=native_frees;h::free_0668c0(reinterpret_cast<void*>(a),heap,heap_fx);
        check(native_frees==before+1);arrays.erase(it);
    }
};
void native_path(){
    Native e;const auto before=native_frees;
    check(lo::initialize_lowio_067890(e));
    check(e.recursive==1&&e.inherited==1&&e.standard==1&&e.arrays.size()==1);
    e.state.arrays_0a54a0[64]=e.create();e.state.arrays_0a54a0[127]=e.create();
    check(lo::uninitialize_lowio_0678cc(e.state,e));
    check(e.deleted==192&&native_frees==before+3&&e.arrays.empty());
    check(e.state.capacity_0a58a0==64&&e.held==0);
    check(std::all_of(e.state.arrays_0a54a0.begin(),e.state.arrays_0a54a0.end(),[](auto a){return a==0;}));
    check(lo::uninitialize_lowio_0678cc(e.state,e)&&e.deleted==192&&native_frees==before+3);
}
}
int main(){try{trace_paths();native_path();std::cout<<"PASS "<<checks<<" lowio lifecycle checks;192 typed native CS deletions;3 owned arrays\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}
