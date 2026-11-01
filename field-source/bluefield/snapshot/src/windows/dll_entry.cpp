// SPDX-License-Identifier: GPL-3.0-only
#include "src/libraries/libraries.hpp"
#include <system_error>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include "src/windows/hooks.hpp"
#include "src/windows/unload_guard.hpp"
#include "src/paths/module_relative.hpp"
#include <filesystem>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
namespace {
HMODULE owned_module=nullptr;
HANDLE worker_handle=nullptr,stop_event=nullptr;
volatile LONG shutdown_busy=0;
unsigned worker_thread_id=0;
SRWLOCK active_service_lock=SRWLOCK_INIT;
preloader_l::Service* active_service=nullptr;
std::filesystem::path module_path(HMODULE module){
 std::vector<wchar_t> buffer(512);
 for(;;){DWORD n=GetModuleFileNameW(module,buffer.data(),static_cast<DWORD>(buffer.size()));if(!n)throw std::runtime_error("module path failed");if(n<buffer.size())return std::filesystem::path(buffer.data(),buffer.data()+n);if(buffer.size()>=32768)throw std::length_error("module path limit");buffer.resize(buffer.size()*2);}
}
std::filesystem::path environment_directory(const wchar_t* name){
 DWORD n=GetEnvironmentVariableW(name,nullptr,0);if(!n)throw std::runtime_error("required system directory is unavailable");
 std::vector<wchar_t> bytes(n);const auto got=GetEnvironmentVariableW(name,bytes.data(),n);if(!got||got>=n)throw std::runtime_error("system directory changed during read");return std::filesystem::path(bytes.data(),bytes.data()+got);
}
class EnvironmentLease {
 struct Item {std::string name,value;bool existed;};std::vector<Item> items_;
public:
 EnvironmentLease(){
  for(const auto* name:{"EALsxPort","ContentId","EAGameLocale","OriginEmuCmdArgs"}){
   SetLastError(ERROR_SUCCESS);DWORD n=GetEnvironmentVariableA(name,nullptr,0);const auto error=GetLastError();
   std::string value(n?static_cast<size_t>(n):1,'\0');bool exists=n!=0||error!=ERROR_ENVVAR_NOT_FOUND;
   if(n){DWORD read=GetEnvironmentVariableA(name,value.data(),n);if(read>=n)throw std::runtime_error("environment changed during capture");value.resize(read);}else value.clear();items_.push_back({name,std::move(value),exists});
  }
 }
 bool restore() noexcept{bool restored=true;for(const auto& item:items_)if(!SetEnvironmentVariableA(item.name.c_str(),item.existed?item.value.c_str():nullptr))restored=false;return restored;}
};
EnvironmentLease* environment_lease=nullptr;
class ExtraModules final:private preloader_l::libraries::Effects {
 preloader_l::Environment& env_;
 preloader_l::libraries::NativeSystemCalls calls_;
 preloader_l::libraries::State state_;
 void begin_load_log_0863e0() override{env_.log(preloader_l::LogEvent::phase);}
 void append_log_path_and_flush(std::string_view) override{env_.log(preloader_l::LogEvent::phase);}
 void message_038bc0(std::string title,std::string text,std::uint32_t flags) override{MessageBoxA(nullptr,text.c_str(),title.c_str(),flags);}
 [[noreturn]] void throw_runtime_00cd70(int code) override{throw std::runtime_error("extra-library conversion code="+std::to_string(code));}
 [[noreturn]] void throw_filesystem_00cdb0(std::uint32_t code) override{throw std::system_error(static_cast<int>(code),std::system_category(),"extra-library conversion");}
 void wait_steps_036880(int) override{throw std::logic_error("phase scheduling is not bound through load_list");}
 void finish_flag_036820() override{throw std::logic_error("phase scheduling is not bound through load_list");}
public:
 explicit ExtraModules(preloader_l::Environment& env,const preloader_l::configuration::State& state):env_(env){
  for(const auto& path:state.extra_dlls_from_main)state_.from_main.push_back(preloader_l::native_path(path).wstring());
  for(const auto& path:state.extra_dlls)state_.from_worker.push_back(preloader_l::native_path(path).wstring());
 }
 void load(bool from_main){
  const auto& paths=from_main?state_.from_main:state_.from_worker;
  auto loaded=preloader_l::libraries::load_list_035260(calls_,*this,paths);
  try{state_.loaded.insert(state_.loaded.end(),loaded.begin(),loaded.end());}
  catch(...){for(auto module:loaded)calls_.free_library(module);throw;}
 }
 ~ExtraModules(){preloader_l::libraries::unload_035ab0(calls_,state_);}
};
unsigned __stdcall runtime_worker(void*) noexcept{
 std::unique_ptr<preloader_l::Environment> environment;
 std::unique_ptr<preloader_l::Service> service;
 std::unique_ptr<preloader_l::NativeHooks> hooks;
 std::unique_ptr<ExtraModules> extras;
 try{
  environment_lease=new EnvironmentLease();
  environment=std::make_unique<preloader_l::Environment>(preloader_l::RuntimePaths::from_system(
   module_path(owned_module),module_path(nullptr),environment_directory(L"LOCALAPPDATA"),environment_directory(L"PROGRAMDATA")));
  environment->log(preloader_l::LogEvent::startup);
  environment->log(preloader_l::LogEvent::phase);
  service=std::make_unique<preloader_l::Service>(*environment);
  AcquireSRWLockExclusive(&active_service_lock);active_service=service.get();ReleaseSRWLockExclusive(&active_service_lock);
  if(WaitForSingleObject(stop_event,0)==WAIT_OBJECT_0)service->request_stop();
  service->prepare();
  environment->log(preloader_l::LogEvent::phase);
  hooks=std::make_unique<preloader_l::NativeHooks>(*environment,*service);
  environment->log(preloader_l::LogEvent::phase);
  hooks->install();
  environment->log(preloader_l::LogEvent::phase);
  environment->log(preloader_l::LogEvent::phase);
  extras=std::make_unique<ExtraModules>(*environment,service->application().configuration());
  service->activate_async([&extras](bool from_main){extras->load(from_main);});
  service->wait_startup();
  environment->log(preloader_l::LogEvent::phase);
  environment->log(preloader_l::LogEvent::phase);

  environment->log(preloader_l::LogEvent::phase);
  environment->log(preloader_l::LogEvent::ready);
  if(WaitForSingleObject(stop_event,INFINITE)!=WAIT_OBJECT_0)throw std::runtime_error("stop wait failed");
 }catch(const std::exception&){if(environment){try{environment->log(preloader_l::LogEvent::startup_failed);}catch(...){}}}
 catch(...){if(environment){try{environment->log(preloader_l::LogEvent::startup_failed);}catch(...){}}}
 // Failure and ordinary shutdown share the same ownership cleanup path.
 try {
  if(service)service->stop();
  if(hooks){while(!hooks->quiesce(std::chrono::milliseconds(100)))Sleep(10);hooks.reset();}
  extras.reset();
  AcquireSRWLockExclusive(&active_service_lock);active_service=nullptr;ReleaseSRWLockExclusive(&active_service_lock);
  service.reset();if(environment)environment->log(preloader_l::LogEvent::stopped);
 }catch(...){
  if(environment)environment->log(preloader_l::LogEvent::shutdown_failed);
  for(;;)Sleep(1000); // Host shutdown returns timeout; unsafe unload is refused.
 }
 // All CRT-backed objects are destroyed before _beginthreadex's trampoline
 // runs its CRT thread cleanup. The module self-pin remains until host shutdown.
 return 0;
}
}
extern "C" void SURGE() noexcept {}
extern "C" uint32_t __stdcall DllShutdown(uint32_t timeout_ms) noexcept{
 if(GetCurrentThreadId()==worker_thread_id)return 1;
 if(InterlockedCompareExchange(&shutdown_busy,1,0)!=0)return 1;
 preloader_l::set_shutdown_caller(GetCurrentThreadId());
 uint32_t result=0;
 if(stop_event&&!SetEvent(stop_event))result=3;
 // Hold the published owner alive while cancelling phase waits. The worker
 // clears this pointer under the exclusive lock before destroying Service.
 AcquireSRWLockShared(&active_service_lock);
 try{if(active_service)active_service->request_stop();}catch(...){result=3;}
 ReleaseSRWLockShared(&active_service_lock);
 if(!result&&worker_handle){
  DWORD wait=WaitForSingleObject(worker_handle,timeout_ms==UINT32_MAX?INFINITE:timeout_ms);
  if(wait==WAIT_TIMEOUT)result=2;else if(wait!=WAIT_OBJECT_0)result=3;
 }
 if(!result&&environment_lease){if(!environment_lease->restore())result=9;else{delete environment_lease;environment_lease=nullptr;}}
 if(!result&&worker_handle){if(!CloseHandle(worker_handle))result=7;else worker_handle=nullptr;}
 if(!result&&stop_event){if(!CloseHandle(stop_event))result=7;else stop_event=nullptr;}
 if(!result&&owned_module){HMODULE pin=owned_module;owned_module=nullptr;if(!FreeLibrary(pin)){owned_module=pin;result=8;}}
 InterlockedExchange(&shutdown_busy,0);return result;
}
extern "C" BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID) noexcept{
 if(reason!=DLL_PROCESS_ATTACH)return TRUE;
 // No waits or configuration/hook work under loader lock. The CRT-aware worker
 // owns one real module reference and begins after attach notifications finish.
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,reinterpret_cast<LPCWSTR>(&DllMain),&owned_module))return FALSE;
 stop_event=CreateEventW(nullptr,TRUE,FALSE,nullptr);
 if(!stop_event){FreeLibrary(owned_module);owned_module=nullptr;return FALSE;}
 worker_handle=reinterpret_cast<HANDLE>(_beginthreadex(nullptr,0,&runtime_worker,nullptr,0,&worker_thread_id));
 if(!worker_handle){CloseHandle(stop_event);stop_event=nullptr;FreeLibrary(owned_module);owned_module=nullptr;return FALSE;}
 (void)module;return TRUE;
}
