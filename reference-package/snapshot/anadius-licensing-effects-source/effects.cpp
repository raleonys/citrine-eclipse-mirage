#include "effects.hpp"
#include "../anadius-application-io-source/files.hpp"
#include "../anadius-path-source/paths.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include "../anadius-token-source/token.hpp"
#include <bit>
#include <filesystem>
#include <optional>
#include <type_traits>

namespace anadius::licensing_effects {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
Options checked(Options o){if(!o.stream||!o.dialog||!o.files||!o.text||!o.stats||!o.encoding||!o.errors||!o.clock||!o.formatter||!o.module||!o.procedure||!o.exit_process)throw std::invalid_argument("Complete owned licensing providers and SDK calls required");return o;}
}
struct Core::Impl {
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    bootstrap_backend::CallbackStateView view;online_factory::Online& online;Options options;
    bool busy=false,poisoned=false,retiring=false;content_handlers_recovered::CallCheckpoint checkpoint{};
    void* handler_context=nullptr;void (*handler_check)(void*)=nullptr;
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Licensing requires original Graph worker/fiber");}
    void check()const{Error error;affinity();if(poisoned||retiring)throw std::logic_error("Licensing admission closed or unknown");files.ensure_known();if(handler_check)handler_check(handler_context);if(checkpoint.check)checkpoint.check(checkpoint.context);}
    template<class F>std::invoke_result_t<F> leaf(F&& f){check();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();check();}else{std::invoke_result_t<F> result=f();check();return result;}}
    struct Call {Impl& v;explicit Call(Impl& v):v(v){v.affinity();if(v.busy){v.poisoned=true;throw std::logic_error("Licensing operation is nonreentrant");}v.check();v.busy=true;}~Call(){v.busy=false;}};
    application_io::GuardedFiles files{*options.files,{this,[](void* p){static_cast<Impl*>(p)->check();}}};
    struct Encoding final:encoding_recovered::Calls {Impl& v;explicit Encoding(Impl& v):v(v){}
        std::uint32_t locale_codepage_0629ec()override{return v.leaf([&]{return v.options.encoding->locale_codepage_0629ec();});}
        bool are_file_apis_ansi()override{return v.leaf([&]{return v.options.encoding->are_file_apis_ansi();});}
        int multi_byte_to_wide(std::uint32_t c,std::uint32_t f,const char* s,int n,char16_t* p,int z)override{return v.leaf([&]{return v.options.encoding->multi_byte_to_wide(c,f,s,n,p,z);});}
        int wide_to_multi_byte(std::uint32_t c,std::uint32_t f,const char16_t* s,int n,char* p,int z,bool* u)override{return v.leaf([&]{return v.options.encoding->wide_to_multi_byte(c,f,s,n,p,z,u);});}
        std::uint32_t last_error()override{return v.leaf([&]{return v.options.encoding->last_error();});}
    } encoding{*this};
    struct Clock final:achievement_runtime::Clock {Impl& v;explicit Clock(Impl& v):v(v){}
        std::int64_t time_05eeb8()override{return v.leaf([&]{return v.options.clock->time_05eeb8();});}
        std::optional<achievement_runtime::BrokenDown> gmtime_05fffc(std::int64_t n)override{return v.leaf([&]{return v.options.clock->gmtime_05fffc(n);});}
    } clock{*this};
    struct Formatter final:achievement_runtime::Formatter {Impl& v;explicit Formatter(Impl& v):v(v){}
        std::string put_time_03d610(const achievement_runtime::BrokenDown& b,std::string_view p)override{return v.leaf([&]{return v.options.formatter->put_time_03d610(b,p);});}
    } formatter{*this};
    struct Text final:hash_recovered::TextEnvironment {Impl& v;explicit Text(Impl& v):v(v){}
        std::string format_digest_043270(const hash_recovered::DigestWords& w)override{return v.leaf([&]{return v.options.text->format_digest_043270(w);});}
        std::int32_t tolower_05fca0(std::int32_t c)override{return v.leaf([&]{return v.options.text->tolower_05fca0(c);});}
    } text{*this};
    struct HashInput final:hash_recovered::InputStream {std::unique_ptr<file_recovered::Input> input;explicit HashInput(std::unique_ptr<file_recovered::Input> p):input(std::move(p)){if(!input)throw std::invalid_argument("Null hash input");}
        std::uint32_t initial_state()const override{return input->initial_state();}
        std::int32_t peek_043790()override{return input->peek_043790();}
        std::size_t read_03b2a0(std::span<std::uint8_t> b)override{return input->read_03b2a0(b);}
        void close_03ae70()override{input->close_03ae70();}
    };
    struct HashFiles final:hash_recovered::FileFactory {Impl& v;explicit HashFiles(Impl& v):v(v){}
        std::unique_ptr<hash_recovered::InputStream> open_input_039f70(const configuration::Path& p,std::uint32_t m,std::uint32_t s)override{return std::make_unique<HashInput>(v.files.open_input_039f70(p,m,s));}
    } hash_files{*this};
    license_recovered::FileStatus status(const configuration::Path& p,std::uint32_t flags){const auto r=status_recovered::status_01ea20(p,flags,stats);return {r.kind,std::bit_cast<std::int32_t>(r.error)};}
    struct Stats final:status_recovered::Stats {Impl& v;explicit Stats(Impl& v):v(v){}
        status_recovered::StatsResult query(const configuration::Path& p,std::uint32_t flags)override{return v.leaf([&]{return v.options.stats->query(p,flags);});}
    } stats{*this};
    [[noreturn]] void fs_error(std::string_view operation,std::int32_t code,const configuration::Path& p){throw std::filesystem::filesystem_error(std::string(operation),std::filesystem::path(p),std::error_code(code,std::system_category()));}
    struct Token final:token_recovered::Dependencies {Impl& v;explicit Token(Impl& v):v(v){}
        token_recovered::FileStatus status_01ea20(const configuration::Path& p,std::uint32_t flags)override{const auto r=v.status(p,flags);return {r.kind,r.error};}
        [[noreturn]] void filesystem_error_018130(std::string_view op,std::int32_t c,const configuration::Path& p)override{v.fs_error(op,c,p);}
        std::string read_binary_03b3f0(const configuration::Path& p)override{return file_recovered::read_binary_03b3f0(p,v.files);}
    } token{*this};
    Impl(bootstrap_backend::CallbackStateView v,online_factory::Online& o,Options options):view(v),online(o),options(checked(std::move(options))){if(!online.matches(view))throw std::invalid_argument("Licensing Online must match exact Runtime view");}
};
Core::Core(bootstrap_backend::CallbackStateView v,online_factory::Online& o,Options options):impl_(nullptr){impl_=std::make_unique<Impl>(v,o,std::move(options));}
Core::Impl& Core::attached()const{if(!impl_)throw std::logic_error("Licensing is unattached");return *impl_;}
Core::~Core()noexcept{if(!impl_)return;auto& v=*impl_;if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy||v.checkpoint.check||v.files.has_pending())std::terminate();v.retiring=true;{Options retiring=std::move(v.options);}}
template<class F>auto Core::invoke(F&& f,bool control)->std::invoke_result_t<F>{auto& v=attached();Impl::Call call(v);try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();v.check();}else{std::invoke_result_t<F> result=f();v.check();return result;}}catch(const NoResponseRequiredError&){if(!control){v.poisoned=true;throw;}try{v.check();}catch(...){v.poisoned=true;throw;}throw;}catch(...){v.poisoned=true;throw;}}
const char* Core::online_auth_virtual_30(const char* s){return invoke([&]{return impl_->leaf([&]{return impl_->online.auth_virtual_30(impl_->view.runtime.online_object,s);});});}
std::string Core::copy_online_auth(const char* s){return invoke([&]{return impl_->leaf([&]{return impl_->online.copy_auth(impl_->view.runtime.online_object,s);});});}
license_recovered::FileStatus Core::status_01ea20(const configuration::Path& p,std::uint32_t f){return invoke([&]{return impl_->status(p,f);});}
[[noreturn]]void Core::filesystem_error_018130(std::string_view op,std::int32_t c,const configuration::Path& p){invoke([&]{impl_->fs_error(op,c,p);});std::terminate();}
bool Core::file_hash_matches_042d20(const configuration::Path& p,std::string expected){return invoke([&]{return hash_recovered::file_hash_matches_042d20(p,std::move(expected),impl_->hash_files,impl_->text);});}
std::string Core::load_token_02afa0(const configuration::Path& p){return invoke([&]{return token_recovered::load_token_02afa0(p,impl_->token);});}
configuration::Path Core::filesystem_widen_006d70(std::string_view s){return invoke([&]{return encoding_recovered::filesystem_widen_006d70(s,impl_->encoding,*impl_->options.errors);});}
configuration::Path Core::filename_00e120(const configuration::Path& p){return invoke([&]{return path_recovered::filename_00e120(p);});}
configuration::Path Core::parent_path_019820(const configuration::Path& p){return invoke([&]{return path_recovered::parent_path_019820(p);});}
configuration::Path Core::join_016b00(const configuration::Path& a,const configuration::Path& b){return invoke([&]{return path_recovered::join_016b00(a,b);});}
std::string Core::path_utf8_01e3f0(const configuration::Path& p){return invoke([&]{return encoding_recovered::path_utf8_01e3f0(p,impl_->encoding,*impl_->options.errors);});}
std::string Core::current_time_041b60_null_true(){return invoke([&]{return achievement_runtime::utc_time_041b60(impl_->clock,impl_->formatter,nullptr,1);});}
bool Core::wine_detected_045010(){return invoke([&]{auto& v=*impl_;const auto m=v.leaf([&]{return v.options.module("ntdll.dll");});return m&&v.leaf([&]{return v.options.procedure(m,"wine_get_version");})!=nullptr;});}
void Core::log_literal_006880(std::string_view s){invoke([&]{const std::string owned(s);const auto lease=shared_log::acquire(*impl_->options.stream);impl_->leaf([&]{*impl_->options.stream<<owned.c_str();});});}
void Core::log_string_006860(std::string_view s){invoke([&]{const auto lease=shared_log::acquire(*impl_->options.stream);impl_->leaf([&]{*impl_->options.stream<<s;});});}
bool Core::write_ticket_03c1d0(const configuration::Path& p,std::string_view s){return invoke([&]{return file_recovered::write_binary_03c1d0(p,s,impl_->files);});}
void Core::dialog_038bc0(std::string_view title,std::string_view s,std::uint32_t f){invoke([&]{impl_->leaf([&]{impl_->options.dialog->show(title,s,f);});});}
[[noreturn]]void Core::exit_05cdc0(std::int32_t code){invoke([&]{impl_->check();impl_->options.exit_process(code);std::terminate();});std::terminate();}
[[noreturn]]void Core::no_response_02bcd0_04d228(){invoke([]{throw NoResponseRequiredError{};},true);std::terminate();}
void Core::unknown_message_07bd70(const messages_recovered::UnknownMessageInput& input){invoke([&]{
    auto& v=*impl_;
    auto newline=[&]{const auto c=v.leaf([&]{const auto lease=shared_log::acquire(*v.options.stream);return v.options.stream->widen('\n');});v.leaf([&]{const auto lease=shared_log::acquire(*v.options.stream);v.options.stream->put(c);});v.leaf([&]{const auto lease=shared_log::acquire(*v.options.stream);v.options.stream->flush();});};
    v.leaf([&]{const auto lease=shared_log::acquire(*v.options.stream);*v.options.stream<<"[UNKNOWN MESSAGE]";});newline();
    const auto counted=v.leaf([&]{return input.view();});
    v.leaf([&]{const auto lease=shared_log::acquire(*v.options.stream);*v.options.stream<<counted;});newline();
    const auto owned=v.leaf([&]{return std::string(input.view());});
    v.leaf([&]{v.options.dialog->show("UNKNOWN MESSAGE",owned,0x10);});
});}
void Core::bind_content_checkpoint(content_handlers_recovered::CallCheckpoint c){auto& v=attached();v.affinity();if(v.busy||v.checkpoint.check){v.poisoned=true;throw std::logic_error("Licensing checkpoint overlaps active call");}v.check();if(!c.context||!c.check)throw std::invalid_argument("Complete licensing checkpoint required");{Error error;c.check(c.context);}v.checkpoint=c;}
void Core::clear_content_checkpoint()noexcept{if(!impl_)return;auto& v=*impl_;if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy)std::terminate();v.checkpoint={};}

struct Handlers::Impl {
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    bootstrap_backend::CallbackStateView view;std::shared_ptr<AttemptState> attempts;std::unique_ptr<Core> core;
    std::optional<license_recovered::Handlers> handlers;bool busy=false,retiring=false,poisoned=false;
    void* admission_context=nullptr;void (*admission_check)(void*)=nullptr;
    Impl(bootstrap_backend::CallbackStateView v,online_factory::Online& o,std::shared_ptr<AttemptState> state,Options options):view(v),attempts(std::move(state)),core(std::make_unique<Core>(v,o,std::move(options))){if(!attempts)throw std::invalid_argument("Explicit live licensing attempt/block cells required");}
    void check()const{Error error;if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber()||retiring||poisoned)throw std::logic_error("Licensing handler admission unavailable");}
    void admit()const{check();if(admission_check)admission_check(admission_context);}
    struct Call {Impl& v;explicit Call(Impl& v,bool authentication=true):v(v){v.check();if(v.busy){v.poisoned=true;throw std::logic_error("Licensing handler is nonreentrant");}if(authentication&&!v.handlers)throw std::logic_error("Licensing authentication is not bound");v.busy=true;}~Call(){v.busy=false;}};
    struct Node final:messages_recovered::XmlNode {Impl& v;const messages_recovered::XmlNode& raw;Node(Impl& v,const messages_recovered::XmlNode& n):v(v),raw(n){}
        template<class F>std::invoke_result_t<F> call(F&& f)const{v.admit();std::invoke_result_t<F> result=f();v.admit();return result;}
        std::string_view name()const override{return call([&]{return raw.name();});}
        const messages_recovered::XmlNode* first_element()const override{return call([&]{return raw.first_element();});}
        const messages_recovered::XmlNode* next_element()const override{return call([&]{return raw.next_element();});}
        std::optional<std::string_view> attribute(std::string_view k)const override{return call([&]{return raw.attribute(k);});}
    };
    struct Xml final:handlers_recovered::Xml {Impl& v;handlers_recovered::Xml& raw;Xml(Impl& v,handlers_recovered::Xml& x):v(v),raw(x){}
        const char* attribute_002cc0(handlers_recovered::Request r,const char* k)override{v.admit();const auto result=raw.attribute_002cc0(r,k);v.admit();return result;}
        void open_004080(const char* n,bool c)override{v.admit();raw.open_004080(n,c);v.admit();}
        void attribute_005560(const char* k,const char* s)override{v.admit();raw.attribute_005560(k,s);v.admit();}
        void close_virtual_48(bool c)override{v.admit();raw.close_virtual_48(c);v.admit();}
    };
};
Handlers::Handlers(bootstrap_backend::CallbackStateView v,online_factory::Online& o,std::shared_ptr<AttemptState> state,Options options):impl_(nullptr){impl_=std::make_unique<Impl>(v,o,std::move(state),std::move(options));auto& c=*impl_->core->impl_;c.handler_context=impl_.get();c.handler_check=[](void* p){static_cast<Impl*>(p)->check();};impl_->admission_context=impl_->core.get();impl_->admission_check=[](void* p){static_cast<Core*>(p)->impl_->check();};}
Handlers::Impl& Handlers::attached()const{if(!impl_)throw std::logic_error("Licensing handlers unattached");return *impl_;}
Handlers::~Handlers()noexcept{if(!impl_)return;auto& v=*impl_;if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy)std::terminate();v.retiring=true;v.handlers.reset();v.core.reset();v.attempts.reset();}
void Handlers::bind_authentication_code(const license_recovered::AuthenticationCode& code){auto& v=attached();v.check();if(v.busy){v.poisoned=true;throw std::logic_error("Licensing handler scope is active");}if(v.handlers)throw std::logic_error("Licensing authentication binds once before dispatch");auto& c=v.view.configuration;v.handlers.emplace(license_recovered::StateReferences{code.value(),v.view.runtime.online,c.fake_auth,c.denuvo_token,c.dll_hash,c.exe_hash,c.content_id,c.denuvo_dll,c.executable_path,c.license_directory,c.game_attributes,v.attempts->attempted_files,v.attempts->blocked},*v.core);v.handlers->bind_authentication_code(code);}
template<class F>bool Handlers::invoke_request(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x,F&& f){auto& v=attached();Impl::Call call(v);Impl::Node node(v,n);Impl::Xml xml(v,x);try{v.admit();const auto result=f(*v.handlers,node,xml);v.admit();return result;}catch(const NoResponseRequiredError&){try{v.admit();}catch(...){v.poisoned=true;throw;}throw;}catch(...){v.poisoned=true;throw;}}
bool Handlers::auth_code_02e3b0(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x){return invoke_request(n,x,[](auto& h,const auto& node,auto& xml){return h.auth_code_02e3b0(node,xml);});}
bool Handlers::request_license_0320c0(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x){return invoke_request(n,x,[](auto& h,const auto& node,auto& xml){return h.request_license_0320c0(node,xml);});}
bool Handlers::invalidate_license_030c50(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x){return invoke_request(n,x,[](auto& h,const auto& node,auto& xml){return h.invalidate_license_030c50(node,xml);});}
void Handlers::unknown_message_07bd70(const messages_recovered::UnknownMessageInput& input){auto& v=attached();Impl::Call call(v,false);try{v.admit();v.core->unknown_message_07bd70(input);v.admit();}catch(...){v.poisoned=true;throw;}}
void Handlers::bind_content_checkpoint(content_handlers_recovered::CallCheckpoint c){auto& v=attached();v.check();if(v.busy){v.poisoned=true;throw std::logic_error("Licensing handler scope is active");}v.core->bind_content_checkpoint(c);}
void Handlers::clear_content_checkpoint()noexcept{if(!impl_||impl_->retiring||!impl_->core)return;impl_->core->clear_content_checkpoint();}
static_assert(!std::is_abstract_v<Core>&&!std::is_abstract_v<Handlers>);
}
