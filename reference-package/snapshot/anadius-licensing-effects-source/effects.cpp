#include "effects.hpp"
#include "../anadius-application-io-source/files.hpp"
#include "../anadius-path-source/paths.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include "../anadius-token-source/token.hpp"
#include <bit>
#include <filesystem>
#include <optional>
#include <mutex>
#include <atomic>
#include <list>
#include <exception>
#include <type_traits>

namespace anadius::licensing_effects {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
struct OwnerHealth {std::atomic<bool> failed=false;};
[[noreturn]]void reject(std::unique_lock<std::mutex>& lock,const char* text){lock.unlock();throw std::logic_error(text);}
Options checked(Options o){if(!o.stream||!o.dialog||!o.files||!o.text||!o.stats||!o.encoding||!o.errors||!o.clock||!o.formatter||!o.module||!o.procedure||!o.exit_process)throw std::invalid_argument("Complete owned licensing providers and SDK calls required");return o;}
}
struct Core::Impl {
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    bootstrap_backend::CallbackStateView view;online_factory::Online& online;Options options;
    mutable std::mutex gate;

    std::shared_ptr<OwnerHealth> health=std::make_shared<OwnerHealth>();
    Impl* owner=nullptr;std::shared_ptr<const application_io::CallContext> context;
    bool preparing=false,closing=false,closed=false;
    std::list<std::shared_ptr<Impl>> calls;
    struct Admission {Admission* next;DWORD thread;void* fiber;};Admission* admissions=nullptr;
    Impl& root(){return owner?*owner:*this;}
    bool failed()const noexcept{return poisoned||health->failed.load();}
    bool overlap_locked(const Impl* self=nullptr)const noexcept{
        for(auto* p=admissions;p;p=p->next)if(p->thread==::GetCurrentThreadId()&&p->fiber==::GetCurrentFiber())return true;
        for(const auto& p:calls){const auto& v=*p;if(&v!=self&&v.thread==::GetCurrentThreadId()&&v.fiber==::GetCurrentFiber()&&(v.preparing||v.busy||v.checking||v.closing))return true;}return false;
    }
    bool busy=false,checking=false,binding=false,poisoned=false,retiring=false;content_handlers_recovered::CallCheckpoint checkpoint{};
    struct JobCall {
        Impl& v;socket_recovered::SessionMemoryDomain& parent;
        const socket_recovered::SessionMemoryDomain::JobPermit permit;const content_handlers_recovered::CallCheckpoint checkpoint;
        const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
        JobCall(Impl& v,socket_recovered::SessionMemoryDomain& p,const socket_recovered::SessionMemoryDomain::JobPermit& proof,content_handlers_recovered::CallCheckpoint c):v(v),parent(p),permit(proof),checkpoint(c){}
        bool actor()const{return thread==::GetCurrentThreadId()&&fiber==::GetCurrentFiber();}
        void validate(){if(!actor())throw std::logic_error("Licensing child belongs to another actor");permit.check(parent);(void)parent.buffer_context();Error error;std::lock_guard lock(v.gate);if(v.job!=this||v.retiring||v.failed())throw std::logic_error("Licensing child source ended or was poisoned");}
    };
    JobCall* job=nullptr; // stack descriptor; removed before synchronous return
    void* handler_context=nullptr;void (*handler_check)(void*)=nullptr;
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Licensing requires original Graph worker/fiber");}
    void actor()const{if(job){if(!job->actor())throw std::logic_error("Licensing is reserved by another actor");}else affinity();} // gate held
    void poison(){health->failed=true;Error error;auto& r=root();std::lock_guard lock(r.gate);r.poisoned=true;}
    void check(){
        if(context){
            context->validate();auto& r=root();
            {Error error;std::unique_lock lock(r.gate);if(closed||r.retiring||failed())reject(lock,"Licensing Core view closed or unknown");if(checking){health->failed=true;reject(lock,"Licensing Core view checkpoint cannot reenter");}checking=true;}
            try{files.ensure_known();if(handler_check)handler_check(handler_context);context->check();files.ensure_known();}
            catch(...){health->failed=true;Error error;std::lock_guard lock(r.gate);checking=false;throw;}
            {Error error;std::unique_lock lock(r.gate);checking=false;if(closed||r.retiring||failed())reject(lock,"Licensing Core view checkpoint was poisoned");}
            return;
        }

        Error error;JobCall* selected=nullptr;content_handlers_recovered::CallCheckpoint plain;
        {std::lock_guard lock(gate);actor();if(failed()||retiring)throw std::logic_error("Licensing admission closed or unknown");if(checking){poisoned=true;health->failed=true;throw std::logic_error("Licensing admission cannot reenter");}checking=true;selected=job;plain=checkpoint;}
        try{files.ensure_known();if(handler_check)handler_check(handler_context);if(selected){selected->validate();selected->checkpoint.check(selected->checkpoint.context);selected->validate();}else if(plain.check)plain.check(plain.context);files.ensure_known();}
        catch(...){std::lock_guard lock(gate);checking=false;poisoned=true;health->failed=true;throw;}
        {std::lock_guard lock(gate);checking=false;if(failed()||retiring)throw std::logic_error("Licensing admission was poisoned");}
    }
    template<class F>std::invoke_result_t<F> leaf(F&& f){check();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();check();}else{std::invoke_result_t<F> result=f();check();return result;}}

    struct Call {Impl& v;explicit Call(Impl& state):v(state){
        if(v.context){
            v.context->validate();auto& r=v.root();
            {Error error;std::unique_lock lock(r.gate);
                if(v.closed||r.retiring||v.failed())reject(lock,"Licensing Core view closed or unknown");
                if(v.preparing||v.busy||v.checking||v.closing||r.overlap_locked(&v)||(r.job&&r.job->actor())){v.health->failed=true;reject(lock,"Licensing Core view cannot reenter");}
                v.busy=true;
            }
        }else{Error error;std::unique_lock lock(v.gate);v.actor();if(v.busy||v.binding||v.checking||v.overlap_locked()){v.health->failed=true;reject(lock,"Licensing operation is nonreentrant");}if(v.failed()||v.retiring)reject(lock,"Licensing admission closed");v.busy=true;}
        try{v.check();}catch(...){v.health->failed=true;Error error;auto& r=v.root();std::lock_guard lock(r.gate);v.busy=false;throw;}
    }~Call(){Error error;auto& r=v.root();std::lock_guard lock(r.gate);v.busy=false;}};
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
    Impl(Impl& r,std::shared_ptr<const application_io::CallContext> c):view(r.view),online(r.online),options(r.options),health(r.health),owner(&r),context(std::move(c)),preparing(true){}

};
Core::Core(bootstrap_backend::CallbackStateView v,online_factory::Online& o,Options options):impl_(nullptr){impl_=std::make_shared<Impl>(v,o,std::move(options));}
Core::Core(std::shared_ptr<Impl> state):impl_(std::move(state)){}
Core::Impl& Core::attached()const{if(!impl_)throw std::logic_error("Licensing is unattached");return *impl_;}
Core::~Core()noexcept{if(!impl_)return;auto& v=*impl_;if(v.context)return; // Registry retains an unclosed prefix; wrappers retire explicitly.
    {Error error;std::lock_guard lock(v.gate);if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy||v.binding||v.checking||v.job||v.checkpoint.check||v.files.has_pending()||!v.calls.empty()||v.admissions)std::terminate();v.retiring=true;}{Options retiring=std::move(v.options);}}
template<class F>auto Core::invoke(F&& f,bool control)->std::invoke_result_t<F>{
    auto& v=attached();Impl::Call call(v);
    // Catch control only around the source action. Admission/postchecks poison
    // independently, even if an opaque checkpoint throws this same C++ type.
    auto source=[&]() -> std::invoke_result_t<F>{try{return f();}catch(const NoResponseRequiredError&){if(!control)v.poison();else v.check();throw;}catch(...){v.poison();throw;}};
    if constexpr(std::is_void_v<std::invoke_result_t<F>>){source();v.check();}else{std::invoke_result_t<F> result=source();v.check();return result;}
}
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
void Core::run_job_action(socket_recovered::SessionMemoryDomain& parent,const socket_recovered::SessionMemoryDomain::JobPermit& permit,content_handlers_recovered::CallCheckpoint checkpoint,JobAction action){
    Error error;permit.check(parent);auto& v=attached();
    if(!checkpoint.context||!checkpoint.check||!action.context||!action.call)throw std::invalid_argument("Complete synchronous Licensing child call required");
    if(v.thread==::GetCurrentThreadId())throw std::logic_error("Licensing child requires distinct worker");
    Impl::JobCall call(v,parent,permit,checkpoint);
    {std::unique_lock lock(v.gate);if(v.job||v.busy||v.binding||v.checking||v.checkpoint.check||v.overlap_locked()){v.poisoned=true;v.health->failed=true;reject(lock,"Licensing child overlaps active source");}if(v.failed()||v.retiring)reject(lock,"Licensing admission closed");v.job=&call;v.binding=true;}
    std::exception_ptr control;
    try{
        v.check();
        {std::lock_guard lock(v.gate);if(v.failed()||v.retiring||v.job!=&call||!v.binding||v.busy||v.checking)throw std::logic_error("Licensing child preparation was poisoned");v.binding=false;}
        try{action.call(action.context);}catch(const NoResponseRequiredError&){control=std::current_exception();}
        // A marker from a denied probe/lower call has already failed() Core.
        // Completion is outside the control catch: it must never be retried.
        v.check();if(v.files.has_pending())throw std::logic_error("Licensing child retains raw file ownership");
        {std::lock_guard lock(v.gate);if(v.failed()||v.retiring||v.job!=&call||v.binding||v.busy||v.checking)throw std::logic_error("Licensing child completion was poisoned");v.binding=true;}
        v.check();
        {std::lock_guard lock(v.gate);if(v.failed()||v.retiring||v.job!=&call||!v.binding||v.busy||v.checking)throw std::logic_error("Licensing child retirement was poisoned");v.job=nullptr;v.binding=false;}
    }catch(const NoResponseRequiredError&){v.poison();std::lock_guard lock(v.gate);if(v.busy||v.checking)std::terminate();v.job=nullptr;v.binding=false;throw std::logic_error("Licensing child probe/control completion failed");}
    catch(...){v.poison();std::lock_guard lock(v.gate);if(v.busy||v.checking)std::terminate();v.job=nullptr;v.binding=false;throw;}
    if(control)std::rethrow_exception(control);
}
void Core::bind_content_checkpoint(content_handlers_recovered::CallCheckpoint c){
    auto& v=attached();v.affinity();if(!c.context||!c.check)throw std::invalid_argument("Complete licensing checkpoint required");
    {Error error;std::lock_guard lock(v.gate);if(v.job||v.busy||v.binding||v.checking||v.checkpoint.check){v.poisoned=true;v.health->failed=true;throw std::logic_error("Licensing checkpoint overlaps active call");}if(v.failed()||v.retiring)throw std::logic_error("Licensing admission closed");v.binding=true;}
    try{v.check();{Error error;c.check(c.context);}v.check();Error error;std::lock_guard lock(v.gate);if(v.failed()||v.retiring||!v.binding)throw std::logic_error("Licensing checkpoint binding was poisoned");v.checkpoint=c;v.binding=false;}
    catch(...){v.poison();Error error;std::lock_guard lock(v.gate);v.binding=false;throw;}
}
void Core::clear_content_checkpoint()noexcept{if(!impl_)return;auto& v=*impl_;Error error;std::lock_guard lock(v.gate);if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.job||v.busy||v.binding||v.checking)std::terminate();v.checkpoint={};}

struct Handlers::Impl {
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    bootstrap_backend::CallbackStateView view;std::shared_ptr<AttemptState> attempts;std::unique_ptr<Core> core;

    std::shared_ptr<OwnerHealth> health=std::make_shared<OwnerHealth>();
    Impl* owner=nullptr;std::shared_ptr<const application_io::CallContext> context;
    bool preparing=false,closing=false,closed=false;
    std::list<std::shared_ptr<Impl>> calls;
    struct Admission {Admission* next;DWORD thread;void* fiber;};Admission* admissions=nullptr;
    Impl& root(){return owner?*owner:*this;}
    bool failed()const noexcept{return poisoned||health->failed.load();}
    bool overlap_locked(const Impl* self=nullptr)const noexcept{
        for(auto* p=admissions;p;p=p->next)if(p->thread==::GetCurrentThreadId()&&p->fiber==::GetCurrentFiber())return true;
        for(const auto& p:calls){const auto& v=*p;if(&v!=self&&v.thread==::GetCurrentThreadId()&&v.fiber==::GetCurrentFiber()&&(v.preparing||v.busy||v.checking||v.closing))return true;}return false;
    }
    const license_recovered::AuthenticationCode* authentication=nullptr;
    mutable std::mutex gate;std::optional<license_recovered::Handlers> handlers;bool busy=false,checking=false,binding=false,retiring=false,poisoned=false;
    struct JobCall {
        Impl& v;socket_recovered::SessionMemoryDomain& parent;
        const socket_recovered::SessionMemoryDomain::JobPermit permit;const content_handlers_recovered::CallCheckpoint checkpoint;
        const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
        JobCall(Impl& v,socket_recovered::SessionMemoryDomain& p,const socket_recovered::SessionMemoryDomain::JobPermit& proof,content_handlers_recovered::CallCheckpoint c):v(v),parent(p),permit(proof),checkpoint(c){}
        bool actor()const{return thread==::GetCurrentThreadId()&&fiber==::GetCurrentFiber();}
        void validate(){if(!actor())throw std::logic_error("Licensing handler child belongs to another actor");permit.check(parent);(void)parent.buffer_context();Error error;std::lock_guard lock(v.gate);if(v.job!=this||v.retiring||v.failed())throw std::logic_error("Licensing handler child source ended or was poisoned");}
    };
    JobCall* job=nullptr;
    void* admission_context=nullptr;void (*admission_check)(void*)=nullptr;
    Impl(bootstrap_backend::CallbackStateView v,online_factory::Online& o,std::shared_ptr<AttemptState> state,Options options):view(v),attempts(std::move(state)),core(std::make_unique<Core>(v,o,std::move(options))),health(core->impl_->health){if(!attempts)throw std::invalid_argument("Explicit live licensing attempt/block cells required");}
    void install(const license_recovered::AuthenticationCode& code){
        auto& c=view.configuration;
        handlers.emplace(license_recovered::StateReferences{code.value(),view.runtime.online,c.fake_auth,c.denuvo_token,c.dll_hash,c.exe_hash,c.content_id,c.denuvo_dll,c.executable_path,c.license_directory,c.game_attributes,attempts->attempted_files,attempts->blocked},*core);
        handlers->bind_authentication_code(code);authentication=&code;
    }
    Impl(Impl& r,std::shared_ptr<const application_io::CallContext> c)
        :view(r.view),attempts(r.attempts),core(new Core(std::make_shared<Core::Impl>(*r.core->impl_,c))),
         health(r.health),owner(&r),context(std::move(c)),preparing(true){
        if(r.authentication)install(*r.authentication);
        auto& lower=*core->impl_;lower.preparing=false;lower.handler_context=this;lower.handler_check=[](void* p){static_cast<Impl*>(p)->check();};
        admission_context=core.get();admission_check=[](void* p){static_cast<Core*>(p)->impl_->check();};
    }
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Licensing handlers require original worker/fiber");}
    void actor()const{if(job){if(!job->actor())throw std::logic_error("Licensing handlers reserved by another actor");}else affinity();} // gate held
    void poison(){health->failed=true;Error error;auto& r=root();std::lock_guard lock(r.gate);r.poisoned=true;}
    // Core.check calls only this metadata/permit check, never admit/Core again.
    void check(){
        if(context){context->validate();Error error;auto& r=root();std::unique_lock lock(r.gate);if(closed||r.retiring||failed())reject(lock,"Licensing Handler view metadata closed or unknown");return;}
        Error error;JobCall* selected=nullptr;{std::lock_guard lock(gate);actor();if(retiring||failed())throw std::logic_error("Licensing handler admission unavailable");selected=job;}if(selected)try{selected->validate();}catch(...){poison();throw;}}
    void admit(){
        if(context){
            context->validate();auto& r=root();
            {Error error;std::unique_lock lock(r.gate);if(closed||r.retiring||failed())reject(lock,"Licensing Handler view closed or unknown");if(checking){health->failed=true;reject(lock,"Licensing Handler view checkpoint cannot reenter");}checking=true;}
            try{check();if(admission_check)admission_check(admission_context);check();}
            catch(...){health->failed=true;Error error;std::lock_guard lock(r.gate);checking=false;throw;}
            {Error error;std::unique_lock lock(r.gate);checking=false;if(closed||r.retiring||failed())reject(lock,"Licensing Handler view checkpoint was poisoned");}
            return;
        }
        Error error;{std::lock_guard lock(gate);actor();if(checking){poisoned=true;health->failed=true;throw std::logic_error("Licensing handler admission cannot reenter");}if(retiring||failed())throw std::logic_error("Licensing handler admission closed");checking=true;}
        try{check();if(admission_check)admission_check(admission_context);check();}
        catch(...){std::lock_guard lock(gate);checking=false;poisoned=true;health->failed=true;throw;}
        {std::lock_guard lock(gate);checking=false;if(retiring||failed())throw std::logic_error("Licensing handler admission was poisoned");}
    }
    // Scope completion uses its own proof; Core's child scope has already ended.
    void scope_check(JobCall& call){
        Error error;{std::lock_guard lock(gate);actor();if(checking){poisoned=true;health->failed=true;throw std::logic_error("Licensing handler scope check cannot reenter");}if(retiring||failed()||job!=&call)throw std::logic_error("Licensing handler scope closed");checking=true;}
        try{call.validate();call.checkpoint.check(call.checkpoint.context);call.validate();}
        catch(...){std::lock_guard lock(gate);checking=false;poisoned=true;health->failed=true;throw;}
        {std::lock_guard lock(gate);checking=false;if(retiring||failed())throw std::logic_error("Licensing handler scope was poisoned");}
    }

    struct Call {Impl& v;explicit Call(Impl& state,bool authentication=true):v(state){
        if(v.context){
            v.context->validate();auto& r=v.root();Error error;std::unique_lock lock(r.gate);
            if(v.closed||r.retiring||v.failed())reject(lock,"Licensing Handler view closed or unknown");
            if(authentication&&!v.handlers)reject(lock,"Licensing authentication is not bound");
            if(v.preparing||v.busy||v.checking||v.closing||r.overlap_locked(&v)||(r.job&&r.job->actor())){v.health->failed=true;reject(lock,"Licensing Handler view cannot reenter");}
            v.busy=true;
        }else{Error error;std::unique_lock lock(v.gate);v.actor();if(v.busy||v.binding||v.checking||v.overlap_locked()){v.health->failed=true;reject(lock,"Licensing handler is nonreentrant");}if(v.retiring||v.failed())reject(lock,"Licensing handler admission closed");if(authentication&&!v.handlers)reject(lock,"Licensing authentication is not bound");v.busy=true;}
        try{v.admit();}catch(...){v.health->failed=true;Error error;auto& r=v.root();std::lock_guard lock(r.gate);v.busy=false;throw;}
    }~Call(){Error error;auto& r=v.root();std::lock_guard lock(r.gate);v.busy=false;}};
    struct Node final:messages_recovered::XmlNode {Impl& v;const messages_recovered::XmlNode& raw;Node(Impl& v,const messages_recovered::XmlNode& n):v(v),raw(n){}
        template<class F>std::invoke_result_t<F> call(F&& f)const{try{v.admit();std::invoke_result_t<F> result=f();v.admit();return result;}catch(...){v.poison();throw;}}
        std::string_view name()const override{return call([&]{return raw.name();});}
        const messages_recovered::XmlNode* first_element()const override{return call([&]{return raw.first_element();});}
        const messages_recovered::XmlNode* next_element()const override{return call([&]{return raw.next_element();});}
        std::optional<std::string_view> attribute(std::string_view k)const override{return call([&]{return raw.attribute(k);});}
    };
    struct Xml final:handlers_recovered::Xml {Impl& v;handlers_recovered::Xml& raw;Xml(Impl& v,handlers_recovered::Xml& x):v(v),raw(x){}
        template<class F>std::invoke_result_t<F> call(F&& f){try{v.admit();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();v.admit();}else{std::invoke_result_t<F> result=f();v.admit();return result;}}catch(...){v.poison();throw;}}
        const char* attribute_002cc0(handlers_recovered::Request r,const char* k)override{return call([&]{return raw.attribute_002cc0(r,k);});}
        void open_004080(const char* n,bool c)override{call([&]{raw.open_004080(n,c);});}
        void attribute_005560(const char* k,const char* s)override{call([&]{raw.attribute_005560(k,s);});}
        void close_virtual_48(bool c)override{call([&]{raw.close_virtual_48(c);});}
    };
};
Handlers::Handlers(bootstrap_backend::CallbackStateView v,online_factory::Online& o,std::shared_ptr<AttemptState> state,Options options):impl_(nullptr){impl_=std::make_shared<Impl>(v,o,std::move(state),std::move(options));auto& c=*impl_->core->impl_;c.handler_context=impl_.get();c.handler_check=[](void* p){static_cast<Impl*>(p)->check();};impl_->admission_context=impl_->core.get();impl_->admission_check=[](void* p){static_cast<Core*>(p)->impl_->check();};}
Handlers::Handlers(std::shared_ptr<Impl> state):impl_(std::move(state)){}
Handlers::Impl& Handlers::attached()const{if(!impl_)throw std::logic_error("Licensing handlers unattached");return *impl_;}
Handlers::~Handlers()noexcept{if(!impl_)return;auto& v=*impl_;if(v.context)return; // Registry retains an unclosed prefix; wrappers retire explicitly.
    {Error error;std::lock_guard lock(v.gate);if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.job||v.busy||v.binding||v.checking||v.admissions||!v.calls.empty())std::terminate();v.retiring=true;}v.handlers.reset();v.core.reset();v.attempts.reset();}
void Handlers::bind_authentication_code(const license_recovered::AuthenticationCode& code){
    auto& v=attached();v.affinity();
    {Error error;std::unique_lock lock(v.gate);if(v.job||v.busy||v.binding||v.checking||v.admissions||!v.calls.empty()){v.poisoned=true;v.health->failed=true;reject(lock,"Licensing handler scope is active");}if(v.retiring||v.failed())reject(lock,"Licensing handler admission closed");if(v.handlers||v.authentication)reject(lock,"Licensing authentication binds once before dispatch");v.binding=true;}
    try{v.check();v.install(code);v.check();Error error;std::lock_guard lock(v.gate);if(v.retiring||v.failed()||!v.binding)throw std::logic_error("Licensing authentication binding was poisoned");v.binding=false;}
    catch(...){v.poison();Error error;std::lock_guard lock(v.gate);v.binding=false;throw;}
}
template<class F>bool Handlers::invoke_request(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x,F&& f){auto& v=attached();Impl::Call call(v);Impl::Node node(v,n);Impl::Xml xml(v,x);bool result;try{result=f(*v.handlers,node,xml);}catch(const NoResponseRequiredError&){v.admit();throw;}catch(...){v.poison();throw;}v.admit();return result;}
bool Handlers::auth_code_02e3b0(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x){return invoke_request(n,x,[](auto& h,const auto& node,auto& xml){return h.auth_code_02e3b0(node,xml);});}
bool Handlers::request_license_0320c0(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x){return invoke_request(n,x,[](auto& h,const auto& node,auto& xml){return h.request_license_0320c0(node,xml);});}
bool Handlers::invalidate_license_030c50(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x){return invoke_request(n,x,[](auto& h,const auto& node,auto& xml){return h.invalidate_license_030c50(node,xml);});}
void Handlers::unknown_message_07bd70(const messages_recovered::UnknownMessageInput& input){auto& v=attached();Impl::Call call(v,false);try{v.core->unknown_message_07bd70(input);v.admit();}catch(...){v.poison();throw;}}
void Handlers::run_job_action(socket_recovered::SessionMemoryDomain& parent,const socket_recovered::SessionMemoryDomain::JobPermit& permit,content_handlers_recovered::CallCheckpoint checkpoint,JobAction action){
    Error error;permit.check(parent);auto& v=attached();
    if(!checkpoint.context||!checkpoint.check||!action.context||!action.call)throw std::invalid_argument("Complete synchronous Licensing handler child call required");
    if(v.thread==::GetCurrentThreadId())throw std::logic_error("Licensing handler child requires distinct worker");
    Impl::JobCall call(v,parent,permit,checkpoint);
    {std::unique_lock lock(v.gate);if(v.job||v.busy||v.binding||v.checking||v.overlap_locked()){v.poisoned=true;v.health->failed=true;reject(lock,"Licensing handler child overlaps active source");}if(v.retiring||v.failed())reject(lock,"Licensing handler admission closed");v.job=&call;v.binding=true;}
    std::exception_ptr control;
    try{
        v.scope_check(call);
        {std::lock_guard lock(v.gate);if(v.failed()||v.retiring||v.job!=&call||!v.binding||v.busy||v.checking)throw std::logic_error("Licensing handler child preparation was poisoned");v.binding=false;}
        try{v.core->run_job_action(parent,permit,checkpoint,{action.context,action.call});}catch(const NoResponseRequiredError&){control=std::current_exception();}
        v.scope_check(call);
        {std::lock_guard lock(v.gate);if(v.failed()||v.retiring||v.job!=&call||v.binding||v.busy||v.checking)throw std::logic_error("Licensing handler child completion was poisoned");v.binding=true;}
        v.scope_check(call);
        {std::lock_guard lock(v.gate);if(v.failed()||v.retiring||v.job!=&call||!v.binding||v.busy||v.checking)throw std::logic_error("Licensing handler child retirement was poisoned");v.job=nullptr;v.binding=false;}
    }catch(const NoResponseRequiredError&){v.poison();std::lock_guard lock(v.gate);if(v.busy||v.checking)std::terminate();v.job=nullptr;v.binding=false;throw std::logic_error("Licensing handler child probe/control completion failed");}
    catch(...){v.poison();std::lock_guard lock(v.gate);if(v.busy||v.checking)std::terminate();v.job=nullptr;v.binding=false;throw;}
    if(control)std::rethrow_exception(control);
}
void Handlers::bind_content_checkpoint(content_handlers_recovered::CallCheckpoint c){
    auto& v=attached();v.affinity();
    {Error error;std::lock_guard lock(v.gate);if(v.job||v.busy||v.binding||v.checking){v.poisoned=true;v.health->failed=true;throw std::logic_error("Licensing handler scope is active");}if(v.retiring||v.failed())throw std::logic_error("Licensing handler admission closed");v.binding=true;}
    try{v.check();v.core->bind_content_checkpoint(c);v.check();Error error;std::lock_guard lock(v.gate);if(v.retiring||v.failed()||!v.binding)throw std::logic_error("Licensing handler checkpoint binding was poisoned");v.binding=false;}
    catch(...){v.poison();Error error;std::lock_guard lock(v.gate);v.binding=false;throw;}
}
void Handlers::clear_content_checkpoint()noexcept{if(!impl_)return;auto& v=*impl_;{Error error;std::lock_guard lock(v.gate);if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.job||v.busy||v.binding||v.checking)std::terminate();if(v.retiring||!v.core)return;v.binding=true;}v.core->clear_content_checkpoint();{Error error;std::lock_guard lock(v.gate);v.binding=false;}}

std::shared_ptr<Core::CallView> Core::make_call_view(const application_io::CallContext& context){
    context.validate();auto owner=impl_;if(!owner)throw std::logic_error("Licensing provider is unattached");
    Impl::Admission admission{nullptr,context.thread(),context.fiber()};
    {Error error;std::unique_lock lock(owner->gate);
        if(owner->retiring||owner->failed())reject(lock,"Licensing call-view owner is unavailable");
        if(context.thread()==owner->thread)reject(lock,"Licensing call view requires a distinct child worker");
        if(owner->binding||owner->overlap_locked()||(owner->job&&owner->job->actor())){owner->health->failed=true;reject(lock,"Licensing call-view preparation cannot reenter");}
        admission.next=owner->admissions;owner->admissions=&admission;
    }
    struct Pending {Impl& owner;Impl::Admission& admission;bool linked=true;
        void remove()noexcept{auto** p=&owner.admissions;while(*p&&*p!=&admission)p=&(*p)->next;if(!*p)std::terminate();*p=admission.next;linked=false;}
        ~Pending(){Error error;if(linked){std::lock_guard lock(owner.gate);remove();}}
    } pending{*owner,admission};
    std::list<std::shared_ptr<Impl>> prepared;std::shared_ptr<Impl> state;
    try{
        {Error error;prepared.emplace_back();auto proof=std::make_shared<const application_io::CallContext>(context);
            state=std::make_shared<Impl>(*owner,std::move(proof));prepared.front()=state;
            std::unique_lock lock(owner->gate);owner->calls.splice(owner->calls.end(),prepared);pending.remove();
            if(owner->failed()||owner->retiring)reject(lock,"Licensing call-view allocation was poisoned");
        }
        state->check();std::shared_ptr<CallView> result;
        {Error error;result=std::shared_ptr<CallView>(new CallView(owner,std::shared_ptr<Core>(new Core(state))));}
        state->check();
        {Error error;std::unique_lock lock(owner->gate);if(owner->failed()||owner->retiring)reject(lock,"Licensing call-view publication was poisoned");state->preparing=false;}
        return result;
    }catch(...){if(state)state->poison();throw;}
}
Core::CallView::CallView(std::shared_ptr<Core::Impl> owner,std::shared_ptr<Core> execution):owner_(std::move(owner)),execution_(std::move(execution)){}
Core::CallView::~CallView()noexcept{Error error;std::lock_guard lock(owner_->gate);if(!execution_->impl_->closed)owner_->health->failed=true;}

std::shared_ptr<Handlers::CallView> Handlers::make_call_view(const application_io::CallContext& context){
    context.validate();auto owner=impl_;if(!owner)throw std::logic_error("Licensing provider is unattached");
    Impl::Admission admission{nullptr,context.thread(),context.fiber()};
    {Error error;std::unique_lock lock(owner->gate);
        if(owner->retiring||owner->failed())reject(lock,"Licensing call-view owner is unavailable");
        if(context.thread()==owner->thread)reject(lock,"Licensing call view requires a distinct child worker");
        if(owner->binding||owner->overlap_locked()||(owner->job&&owner->job->actor())){owner->health->failed=true;reject(lock,"Licensing call-view preparation cannot reenter");}
        admission.next=owner->admissions;owner->admissions=&admission;
    }
    struct Pending {Impl& owner;Impl::Admission& admission;bool linked=true;
        void remove()noexcept{auto** p=&owner.admissions;while(*p&&*p!=&admission)p=&(*p)->next;if(!*p)std::terminate();*p=admission.next;linked=false;}
        ~Pending(){Error error;if(linked){std::lock_guard lock(owner.gate);remove();}}
    } pending{*owner,admission};
    std::list<std::shared_ptr<Impl>> prepared;std::shared_ptr<Impl> state;
    try{
        {Error error;prepared.emplace_back();auto proof=std::make_shared<const application_io::CallContext>(context);
            state=std::make_shared<Impl>(*owner,std::move(proof));prepared.front()=state;
            std::unique_lock lock(owner->gate);owner->calls.splice(owner->calls.end(),prepared);pending.remove();
            if(owner->failed()||owner->retiring)reject(lock,"Licensing call-view allocation was poisoned");
        }
        state->admit();std::shared_ptr<CallView> result;
        {Error error;result=std::shared_ptr<CallView>(new CallView(owner,std::shared_ptr<Handlers>(new Handlers(state))));}
        state->admit();
        {Error error;std::unique_lock lock(owner->gate);if(owner->failed()||owner->retiring)reject(lock,"Licensing call-view publication was poisoned");state->preparing=false;}
        return result;
    }catch(...){if(state)state->poison();throw;}
}
Handlers::CallView::CallView(std::shared_ptr<Handlers::Impl> owner,std::shared_ptr<Handlers> execution):owner_(std::move(owner)),execution_(std::move(execution)){}
Handlers::CallView::~CallView()noexcept{Error error;std::lock_guard lock(owner_->gate);if(!execution_->impl_->closed)owner_->health->failed=true;}

void Core::CallView::close(){
    auto& v=*execution_->impl_;auto& r=*owner_;
    {Error error;std::lock_guard lock(r.gate);if(v.closed)return;}v.context->validate();
    {Error error;std::unique_lock lock(r.gate);if(r.failed()||r.retiring)reject(lock,"Licensing Core view close unavailable");
        if(v.preparing||v.busy||v.checking||v.closing||r.overlap_locked(&v)||(r.job&&r.job->actor())){r.health->failed=true;reject(lock,"Licensing Core view close cannot reenter");}v.closing=true;}
    std::list<std::shared_ptr<Impl>> retired;
    try{
        v.check();if(v.files.has_pending())throw std::logic_error("Licensing Core view retains raw file ownership");
        {Error error;Options retiring=std::move(v.options);}v.check();
        {Error error;std::unique_lock lock(r.gate);if(r.failed()||r.retiring||!v.closing||v.busy||v.checking)reject(lock,"Licensing Core view retirement poisoned");
            auto at=r.calls.begin();while(at!=r.calls.end()&&at->get()!=&v)++at;if(at==r.calls.end())std::terminate();v.closed=true;v.closing=false;retired.splice(retired.end(),r.calls,at);}
        {Error error;retired.clear();}
    }catch(...){v.poison();throw;}
}

void Handlers::CallView::close(){
    auto& v=*execution_->impl_;auto& r=*owner_;
    {Error error;std::lock_guard lock(r.gate);if(v.closed)return;}v.context->validate();
    {Error error;std::unique_lock lock(r.gate);if(r.failed()||r.retiring)reject(lock,"Licensing Handler view close unavailable");
        if(v.preparing||v.busy||v.checking||v.closing||r.overlap_locked(&v)||(r.job&&r.job->actor())){r.health->failed=true;reject(lock,"Licensing Handler view close cannot reenter");}v.closing=true;}
    std::list<std::shared_ptr<Impl>> retired;
    try{
        v.admit();auto& lower=*v.core->impl_;
        if(lower.files.has_pending())throw std::logic_error("Licensing Handler view retains raw Core file ownership");
        {Error error;std::unique_lock lock(lower.root().gate);if(lower.busy||lower.checking||lower.closed)reject(lock,"Licensing nested Core retirement is active");lower.closing=true;}
        v.handlers.reset();{Error error;Options retiring=std::move(lower.options);}v.attempts.reset();
        v.admit(); // the same Context/metadata backlinks remain published
        {Error error;std::unique_lock lock(lower.root().gate);if(lower.failed()||lower.busy||lower.checking)reject(lock,"Licensing nested Core retirement was poisoned");lower.closed=true;lower.closing=false;}
        {Error error;std::unique_lock lock(r.gate);if(r.failed()||r.retiring||!v.closing||v.busy||v.checking)reject(lock,"Licensing Handler view retirement poisoned");
            auto at=r.calls.begin();while(at!=r.calls.end()&&at->get()!=&v)++at;if(at==r.calls.end())std::terminate();v.closed=true;v.closing=false;retired.splice(retired.end(),r.calls,at);}
        {Error error;retired.clear();}
    }catch(...){v.poison();throw;}
}
const char* Core::CallView::online_auth_virtual_30(const char* x){return execution_->online_auth_virtual_30(x);}
std::string Core::CallView::copy_online_auth(const char* x){return execution_->copy_online_auth(x);}
license_recovered::FileStatus Core::CallView::status_01ea20(const configuration::Path& p,std::uint32_t f){return execution_->status_01ea20(p,f);}
[[noreturn]]void Core::CallView::filesystem_error_018130(std::string_view op,std::int32_t c,const configuration::Path& p){execution_->filesystem_error_018130(op,c,p);std::terminate();}
bool Core::CallView::file_hash_matches_042d20(const configuration::Path& p,std::string expected){return execution_->file_hash_matches_042d20(p,std::move(expected));}
std::string Core::CallView::load_token_02afa0(const configuration::Path& p){return execution_->load_token_02afa0(p);}
configuration::Path Core::CallView::filesystem_widen_006d70(std::string_view x){return execution_->filesystem_widen_006d70(x);}
configuration::Path Core::CallView::filename_00e120(const configuration::Path& p){return execution_->filename_00e120(p);}
configuration::Path Core::CallView::parent_path_019820(const configuration::Path& p){return execution_->parent_path_019820(p);}
configuration::Path Core::CallView::join_016b00(const configuration::Path& p,const configuration::Path& q){return execution_->join_016b00(p,q);}
std::string Core::CallView::path_utf8_01e3f0(const configuration::Path& p){return execution_->path_utf8_01e3f0(p);}
std::string Core::CallView::current_time_041b60_null_true(){return execution_->current_time_041b60_null_true();}
bool Core::CallView::wine_detected_045010(){return execution_->wine_detected_045010();}
void Core::CallView::log_literal_006880(std::string_view x){execution_->log_literal_006880(x);}
void Core::CallView::log_string_006860(std::string_view x){execution_->log_string_006860(x);}
bool Core::CallView::write_ticket_03c1d0(const configuration::Path& p,std::string_view x){return execution_->write_ticket_03c1d0(p,x);}
void Core::CallView::dialog_038bc0(std::string_view t,std::string_view x,std::uint32_t f){execution_->dialog_038bc0(t,x,f);}
[[noreturn]]void Core::CallView::exit_05cdc0(std::int32_t c){execution_->exit_05cdc0(c);std::terminate();}
[[noreturn]]void Core::CallView::no_response_02bcd0_04d228(){execution_->no_response_02bcd0_04d228();std::terminate();}
void Core::CallView::unknown_message_07bd70(const messages_recovered::UnknownMessageInput& x){execution_->unknown_message_07bd70(x);}
bool Handlers::CallView::auth_code_02e3b0(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x){return execution_->auth_code_02e3b0(n,x);}
bool Handlers::CallView::request_license_0320c0(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x){return execution_->request_license_0320c0(n,x);}
bool Handlers::CallView::invalidate_license_030c50(const messages_recovered::XmlNode& n,handlers_recovered::Xml& x){return execution_->invalidate_license_030c50(n,x);}
void Handlers::CallView::unknown_message_07bd70(const messages_recovered::UnknownMessageInput& x){execution_->unknown_message_07bd70(x);}
static_assert(!std::is_abstract_v<Core>&&!std::is_abstract_v<Handlers>);
static_assert(!std::is_abstract_v<Core::CallView>&&!std::is_abstract_v<Handlers::CallView>);
}
