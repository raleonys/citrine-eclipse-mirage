#include "platform.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include "../anadius-online-version-source/version.hpp"
#include <bit>
#include <filesystem>
#include <stdexcept>
#include <type_traits>
#include <functional>
#include <utility>

namespace anadius::online_platform {
namespace {
// Type-erased lower callbacks are constructed/destroyed inside the error guard.
// Their allocation metadata cannot replace the actual lower API error state.
template<class F,class G>auto managed(F&& fn,G&& invoke){
    struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}void restore()const{::SetLastError(value);}void capture(){value=::GetLastError();}} error;
    using Result=decltype(fn());std::function<Result()> lower(std::forward<F>(fn));
    error.restore();
    try{const auto result=invoke(lower);error.capture();return result;}catch(...){error.capture();throw;}
}
Resources checked(Resources r){
    if(!r.os||!r.encoding||!r.errors||!r.stats||!r.files||!r.get_proc)
        throw std::invalid_argument("Complete owned online OS/path/file resources required");
    if(r.native_objects&&!r.version)throw std::invalid_argument("Native online objects require the same owned module/version binding");
    if(r.native_queries&&!r.native_objects)throw std::invalid_argument("Native online queries require adopted native objects");
    return r;
}
const char* literal(std::string_view value){
    // Existing035bd0 uses complete retained C-string literals. Map only those
    // values to owned static NUL-terminated storage; never scan a borrowed view.
    for(const auto* text:{"online dll found!","version function not found",
        "online library version mismatch!","factory function not found",
        "authentication failed, ","starting offline","authentication ok"})
        if(value==text)return text;
    throw std::domain_error("Literal outside observed online logging domain");
}
}
OnlinePlatform::OnlinePlatform(std::shared_ptr<std::ostream> stream,
    std::shared_ptr<library_effects::Dialog> dialog,std::unique_ptr<online_recovered::Effects> lower,Resources resources)
    :resources_(checked(std::move(resources))),stream_(std::move(stream)),dialog_(std::move(dialog)),lower_(std::move(lower)),
     recovered_(*this,*resources_.os,*resources_.stats,*resources_.encoding,*resources_.errors,*resources_.files){
    if(!stream_||!dialog_||!lower_)throw std::invalid_argument("Owned stream, dialog and foreign/phase provider required");
}
std::uint32_t OnlinePlatform::suppress_errors_0450a0(){return recovered_.suppress_errors_0450a0();}
void OnlinePlatform::restore_error_mode(std::uint32_t mode){recovered_.restore_error_mode(mode);}
configuration::Path OnlinePlatform::filesystem_widen_006d70(std::string_view value){
    return recovered_.filesystem_widen_006d70(value);
}
configuration::Path OnlinePlatform::parent_path_019820(const configuration::Path& p){return recovered_.parent_path_019820(p);}
configuration::Path OnlinePlatform::join_016b00(const configuration::Path& p,const configuration::Path& q){return recovered_.join_016b00(p,q);}
online_recovered::Module OnlinePlatform::load_copy_0450d0(const configuration::Path& p){
    if(resources_.version)return managed([&]{return recovered_.load_copy_0450d0(p);},[&](const auto& lower){return resources_.version->load(lower);});
    return recovered_.load_copy_0450d0(p);
}
online_recovered::Procedure OnlinePlatform::get_proc_address(online_recovered::Module module,std::string_view name){
    // Observed export-name domain only. Native GetProcAddress requires a
    // terminated string; ordinals/arbitrary borrowed views are not adopted.
    const char* symbol;
    if(name=="version_check")symbol="version_check";
    else if(name=="create_online")symbol="create_online";
    else throw std::domain_error("Export name outside observed online domain");
    const auto lookup=[&]{return reinterpret_cast<online_recovered::Procedure>(resources_.get_proc(reinterpret_cast<HMODULE>(module),symbol));};
    if(name=="version_check"&&resources_.version)return managed(lookup,[&](const auto& lower){return resources_.version->resolve(module,lower);});
    if(name=="create_online"&&resources_.native_objects)return managed(lookup,[&](const auto& lower){return resources_.version->resolve_factory(module,lower);});
    return lookup();
}
bool OnlinePlatform::free_library(online_recovered::Module module){
    if(resources_.version)return managed([&]{return recovered_.free_library(module);},[&](const auto& lower){return resources_.version->free(module,lower);});
    return recovered_.free_library(module);
}
status_recovered::Status OnlinePlatform::status_01ea20(const configuration::Path& p,std::uint32_t flags){
    return recovered_.status_01ea20(p,flags);
}
[[noreturn]] void OnlinePlatform::filesystem_error_018130(std::string_view operation,std::uint32_t code,const configuration::Path& path){
    throw std::filesystem::filesystem_error(std::string(operation),std::filesystem::path(path),
        std::error_code(std::bit_cast<std::int32_t>(code),std::system_category()));
}
std::string OnlinePlatform::read_binary_03b3f0(const configuration::Path& path){return recovered_.read_binary_03b3f0(path);}
bool OnlinePlatform::write_binary_03c1d0(const configuration::Path& path,std::string_view value){return recovered_.write_binary_03c1d0(path,value);}
void OnlinePlatform::log_literal_006880(std::string_view text){const auto* selected=literal(text);const auto shared=shared_log::acquire(*stream_);*stream_<<selected;}
void OnlinePlatform::log_finish_0088e0(){const auto shared=shared_log::acquire(*stream_);*stream_<<std::endl;}
void OnlinePlatform::dialog_038bc0(std::string_view title,std::string_view text,std::uint32_t flags){dialog_->show(title,text,flags);}
std::int32_t OnlinePlatform::call_version_check(online_recovered::Procedure p){return resources_.version?resources_.version->call(p):lower_->call_version_check(p);}
online_recovered::Object OnlinePlatform::call_create_online(online_recovered::Procedure p,const char16_t* cookie){return resources_.native_objects?resources_.version->create(p,cookie):lower_->call_create_online(p,cookie);}
std::int32_t OnlinePlatform::status_virtual_08(online_recovered::Object o){if(resources_.native_queries)return resources_.version->status(o);return resources_.native_objects?resources_.version->object_call(o,[&]{return lower_->status_virtual_08(o);}):lower_->status_virtual_08(o);}
const char* OnlinePlatform::username_virtual_10(online_recovered::Object o){if(resources_.native_queries)return resources_.version->getter(o,online_query::Slot::Username10);return resources_.native_objects?resources_.version->object_call(o,[&]{return lower_->username_virtual_10(o);}):lower_->username_virtual_10(o);}
const char* OnlinePlatform::persona_virtual_18(online_recovered::Object o){if(resources_.native_queries)return resources_.version->getter(o,online_query::Slot::Persona18);return resources_.native_objects?resources_.version->object_call(o,[&]{return lower_->persona_virtual_18(o);}):lower_->persona_virtual_18(o);}
const char* OnlinePlatform::user_id_virtual_20(online_recovered::Object o){if(resources_.native_queries)return resources_.version->getter(o,online_query::Slot::UserId20);return resources_.native_objects?resources_.version->object_call(o,[&]{return lower_->user_id_virtual_20(o);}):lower_->user_id_virtual_20(o);}
std::string OnlinePlatform::copy_username(Object o){if(resources_.native_queries)return resources_.version->copy_getter(o,online_query::Slot::Username10);return resources_.native_objects?resources_.version->object_call(o,[&]{return lower_->copy_username(o);}):lower_->copy_username(o);}
std::string OnlinePlatform::copy_persona(Object o){if(resources_.native_queries)return resources_.version->copy_getter(o,online_query::Slot::Persona18);return resources_.native_objects?resources_.version->object_call(o,[&]{return lower_->copy_persona(o);}):lower_->copy_persona(o);}
std::string OnlinePlatform::copy_user_id(Object o){if(resources_.native_queries)return resources_.version->copy_getter(o,online_query::Slot::UserId20);return resources_.native_objects?resources_.version->object_call(o,[&]{return lower_->copy_user_id(o);}):lower_->copy_user_id(o);}
void OnlinePlatform::destroy_virtual_00(online_recovered::Object o){if(resources_.native_objects)resources_.version->destroy(o);else lower_->destroy_virtual_00(o);}
void OnlinePlatform::wait_steps_036880(std::int32_t count){lower_->wait_steps_036880(count);}
void OnlinePlatform::finish_flag_036820(){lower_->finish_flag_036820();}
bootstrap_backend::Resources bind_online_platform(bootstrap_backend::Resources r,std::shared_ptr<std::ostream> stream,
    std::shared_ptr<library_effects::Dialog> dialog,Resources resources){
    r.online=std::make_unique<OnlinePlatform>(std::move(stream),std::move(dialog),std::move(r.online),std::move(resources));
    return r;
}
static_assert(!std::is_abstract_v<OnlinePlatform>);
static_assert(!std::is_move_constructible_v<OnlinePlatform>);
static_assert(sizeof(wchar_t)==sizeof(char16_t));
} // namespace anadius::online_platform
