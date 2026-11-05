#include "effects.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <windows.h>
#include <stdexcept>
#include <mutex>

namespace anadius::information_effects {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
Options checked(Options o){if(!o.stream||!o.clock||!o.formatter)throw std::invalid_argument("Owned Information stream/clock/formatter required");return o;}
}
struct Information::Impl {
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    mutable std::mutex gate;Options options;bool busy=false,checking=false,binding=false,poisoned=false,retiring=false;content_handlers_recovered::CallCheckpoint checkpoint{};
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
Information::Information(Options o):impl_(nullptr){impl_=std::make_unique<Impl>(std::move(o));}
Information::Impl& Information::attached()const{if(!impl_)throw std::logic_error("Information is unattached");return *impl_;}
Information::~Information()noexcept{if(!impl_)return;auto& v=*impl_;{Error error;std::lock_guard lock(v.gate);if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy||v.checking||v.binding||v.job||v.checkpoint.check)std::terminate();v.retiring=true;}{Options retired=std::move(v.options);}}
template<class F>auto Information::invoke(F&& f)->std::invoke_result_t<F>{auto& v=attached();Impl::Call call(v);try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();v.check();}else{std::invoke_result_t<F> result=f();v.check();return result;}}catch(...){v.poison();throw;}}
void Information::log_profile_name_084f48(std::string_view name){invoke([&]{impl_->log(name);});}
std::string Information::current_time_041b60(){return invoke([&]{return achievement_runtime::utc_time_041b60(impl_->clock,impl_->formatter,nullptr,0);});}
template<class F>auto Information::invoke_job(F&& f,socket_recovered::SessionMemoryDomain& parent,const socket_recovered::SessionMemoryDomain::JobPermit& permit,content_handlers_recovered::CallCheckpoint checkpoint)->std::invoke_result_t<F>{
    Error error;permit.check(parent);auto& v=attached();
    if(!checkpoint.context||!checkpoint.check)throw std::invalid_argument("Synchronous child Information admission required");
    if(v.thread==::GetCurrentThreadId())throw std::logic_error("Information child call requires distinct worker");
    Impl::JobCall call(v,parent,permit,checkpoint); // no allocation or owned callable transfer
    {std::lock_guard lock(v.gate);if(v.busy||v.binding||v.checking||v.job||v.checkpoint.check){v.poisoned=true;throw std::logic_error("Information child scope overlaps active call");}if(v.poisoned||v.retiring)throw std::logic_error("Information admission closed");v.busy=true;v.job=&call;}
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
static_assert(!std::is_abstract_v<Information>);
}
