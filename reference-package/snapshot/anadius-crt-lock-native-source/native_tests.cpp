#include "native.hpp"
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <type_traits>

namespace n=anadius::crt_lock_native;
namespace l=anadius::crt_locks_recovered;
namespace {
unsigned checks{};
void check(bool value){++checks;if(!value)throw std::runtime_error("Native lock check failed");}
template<class E,class F>void rejects(F&& function){
    try{function();}catch(const E&){check(true);return;}
    throw std::runtime_error("Expected native lock boundary rejection");
}
struct Fixture {
    unsigned spin_calls{},ex_calls{},delete_calls{},enter_calls{},leave_calls{};
    CRITICAL_SECTION* last{};
    DWORD spin{},flags{};
    BOOL result{};
};
Fixture* fixture{};
BOOL WINAPI spin(CRITICAL_SECTION* p,DWORD count){
    ++fixture->spin_calls;fixture->last=p;fixture->spin=count;
    return fixture->result;
}
BOOL WINAPI ex(CRITICAL_SECTION* p,DWORD count,DWORD flags){
    ++fixture->ex_calls;fixture->last=p;fixture->spin=count;fixture->flags=flags;
    return fixture->result;
}
void WINAPI destroy(CRITICAL_SECTION* p){++fixture->delete_calls;fixture->last=p;}
void WINAPI enter(CRITICAL_SECTION* p){++fixture->enter_calls;fixture->last=p;}
void WINAPI leave(CRITICAL_SECTION* p){++fixture->leave_calls;fixture->last=p;}
n::Api injected(){return {spin,destroy,enter,leave};}
constexpr l::LockRef a{l::Bank::acrt,0},v{l::Bank::vcrt,0};
const auto ex_address=reinterpret_cast<n::Address>(&ex);
void injected_edges(){
    Fixture f;fixture=&f;
    {
        auto bad=injected();bad.leave=nullptr;
        rejects<std::invalid_argument>([&]{n::Banks b(bad);});
    }
    n::Banks b(injected());
    check(!b.initialized(a));check(!b.initialized(v));
    check(b.storage(a)!=b.storage(v));
    check(reinterpret_cast<std::uintptr_t>(b.storage(a))%alignof(CRITICAL_SECTION)==0);
    for(const auto ref:{l::LockRef{l::Bank::acrt,-1},{l::Bank::acrt,14},
            {l::Bank::vcrt,-1},{l::Bank::vcrt,1},
            {l::Bank::acrt,std::numeric_limits<std::int64_t>::max()},
            {static_cast<l::Bank>(7),0}}){
        rejects<std::out_of_range>([&]{(void)b.storage(ref);});
        rejects<std::out_of_range>([&]{(void)b.initialized(ref);});
        rejects<std::out_of_range>([&]{(void)b.initialize_and_spin(ref,0);});
        rejects<std::out_of_range>([&]{b.enter(ref);});
        rejects<std::out_of_range>([&]{b.leave(ref);});
        rejects<std::out_of_range>([&]{b.destroy(ref);});
    }
    rejects<std::logic_error>([&]{b.enter(a);});
    rejects<std::logic_error>([&]{b.leave(a);});
    rejects<std::logic_error>([&]{b.destroy(a);});
    rejects<std::invalid_argument>([&]{(void)b.init_spin_at(nullptr,0);});
    CRITICAL_SECTION foreign{};
    rejects<std::invalid_argument>([&]{(void)b.init_spin_at(&foreign,0);});
    rejects<std::invalid_argument>([&]{(void)b.init_spin_at(static_cast<char*>(b.storage(a))+1,0);});
    rejects<std::invalid_argument>([&]{(void)b.init_ex_at(ex_address,&foreign,0,0);});
    rejects<std::invalid_argument>([&]{(void)b.call_initialize_ex(0,a,0,0);});
    check(f.spin_calls==0&&f.ex_calls==0&&f.delete_calls==0&&f.enter_calls==0&&f.leave_calls==0);
    f.result=0;
    check(b.initialize_and_spin(a,0xfedcba98u)==0);
    check(!b.initialized(a)&&f.spin_calls==1&&f.spin==0xfedcba98u&&f.last==b.storage(a));
    check(b.call_initialize_ex(ex_address,v,4000,0x12345678u)==0);
    check(!b.initialized(v)&&f.ex_calls==1&&f.spin==4000&&f.flags==0x12345678u);
    f.result=static_cast<BOOL>(0x80000100u);
    check(b.initialize_and_spin(a,1)==0x80000100ULL);
    check(b.initialized(a));
    rejects<std::logic_error>([&]{(void)b.initialize_and_spin(a,0);});
    rejects<std::logic_error>([&]{(void)b.call_initialize_ex(ex_address,a,0,0);});
    check(f.spin_calls==2&&f.ex_calls==1);
    b.enter(a);b.enter(a);b.leave(a);b.leave(a);
    check(f.enter_calls==2&&f.leave_calls==2&&f.last==b.storage(a));
    b.destroy(a);check(!b.initialized(a)&&f.delete_calls==1);
    rejects<std::logic_error>([&]{b.destroy(a);});
    f.result=-1;
    check(b.call_initialize_ex(ex_address,v,0xffffffffu,0x87654321u)==0xffffffffULL);
    check(b.initialized(v)&&f.ex_calls==2&&f.flags==0x87654321u);
    b.destroy(v);check(!b.initialized(v)&&f.delete_calls==2);
    f.result=-2147483647;
    check(b.init_ex_at(ex_address,b.storage(a),7,9)==-2147483647);
    b.destroy(a);
    check(b.init_spin_at(b.storage(v),11)==-2147483647);
    b.destroy(v);
    // All bank objects are stable, distinct, and individually tracked.
    for(std::int64_t i=0;i<14;++i){
        const l::LockRef ref{l::Bank::acrt,i};
        check(!b.initialized(ref));
        check(b.initialize_and_spin(ref,4000)==0x80000001ULL);
        check(b.initialized(ref)&&f.last==b.storage(ref));
        if(i!=0)check(b.storage(ref)!=b.storage(a));
    }
    for(std::int64_t i=13;i>=0;--i){b.destroy({l::Bank::acrt,i});check(!b.initialized({l::Bank::acrt,i}));}
}
void real_calls(){
    n::Banks b;
    check(b.initialize_and_spin(a,4000)!=0);
    auto* object=static_cast<CRITICAL_SECTION*>(b.storage(a));
    check(object->SpinCount==4000);
    b.enter(a);b.enter(a);b.leave(a);b.leave(a);
    unsigned counter{};
    const auto worker=[&]{for(unsigned i=0;i<20000;++i){b.enter(a);++counter;b.leave(a);}};
    std::thread first(worker),second(worker);
    first.join();second.join();
    check(counter==40000);check(b.initialized(a));
    b.destroy(a);check(!b.initialized(a));
    // Already-loaded Windows system module only. No target module load.
    const auto kernel=::GetModuleHandleW(L"kernel32.dll");check(kernel!=nullptr);
    const auto proc=::GetProcAddress(kernel,"InitializeCriticalSectionEx");check(proc!=nullptr);
    check(b.call_initialize_ex(reinterpret_cast<n::Address>(proc),v,4000,0)!=0);
    b.enter(v);b.leave(v);b.destroy(v);check(!b.initialized(v));
}
}
int main(int argc,char** argv){
    static_assert(!std::is_copy_constructible_v<n::Banks>);
    static_assert(!std::is_move_constructible_v<n::Banks>);
    if(argc==2&&std::string_view(argv[1])=="--live-bank"){
        std::set_terminate([]{std::_Exit(88);});
        Fixture f;fixture=&f;f.result=1;
        {n::Banks b(injected());(void)b.initialize_and_spin(a,0);}
        return 99;
    }
    try{injected_edges();real_calls();std::printf("PASS: %u native lock checks\n",checks);}
    catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}
}
