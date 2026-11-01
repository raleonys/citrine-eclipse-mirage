#include "runtime.hpp"
#include <appmodel.h>
#include <roapi.h>
#include <winternl.h>
#include <cstring>
#include <exception>
#include <list>
#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace anadius::thread_domain {
namespace {
struct LastError {
    DWORD value=::GetLastError();
    ~LastError(){::SetLastError(value);}
};
std::uint32_t process_flags(){
    const auto* teb=::NtCurrentTeb();
    if(!teb||!teb->ProcessEnvironmentBlock||!teb->ProcessEnvironmentBlock->ProcessParameters)
        throw std::runtime_error("Current process parameters unavailable");
    // Retained x64 native source selects Flags at offset +8. SDK exposes the
    // containing process-parameter prefix as reserved bytes, not a guessed
    // full private structure. This reads only the CURRENT process's live data.
    std::uint32_t value;
    const auto* parameters=teb->ProcessEnvironmentBlock->ProcessParameters;
    std::memcpy(&value,reinterpret_cast<const std::byte*>(parameters)+8,sizeof(value));
    return value;
}
template<class Char>void no_nul(std::basic_string_view<Char> s){
    if(s.find(Char{})!=s.npos)throw std::invalid_argument("Embedded NUL outside runtime SDK name domain");
}
}
NativeRuntimeApi native_runtime_api() noexcept{return {::LoadLibraryExW,::GetProcAddress,::FreeLibrary,::GetLastError,&process_flags};}
struct NativeRuntime::Impl {
    enum class Kind {Pending,Owned,Releasing,Unknown};
    struct Ticket {HMODULE module=nullptr;Kind kind=Kind::Pending;};
    NativeRuntimeApi api;mutable std::mutex gate;std::list<Ticket> tickets;
    std::size_t active=0;bool closing=false,closed=false,poisoned=false,control_busy=false;
    explicit Impl(NativeRuntimeApi a):api(a){}
    struct Call {
        Impl& owner;
        explicit Call(Impl& v):owner(v){
            std::lock_guard lock(v.gate);
            if(v.closing||v.closed||v.poisoned)throw std::logic_error("Runtime API admission is closed");
            ++v.active;
        }
        ~Call(){LastError error;std::lock_guard lock(owner.gate);--owner.active;}
    };
    using Iterator=std::list<Ticket>::iterator;
    Iterator release_ticket(HMODULE module){
        for(auto it=tickets.begin();it!=tickets.end();++it)
            if(it->module==module&&it->kind==Kind::Owned){it->kind=Kind::Releasing;return it;}
        throw std::invalid_argument("Runtime release requires an owned module reference");
    }
    bool release(Iterator it,LastError& error){
        BOOL result;
        ::SetLastError(error.value);
        try{result=api.release(it->module);error.value=api.error();}
        catch(...){
            error.value=api.error();std::lock_guard lock(gate);
            it->kind=Kind::Unknown;poisoned=true;throw;
        }
        {std::lock_guard lock(gate);if(result)tickets.erase(it);else it->kind=Kind::Owned;}
        return result!=FALSE;
    }
};
NativeRuntime::NativeRuntime(NativeRuntimeApi api){
    if(!api.load||!api.procedure||!api.release||!api.error||!api.process_flags)
        throw std::invalid_argument("Complete runtime SDK API required");
    impl_=std::make_unique<Impl>(api);
}
NativeRuntime::~NativeRuntime() noexcept {
    std::lock_guard lock(impl_->gate);
    // Do not unload cached procedure code while workers or their RO/FLS exit
    // paths may still use it. A never-loaded constructor needs no close call.
    if(!impl_->tickets.empty()||impl_->active||impl_->control_busy)std::terminate();
}
crt_runtime_recovered::Module NativeRuntime::load_library_ex_w(std::u16string_view name,crt_runtime_recovered::Module file,std::uint32_t flags){
    LastError error;Impl::Call call(*impl_);no_nul(name);const std::wstring path(name.begin(),name.end());
    Impl::Iterator ticket;
    {std::lock_guard lock(impl_->gate);ticket=impl_->tickets.emplace(impl_->tickets.end());}
    HMODULE module;
    ::SetLastError(error.value);
    try{module=impl_->api.load(path.c_str(),reinterpret_cast<HANDLE>(file),static_cast<DWORD>(flags));error.value=impl_->api.error();}
    catch(...){
        error.value=impl_->api.error();std::lock_guard lock(impl_->gate);
        ticket->kind=Impl::Kind::Unknown;impl_->poisoned=true;throw;
    }
    {std::lock_guard lock(impl_->gate);
        if(module){ticket->module=module;ticket->kind=Impl::Kind::Owned;}
        else impl_->tickets.erase(ticket);
    }
    return reinterpret_cast<crt_runtime_recovered::Module>(module);
}
std::uint32_t NativeRuntime::last_error(){return impl_->api.error();}
bool NativeRuntime::free_library(crt_runtime_recovered::Module module){
    LastError error;Impl::Call call(*impl_);Impl::Iterator ticket;
    {std::lock_guard lock(impl_->gate);ticket=impl_->release_ticket(reinterpret_cast<HMODULE>(module));}
    return impl_->release(ticket,error);
}
crt_runtime_recovered::Address NativeRuntime::get_proc_address(crt_runtime_recovered::Module module,std::string_view name){
    LastError error;Impl::Call call(*impl_);no_nul(name);const std::string text(name);
    {
        std::lock_guard lock(impl_->gate);bool owned=false;
        for(const auto& t:impl_->tickets)if(t.module==reinterpret_cast<HMODULE>(module)&&t.kind==Impl::Kind::Owned){owned=true;break;}
        if(!owned)throw std::invalid_argument("Runtime procedure lookup requires an owned module reference");
    }
    ::SetLastError(error.value);const auto result=impl_->api.procedure(reinterpret_cast<HMODULE>(module),text.c_str());error.value=impl_->api.error();
    return reinterpret_cast<crt_runtime_recovered::Address>(result);
}
std::uint32_t NativeRuntime::process_parameters_flags(){
    LastError error;Impl::Call call(*impl_);::SetLastError(error.value);
    const auto result=impl_->api.process_flags();error.value=impl_->api.error();return result;
}
std::int32_t NativeRuntime::call_ro_initialize(crt_runtime_recovered::Address address,std::uint32_t mode){
    LastError error;Impl::Call call(*impl_);
    if(!address||mode>1)throw std::invalid_argument("Valid rebuilt RoInitialize export and RO_INIT_TYPE required");
    ::SetLastError(error.value);const auto result=reinterpret_cast<decltype(&::RoInitialize)>(address)(static_cast<RO_INIT_TYPE>(mode));error.value=impl_->api.error();
    return static_cast<std::int32_t>(result);
}
void NativeRuntime::call_ro_uninitialize(crt_runtime_recovered::Address address){
    LastError error;Impl::Call call(*impl_);
    if(!address)throw std::invalid_argument("Valid rebuilt RoUninitialize export required");
    ::SetLastError(error.value);reinterpret_cast<decltype(&::RoUninitialize)>(address)();error.value=impl_->api.error();
}
std::uint32_t NativeRuntime::call_thread_initialization_policy(crt_runtime_recovered::Address address,std::uintptr_t token,std::int32_t* output){
    LastError error;Impl::Call call(*impl_);
    if(!address||!output)throw std::invalid_argument("Valid rebuilt AppPolicy export and live output cell required");
    static_assert(sizeof(AppPolicyThreadInitializationType)==sizeof(*output));
    AppPolicyThreadInitializationType policy;
    // Output-only SDK cell: bytewise transfer preserves caller initial/output
    // bits even when the export returns failure, without reading a non-enum
    // bit pattern as an enum C++ value. No forged int32_t*/enum* alias is used.
    std::memcpy(&policy,output,sizeof(policy));::SetLastError(error.value);
    const auto result=reinterpret_cast<decltype(&::AppPolicyGetThreadInitializationType)>(address)(reinterpret_cast<HANDLE>(token),&policy);
    error.value=impl_->api.error();std::memcpy(output,&policy,sizeof(policy));return static_cast<std::uint32_t>(result);
}
bool NativeRuntime::close(){
    LastError error;
    {
        std::lock_guard lock(impl_->gate);
        if(impl_->closed)return true;
        if(impl_->control_busy)throw std::logic_error("Runtime close reentry");
        impl_->closing=true;
        if(impl_->active||impl_->poisoned)return false;
        impl_->control_busy=true;
    }
    struct Control {Impl& value;~Control(){LastError error;std::lock_guard lock(value.gate);value.control_busy=false;}} control{*impl_};
    for(;;){
        Impl::Iterator ticket;
        {
            std::lock_guard lock(impl_->gate);
            if(impl_->tickets.empty()){impl_->closed=true;return true;}
            ticket=impl_->tickets.begin();
            if(ticket->kind!=Impl::Kind::Owned)return false;
            ticket->kind=Impl::Kind::Releasing;
        }
        if(!impl_->release(ticket,error))return false;
    }
}
std::size_t NativeRuntime::outstanding_modules() const {LastError error;std::lock_guard lock(impl_->gate);return impl_->tickets.size();}
bool NativeRuntime::closed() const {LastError error;std::lock_guard lock(impl_->gate);return impl_->closed;}
bool NativeRuntime::tracking_poisoned() const {LastError error;std::lock_guard lock(impl_->gate);return impl_->poisoned;}
static_assert(!std::is_abstract_v<NativeRuntime>);
static_assert(sizeof(std::uintptr_t)==8&&sizeof(LONG)==4);
} // namespace anadius::thread_domain
