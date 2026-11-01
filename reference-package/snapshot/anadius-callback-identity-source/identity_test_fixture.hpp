#pragma once
#include "identity.hpp"
#include "../anadius-foreign-callback-source/foreign_test_fixture.hpp"

namespace b21_test {
using namespace anadius;namespace ci=callback_identity;namespace td=thread_domain;namespace ptd=crt_ptd_recovered;
inline unsigned checks=0;
inline void require(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
template<class E,class F>void rejects(F&& fn,const char* message){bool caught=false;try{fn();}catch(const E&){caught=true;}require(caught,message);}
inline std::atomic<unsigned> fls_reads=0;
inline thread_local std::function<void()> get_hook;
inline thread_local bool inside_get=false,throw_get=false,zero_detach=false;
inline PVOID WINAPI get(DWORD index){
    ++fls_reads;
    if(throw_get){::SetLastError(900);throw std::runtime_error("own FLS getter throw");}
    if(get_hook&&!inside_get){inside_get=true;try{get_hook();}catch(...){inside_get=false;throw;}inside_get=false;}
    return ::FlsGetValue(index);
}
inline BOOL WINAPI set(DWORD index,PVOID value){
    if(zero_detach&&value==ptd::construction_sentinel()){::SetLastError(0);return FALSE;}
    return b18_test::set(index,value);
}
struct Fixture {
    b6_core_test::Fixture heap;
    Fixture(){
        get_hook={};inside_get=throw_get=zero_detach=false;
        b18_test::deny_set_nonnull=b18_test::deny_set_sentinel=b18_test::deny_set_null=b18_test::deny_root_free=false;
        b18_test::throw_alloc=b18_test::throw_free=b18_test::throw_construct=false;
        heap.domain.reset();auto apis=b6_core_test::apis();
        apis.heap={b6_core_test::get_heap,b18_test::alloc,::HeapReAlloc,b18_test::free,::HeapSize,::GetLastError};
        apis.fls.get=get;apis.fls.set=set;apis.locks.enter=b18_test::enter_lock;
        heap.domain=td::DomainStorage::create(b6_core_test::seed(),apis);
        if(!heap.domain->initialize())throw std::runtime_error("own checked domain init");
    }
    ~Fixture(){get_hook={};throw_get=zero_detach=false;}
};
} // namespace b21_test
