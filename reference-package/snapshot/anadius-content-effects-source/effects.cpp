#include "effects.hpp"
#include "../anadius-application-io-source/files.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include "../anadius-path-source/paths.hpp"
#include <stdexcept>
#include <type_traits>
#include <exception>
#include <mutex>
#include <list>

namespace anadius::content_effects {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
[[noreturn]]void reject(std::unique_lock<std::mutex>& lock,const char* text){lock.unlock();throw std::logic_error(text);}
Options checked(Options o){
    if(!o.stream||!o.dialog||!o.filesystem||!o.files||!o.clock||!o.formatter||!o.encoding||!o.errors||!o.atoi||!o.copy_file)
        throw std::invalid_argument("Complete owned content providers and SDK calls required");
    return o;
}
}
struct Content::Impl {
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    bootstrap_backend::CallbackStateView view;const achievement_storage::Paths& paths;online_factory::Online& online;
    mutable std::mutex gate;Options options;bool retiring=false,busy=false,checking=false,binding=false,poisoned=false;content_handlers_recovered::CallCheckpoint checkpoint{};
    Impl* owner=nullptr; // registry-owned execution never owns its registry
    const std::optional<application_io::CallContext> context;
    bool closed=false,closing=false,preparing=false;
    // Only the creator Impl owns this list. Unknown executions deliberately
    // retain their private GuardedFiles; no guessed file cleanup on unwind.
    std::list<std::shared_ptr<Impl>> calls;
    struct Admission {Admission* next;DWORD thread;void* fiber;};
    Admission* admissions=nullptr;
    Impl& root(){return owner?*owner:*this;}
    bool same_actor()const noexcept{return thread==::GetCurrentThreadId()&&fiber==::GetCurrentFiber();}
    bool overlap_locked(const Impl* self=nullptr)const noexcept{
        for(auto* a=admissions;a;a=a->next)if(a->thread==::GetCurrentThreadId()&&a->fiber==::GetCurrentFiber())return true;
        for(const auto& c:calls)if(c.get()!=self&&c->same_actor()&&(c->preparing||c->busy||c->checking||c->closing))return true;
        return false;
    }
    struct JobCall {
        Impl& v;socket_recovered::SessionMemoryDomain& parent;
        const socket_recovered::SessionMemoryDomain::JobPermit permit;const content_handlers_recovered::CallCheckpoint checkpoint;
        const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
        JobCall(Impl& owner,socket_recovered::SessionMemoryDomain& p,const socket_recovered::SessionMemoryDomain::JobPermit& proof,content_handlers_recovered::CallCheckpoint c):v(owner),parent(p),permit(proof),checkpoint(c){}
        bool actor()const{return thread==::GetCurrentThreadId()&&fiber==::GetCurrentFiber();}
        void validate(){if(!actor())throw std::logic_error("Content child scope belongs to another actor");permit.check(parent);(void)parent.buffer_context();Error error;std::lock_guard lock(v.gate);if(v.job!=this||v.retiring||v.poisoned)throw std::logic_error("Content child source ended or was poisoned");}
    };
    JobCall* job=nullptr; // synchronous with_job descriptor; never retained after scope
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Content requires original Graph worker/fiber");}
    void actor()const{if(context){if(!context->actor())throw std::logic_error("Content view belongs to another actor");}else if(job){if(!job->actor())throw std::logic_error("Content is reserved by another child actor");}else affinity();} // gate held
    void poison(){Error error;auto& r=root();std::lock_guard lock(r.gate);r.poisoned=true;}
    void check(){
        if(context){
            Error error;context->validate();auto& r=root();bool denied=false,reentered=false;
            {std::lock_guard lock(r.gate);denied=closed||r.retiring||r.poisoned;if(!denied){reentered=checking;if(reentered)r.poisoned=true;else checking=true;}}
            if(denied)throw std::logic_error("Content view closed or unknown");
            if(reentered)throw std::logic_error("Content view checkpoint cannot reenter");
            try{files.ensure_known();context->check();files.ensure_known();}
            catch(...){std::lock_guard lock(r.gate);checking=false;r.poisoned=true;throw;}
            {std::lock_guard lock(r.gate);checking=false;denied=closed||r.retiring||r.poisoned;}
            if(denied)throw std::logic_error("Content view checkpoint was poisoned");
            return;
        }
        Error error;JobCall* selected=nullptr;content_handlers_recovered::CallCheckpoint plain;
        {std::lock_guard lock(gate);actor();if(retiring||poisoned)throw std::logic_error("Content admission closed or unknown");if(checking){poisoned=true;throw std::logic_error("Content admission cannot reenter");}checking=true;selected=job;plain=checkpoint;}
        try{files.ensure_known();if(selected){selected->validate();selected->checkpoint.check(selected->checkpoint.context);selected->validate();}else if(plain.check)plain.check(plain.context);files.ensure_known();}
        catch(...){std::lock_guard lock(gate);checking=false;poisoned=true;throw;}
        {std::lock_guard lock(gate);checking=false;if(retiring||poisoned)throw std::logic_error("Content admission was poisoned");}
    }
    template<class F>std::invoke_result_t<F> leaf(F&& f){check();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();check();}else{std::invoke_result_t<F> result=f();check();return result;}}
    struct Call {Impl& v;explicit Call(Impl& v):v(v){
        if(v.context){
            // Invalid/foreign/expired handles are preconditions, not failures
            // of a different healthy invocation.
            v.context->validate();auto& r=v.root();bool denied=false,reentered=false;
            {Error error;std::lock_guard lock(r.gate);denied=v.closed||r.retiring||r.poisoned;if(!denied){reentered=v.preparing||v.busy||v.checking||v.closing||r.overlap_locked(&v)||(r.job&&r.job->actor());if(reentered)r.poisoned=true;else v.busy=true;}}
            if(denied)throw std::logic_error("Content view closed or unknown");
            if(reentered)throw std::logic_error("Content view operation is nonreentrant");
        }else{Error error;std::unique_lock lock(v.gate);v.actor();if(v.busy||v.binding||v.checking||v.overlap_locked()){v.poisoned=true;reject(lock,"Content operation is nonreentrant");}if(v.retiring||v.poisoned)reject(lock,"Content admission closed");v.busy=true;}
        try{v.check();}catch(...){Error error;auto& r=v.root();std::lock_guard lock(r.gate);v.busy=false;throw;}
    }~Call(){Error error;auto& r=v.root();std::lock_guard lock(r.gate);v.busy=false;}};
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
    Impl(std::shared_ptr<Impl> root,const application_io::CallContext& call)
        :view(root->view),paths(root->paths),online(root->online),options(root->options),owner(root.get()),context(call),preparing(true){}
};
Content::Content(bootstrap_backend::CallbackStateView v,const achievement_storage::Paths& p,online_factory::Online& o,Options options)
    :impl_(nullptr){impl_=std::make_shared<Impl>(v,p,o,std::move(options));}
Content::Impl& Content::attached()const{if(!impl_)throw std::logic_error("Content is unattached");return *impl_;}
Content::~Content()noexcept{if(!impl_)return;auto& v=*impl_;{Error error;std::lock_guard lock(v.gate);if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy||v.binding||v.checking||v.job||v.checkpoint.check||v.files.has_pending()||!v.calls.empty()||v.admissions)std::terminate();v.retiring=true;}{Options retiring=std::move(v.options);}}
template<class F>auto Content::invoke(F&& f)->std::invoke_result_t<F>{auto& v=attached();Impl::Call call(v);try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();v.check();}else{std::invoke_result_t<F> result=f();v.check();return result;}}catch(...){v.poison();throw;}}
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
void Content::run_job_action(socket_recovered::SessionMemoryDomain& parent,const socket_recovered::SessionMemoryDomain::JobPermit& permit,content_handlers_recovered::CallCheckpoint checkpoint,JobAction action){
    Error error;permit.check(parent);auto& v=attached();
    if(!checkpoint.context||!checkpoint.check||!action.context||!action.call)throw std::invalid_argument("Complete synchronous Content job call required");
    if(v.thread==::GetCurrentThreadId())throw std::logic_error("Content child scope requires distinct worker");
    Impl::JobCall call(v,parent,permit,checkpoint);
    {std::unique_lock lock(v.gate);if(v.job||v.busy||v.binding||v.checking||v.checkpoint.check||v.overlap_locked()){v.poisoned=true;reject(lock,"Content child scope overlaps active source");}if(v.retiring||v.poisoned)reject(lock,"Content admission closed");v.job=&call;v.binding=true;}
    try{
        v.check();
        {std::lock_guard lock(v.gate);if(v.poisoned||v.retiring||v.job!=&call||!v.binding||v.busy||v.checking)throw std::logic_error("Content child preparation was poisoned");v.binding=false;}
        action.call(action.context);v.check();
        if(v.files.has_pending())throw std::logic_error("Content child scope retains raw file ownership");
        {std::lock_guard lock(v.gate);if(v.poisoned||v.retiring||v.job!=&call||v.binding||v.busy||v.checking)throw std::logic_error("Content child completion was poisoned");v.binding=true;}
        v.check(); // final source proof while provider reentry is sealed
        {std::lock_guard lock(v.gate);if(v.poisoned||v.retiring||v.job!=&call||!v.binding||v.busy||v.checking)throw std::logic_error("Content child retirement was poisoned");v.job=nullptr;v.binding=false;}
    }catch(...){v.poison();std::lock_guard lock(v.gate);if(v.busy||v.checking)std::terminate();v.job=nullptr;v.binding=false;throw;}
}
void Content::bind_content_checkpoint(content_handlers_recovered::CallCheckpoint c){
    auto& v=attached();v.affinity();if(!c.context||!c.check)throw std::invalid_argument("Complete content checkpoint required");
    {Error error;std::lock_guard lock(v.gate);if(v.job||v.busy||v.binding||v.checking||v.checkpoint.check){v.poisoned=true;throw std::logic_error("Content checkpoint overlaps active call");}if(v.poisoned||v.retiring)throw std::logic_error("Content admission closed");v.binding=true;}
    try{v.check();{Error error;c.check(c.context);}v.check();Error error;std::lock_guard lock(v.gate);if(v.poisoned||v.retiring||!v.binding)throw std::logic_error("Content checkpoint binding was poisoned");v.checkpoint=c;v.binding=false;}
    catch(...){v.poison();Error error;std::lock_guard lock(v.gate);v.binding=false;throw;}
}
void Content::clear_content_checkpoint()noexcept{if(!impl_)return;auto& v=*impl_;Error error;std::lock_guard lock(v.gate);if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.job||v.busy||v.binding||v.checking)std::terminate();v.checkpoint={};}
std::shared_ptr<Content::CallView> Content::make_call_view(const application_io::CallContext& context){
    context.validate();auto owner=impl_;if(!owner)throw std::logic_error("Content is unattached");
    Impl::Admission admission{nullptr,context.thread(),context.fiber()};
    {Error error;std::unique_lock lock(owner->gate);
        if(owner->retiring||owner->poisoned)reject(lock,"Content call-view owner is unavailable");
        if(context.thread()==owner->thread)reject(lock,"Content call view requires a distinct child worker");
        if(owner->overlap_locked()||(owner->job&&owner->job->actor())){owner->poisoned=true;reject(lock,"Content call-view preparation cannot reenter");}
        admission.next=owner->admissions;owner->admissions=&admission;
    }
    struct Pending {Impl& owner;Impl::Admission& admission;bool linked=true;
        void remove()noexcept{auto** p=&owner.admissions;while(*p&&*p!=&admission)p=&(*p)->next;if(!*p)std::terminate();*p=admission.next;linked=false;}
        ~Pending(){Error error;if(linked){std::lock_guard lock(owner.gate);remove();}}
    } pending{*owner,admission};
    std::list<std::shared_ptr<Impl>> prepared;
    std::shared_ptr<Impl> state;
    try{
        {Error error;prepared.emplace_back();state=std::make_shared<Impl>(owner,context);prepared.front()=state;
            std::unique_lock lock(owner->gate);owner->calls.splice(owner->calls.end(),prepared);pending.remove();
            if(owner->poisoned||owner->retiring)reject(lock,"Content call-view allocation was poisoned");
        }
        state->check();std::shared_ptr<CallView> result;
        {Error error;result=std::shared_ptr<CallView>(new CallView(owner,state));}
        state->check();
        {Error error;std::unique_lock lock(owner->gate);if(owner->poisoned||owner->retiring)reject(lock,"Content call-view publication was poisoned");state->preparing=false;}
        return result;
    }catch(...){if(state)state->poison();throw;}
}
Content::CallView::CallView(std::shared_ptr<Impl> owner,std::shared_ptr<Impl> state):owner_(std::move(owner)),impl_(std::move(state)){}
Content::CallView::~CallView()noexcept{Error error;auto& r=impl_->root();std::lock_guard lock(r.gate);if(!impl_->closed)r.poisoned=true;}
template<class F>auto Content::CallView::invoke(F&& f)->std::invoke_result_t<F>{auto& v=*impl_;Impl::Call call(v);try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();v.check();}else{std::invoke_result_t<F> result=f();v.check();return result;}}catch(...){v.poison();throw;}}
void Content::CallView::close(){
    auto& v=*impl_;auto& r=v.root();
    {Error error;std::lock_guard lock(r.gate);if(v.closed)return;}
    v.context->validate();
    {Error error;std::unique_lock lock(r.gate);
        if(r.poisoned||r.retiring)reject(lock,"Content call-view close unavailable");
        if(v.preparing||v.busy||v.checking||v.closing||r.overlap_locked(&v)||(r.job&&r.job->actor())){r.poisoned=true;reject(lock,"Content call-view close cannot reenter");}
        v.closing=true;
    }
    std::list<std::shared_ptr<Impl>> retired;
    try{
        v.check();if(v.files.has_pending())throw std::logic_error("Content call view retains raw file ownership");
        // Retire captured lower owners while the view remains published and
        // nonreentrant. Engine/GuardedFiles retain only empty metadata later.
        {Error error;Options retiring=std::move(v.options);}
        v.check();
        {Error error;std::unique_lock lock(r.gate);
            if(r.poisoned||r.retiring||!v.closing||v.busy||v.checking)reject(lock,"Content call-view retirement was poisoned");
            auto found=r.calls.begin();while(found!=r.calls.end()&&found->get()!=&v)++found;if(found==r.calls.end())std::terminate();
            v.closed=true;v.closing=false;retired.splice(retired.end(),r.calls,found);
        }
        {Error error;retired.clear();}
    }catch(...){v.poison();throw;}
}
std::int32_t Content::CallView::atoi_05ee08(std::string_view s){return invoke([&]{return impl_->engine.atoi_05ee08(s);});}
void Content::CallView::unknown_entitlement_field(std::string_view s){invoke([&]{impl_->engine.unknown_entitlement_field(s);});}
std::string Content::CallView::utc_time_041b60(std::uint64_t& n){return invoke([&]{return impl_->engine.utc_time_041b60(n);});}
std::unique_ptr<content_handlers_recovered::DocumentWriter> Content::CallView::printer_0019b0(){return invoke([&]{return impl_->engine.printer_0019b0();});}
void Content::CallView::write_xml_03c1d0(const configuration::Path& p,std::string_view s){invoke([&]{impl_->engine.write_xml_03c1d0(p,s);});}
void Content::CallView::save_steam_01f9a0(const content_handlers_recovered::AchievementMap& m){invoke([&]{impl_->engine.save_steam_01f9a0(m);});}
configuration::Path Content::CallView::filesystem_widen_006d70(std::string_view s){return invoke([&]{return impl_->engine.filesystem_widen_006d70(s);});}
configuration::Path Content::CallView::path_append_016b50(const configuration::Path& a,const configuration::Path& b){return invoke([&]{return impl_->engine.path_append_016b50(a,b);});}
bool Content::CallView::online_avatar_virtual_28(const configuration::Path& p){return invoke([&]{return impl_->engine.online_avatar_virtual_28(p);});}
bool Content::CallView::request_online_avatar(const configuration::Path& p,const char* o,std::int32_t n){return invoke([&]{return impl_->engine.avatar(p,o,n);});}
bool Content::CallView::copy_overwrite_048d20(const configuration::Path& a,const configuration::Path& b){return invoke([&]{return impl_->engine.copy_overwrite_048d20(a,b);});}
std::string Content::CallView::path_to_utf8_031ca0(const configuration::Path& p){return invoke([&]{return impl_->engine.path_to_utf8_031ca0(p);});}
void Content::CallView::log_saved_size_085530(std::int32_t n){invoke([&]{impl_->engine.log_saved_size_085530(n);});}
static_assert(!std::is_abstract_v<Content>);
static_assert(!std::is_abstract_v<Content::CallView>);
}
