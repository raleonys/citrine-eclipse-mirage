#pragma once
#include "domain.hpp"
#include <appmodel.h>
#include <roapi.h>
#include <array>
#include <bit>
#include <cstring>
#include <exception>
#include <functional>
#include <map>
#include <source_location>
#include <stdexcept>
#include <string>
#include <vector>

namespace b6_core_test {
namespace td=anadius::thread_domain;
namespace ptd=anadius::crt_ptd_recovered;
inline unsigned checks=0;
inline void check(bool value,const char* message,std::source_location where=std::source_location::current()){
    ++checks;if(!value)throw std::runtime_error(std::string(message)+" at line "+std::to_string(where.line())+" after "+std::to_string(checks)+" checks");
}
template<class E,class F>void rejects(F&& f,const char* message){bool seen=false;try{f();}catch(const E&){seen=true;}check(seen,message);}
inline std::uintptr_t number(HMODULE value){return reinterpret_cast<std::uintptr_t>(value);}
inline HMODULE module(std::uintptr_t value){return reinterpret_cast<HMODULE>(value);}
struct RuntimeState {
    std::vector<std::wstring> names;
    std::vector<std::uintptr_t> released;
    std::vector<std::string> procedures;
    std::function<void()> load_hook,release_hook;
    unsigned release_failure_call=0,flag_calls=0,ro_calls=0,unro_calls=0,policy_calls=0;
    bool fail_load=false,throw_load=false;
    DWORD incoming_load_error=0;HANDLE file=nullptr;DWORD flags=0;
    std::uint32_t process_flags=0;
    std::int32_t policy=1;
    LONG policy_result=0;
    HRESULT ro_result=0;
    HANDLE policy_token=nullptr;RO_INIT_TYPE ro_mode=RO_INIT_SINGLETHREADED;
};
inline RuntimeState* runtime_state=nullptr;
inline HRESULT WINAPI ro_initialize(RO_INIT_TYPE mode){auto& s=*runtime_state;++s.ro_calls;s.ro_mode=mode;::SetLastError(0x661);return s.ro_result;}
inline void WINAPI ro_uninitialize(){++runtime_state->unro_calls;::SetLastError(0x662);}
inline LONG WINAPI policy(HANDLE token,AppPolicyThreadInitializationType* out){
    auto& s=*runtime_state;++s.policy_calls;s.policy_token=token;std::memcpy(out,&s.policy,sizeof(s.policy));::SetLastError(0x663);return s.policy_result;
}
inline HMODULE WINAPI load(LPCWSTR name,HANDLE file,DWORD flags){
    auto& s=*runtime_state;s.incoming_load_error=::GetLastError();s.file=file;s.flags=flags;s.names.emplace_back(name);
    if(s.load_hook)s.load_hook();if(s.throw_load)throw std::runtime_error("unknown-load-outcome");
    ::SetLastError(0x651);if(s.fail_load)return nullptr;
    return module(std::wstring_view(name).find(L"appmodel")!=std::wstring_view::npos?0x200:0x100);
}
inline FARPROC WINAPI procedure(HMODULE m,LPCSTR name){
    check(m==module(0x100)||m==module(0x200),"SDK procedure receives owned injected module");runtime_state->procedures.emplace_back(name);::SetLastError(0x652);
    if(std::string_view(name)=="RoInitialize")return std::bit_cast<FARPROC>(&ro_initialize);
    if(std::string_view(name)=="RoUninitialize")return std::bit_cast<FARPROC>(&ro_uninitialize);
    if(std::string_view(name)=="AppPolicyGetThreadInitializationType")return std::bit_cast<FARPROC>(&policy);
    return nullptr;
}
inline BOOL WINAPI release(HMODULE m){
    auto& s=*runtime_state;s.released.push_back(number(m));if(s.release_hook)s.release_hook();::SetLastError(0x653);
    return s.release_failure_call!=s.released.size();
}
inline std::uint32_t flags(){++runtime_state->flag_calls;::SetLastError(0x654);return runtime_state->process_flags;}
inline td::NativeRuntimeApi runtime_api(){return {load,procedure,release,::GetLastError,flags};}

struct Observation {
    bool no_heap=false,deny_alloc=false,deny_slot=false,deny_publish=false,deny_release=false,wide_lock_success=false;
    unsigned heap_gets=0,allocations=0,frees=0,slot_allocations=0,slot_releases=0,lock_failure_call=0;
    std::vector<LPCRITICAL_SECTION> initialized,destroyed;
    std::map<void*,SIZE_T> blocks;
};
inline Observation* observed=nullptr;
inline HANDLE WINAPI get_heap(){++observed->heap_gets;return observed->no_heap?nullptr: ::GetProcessHeap();}
inline LPVOID WINAPI alloc(HANDLE h,DWORD flags,SIZE_T n){
    auto& s=*observed;++s.allocations;
    check(h==::GetProcessHeap()&&flags==HEAP_ZERO_MEMORY,"domain source calloc uses borrowed process heap and zero flag");
    if(s.deny_alloc){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return nullptr;}
    auto* out=::HeapAlloc(h,flags,n);const auto error=::GetLastError();if(out)s.blocks.emplace(out,n);::SetLastError(error);return out;
}
inline BOOL WINAPI free_heap(HANDLE h,DWORD flags,LPVOID value){
    auto& s=*observed;check(h==::GetProcessHeap()&&!flags&&s.blocks.contains(value),"domain free routes an owned real process-heap block");
    const auto out=::HeapFree(h,flags,value);const auto error=::GetLastError();if(out){s.blocks.erase(value);++s.frees;}::SetLastError(error);return out;
}
inline DWORD WINAPI slot_alloc(PFLS_CALLBACK_FUNCTION callback){
    ++observed->slot_allocations;if(observed->deny_slot){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FLS_OUT_OF_INDEXES;}return ::FlsAlloc(callback);
}
inline BOOL WINAPI slot_free(DWORD index){
    ++observed->slot_releases;if(observed->deny_release){::SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return ::FlsFree(index);
}
inline BOOL WINAPI slot_set(DWORD index,PVOID value){
    if(observed->deny_publish&&value&&value!=ptd::construction_sentinel()){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}return ::FlsSetValue(index,value);
}
inline BOOL WINAPI lock_init(LPCRITICAL_SECTION value,DWORD spin){
    auto& s=*observed;s.initialized.push_back(value);check(spin==0x400,"domain lock selection preserves explicit spin policy");
    if(s.lock_failure_call==s.initialized.size()){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    const auto result=::InitializeCriticalSectionAndSpinCount(value,spin);return result&&s.wide_lock_success?BOOL{0x100}:result;
}
inline void WINAPI lock_destroy(LPCRITICAL_SECTION value){observed->destroyed.push_back(value);::DeleteCriticalSection(value);}
inline td::Apis apis(){
    td::Apis a;a.heap={get_heap,alloc,::HeapReAlloc,free_heap,::HeapSize,::GetLastError};
    a.fls={::FlsGetValue,slot_set,slot_alloc,slot_free,::GetLastError,::SetLastError};
    a.locks={lock_init,lock_destroy,::EnterCriticalSection,::LeaveCriticalSection};a.runtime=runtime_api();return a;
}
inline td::Seed seed(){
    auto identity=std::make_shared<std::array<std::byte,64>>();
    return {0x5a1b2c3d4e5f6071ULL,0,nullptr,nullptr,1,reinterpret_cast<std::uintptr_t>(identity.get()),identity};
}
struct UnusedHeap final:anadius::crt_heap_recovered::Effects {
    std::uintptr_t get_process_heap()override{throw std::logic_error("unused standalone locale heap");}
    void* heap_alloc(std::uintptr_t,std::uint32_t,std::uint64_t)override{throw std::logic_error("unused standalone locale heap");}
    bool heap_free(std::uintptr_t,std::uint32_t,void*)override{throw std::logic_error("unused standalone locale heap");}
    std::uint32_t last_error()override{throw std::logic_error("unused standalone locale heap");}
    std::uint32_t errno_from_os_error_05d650(std::uint32_t)override{throw std::logic_error("unused standalone locale heap");}
    void write_error_slot_05d724(std::uint32_t)override{throw std::logic_error("unused standalone locale heap");}
    void lock_0625f8(std::uint32_t)override{throw std::logic_error("unused standalone locale heap");}
    void unlock_06264c(std::uint32_t)override{throw std::logic_error("unused standalone locale heap");}
    std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t)override{throw std::logic_error("unused standalone locale heap");}
};
struct Fixture {
    Observation memory;RuntimeState os;std::shared_ptr<td::DomainStorage> domain;
    Fixture(){check(!observed&&!runtime_state,"core fixtures do not overlap native FLS domains");observed=&memory;runtime_state=&os;domain=td::DomainStorage::create(seed(),apis());}
    ~Fixture(){
        memory.no_heap=false;memory.deny_alloc=false;memory.deny_slot=false;memory.deny_publish=false;memory.deny_release=false;memory.lock_failure_call=0;
        if(domain){try{if(!domain->close())std::terminate();}catch(...){std::terminate();}domain.reset();}
        observed=nullptr;runtime_state=nullptr;
    }
};
} // namespace b6_core_test
