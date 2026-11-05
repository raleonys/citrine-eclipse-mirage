#include "effects.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <windows.h>
#include <stdexcept>
#include <mutex>
#include <list>
#include <algorithm>

namespace anadius::information_effects {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
Options checked(Options o){if(!o.stream||!o.clock||!o.formatter)throw std::invalid_argument("Owned Information stream/clock/formatter required");return o;}
[[noreturn]]void reject(std::unique_lock<std::mutex>& lock,const char* text){lock.unlock();throw std::logic_error(text);}
}
struct Information::Impl {
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    mutable std::mutex gate;Options options;bool busy=false,checking=false,binding=false,poisoned=false,retiring=false;content_handlers_recovered::CallCheckpoint checkpoint{};
    struct Admission {const application_io::CallContext& context;Admission* next=nullptr;};
    Admission* admissions=nullptr;
    std::list<std::shared_ptr<CallView::State>> views;
    bool view_actor_busy(DWORD,void*,const CallView::State* except=nullptr)const noexcept;
    void remove(Admission& admission)noexcept{auto** at=&admissions;while(*at&&*at!=&admission)at=&(*at)->next;if(!*at)std::terminate();*at=admission.next;}
    struct JobCall {
        Impl& v;socket_recovered::SessionMemoryDomain& parent;
        const socket_recovered::SessionMemoryDomain::JobPermit permit;
        const content_handlers_recovered::CallCheckpoint checkpoint;
        const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
        JobCall(Impl& owner,socket_recovered::SessionMemoryDomain& memory,const socket_recovered::SessionMemoryDomain::JobPermit& proof,content_handlers_recovered::CallCheckpoint c):v(owner),parent(memory),permit(proof),checkpoint(c){}
        void validate(){
            if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Information child call belongs to another actor");
            permit.check(parent);(void)parent.buffer_context();
            {Error error;std::lock_guard lock(v.gate);if(v.job!=this||!v.busy||v.poisoned||v.retiring)throw std::logic_error("Information child admission ended or was poisoned");}
        }
    };
    JobCall* job=nullptr; // synchronous, only published while owning call is busy
    explicit Impl(Options o):options(checked(std::move(o))){}
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Information requires original Graph worker/fiber");}
    void poison(){Error error;std::lock_guard lock(gate);poisoned=true;}
    void check(){
        Error error;JobCall* selected=nullptr;content_handlers_recovered::CallCheckpoint plain;
        {
            std::lock_guard lock(gate);selected=job;if(selected){if(selected->thread!=::GetCurrentThreadId()||selected->fiber!=::GetCurrentFiber())throw std::logic_error("Information child call belongs to another actor");}else affinity();
            if(retiring||poisoned)throw std::logic_error("Information admission closed or unknown");
            if(checking){poisoned=true;throw std::logic_error("Information admission cannot reenter");}checking=true;plain=checkpoint;
        }
        try{if(selected){selected->validate();selected->checkpoint.check(selected->checkpoint.context);selected->validate();}else if(plain.check)plain.check(plain.context);}
        catch(...){std::lock_guard lock(gate);checking=false;poisoned=true;throw;}
        {std::lock_guard lock(gate);checking=false;if(poisoned||retiring)throw std::logic_error("Information admission was poisoned");}
    }
    template<class F>std::invoke_result_t<F> leaf(F&& f){check();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();check();}else{std::invoke_result_t<F> result=f();check();return result;}}
    struct Call {Impl& v;explicit Call(Impl& v):v(v){v.affinity();{Error error;std::lock_guard lock(v.gate);if(v.busy||v.binding||v.checking){v.poisoned=true;throw std::logic_error("Information operation is nonreentrant");}if(v.poisoned||v.retiring)throw std::logic_error("Information admission closed");v.busy=true;}try{v.check();}catch(...){Error error;std::lock_guard lock(v.gate);v.busy=false;throw;}}~Call(){Error error;std::lock_guard lock(v.gate);v.busy=false;}};
    struct Clock final:achievement_runtime::Clock {Impl& v;explicit Clock(Impl& v):v(v){}
        std::int64_t time_05eeb8()override{return v.leaf([&]{return v.options.clock->time_05eeb8();});}
        std::optional<achievement_runtime::BrokenDown> gmtime_05fffc(std::int64_t n)override{return v.leaf([&]{return v.options.clock->gmtime_05fffc(n);});}
    } clock{*this};
    struct Formatter final:achievement_runtime::Formatter {Impl& v;explicit Formatter(Impl& v):v(v){}
        std::string put_time_03d610(const achievement_runtime::BrokenDown& b,std::string_view p)override{return v.leaf([&]{return v.options.formatter->put_time_03d610(b,p);});}
    } formatter{*this};
    void newline(){const auto c=leaf([&]{return options.stream->widen('\n');});leaf([&]{options.stream->put(c);});leaf([&]{options.stream->flush();});}
    void log(std::string_view name){const auto lease=shared_log::acquire(*options.stream);leaf([&]{*options.stream<<"profile name: ";});leaf([&]{*options.stream<<name;});newline();}
};
// One registered execution per explicit message, over the owner's immutable
// lower identities. No native/provider operation occurs under the registry gate.
struct Information::CallView::State {
    Information::Impl& owner;const application_io::CallContext context;
    bool preparing=true,active=false,checking=false,closing=false,closed=false,failed=false;
    State(Information::Impl& v,const application_io::CallContext& c):owner(v),context(c){}
    void poison()noexcept{Error error;std::unique_lock lock(owner.gate);failed=true;owner.poisoned=true;}
    void check(){
        context.validate();
        {Error error;std::unique_lock lock(owner.gate);
            if(owner.poisoned||owner.retiring||closed||failed)reject(lock,"Information call view admission closed");
            if(checking){failed=true;owner.poisoned=true;reject(lock,"Information call view checkpoint cannot reenter");}
            checking=true;
        }
        try{context.check();}
        catch(...){Error error;std::unique_lock lock(owner.gate);checking=false;failed=true;owner.poisoned=true;throw;}
        {Error error;std::unique_lock lock(owner.gate);checking=false;
            if(owner.poisoned||owner.retiring||closed||failed)reject(lock,"Information call view checkpoint was poisoned");
        }
    }
    struct Call {
        State& v;
        explicit Call(State& state):v(state){
            // Wrong/stale contexts reject before changing shared owner health.
            v.context.validate();
            {Error error;std::unique_lock lock(v.owner.gate);
                if(v.closed)reject(lock,"Information call view is closed");
                if(v.owner.poisoned||v.owner.retiring||v.failed)reject(lock,"Information call view owner is unavailable");
                if(v.owner.view_actor_busy(v.context.thread(),v.context.fiber(),&v)||
                   (v.owner.job&&v.owner.job->thread==v.context.thread()&&v.owner.job->fiber==v.context.fiber())){
                    v.failed=true;v.owner.poisoned=true;reject(lock,"Information call view cannot reenter another execution on its actor");
                }
                if(v.preparing||v.active||v.checking||v.closing){v.failed=true;v.owner.poisoned=true;reject(lock,"Information call view cannot reenter");}
                v.active=true;
            }
            try{v.check();}catch(...){Error error;std::unique_lock lock(v.owner.gate);v.active=false;v.failed=true;v.owner.poisoned=true;throw;}
        }
        ~Call(){Error error;std::unique_lock lock(v.owner.gate);v.active=false;}
    };
    template<class F>std::invoke_result_t<F> leaf(F&& action){check();if constexpr(std::is_void_v<std::invoke_result_t<F>>){action();check();}else{std::invoke_result_t<F> result=action();check();return result;}}
    struct Clock final:achievement_runtime::Clock {State& v;explicit Clock(State& v):v(v){}
        std::int64_t time_05eeb8()override{return v.leaf([&]{return v.owner.options.clock->time_05eeb8();});}
        std::optional<achievement_runtime::BrokenDown> gmtime_05fffc(std::int64_t n)override{return v.leaf([&]{return v.owner.options.clock->gmtime_05fffc(n);});}
    } clock{*this};
    struct Formatter final:achievement_runtime::Formatter {State& v;explicit Formatter(State& v):v(v){}
        std::string put_time_03d610(const achievement_runtime::BrokenDown& fields,std::string_view pattern)override{return v.leaf([&]{return v.owner.options.formatter->put_time_03d610(fields,pattern);});}
    } formatter{*this};
    void log(std::string_view name){
        const auto lease=shared_log::acquire(*owner.options.stream);
        leaf([&]{*owner.options.stream<<"profile name: ";});leaf([&]{*owner.options.stream<<name;});
        const auto newline=leaf([&]{return owner.options.stream->widen('\n');});
        leaf([&]{owner.options.stream->put(newline);});leaf([&]{owner.options.stream->flush();});
    }
};
bool Information::Impl::view_actor_busy(DWORD thread,void* fiber,const CallView::State* except)const noexcept{
    for(auto* pending=admissions;pending;pending=pending->next)
        if(pending->context.thread()==thread&&pending->context.fiber()==fiber)return true;
    for(const auto& view:views)
        if(view.get()!=except&&view->context.thread()==thread&&view->context.fiber()==fiber&&
           (view->preparing||view->active||view->checking||view->closing))return true;
    return false; // caller holds gate; never scans or borrows payload catalogs
}
Information::Information(Options o):impl_(nullptr){impl_=std::make_shared<Impl>(std::move(o));}
Information::Impl& Information::attached()const{if(!impl_)throw std::logic_error("Information is unattached");return *impl_;}
Information::~Information()noexcept{if(!impl_)return;auto& v=*impl_;{Error error;std::lock_guard lock(v.gate);if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy||v.checking||v.binding||v.job||v.checkpoint.check||v.admissions||!v.views.empty())std::terminate();v.retiring=true;}{Options retired=std::move(v.options);}}
template<class F>auto Information::invoke(F&& f)->std::invoke_result_t<F>{auto& v=attached();Impl::Call call(v);try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();v.check();}else{std::invoke_result_t<F> result=f();v.check();return result;}}catch(...){v.poison();throw;}}
void Information::log_profile_name_084f48(std::string_view name){invoke([&]{impl_->log(name);});}
std::string Information::current_time_041b60(){return invoke([&]{return achievement_runtime::utc_time_041b60(impl_->clock,impl_->formatter,nullptr,0);});}
template<class F>auto Information::invoke_job(F&& f,socket_recovered::SessionMemoryDomain& parent,const socket_recovered::SessionMemoryDomain::JobPermit& permit,content_handlers_recovered::CallCheckpoint checkpoint)->std::invoke_result_t<F>{
    Error error;permit.check(parent);auto& v=attached();
    if(!checkpoint.context||!checkpoint.check)throw std::invalid_argument("Synchronous child Information admission required");
    if(v.thread==::GetCurrentThreadId())throw std::logic_error("Information child call requires distinct worker");
    Impl::JobCall call(v,parent,permit,checkpoint); // no allocation or owned callable transfer
    {std::unique_lock lock(v.gate);if(v.busy||v.binding||v.checking||v.job||v.checkpoint.check||v.view_actor_busy(call.thread,call.fiber)){v.poisoned=true;reject(lock,"Information child scope overlaps active call");}if(v.poisoned||v.retiring)reject(lock,"Information admission closed");v.busy=true;v.job=&call;}
    const auto finish=[&]{v.check();std::lock_guard lock(v.gate);if(v.poisoned||v.retiring||v.job!=&call||!v.busy||v.checking||v.binding)throw std::logic_error("Information child completion was poisoned");v.job=nullptr;v.busy=false;};
    try{v.check();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();finish();}else{std::invoke_result_t<F> result=f();finish();return result;}}
    catch(...){v.poison();std::lock_guard lock(v.gate);v.job=nullptr;v.busy=false;throw;}
}
void Information::log_profile_name_job(std::string_view name,socket_recovered::SessionMemoryDomain& parent,const socket_recovered::SessionMemoryDomain::JobPermit& permit,content_handlers_recovered::CallCheckpoint c){invoke_job([&]{impl_->log(name);},parent,permit,c);}
std::string Information::current_time_job(socket_recovered::SessionMemoryDomain& parent,const socket_recovered::SessionMemoryDomain::JobPermit& permit,content_handlers_recovered::CallCheckpoint c){return invoke_job([&]{return achievement_runtime::utc_time_041b60(impl_->clock,impl_->formatter,nullptr,0);},parent,permit,c);}
void Information::bind_content_checkpoint(content_handlers_recovered::CallCheckpoint c){
    auto& v=attached();v.affinity();if(!c.context||!c.check)throw std::invalid_argument("Complete Information checkpoint required");
    {Error error;std::lock_guard lock(v.gate);if(v.busy||v.binding||v.checking||v.job||v.checkpoint.check){v.poisoned=true;throw std::logic_error("Information checkpoint overlaps active call");}if(v.poisoned||v.retiring)throw std::logic_error("Information admission closed");v.binding=true;}
    try{v.check();{Error error;c.check(c.context);}v.check();Error error;std::lock_guard lock(v.gate);if(v.poisoned||v.retiring||!v.binding)throw std::logic_error("Information checkpoint binding was poisoned");v.checkpoint=c;v.binding=false;}
    catch(...){v.poison();Error error;std::lock_guard lock(v.gate);v.binding=false;throw;}
}
void Information::clear_content_checkpoint()noexcept{if(!impl_)return;auto& v=*impl_;Error error;std::lock_guard lock(v.gate);if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy||v.binding||v.checking||v.job)std::terminate();v.checkpoint={};}
std::shared_ptr<Information::CallView> Information::make_call_view(const application_io::CallContext& context){
    context.validate();auto owner=impl_;if(!owner)throw std::logic_error("Information is unattached");
    Impl::Admission admission{context};
    {
        Error error;std::unique_lock lock(owner->gate);
        if(owner->poisoned||owner->retiring)reject(lock,"Information call-view owner is unavailable");
        if(context.thread()==owner->thread)reject(lock,"Information call view requires a distinct child worker");
        for(auto* pending=owner->admissions;pending;pending=pending->next)
            if(pending->context.thread()==context.thread()&&pending->context.fiber()==context.fiber()){
                owner->poisoned=true;reject(lock,"Information call-view preparation cannot reenter");
            }
        for(const auto& view:owner->views)
            if(view->context.thread()==context.thread()&&view->context.fiber()==context.fiber()&&
               (view->preparing||view->active||view->checking||view->closing)){
                view->failed=true;owner->poisoned=true;reject(lock,"Information call-view actor is already executing");
            }
        if(owner->job&&owner->job->thread==context.thread()&&owner->job->fiber==context.fiber()){
            owner->poisoned=true;reject(lock,"Information call view cannot reenter a legacy child scope");
        }
        admission.next=owner->admissions;owner->admissions=&admission;
    }
    struct Pending {Impl& owner;Impl::Admission& admission;bool linked=true;
        ~Pending(){Error error;if(linked){std::unique_lock lock(owner.gate);owner.remove(admission);}}
    } pending{*owner,admission};
    std::list<std::shared_ptr<CallView::State>> prepared;
    std::shared_ptr<CallView::State> state;
    try{
        {Error error;prepared.emplace_back();state=std::make_shared<CallView::State>(*owner,context);prepared.front()=state;
            std::unique_lock lock(owner->gate);owner->views.splice(owner->views.end(),prepared);owner->remove(admission);pending.linked=false;
            if(owner->poisoned||owner->retiring){state->failed=true;reject(lock,"Information call-view allocation was poisoned");}
        }
        state->check();
        std::shared_ptr<CallView> result;
        {Error error;result=std::shared_ptr<CallView>(new CallView(owner,state));}
        state->check();
        {Error error;std::unique_lock lock(owner->gate);
            if(owner->poisoned||owner->retiring||state->failed)reject(lock,"Information call-view publication was poisoned");
            state->preparing=false;
        }
        return result;
    }catch(...){if(state)state->poison();throw;}
}
Information::CallView::CallView(std::shared_ptr<Information::Impl> owner,std::shared_ptr<State> state):owner_(std::move(owner)),state_(std::move(state)){}
Information::CallView::~CallView()noexcept{
    Error error;std::unique_lock lock(owner_->gate);
    if(!state_->closed){state_->failed=true;owner_->poisoned=true;}
    // Unclosed state stays in owner_->views. Never probe stale callback backing
    // or speculate normal cleanup during wrapper destruction.
}
void Information::CallView::log_profile_name_084f48(std::string_view name){
    State::Call call(*state_);try{state_->log(name);state_->check();}catch(...){state_->poison();throw;}
}
std::string Information::CallView::current_time_041b60(){
    State::Call call(*state_);try{auto result=achievement_runtime::utc_time_041b60(state_->clock,state_->formatter,nullptr,0);state_->check();return result;}catch(...){state_->poison();throw;}
}
void Information::CallView::close(){
    {Error error;std::unique_lock lock(owner_->gate);if(state_->closed)return;}
    state_->context.validate();
    {Error error;std::unique_lock lock(owner_->gate);
        if(owner_->poisoned||owner_->retiring||state_->failed)reject(lock,"Information call-view close is unavailable");
        if(owner_->view_actor_busy(state_->context.thread(),state_->context.fiber(),state_.get())||
           (owner_->job&&owner_->job->thread==state_->context.thread()&&owner_->job->fiber==state_->context.fiber())){
            state_->failed=true;owner_->poisoned=true;reject(lock,"Information call-view close cannot reenter another actor execution");
        }
        if(state_->preparing||state_->active||state_->checking||state_->closing){state_->failed=true;owner_->poisoned=true;reject(lock,"Information call-view close cannot reenter");}
        state_->closing=true;
    }
    std::list<std::shared_ptr<State>> retired;
    try{
        state_->check();
        {Error error;std::unique_lock lock(owner_->gate);
            if(owner_->poisoned||owner_->retiring||state_->failed||!state_->closing||state_->active||state_->checking)
                reject(lock,"Information call-view retirement was poisoned");
            const auto found=std::find(owner_->views.begin(),owner_->views.end(),state_);
            if(found==owner_->views.end())std::terminate();
            state_->closed=true;state_->closing=false;retired.splice(retired.end(),owner_->views,found);
        }
        {Error error;retired.clear();}
    }catch(...){state_->poison();throw;}
}
static_assert(!std::is_abstract_v<Information>);
static_assert(!std::is_abstract_v<Information::CallView>);
}
