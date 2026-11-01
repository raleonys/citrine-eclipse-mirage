// SPDX-License-Identifier: GPL-3.0-only
#include "src/profiles/online_session.hpp"
#include "src/application/providers.hpp"
#include "src/libraries/libraries.hpp"
#ifdef _WIN32
#include <windows.h>
#endif
namespace preloader_l {
namespace {
template<class Fn>Fn procedure(std::uintptr_t pointer){if(!pointer)throw SessionFailure("unbound online procedure");return reinterpret_cast<Fn>(pointer);}
template<class Fn>Fn method(online::Object object,size_t slot){
 if(!object)throw SessionFailure("unbound online object");
 return procedure<Fn>((*reinterpret_cast<std::uintptr_t**>(object))[slot]);
}
}
OnlineSession::OnlineSession(configuration::State& s,Environment& e,std::uint8_t& online):state_(s),env_(e),online_(online){}
OnlineSession::~OnlineSession(){try{close();}catch(...){std::terminate();}}
OnlineSession::ProfileResult OnlineSession::initialize(){
 // Native provider calls run without the application's request-state lock.
 // Stage only the fields this source worker writes; publish after completion.
 ProfileResult result{state_.username,state_.persona_id,state_.user_id,state_.avatar,0};
 online::StateRefs refs{state_.executable_path,state_.cookie_path,state_.last_profile_path,state_.last_avatar_path,result.avatar,module_,object_,result.username,result.persona_id,result.user_id,state_.use_last_online_profile,result.online};
 online::worker_035b60(refs,*this);
 if(!finished_)throw SessionFailure("online initialization did not complete");
 return result;
}
void OnlineSession::close(){
 if(object_){destroy_virtual_00(object_);object_=0;}
 if(module_){if(!free_library(module_))throw SessionFailure("online module close pending");module_=0;}
 if(error_mode_)restore_error_mode(*error_mode_);online_=0;
}
const char* OnlineSession::auth(const char* client){
 using Fn=const char*(*)(void*,const char*);
 const auto* value=method<Fn>(object_,6)(reinterpret_cast<void*>(object_),client);
 if(!value)throw SessionFailure("online auth returned null");auth_storage_=value;return auth_storage_.c_str();
}
bool OnlineSession::avatar(const Path& path,std::string_view owner,std::int32_t size){
 using Fn=std::uint8_t(*)(void*,const wchar_t*,const char*,std::int32_t);
 std::wstring wide(path.begin(),path.end());const std::string user(owner);
 return method<Fn>(object_,5)(reinterpret_cast<void*>(object_),wide.c_str(),user.c_str(),size)!=0;
}
std::uint32_t OnlineSession::suppress_errors_0450a0(){
#ifdef _WIN32
 libraries::NativeSystemCalls calls;const auto previous=libraries::suppress_errors_0450a0(calls);error_mode_=previous;return previous;
#else
 error_mode_=0;return 0;
#endif
}
void OnlineSession::restore_error_mode(std::uint32_t mode){
#ifdef _WIN32
 SetErrorMode(mode);
#else
 (void)mode;
#endif
 error_mode_.reset();
}
online::Module OnlineSession::load_copy_0450d0(const Path& path){
 if(native_path(path).filename()!=provider_filename)return 0;
 const auto file=env_.paths.provider(); // only trusted module directory; never cwd/EXE fallback
 if(!std::filesystem::is_regular_file(file))return 0;
#ifdef _WIN32
 return reinterpret_cast<online::Module>(LoadLibraryExW(file.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32));
#else
 throw SessionFailure("optional native provider requires Windows");
#endif
}

online::Procedure OnlineSession::get_proc_address(online::Module module,std::string_view name){
#ifdef _WIN32
 const std::string key(name);return reinterpret_cast<online::Procedure>(GetProcAddress(reinterpret_cast<HMODULE>(module),key.c_str()));
#else
 (void)module;(void)name;throw SessionFailure("Windows online provider ABI");
#endif
}
std::int32_t OnlineSession::call_version_check(online::Procedure p){return procedure<std::int32_t(*)()>(p)();}
online::Object OnlineSession::call_create_online(online::Procedure p,const char16_t* cookie){env_.paths.require_write(native_path(Path(cookie)));std::filesystem::create_directories(env_.paths.profile_cache());cookie_parameter_.clear();while(*cookie)cookie_parameter_.push_back(static_cast<wchar_t>(*cookie++));return reinterpret_cast<online::Object>(procedure<void*(*)(const wchar_t*)>(p)(cookie_parameter_.c_str()));}
std::int32_t OnlineSession::status_virtual_08(online::Object o){return method<std::int32_t(*)(void*)>(o,1)(reinterpret_cast<void*>(o));}
const char* OnlineSession::username_virtual_10(online::Object o){return method<const char*(*)(void*)>(o,2)(reinterpret_cast<void*>(o));}
const char* OnlineSession::persona_virtual_18(online::Object o){return method<const char*(*)(void*)>(o,3)(reinterpret_cast<void*>(o));}
const char* OnlineSession::user_id_virtual_20(online::Object o){return method<const char*(*)(void*)>(o,4)(reinterpret_cast<void*>(o));}
void OnlineSession::destroy_virtual_00(online::Object o){method<void(*)(void*)>(o,0)(reinterpret_cast<void*>(o));}
bool OnlineSession::free_library(online::Module module){
#ifdef _WIN32
 if(!FreeLibrary(reinterpret_cast<HMODULE>(module)))throw SessionFailure("online FreeLibrary failed");return true;
#else
 (void)module;throw SessionFailure("Windows online provider ABI");
#endif
}
status_recovered::Status OnlineSession::status_01ea20(const Path& p,std::uint32_t flags){
 return file_status(p,flags);
}
[[noreturn]] void OnlineSession::filesystem_error_018130(std::string_view op,std::uint32_t n,const Path&){throw SessionFailure(std::string(op)+": "+std::to_string(n));}
void OnlineSession::dialog_038bc0(std::string_view title,std::string_view text,std::uint32_t flags){
 env_.log(LogEvent::provider);
#ifdef _WIN32
 const std::string caption(title),body(text);MessageBoxA(nullptr,body.c_str(),caption.c_str(),flags);
#else
 (void)title;(void)text;(void)flags;
#endif
}
void OnlineSession::wait_steps_036880(std::int32_t){if(!state_.server_port)throw SessionFailure("online provider started before transport bind");}
}

namespace preloader_l {
std::string OnlineSession::read_binary_03b3f0(const Path& p){
 // The source reader returns raw bytes. Parsing remains in online.cpp.
 return read_file(p);
}
}
