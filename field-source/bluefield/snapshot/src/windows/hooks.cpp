// SPDX-License-Identifier: GPL-3.0-only
#include "src/windows/hooks.hpp"
#include <ws2tcpip.h>
#include "src/windows/hook_callbacks.hpp"
#include "src/hooks/callback_lock.hpp"
#include "src/windows/unload_guard.hpp"
#include "src/application/providers.hpp"
#include "third_party/minhook/include/MinHook.h"
#include <array>
#include <cstring>
#include <cwctype>
#include <thread>
#include <algorithm>
#include <cstdlib>
#include <climits>
namespace preloader_l {
namespace {
std::u16string read_wide(const void* pointer){
 if(!pointer)throw std::invalid_argument("null UTF16 source");
 std::u16string text;auto* bytes=static_cast<const std::byte*>(pointer);
 for(size_t i=0;;++i){char16_t unit;std::memcpy(&unit,bytes+i*sizeof unit,sizeof unit);if(!unit)return text;text.push_back(unit);}
}
class Wide final:public native_callbacks::WideOperations {
 int compare_utf16(const void* a,const void* b) override{
  auto left=read_wide(a),right=read_wide(b);
  std::wstring x(left.begin(),left.end()),y(right.begin(),right.end());return _wcsicmp(x.c_str(),y.c_str());
 }
 int copy_utf16(void* destination,size_t capacity,const void* source) override{
  auto text=read_wide(source);
  if(!destination||capacity<=text.size())throw std::invalid_argument("UTF16 secure-copy contract");
  std::memcpy(destination,text.c_str(),(text.size()+1)*sizeof(char16_t));return 0;
 }
};
class HookErrors final:public native_callbacks::Errors {
 Environment& env_;
public:
 explicit HookErrors(Environment& e):env_(e){}
 [[noreturn]] void lifecycle(native_callbacks::LifecycleFault) override{throw std::runtime_error("hook binding ownership/busy failure");}
 [[noreturn]] void callback_exception(host::Callback,std::exception_ptr) noexcept override{
  try{env_.log(LogEvent::failure);}catch(...){}
  RaiseFailFastException(nullptr,nullptr,0);std::abort();
 }
};
class Effects final:public native_callbacks::OriginalEffects,public host::Effects {
 Environment& env_;Service& service_;native_callbacks::Resolver& resolver_;
 std::mutex attempts_mutex_;
public:
 bool owns_engine=false;
 Effects(const std::array<std::uintptr_t,host::callback_count>& originals,Wide& wide,Environment& e,Service& s,native_callbacks::Resolver& r):native_callbacks::OriginalEffects(originals,wide),env_(e),service_(s),resolver_(r){}
 void crypt_observed(crypt_query::Selection selection) noexcept override{
  thread_local bool logging=false;if(logging)return;logging=true;
  env_.log(LogEvent::phase);logging=false;
 }
 std::string filesystem_narrow(std::u16string_view value) override{
  std::wstring wide(value.begin(),value.end());if(wide.empty())return {};
  if(wide.size()>INT_MAX)throw std::length_error("filesystem conversion");
  const UINT cp=AreFileApisANSI()?CP_ACP:CP_OEMCP;int n=WideCharToMultiByte(cp,0,wide.data(),static_cast<int>(wide.size()),nullptr,0,nullptr,nullptr);
  if(n<=0)throw std::runtime_error("filesystem conversion failed");std::string bytes(static_cast<size_t>(n),'\0');
  if(WideCharToMultiByte(cp,0,wide.data(),static_cast<int>(wide.size()),bytes.data(),n,nullptr,nullptr)!=n)throw std::runtime_error("filesystem conversion fill failed");return bytes;
 }
 int crt_stricmp(const char* a,const char* b) override{return _stricmp(a,b);}
 int crt_strcpy_s(char* target,size_t n,const char* source) override{return ::strcpy_s(target,n,source);}
 registry::Handle allocate_4_04b0bc() override{return reinterpret_cast<registry::Handle>(new std::byte[4]);}
 void free_04b0f8(registry::Handle handle) override{delete[] reinterpret_cast<std::byte*>(handle);}
 void wait_036880(std::int32_t count) override{
  const auto incoming=GetLastError();
  {callback_lock::Pause pause;(void)service_.wait_steps(count);}
  SetLastError(incoming);
 }
 void wait_steps_036880(std::int32_t count) override{(void)service_.wait_steps(count);}
 std::uint32_t increment_attempts(std::uint32_t& value) override{std::lock_guard lock(attempts_mutex_);return ++value;}
 void log_literal_006880(std::uint64_t) override{env_.log(LogEvent::phase);}
 void log_literal_006880(std::string_view s) override{env_.log(LogEvent::phase);}
 void log_text_007990(std::string_view s) override{env_.log(LogEvent::phase);}
 void finish_log_0088e0() override{env_.log(LogEvent::phase);}
 void log_port_00ad70(std::uint16_t p) override{env_.log(LogEvent::phase);}
 void log_finish_0088e0() override{env_.log(LogEvent::phase);}
 void dialog_038bc0(std::string_view title,std::string_view text,std::uint32_t flags) override{std::string t(title),s(text);MessageBoxA(nullptr,s.c_str(),t.c_str(),flags);}
 [[noreturn]] void exit_05cdc0(std::uint32_t n) override{ExitProcess(n);}
 std::u16string filesystem_widen_006d70(std::string_view s) override{return filesystem_path(s);}
 std::uint64_t version_condition_mask(std::uint64_t a,std::uint32_t t,std::uint8_t c) override{return VerSetConditionMask(a,t,c);}
 bool verify_version(const host::VersionRequest& v,std::uint32_t type,std::uint64_t mask) override{
  OSVERSIONINFOEXW n{};n.dwOSVersionInfoSize=sizeof n;n.dwMajorVersion=v.major;n.dwMinorVersion=v.minor;n.dwBuildNumber=v.build;n.dwPlatformId=v.platform;
  std::copy(v.service_pack_text.begin(),v.service_pack_text.end(),n.szCSDVersion);n.wServicePackMajor=v.service_pack_major;n.wServicePackMinor=v.service_pack_minor;n.wSuiteMask=v.suite;n.wProductType=v.product_type;n.wReserved=v.reserved;
  return VerifyVersionInfoW(&n,type,mask)!=FALSE;
 }
 std::int32_t initialize_engine_046a20() override{auto result=MH_Initialize();owns_engine=result==MH_OK;return result;}
 host::Address get_module_handle(std::u16string_view value) override{std::wstring wide(value.begin(),value.end());return reinterpret_cast<host::Address>(GetModuleHandleW(wide.c_str()));}
 host::Address get_proc_address(host::Address module,std::string_view name) override{std::string key(name);return reinterpret_cast<host::Address>(GetProcAddress(reinterpret_cast<HMODULE>(module),key.c_str()));}
 std::int32_t create_hook_0466e0(host::Address target,host::Callback callback,host::Address& original) override{
  void* trampoline=nullptr;auto result=MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(resolver_.rebuilt_address(callback)),&trampoline);original=reinterpret_cast<host::Address>(trampoline);return result;
 }
 std::int32_t queue_hook_enable(host::Address target) override{return MH_QueueEnableHook(reinterpret_cast<void*>(target));}
 void trace_hook(std::string_view phase,std::string_view name,std::int32_t status) noexcept override{
  try{env_.log(LogEvent::phase);}catch(...){}
 }
 void log_text_006880(std::string_view value) override{env_.log(LogEvent::phase);}
 void log_integer_00af90(std::int32_t n) override{env_.log(LogEvent::phase);}
};
}
struct NativeHooks::Impl {
 Environment& env;Service& service;host::Values values;std::vector<host::Address> targets;
 std::array<host::Address,host::callback_count> originals{};registry::Handles client,language,fabricated;
 std::int32_t ipv4=sizeof(sockaddr_in),ipv6=sizeof(sockaddr_in6);std::uint32_t attempts=0;
 Wide wide;HookErrors errors;native_callbacks::Resolver resolver;Effects effects;
 native_callbacks::Context context;std::unique_ptr<native_callbacks::Binding> binding;
 bool installed=false,disabled=false;
 Impl(Environment& e,Service& s):env(e),service(s),errors(e),effects(originals,wide,e,s,resolver),context{{values,client,language,fabricated,s.application().configuration().registry_spoof},{ipv4,ipv6,s.application().configuration().server_port,attempts},effects,effects,effects,effects}{}
 void install(){
  if(binding)throw std::logic_error("hooks already installed");binding=std::make_unique<native_callbacks::Binding>(context,errors);
  auto& config=service.application().configuration();
  {
   std::lock_guard lock(context.serialization);
   host::initialize_host_00e310({config,values,targets,originals},effects);
   const size_t expected=6+(config.block_reruns?2:0)+(!config.documents.empty()?2:0);
   if(!effects.owns_engine||targets.size()!=expected)throw std::runtime_error("required native hook staging incomplete");
  // A real import keeps CRYPT32 in the DLL dependency graph. The same owned
  // MinHook instance, callback lease and teardown apply to this required hook.
  const auto target=reinterpret_cast<host::Address>(&::CryptQueryObject);
  effects.trace_hook("create begin","CryptQueryObject",0);
  auto result=effects.create_hook_0466e0(target,host::Callback::crypt_query_001264,originals[host::slot(host::Callback::crypt_query_001264)]);
  effects.trace_hook("create end","CryptQueryObject",result);
  if(result!=MH_OK)throw std::runtime_error("CryptQueryObject hook creation failed: "+std::to_string(result));
  effects.trace_hook("queue begin","CryptQueryObject",0);
  result=effects.queue_hook_enable(target);
  effects.trace_hook("queue end","CryptQueryObject",result);
  if(result!=MH_OK)throw std::runtime_error("CryptQueryObject hook queue failed: "+std::to_string(result));
  targets.push_back(target);
  }
  // No detour is live until the full table is staged. Never hold Context's
  // serialization lock while peers can first enter newly enabled detours.
  env.log(LogEvent::phase);
  const auto result=MH_ApplyQueued();
  env.log(LogEvent::phase);
  if(result!=MH_OK)throw std::runtime_error("native hook batch apply failed: "+std::to_string(result));
  installed=true;
  env.log(LogEvent::phase);
 }
 bool quiesce(std::chrono::milliseconds timeout){
  if(!binding)return true;
  if(effects.owns_engine&&!disabled){if(MH_DisableHook(MH_ALL_HOOKS)!=MH_OK)return false;disabled=true;}
  const auto until=std::chrono::steady_clock::now()+timeout;
  while(native_callbacks::active_calls()!=0){if(std::chrono::steady_clock::now()>=until)return false;std::this_thread::sleep_for(std::chrono::milliseconds(1));}
  if(!external_threads_outside_module())return false;
  // Trampolines and callback Context remain alive until leased calls and
  // conservative external-thread stack/register checks both quiesce.
  if(effects.owns_engine&&MH_Uninitialize()!=MH_OK)return false;
  effects.owns_engine=false;binding->close();binding.reset();
  for(auto handle:fabricated)effects.free_04b0f8(handle);fabricated.clear();targets.clear();installed=false;return true;
 }
};
NativeHooks::NativeHooks(Environment& e,Service& s):impl_(std::make_unique<Impl>(e,s)){}
NativeHooks::~NativeHooks(){if(impl_&&!impl_->quiesce(std::chrono::seconds(5)))std::terminate();}
void NativeHooks::install(){impl_->install();}
bool NativeHooks::quiesce(std::chrono::milliseconds n){return impl_->quiesce(n);}
}
