#include "effects.hpp"
#include "../anadius-application-io-source/files.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include "../anadius-path-source/paths.hpp"
#include <stdexcept>
#include <type_traits>
#include <exception>

namespace anadius::content_effects {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
Options checked(Options o){
    if(!o.stream||!o.dialog||!o.filesystem||!o.files||!o.clock||!o.formatter||!o.encoding||!o.errors||!o.atoi||!o.copy_file)
        throw std::invalid_argument("Complete owned content providers and SDK calls required");
    return o;
}
}
struct Content::Impl {
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    bootstrap_backend::CallbackStateView view;const achievement_storage::Paths& paths;online_factory::Online& online;
    Options options;bool retiring=false,busy=false,poisoned=false;content_handlers_recovered::CallCheckpoint checkpoint{};
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Content requires original Graph worker/fiber");}
    void check()const{Error error;affinity();if(retiring||poisoned)throw std::logic_error("Content admission closed or unknown");files.ensure_known();if(checkpoint.check)checkpoint.check(checkpoint.context);}
    template<class F>std::invoke_result_t<F> leaf(F&& f){check();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();check();}else{std::invoke_result_t<F> result=f();check();return result;}}
    struct Call {Impl& v;explicit Call(Impl& v):v(v){v.affinity();if(v.busy){v.poisoned=true;throw std::logic_error("Content operation is nonreentrant");}v.check();v.busy=true;}~Call(){v.busy=false;}};
    struct Clock final:achievement_runtime::Clock {Impl& v;explicit Clock(Impl& v):v(v){}
        std::int64_t time_05eeb8()override{return v.leaf([&]{return v.options.clock->time_05eeb8();});}
        std::optional<achievement_runtime::BrokenDown> gmtime_05fffc(std::int64_t n)override{return v.leaf([&]{return v.options.clock->gmtime_05fffc(n);});}
    } clock{*this};
    struct Formatter final:achievement_runtime::Formatter {Impl& v;explicit Formatter(Impl& v):v(v){}
        std::string put_time_03d610(const achievement_runtime::BrokenDown& b,std::string_view p)override{return v.leaf([&]{return v.options.formatter->put_time_03d610(b,p);});}
    } formatter{*this};
    struct Encoding final:encoding_recovered::Calls {Impl& v;explicit Encoding(Impl& v):v(v){}
        std::uint32_t locale_codepage_0629ec()override{return v.leaf([&]{return v.options.encoding->locale_codepage_0629ec();});}
        bool are_file_apis_ansi()override{return v.leaf([&]{return v.options.encoding->are_file_apis_ansi();});}
        int multi_byte_to_wide(std::uint32_t c,std::uint32_t f,const char* s,int n,char16_t* p,int z)override{return v.leaf([&]{return v.options.encoding->multi_byte_to_wide(c,f,s,n,p,z);});}
        int wide_to_multi_byte(std::uint32_t c,std::uint32_t f,const char16_t* s,int n,char* p,int z,bool* u)override{return v.leaf([&]{return v.options.encoding->wide_to_multi_byte(c,f,s,n,p,z,u);});}
        std::uint32_t last_error()override{return v.leaf([&]{return v.options.encoding->last_error();});}
    } encoding{*this};
    struct Directories final:achievement_writer::Effects {Impl& v;explicit Directories(Impl& v):v(v){}
        void create_directories_018e00(const configuration::Path& p)override{v.leaf([&]{v.options.filesystem->create_directories(p,{&v,[](void* raw){static_cast<Impl*>(raw)->check();}});});}
    } directories{*this};
    application_io::GuardedFiles files{*options.files,{this,[](void* p){static_cast<Impl*>(p)->check();}}};
    void newline(){const auto c=leaf([&]{return options.stream->widen('\n');});leaf([&]{options.stream->put(c);});leaf([&]{options.stream->flush();});}
    struct Engine final:achievement_runtime::PersistenceEffects {
        Impl& v;explicit Engine(Impl& v):PersistenceEffects(v.view.configuration,v.paths,v.clock,v.formatter,v.directories,v.files),v(v){}
        std::int32_t atoi_05ee08(std::string_view s)override{const std::string input(s);return v.leaf([&]{return v.options.atoi(input.c_str());});}
        void unknown_entitlement_field(std::string_view text)override{
            {const auto lease=shared_log::acquire(*v.options.stream);v.leaf([&]{*v.options.stream<<text;});v.newline();}
            v.leaf([&]{v.options.dialog->show("UNKNOWN MESSAGE",text,0x10);});
        }
        configuration::Path filesystem_widen_006d70(std::string_view s)override{return encoding_recovered::filesystem_widen_006d70(s,v.encoding,*v.options.errors);}
        configuration::Path path_append_016b50(const configuration::Path& a,const configuration::Path& b)override{auto result=a;path_recovered::append_016b50(result,b);return result;}
        bool online_avatar_virtual_28(const configuration::Path&)override{throw online_recovered::OutOfEvidenceInput("Avatar requires owner and size; select AvatarRequest");}
        bool avatar(const configuration::Path& p,const char* owner,std::int32_t size){return v.leaf([&]{return v.online.avatar_virtual_28(v.view.runtime.online_object,p,owner,size);});}
        bool copy_overwrite_048d20(const configuration::Path& a,const configuration::Path& b)override{
            static_assert(sizeof(wchar_t)==sizeof(char16_t));return v.leaf([&]{return v.options.copy_file(reinterpret_cast<LPCWSTR>(a.c_str()),reinterpret_cast<LPCWSTR>(b.c_str()),FALSE)!=FALSE;});
        }
        std::string path_to_utf8_031ca0(const configuration::Path& p)override{return encoding_recovered::path_utf8_01e3f0(p,v.encoding,*v.options.errors);}
        void log_saved_size_085530(std::int32_t size)override{const auto lease=shared_log::acquire(*v.options.stream);v.leaf([&]{*v.options.stream<<"saved size: ";});v.leaf([&]{*v.options.stream<<size;});v.newline();}
    } engine{*this};
    Impl(bootstrap_backend::CallbackStateView v,const achievement_storage::Paths& p,online_factory::Online& o,Options options)
        :view(v),paths(p),online(o),options(checked(std::move(options))){if(!online.matches(view))throw std::invalid_argument("Content Online must match exact Runtime view");}
};
Content::Content(bootstrap_backend::CallbackStateView v,const achievement_storage::Paths& p,online_factory::Online& o,Options options)
    :impl_(nullptr){impl_=std::make_unique<Impl>(v,p,o,std::move(options));}
Content::Impl& Content::attached()const{if(!impl_)throw std::logic_error("Content is unattached");return *impl_;}
Content::~Content()noexcept{if(!impl_)return;auto& v=*impl_;if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy||v.checkpoint.check||v.files.has_pending())std::terminate();v.retiring=true;{Options retiring=std::move(v.options);}}
template<class F>auto Content::invoke(F&& f)->std::invoke_result_t<F>{auto& v=attached();Impl::Call call(v);try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();v.check();}else{std::invoke_result_t<F> result=f();v.check();return result;}}catch(...){v.poisoned=true;throw;}}
std::int32_t Content::atoi_05ee08(std::string_view s){return invoke([&]{return impl_->engine.atoi_05ee08(s);});}
void Content::unknown_entitlement_field(std::string_view s){invoke([&]{impl_->engine.unknown_entitlement_field(s);});}
std::string Content::utc_time_041b60(std::uint64_t& n){return invoke([&]{return impl_->engine.utc_time_041b60(n);});}
std::unique_ptr<content_handlers_recovered::DocumentWriter> Content::printer_0019b0(){return invoke([&]{return impl_->engine.printer_0019b0();});}
void Content::write_xml_03c1d0(const configuration::Path& p,std::string_view s){invoke([&]{impl_->engine.write_xml_03c1d0(p,s);});}
void Content::save_steam_01f9a0(const content_handlers_recovered::AchievementMap& m){invoke([&]{impl_->engine.save_steam_01f9a0(m);});}
configuration::Path Content::filesystem_widen_006d70(std::string_view s){return invoke([&]{return impl_->engine.filesystem_widen_006d70(s);});}
configuration::Path Content::path_append_016b50(const configuration::Path& a,const configuration::Path& b){return invoke([&]{return impl_->engine.path_append_016b50(a,b);});}
bool Content::online_avatar_virtual_28(const configuration::Path& p){return invoke([&]{return impl_->engine.online_avatar_virtual_28(p);});}
bool Content::request_online_avatar(const configuration::Path& p,const char* o,std::int32_t n){return invoke([&]{return impl_->engine.avatar(p,o,n);});}
bool Content::copy_overwrite_048d20(const configuration::Path& a,const configuration::Path& b){return invoke([&]{return impl_->engine.copy_overwrite_048d20(a,b);});}
std::string Content::path_to_utf8_031ca0(const configuration::Path& p){return invoke([&]{return impl_->engine.path_to_utf8_031ca0(p);});}
void Content::log_saved_size_085530(std::int32_t n){invoke([&]{impl_->engine.log_saved_size_085530(n);});}
void Content::bind_content_checkpoint(content_handlers_recovered::CallCheckpoint c){auto& v=attached();v.affinity();if(v.busy||v.checkpoint.check){v.poisoned=true;throw std::logic_error("Content checkpoint overlaps active call");}v.check();if(!c.context||!c.check)throw std::invalid_argument("Complete content checkpoint required");{Error error;c.check(c.context);}v.checkpoint=c;}
void Content::clear_content_checkpoint()noexcept{if(!impl_)return;auto& v=*impl_;if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy)std::terminate();v.checkpoint={};}
static_assert(!std::is_abstract_v<Content>);
}
