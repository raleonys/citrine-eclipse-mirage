#include "platform.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <cstddef>
#include <stdexcept>
#include <type_traits>

namespace anadius::host_platform {
namespace {
Resources checked(Resources r){
    if(!r.encoding||!r.errors||!r.api.condition||!r.api.verify||!r.api.module||!r.api.procedure)
        throw std::invalid_argument("Complete owned host encoding/errors and SDK API required");
    return r;
}
std::shared_ptr<Domain> checked(std::shared_ptr<Domain> domain){
    if(!domain)throw std::invalid_argument("Explicit shared host/hook domain required");return domain;
}
const wchar_t* module_name(std::u16string_view name){
    if(name==u"ws2_32.dll")return L"ws2_32.dll";
    if(name==u"KernelBase.dll")return L"KernelBase.dll";
    if(name==u"advapi32.dll")return L"advapi32.dll";
    if(name==u"Kernel32.dll")return L"Kernel32.dll";
    if(name==u"shell32.dll")return L"shell32.dll";
    throw std::domain_error("Module name outside observed host domain");
}
const char* procedure_name(std::string_view name){
    for(const auto* text:{"connect","RegOpenKeyExA","RegOpenKeyExW","RegCloseKey","RegQueryValueExA","RegQueryValueExW",
        "CreateProcessA","CreateProcessW","SHGetFolderPathA","SHGetFolderPathW"})if(name==text)return text;
    throw std::domain_error("Export name outside observed host domain");
}
const char* log_text(std::string_view name){
    for(const auto* text:{"MinHook initialization ","succeeded","failed","Enabling hook for ","Hooking function ",
        " failed; error ","hooking special folders","disabling hook failed","MinHook uninitialization ",
        "connect","RegOpenKeyExA","RegOpenKeyExW","RegCloseKey","RegQueryValueExA","RegQueryValueExW",
        "CreateProcessA","CreateProcessW","SHGetFolderPathA","SHGetFolderPathW"})if(name==text)return text;
    throw std::domain_error("Literal outside observed host/hook logging domain");
}
}
HostApi native_host_api() noexcept{return {::VerSetConditionMask,::VerifyVersionInfoW,::GetModuleHandleW,::GetProcAddress};}
Domain::Domain(std::shared_ptr<std::ostream> stream,std::shared_ptr<recovered_hooks::Api> engine,
    std::shared_ptr<recovered_hooks::CallbackResolver> resolver,std::shared_ptr<void> lifetime,Resources resources)
    :callback_lifetime_(std::move(lifetime)),resources_(checked(std::move(resources))),stream_(std::move(stream)),engine_(std::move(engine)),resolver_(std::move(resolver)){
    if(!callback_lifetime_||!stream_||!engine_||!resolver_)
        throw std::invalid_argument("Explicit callback lifetime, stream, engine and resolver owners required");
}
std::u16string Domain::widen(std::string_view value){return encoding_recovered::filesystem_widen_006d70(value,*resources_.encoding,*resources_.errors);}
std::uint64_t Domain::condition(std::uint64_t mask,std::uint32_t type,std::uint8_t value){return resources_.api.condition(mask,type,value);}
bool Domain::verify(const host_init_recovered::VersionRequest& request,std::uint32_t types,std::uint64_t mask){
    OSVERSIONINFOEXW native{};
    native.dwOSVersionInfoSize=request.size;native.dwMajorVersion=request.major;native.dwMinorVersion=request.minor;
    native.dwBuildNumber=request.build;native.dwPlatformId=request.platform;
    for(std::size_t i=0;i<request.service_pack_text.size();++i)native.szCSDVersion[i]=static_cast<WCHAR>(request.service_pack_text[i]);
    native.wServicePackMajor=request.service_pack_major;native.wServicePackMinor=request.service_pack_minor;
    native.wSuiteMask=request.suite;native.wProductType=request.product_type;native.wReserved=request.reserved;
    return resources_.api.verify(&native,types,mask)!=FALSE;
}
std::uintptr_t Domain::module(std::u16string_view name){return reinterpret_cast<std::uintptr_t>(resources_.api.module(module_name(name)));}
std::uintptr_t Domain::procedure(std::uintptr_t module,std::string_view name){return reinterpret_cast<std::uintptr_t>(resources_.api.procedure(reinterpret_cast<HMODULE>(module),procedure_name(name)));}
void Domain::text(std::string_view value){const auto* selected=log_text(value);const auto shared=shared_log::acquire(*stream_);*stream_<<selected;}
void Domain::integer(std::int32_t value){const auto shared=shared_log::acquire(*stream_);*stream_<<value;}
void Domain::finish(){const auto shared=shared_log::acquire(*stream_);*stream_<<std::endl;}
HostPlatform::HostPlatform(std::shared_ptr<Domain> domain):domain_(checked(std::move(domain))),bridge_(*this,*domain_->engine_,*domain_->resolver_){}
std::u16string HostPlatform::filesystem_widen_006d70(std::string_view value){return domain_->widen(value);}
std::uint64_t HostPlatform::version_condition_mask(std::uint64_t mask,std::uint32_t type,std::uint8_t condition){return domain_->condition(mask,type,condition);}
bool HostPlatform::verify_version(const host_init_recovered::VersionRequest& v,std::uint32_t types,std::uint64_t mask){return domain_->verify(v,types,mask);}
std::int32_t HostPlatform::initialize_engine_046a20(){return bridge_.initialize_engine_046a20();}
std::uintptr_t HostPlatform::get_module_handle(std::u16string_view name){return domain_->module(name);}
std::uintptr_t HostPlatform::get_proc_address(std::uintptr_t module,std::string_view name){return domain_->procedure(module,name);}
std::int32_t HostPlatform::create_hook_0466e0(std::uintptr_t target,host_init_recovered::Callback callback,std::uintptr_t& original){
    auto publication=domain_->resources_.original_publication;
    if(!publication)return bridge_.create_hook_0466e0(target,callback,original);
    publication->before_create(callback,target,original);
    try{
        const auto result=bridge_.create_hook_0466e0(target,callback,original);
        publication->after_create(callback,target,original,result);return result;
    }catch(...){publication->unknown();throw;}
}
std::int32_t HostPlatform::enable_hook_046a10(std::uintptr_t target){
    auto publication=domain_->resources_.original_publication;
    if(!publication)return bridge_.enable_hook_046a10(target);
    publication->before_enable(target);
    try{return bridge_.enable_hook_046a10(target);}catch(...){publication->unknown();throw;}
}
void HostPlatform::log_text_006880(std::string_view value){domain_->text(value);}
void HostPlatform::log_integer_00af90(std::int32_t value){domain_->integer(value);}
void HostPlatform::finish_log_0088e0(){domain_->finish();}
HookCleanup::HookCleanup(std::shared_ptr<Domain> domain):domain_(checked(std::move(domain))),bridge_(*this,*domain_->engine_){}
std::int32_t HookCleanup::disable_hook_046a00(std::uintptr_t target){return bridge_.disable_hook_046a00(target);}
std::int32_t HookCleanup::uninitialize_engine_046ac0(){return bridge_.uninitialize_engine_046ac0();}
void HookCleanup::log_literal_006880(std::string_view value){domain_->text(value);}
void HookCleanup::log_finish_0088e0(){domain_->finish();}
bootstrap_backend::Resources bind_host_platform(bootstrap_backend::Resources r,std::shared_ptr<Domain> domain){
    auto host=std::make_unique<HostPlatform>(domain);auto cleanup=std::make_unique<HookCleanup>(std::move(domain));
    r.host=std::move(host);r.hook_cleanup=std::move(cleanup);return r;
}
static_assert(sizeof(OSVERSIONINFOEXW)==0x11c);
static_assert(offsetof(OSVERSIONINFOEXW,dwOSVersionInfoSize)==0&&offsetof(OSVERSIONINFOEXW,dwMajorVersion)==4&&offsetof(OSVERSIONINFOEXW,dwMinorVersion)==8);
static_assert(offsetof(OSVERSIONINFOEXW,dwBuildNumber)==12&&offsetof(OSVERSIONINFOEXW,dwPlatformId)==16&&offsetof(OSVERSIONINFOEXW,szCSDVersion)==20);
static_assert(offsetof(OSVERSIONINFOEXW,wServicePackMajor)==276&&offsetof(OSVERSIONINFOEXW,wServicePackMinor)==278&&offsetof(OSVERSIONINFOEXW,wSuiteMask)==280);
static_assert(offsetof(OSVERSIONINFOEXW,wProductType)==282&&offsetof(OSVERSIONINFOEXW,wReserved)==283);
static_assert(!std::is_abstract_v<HostPlatform>&&!std::is_abstract_v<HookCleanup>);
static_assert(!std::is_move_constructible_v<HostPlatform>&&!std::is_move_constructible_v<HookCleanup>);
} // namespace anadius::host_platform
