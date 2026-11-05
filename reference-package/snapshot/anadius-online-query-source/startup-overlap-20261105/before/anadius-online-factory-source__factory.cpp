#include "factory.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include "../anadius-module-reference-source/reference.hpp"
#include <bit>
#include <filesystem>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace anadius::online_factory {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}void restore()const{::SetLastError(value);}void capture(){value=::GetLastError();}};
template<class F>F take_function(F& donor){F result(std::move(donor));donor=nullptr;return result;}
const char* literal(std::string_view value){
    for(const auto* text:{"online dll found!","version function not found","online library version mismatch!","factory function not found","authentication failed, ","starting offline","authentication ok"})if(value==text)return text;
    throw std::domain_error("Literal outside observed online logging domain");
}
bool empty(const online_version::Snapshot& s){return !s.module&&!s.procedure&&!s.active&&!s.poisoned&&!s.code_retained&&!s.factory&&!s.object&&!s.object_unknown&&!s.factory_code_retained;}
Options checked(bootstrap_backend::CallbackStateView view,Options& options){
    if(!options.stream||!options.dialog||!options.os||!options.encoding||!options.errors||!options.stats||!options.files||!options.get_proc||!options.code_lease||!lifetime_configuration_valid(options))
        throw std::invalid_argument("Complete owned online leaves, real code lease and selected lifetime configuration required");
    if(view.runtime.online_module||view.runtime.online_object)
        throw std::invalid_argument("New online binding cannot adopt seeded foreign module/object handles");
    return take_options(options);
}
template<class F,class G>auto managed(F&& fn,G&& invoke){
    Error error;using Result=decltype(fn());std::function<Result()> lower(std::forward<F>(fn));error.restore();
    try{const auto result=invoke(lower);error.capture();return result;}catch(...){error.capture();throw;}
}
}
Options take_options(Options& o){
    return Options{std::move(o.lifetime),std::move(o.stream),std::move(o.dialog),std::move(o.os),std::move(o.encoding),std::move(o.errors),std::move(o.stats),std::move(o.files),std::exchange(o.get_proc,nullptr),take_function(o.code_lease),take_function(o.foreign_quiescent),o.lifetime_profile};
}
bool lifetime_configuration_valid(const Options& o)noexcept{
    switch(o.lifetime_profile){
    case LifetimeProfile::Cooperative:return bool(o.foreign_quiescent);
    case LifetimeProfile::OriginalCrt:return !o.foreign_quiescent;
    }return false;
}
void select_native_original_online(Options& o){
    Error error;const auto vacant=[&]{return o.lifetime_profile==LifetimeProfile::Cooperative&&!o.code_lease&&!o.foreign_quiescent;};
    if(!vacant())throw std::invalid_argument("Unselected Online lifetime and empty lease/foreign-proof slots required");
    auto lease=module_reference::make_code_lease();
    if(!vacant())throw std::logic_error("Online selection changed during native lease construction");
    o.code_lease.swap(lease);o.lifetime_profile=LifetimeProfile::OriginalCrt;
}
struct Online::Impl {
    bootstrap_backend::CallbackStateView view;Options options;
    std::unique_ptr<online_version::Binding> binding;
    online_version::Snapshot retired_binding{};
    online_recovered::RecoveredEffects recovered;
    const DWORD controller=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    mutable std::mutex gate;Phase phase=Phase::Open;std::size_t active=0;bool used=false;
    Impl(Online& owner,bootstrap_backend::CallbackStateView v,Options&& o):view(v),options(checked(v,o)),
        recovered(owner,*options.os,*options.stats,*options.encoding,*options.errors,*options.files){
        binding=std::make_unique<online_version::Binding>(std::move(options.code_lease));
        // Binding is adopted and Options still owns all backing before an SBO
        // donor's destructor can call the still-unattached facade.
        options.code_lease={};
    }
    void affinity()const{if(controller!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Online close/retirement requires its controller and fiber");}
    void poison(){Error error;std::lock_guard lock(gate);if(phase!=Phase::Closed&&phase!=Phase::Retiring)phase=Phase::Poisoned;}
    struct Call {
        Impl& v;explicit Call(Impl& value):v(value){Error error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Open){if(v.phase==Phase::Checking)v.phase=Phase::Poisoned;throw std::logic_error("Online source admission is closed");}++v.active;v.used=true;}
        void complete(){Error error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Open)throw std::logic_error("Online provider lost source admission");}
        ~Call(){Error error;std::lock_guard lock(v.gate);--v.active;}
    };
    template<class F>auto invoke(F&& fn)->std::invoke_result_t<F>{
        Call call(*this);
        try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){fn();call.complete();}else{auto result=fn();call.complete();return result;}}
        catch(...){poison();throw;}
    }
    bool actual_empty()const{return !view.runtime.online_module&&!view.runtime.online_object&&binding&&empty(binding->snapshot());}
};
Online::Online(bootstrap_backend::CallbackStateView view,Options&& options):impl_(nullptr){impl_=std::make_unique<Impl>(*this,view,std::move(options));}
Online::Impl& Online::attached()const{if(!impl_)throw std::logic_error("Online facade is unattached");return *impl_;}
Online::~Online()noexcept{
    if(!impl_)return;auto& v=*impl_;
    try{
        Error error;v.affinity();
        {std::lock_guard lock(v.gate);if(v.active||v.phase==Phase::Checking||v.phase==Phase::Poisoned)std::terminate();if(v.phase!=Phase::Closed&&v.used)std::terminate();}
        if(!v.actual_empty())std::terminate();
        {std::lock_guard lock(v.gate);v.phase=Phase::Retiring;v.retired_binding=v.binding->snapshot();}
        // Keep facade/gate/view referents alive through both Binding's code
        // captures and every leaf/predicate/backing deleter. snapshot uses the
        // cached retired metadata while unique_ptr reset has detached Binding.
        v.binding.reset();
        {Options retiring=take_options(v.options);}
    }catch(...){std::terminate();}
}
bool Online::matches(bootstrap_backend::CallbackStateView view)const noexcept{return impl_&&&impl_->view.configuration==&view.configuration&&&impl_->view.runtime==&view.runtime&&&impl_->view.execution==&view.execution;}
Snapshot Online::snapshot()const{
    Error error;auto& v=attached();Phase phase;std::size_t active;online_version::Snapshot binding;
    {std::lock_guard lock(v.gate);phase=v.phase;active=v.active;binding=v.binding?v.binding->snapshot():v.retired_binding;}
    return {phase,active,binding,&v.view.configuration,&v.view.runtime,&v.view.execution};
}
bool Online::close_after_cleanup(){
    Error error;auto& v=attached();v.affinity();
    {std::lock_guard lock(v.gate);if(v.phase==Phase::Closed)return true;if(v.phase==Phase::Retiring)throw std::logic_error("Online facade is retiring");if(v.phase==Phase::Checking){v.phase=Phase::Poisoned;throw std::logic_error("Online close reentry");}if(v.phase==Phase::Poisoned||v.active)return false;v.phase=Phase::Checking;}
    try{
        if(!v.actual_empty()){std::lock_guard lock(v.gate);if(v.phase==Phase::Checking)v.phase=Phase::Open;return false;}
        // Original profile still requires actual local module/object/binding
        // emptiness and zero active calls above; it adds no external proof
        // callback or arbitrary image-unload guarantee.
        const bool final=v.options.lifetime_profile==LifetimeProfile::OriginalCrt||v.options.foreign_quiescent();
        const bool still_empty=v.actual_empty();
        {std::lock_guard lock(v.gate);if(v.phase!=Phase::Checking)return false;if(!still_empty){v.phase=Phase::Poisoned;return false;}v.phase=final?Phase::Closed:Phase::Open;}
        return final;
    }catch(...){v.poison();throw;}
}
std::uint32_t Online::suppress_errors_0450a0(){auto& v=attached();return v.invoke([&]{return v.recovered.suppress_errors_0450a0();});}
void Online::restore_error_mode(std::uint32_t mode){auto& v=attached();v.invoke([&]{v.recovered.restore_error_mode(mode);});}
configuration::Path Online::filesystem_widen_006d70(std::string_view s){auto& v=attached();return v.invoke([&]{return v.recovered.filesystem_widen_006d70(s);});}
configuration::Path Online::parent_path_019820(const configuration::Path& p){auto& v=attached();return v.invoke([&]{return v.recovered.parent_path_019820(p);});}
configuration::Path Online::join_016b00(const configuration::Path& a,const configuration::Path& b){auto& v=attached();return v.invoke([&]{return v.recovered.join_016b00(a,b);});}
online_recovered::Module Online::load_copy_0450d0(const configuration::Path& p){auto& v=attached();return v.invoke([&]{return managed([&]{return v.recovered.load_copy_0450d0(p);},[&](const auto& lower){return v.binding->load(lower);});});}
online_recovered::Procedure Online::get_proc_address(online_recovered::Module module,std::string_view name){
    auto& v=attached();const char* symbol;if(name=="version_check")symbol="version_check";else if(name=="create_online")symbol="create_online";else throw std::domain_error("Unobserved online export");
    return v.invoke([&]{return managed([&]{return reinterpret_cast<online_recovered::Procedure>(v.options.get_proc(reinterpret_cast<HMODULE>(module),symbol));},[&](const auto& lower){return name=="version_check"?v.binding->resolve(module,lower):v.binding->resolve_factory(module,lower);});});
}
std::int32_t Online::call_version_check(online_recovered::Procedure p){auto& v=attached();return v.invoke([&]{return v.binding->call(p);});}
online_recovered::Object Online::call_create_online(online_recovered::Procedure p,const char16_t* cookie){auto& v=attached();return v.invoke([&]{return v.binding->create(p,cookie);});}
std::int32_t Online::status_virtual_08(online_recovered::Object o){auto& v=attached();return v.invoke([&]{return v.binding->status(o);});}
const char* Online::username_virtual_10(online_recovered::Object o){auto& v=attached();return v.invoke([&]{return v.binding->getter(o,online_query::Slot::Username10);});}
const char* Online::persona_virtual_18(online_recovered::Object o){auto& v=attached();return v.invoke([&]{return v.binding->getter(o,online_query::Slot::Persona18);});}
const char* Online::user_id_virtual_20(online_recovered::Object o){auto& v=attached();return v.invoke([&]{return v.binding->getter(o,online_query::Slot::UserId20);});}
std::string Online::copy_username(online_recovered::Object o){auto& v=attached();return v.invoke([&]{return v.binding->copy_getter(o,online_query::Slot::Username10);});}
std::string Online::copy_persona(online_recovered::Object o){auto& v=attached();return v.invoke([&]{return v.binding->copy_getter(o,online_query::Slot::Persona18);});}
std::string Online::copy_user_id(online_recovered::Object o){auto& v=attached();return v.invoke([&]{return v.binding->copy_getter(o,online_query::Slot::UserId20);});}
bool Online::avatar_virtual_28(online_recovered::Object o,const online_recovered::Path& path,const char* owner,std::int32_t size){auto& v=attached();return v.invoke([&]{return v.binding->avatar(o,path.c_str(),owner,size);});}
const char* Online::auth_virtual_30(online_recovered::Object o,const char* client){auto& v=attached();return v.invoke([&]{return v.binding->auth(o,client);});}
std::string Online::copy_auth(online_recovered::Object o,const char* client){auto& v=attached();return v.invoke([&]{return v.binding->copy_auth(o,client);});}
void Online::destroy_virtual_00(online_recovered::Object o){auto& v=attached();v.invoke([&]{v.binding->destroy(o);});}
bool Online::free_library(online_recovered::Module m){auto& v=attached();return v.invoke([&]{return managed([&]{return v.recovered.free_library(m);},[&](const auto& lower){return v.binding->free(m,lower);});});}
status_recovered::Status Online::status_01ea20(const configuration::Path& p,std::uint32_t flags){auto& v=attached();return v.invoke([&]{return v.recovered.status_01ea20(p,flags);});}
[[noreturn]]void Online::filesystem_error_018130(std::string_view operation,std::uint32_t code,const configuration::Path& path){
    auto& v=attached();Impl::Call call(v);
    throw std::filesystem::filesystem_error(std::string(operation),std::filesystem::path(path),std::error_code(std::bit_cast<std::int32_t>(code),std::system_category()));
}
std::string Online::read_binary_03b3f0(const configuration::Path& p){auto& v=attached();return v.invoke([&]{return v.recovered.read_binary_03b3f0(p);});}
bool Online::write_binary_03c1d0(const configuration::Path& p,std::string_view text){auto& v=attached();return v.invoke([&]{return v.recovered.write_binary_03c1d0(p,text);});}
void Online::log_literal_006880(std::string_view text){auto& v=attached();const auto* selected=literal(text);v.invoke([&]{const auto lease=shared_log::acquire(*v.options.stream);*v.options.stream<<selected;});}
void Online::log_finish_0088e0(){auto& v=attached();v.invoke([&]{const auto lease=shared_log::acquire(*v.options.stream);*v.options.stream<<std::endl;});}
void Online::dialog_038bc0(std::string_view title,std::string_view text,std::uint32_t flags){auto& v=attached();v.invoke([&]{v.options.dialog->show(title,text,flags);});}
void Online::wait_steps_036880(std::int32_t count){auto& v=attached();v.invoke([&]{v.view.execution.wait_steps_036880(count);});}
void Online::finish_flag_036820(){auto& v=attached();v.invoke([&]{v.view.execution.finish_flag_036820(v.view.runtime.online_ready);});}
std::unique_ptr<bootstrap_backend::OnlineComponent> make_online(bootstrap_backend::CallbackStateView view,Options& options){return std::unique_ptr<bootstrap_backend::OnlineComponent>(new Online(view,take_options(options)));}
void install_online(bootstrap_backend::Resources& resources,Options& options){
    if(resources.online||resources.online_factory)throw std::invalid_argument("Online target must be empty");
    struct Pending {Options options;bool consumed=false;explicit Pending(Options& o):options(take_options(o)){} };
    auto pending=std::make_shared<Pending>(options);
    resources.online_factory=[pending](bootstrap_backend::CallbackStateView view){if(pending->consumed)throw std::logic_error("Online factory is single-use");pending->consumed=true;return make_online(view,pending->options);};
}
static_assert(!std::is_abstract_v<Online>&&!std::is_move_constructible_v<Online>);
} // namespace anadius::online_factory
