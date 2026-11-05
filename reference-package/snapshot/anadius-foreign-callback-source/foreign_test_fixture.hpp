#pragma once
#include "foreign.hpp"
#include "../anadius-callback-admission-source/admission_test_fixture.hpp"
#include <atomic>
#include <cstring>

namespace b18_test {
using namespace anadius;namespace fc=foreign_callback;namespace nc=native_context;namespace n=native_recovered;namespace td=thread_domain;
inline unsigned checks=0;
[[noreturn]] inline void terminate_own(unsigned code){::TerminateProcess(::GetCurrentProcess(),code);std::_Exit(240);}
inline void require(bool value,const char* message){++checks;if(!value){std::cerr<<"FAILED: "<<message<<'\n';throw std::runtime_error(message);}}
template<class E,class F>void rejects(F&& f,const char* message){bool seen=false;try{f();}catch(const E&){seen=true;}require(seen,message);}
inline std::atomic<bool> deny_set_nonnull=false,deny_set_sentinel=false,deny_set_null=false,deny_root_free=false,throw_alloc=false,throw_free=false,throw_construct=false;
inline thread_local volatile std::sig_atomic_t denied_cleanup_marker=0;
inline void* WINAPI alloc(HANDLE heap,DWORD flags,SIZE_T size){
    auto* value=b16_test::selected_alloc(heap,flags,size);if(throw_alloc)throw std::runtime_error("unknown callback allocation");return value;
}
inline BOOL WINAPI free(HANDLE heap,DWORD flags,void* value){
    if(deny_root_free){denied_cleanup_marker=1;::SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    const auto result=b16_test::selected_free(heap,flags,value);if(throw_free)throw std::runtime_error("unknown callback release");return result;
}
inline BOOL WINAPI set(DWORD index,void* value){
    if((!value&&deny_set_null)||(value==crt_ptd_recovered::construction_sentinel()&&deny_set_sentinel)||
       (value&&value!=crt_ptd_recovered::construction_sentinel()&&deny_set_nonnull)){
        ::SetLastError(ERROR_ACCESS_DENIED);return FALSE;
    }
    return b6_core_test::slot_set(index,value);
}
inline void WINAPI enter_lock(LPCRITICAL_SECTION value){
    if(throw_construct)throw std::runtime_error("unknown PTD construction");::EnterCriticalSection(value);
}
struct Fixture {
    b6_core_test::Fixture heap;b15_test::Driver driver;
    std::shared_ptr<td::ApplicationMemory> memory;
    std::shared_ptr<std::ostringstream> output=std::make_shared<std::ostringstream>();
    std::shared_ptr<b16_test::ConvertState> conversion=std::make_shared<b16_test::ConvertState>();b12_test::Ui ui;
    Fixture(execution_recovered::Publication* publication=nullptr):driver(publication){
        deny_set_nonnull=deny_set_sentinel=deny_set_null=deny_root_free=throw_alloc=throw_free=throw_construct=false;
        heap.domain.reset();auto apis=b6_core_test::apis();
        apis.heap={b6_core_test::get_heap,alloc,::HeapReAlloc,free,::HeapSize,::GetLastError};apis.fls.set=set;apis.locks.enter=enter_lock;
        heap.domain=td::DomainStorage::create(b6_core_test::seed(),apis);
        if(!heap.domain->initialize())throw std::runtime_error("own PTD domain init");memory=heap.domain->make_memory();b12_test::ui=&ui;
    }
    ~Fixture(){b12_test::ui=nullptr;memory.reset();}
    callback_policy::Resources resources(){callback_policy::Resources r;r.memory=memory;r.stream=output;r.encoding=std::make_unique<b16_test::Encoding>(conversion);r.dialog=library_effects::Dialog(b12_test::dialog_api());r.exit=b16_test::selected_exit;return r;}
};
inline void in_thread(const std::function<void()>& work){std::exception_ptr failure;std::thread thread([&]{try{work();}catch(...){failure=std::current_exception();}});thread.join();if(failure)std::rethrow_exception(failure);}
inline int expected_code=150;inline fc::Fault expected_fault=fc::Fault::Overlap;
struct Errors final:n::Errors {
    [[noreturn]] void lifecycle(n::LifecycleFault)override{terminate_own(231);}
    [[noreturn]] void callback_exception(n::Callback,std::exception_ptr failure)noexcept override{
        try{std::rethrow_exception(failure);}catch(const fc::Rejected& e){if(e.fault==expected_fault)terminate_own(expected_code);}catch(...){}
        terminate_own(232);
    }
};
inline Errors permanent_errors;
inline void expected_abort(int){terminate_own(239);}
inline void native_cleanup_abort(int){terminate_own(denied_cleanup_marker?163:239);}
} // namespace b18_test
