#pragma once
#include "admission.hpp"
#include "../anadius-callback-policy-source/policy_test_fixture.hpp"
#include <csignal>
#include <process.h>

namespace b17_test {
using namespace anadius;namespace ca=callback_admission;namespace nc=native_context;namespace n=native_recovered;
inline unsigned checks=0;
inline void require(bool v,const char* message){++checks;if(!v)throw std::runtime_error(message);}
template<class F>void rejects(ca::Fault fault,F&& f,const char* message){bool seen=false;try{f();}catch(const ca::Rejected& e){seen=e.fault==fault;}require(seen,message);}
inline std::size_t expected_callback=0;
inline ca::Fault expected_fault=ca::Fault::Stopped;
inline int expected_code=120;
inline bool expected_unwind=false;
inline unsigned probe_enters=0,probe_leaves=0;
inline DWORD original_incoming=0;
inline bool leave_saw_lease=false;
inline LSTATUS WINAPI own_close(HKEY){original_incoming=::GetLastError();::SetLastError(0x7654);return 7;}
inline LSTATUS WINAPI throwing_close(HKEY){throw std::runtime_error("own controlled original exception");}
inline LSTATUS WINAPI reentering_close(HKEY){
    (void)n::folder_w_entry(nullptr,5,nullptr,0,reinterpret_cast<wchar_t*>(1));return 0;
}
struct ProbeAdmission final:n::Admission {
    bool* destroyed;
    explicit ProbeAdmission(bool* flag=nullptr):destroyed(flag){}
    ~ProbeAdmission(){if(destroyed)*destroyed=true;}
    void enter(n::Callback)override{++probe_enters;::SetLastError(0x1111);}
    void leave(n::Callback)noexcept override{++probe_leaves;leave_saw_lease=n::active_calls()!=0;::SetLastError(0x2222);}
};
struct EntryErrors final:n::Errors {
    [[noreturn]] void lifecycle(n::LifecycleFault)override{throw std::logic_error("unexpected native lifecycle fault");}
    [[noreturn]] void callback_exception(n::Callback callback,std::exception_ptr error)noexcept override{
        if(static_cast<std::size_t>(callback)!=expected_callback)std::_Exit(210);
        if(expected_unwind){
            if(probe_enters==1&&probe_leaves==1&&leave_saw_lease&&n::active_calls()==0)std::_Exit(expected_code);
            std::_Exit(211);
        }
        try{std::rethrow_exception(error);}catch(const ca::Rejected& e){
            if(e.fault==expected_fault&&b15_test::active_probe->closes==0)std::_Exit(expected_code);
        }catch(...){}
        std::_Exit(212);
    }
};
inline EntryErrors permanent_errors;
inline void bad_entry(std::size_t slot){
    switch(slot){
    case 0:(void)n::connect_entry(1,reinterpret_cast<const sockaddr*>(1),16);break;
    case 1:(void)n::reg_open_a_entry(nullptr,reinterpret_cast<const char*>(1),0,0,reinterpret_cast<HKEY*>(1));break;
    case 2:(void)n::reg_open_w_entry(nullptr,reinterpret_cast<const wchar_t*>(1),0,0,reinterpret_cast<HKEY*>(1));break;
    case 3:(void)n::reg_close_entry(reinterpret_cast<HKEY>(1));break;
    case 4:(void)n::reg_query_a_entry(reinterpret_cast<HKEY>(1),reinterpret_cast<const char*>(1),nullptr,nullptr,nullptr,reinterpret_cast<DWORD*>(1));break;
    case 5:(void)n::reg_query_w_entry(reinterpret_cast<HKEY>(1),reinterpret_cast<const wchar_t*>(1),nullptr,nullptr,nullptr,reinterpret_cast<DWORD*>(1));break;
    case 6:(void)n::process_a_entry(reinterpret_cast<const char*>(1),nullptr,nullptr,nullptr,FALSE,0,nullptr,nullptr,nullptr,nullptr);break;
    case 7:(void)n::process_w_entry(reinterpret_cast<const wchar_t*>(1),nullptr,nullptr,nullptr,FALSE,0,nullptr,nullptr,nullptr,nullptr);break;
    case 8:(void)n::folder_a_entry(nullptr,5,nullptr,0,reinterpret_cast<char*>(1));break;
    case 9:(void)n::folder_w_entry(nullptr,5,nullptr,0,reinterpret_cast<wchar_t*>(1));break;
    default:std::_Exit(213);
    }
}
} // namespace b17_test
